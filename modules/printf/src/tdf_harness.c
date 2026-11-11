/* Le testeur des fous · version printf - moteur : fork, capture, comparaison, rapport */
#include "tdf.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/wait.h>
#ifdef __linux__
# include <sys/prctl.h>
#endif

#define C_RED "\033[1;31m"
#define C_GRN "\033[1;32m"
#define C_YEL "\033[1;33m"
#define C_BLU "\033[1;34m"
#define C_MAG "\033[1;35m"
#define C_CYA "\033[0;36m"
#define C_DIM "\033[2m"
#define C_RST "\033[0m"
#define MAX_OUT (64u << 20)

t_opts	g_opt;
int		g_out = 1;
t_shm	*g_shm;
t_run	g_run;
t_buf	g_exp;
int		g_exp_ret;
int		g_child_redirect;
int		g_cur_sev;

static int			g_expfd = -1;
static int			g_errfd = -1;
static int			g_devfull = -1;
static int			g_saved1 = -1;
static const char	*g_cur_file;
static int			g_cur_line;
static const char	*g_cur_desc;

/* ------------------------------------------------------------------ */
/* tampons                                                             */
/* ------------------------------------------------------------------ */
void	buf_add(t_buf *b, const void *p, size_t n)
{
	if (b->len + n + 1 > b->cap)
	{
		b->cap = (b->len + n + 1) * 2;
		b->p = realloc(b->p, b->cap);
	}
	memcpy(b->p + b->len, p, n);
	b->len += n;
	b->p[b->len] = '\0';
}

void	buf_reset(t_buf *b)
{
	b->len = 0;
	if (b->p)
		b->p[0] = '\0';
}

static void	bprintf(t_buf *b, const char *fmt, ...)
{
	char	tmp[4096];
	va_list	ap;
	int		n;

	va_start(ap, fmt);
	n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
	va_end(ap);
	if (n > (int)sizeof(tmp) - 1)
		n = sizeof(tmp) - 1;
	if (n > 0)
		buf_add(b, tmp, (size_t)n);
}

void	tdf_escape(t_buf *dst, const char *s, size_t n, size_t max)
{
	size_t			i;
	unsigned char	c;
	char			t[8];

	buf_add(dst, "\"", 1);
	i = 0;
	while (i < n && i < max)
	{
		c = (unsigned char)s[i++];
		if (c == '\n')
			buf_add(dst, "\\n", 2);
		else if (c == '\t')
			buf_add(dst, "\\t", 2);
		else if (c == '\0')
			buf_add(dst, C_MAG "\\0" C_RST, strlen(C_MAG "\\0" C_RST));
		else if (c == '"' || c == '\\')
		{
			t[0] = '\\';
			t[1] = (char)c;
			buf_add(dst, t, 2);
		}
		else if (c < 32 || c >= 127)
		{
			snprintf(t, sizeof(t), "\\x%02x", c);
			buf_add(dst, t, 4);
		}
		else
			buf_add(dst, &c, 1);
	}
	buf_add(dst, "\"", 1);
	if (n > max)
		bprintf(dst, C_DIM "…(+%zu octets)" C_RST, n - max);
}

/* ------------------------------------------------------------------ */
/* catégories et compteurs                                             */
/* ------------------------------------------------------------------ */
typedef struct s_cat
{
	char	key[16];
	char	title[96];
	int		ok;
	int		ko;
	int		warn;
	int		shown_ko;
	int		shown_warn;
	t_buf	details;
}	t_cat;

static t_cat	g_cat;
static int		g_cat_open;
static int		g_tot_ok;
static int		g_tot_ko;
static int		g_tot_warn;
static int		g_ncat;

static const char	*norm_key(const char *k)
{
	while (*k == '%' && k[1])
		k++;
	return (k);
}

int	tdf_selected(const char *key)
{
	char		tmp[64];
	const char	*o;
	const char	*p;
	size_t		len;

	if (!g_opt.only || !*g_opt.only)
		return (1);
	if (!strncmp(key, "bonus_", 6) && tdf_selected("bonus"))
		return (1);
	o = g_opt.only;
	while (*o)
	{
		p = strchr(o, ',');
		len = p ? (size_t)(p - o) : strlen(o);
		if (len >= sizeof(tmp))
			len = sizeof(tmp) - 1;
		memcpy(tmp, o, len);
		tmp[len] = '\0';
		if (!strcmp(norm_key(tmp), norm_key(key)))
			return (1);
		o += len + (p != NULL);
	}
	return (0);
}

static int	pad_width(const char *s, int w)
{
	int	n;

	n = 0;
	while (*s)
		if ((*s++ & 0xC0) != 0x80)
			n++;
	return (n < w ? w - n : 0);
}


/* ------------------------------------------------------------------ */
/* regroupement des KO par cause (affichage lisible)                    */
/* ------------------------------------------------------------------ */
#define MAX_CAUSE 12
#define MAX_GCAUSE 48
#define MAX_LN 48
#define MAX_SHOW 4

typedef struct s_ln
{
	const char	*o;
	size_t		ol;
	char		c[400];
	int			out;
	int			noise;
}	t_ln;

typedef struct s_ck
{
	char		key[320];
	char		title[240];
	const char	*hint;
	int			cls;
}	t_ck;

typedef struct s_cause
{
	char		key[320];
	char		title[240];
	const char	*hint;
	int			count;
	int			nex;
	t_buf		ex[2];
	char		exhead[2][400];
	char		more[4][112];
	int			nmore;
}	t_cause;

typedef struct s_gcause
{
	char		key[320];
	char		title[240];
	const char	*hint;
	int			count;
}	t_gcause;

static t_cause	g_causes[MAX_CAUSE];
static int		g_ncause;
static int		g_cause_over;
static t_gcause	g_gc[MAX_GCAUSE];
static int		g_ngc;

/* ------------------------------------------------------------------ */
/* sortie « machine » : une ligne par événement, champs séparés par \t  */
/* (lue par le jeu ; les couleurs ANSI, \t et \n sont neutralisés)      */
/* ------------------------------------------------------------------ */
static int	g_mfd = -1;

static void	m_field(t_buf *b, const char *s)
{
	size_t	i;

	i = 0;
	while (s && s[i])
	{
		if (s[i] == '\033' && s[i + 1] == '[')
		{
			i += 2;
			while (s[i] && !((s[i] >= 'A' && s[i] <= 'Z') || (s[i] >= 'a' && s[i] <= 'z')))
				i++;
			if (s[i])
				i++;
			continue ;
		}
		if (s[i] == '\n')
			buf_add(b, "\\n", 2);
		else if (s[i] == '\t')
			buf_add(b, " ", 1);
		else if (s[i] == '\\')
			buf_add(b, "\\\\", 2);
		else
			buf_add(b, s + i, 1);
		i++;
	}
}

static void	m_line(int n, const char **f)
{
	t_buf	b;
	int		i;

	if (g_mfd < 0)
		return ;
	memset(&b, 0, sizeof(b));
	i = -1;
	while (++i < n)
	{
		if (i)
			buf_add(&b, "\t", 1);
		m_field(&b, f[i]);
	}
	buf_add(&b, "\n", 1);
	(void)!write(g_mfd, b.p, b.len);
	free(b.p);
}

static void	m_open(void)
{
	if (g_opt.machine && g_mfd < 0)
		g_mfd = open(g_opt.machine, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
}

/* niveau d'aide 0 : on retire « → ligne du bug » des détails */
static void	help_filter(t_buf *dst, const char *p, size_t len)
{
	const char	*e;
	size_t		l;

	while (len)
	{
		e = memchr(p, '\n', len);
		l = e ? (size_t)(e - p) + 1 : len;
		if (g_opt.help >= 1 || (!memmem(p, l, "ligne du bug", 12)
				&& !memmem(p, l, "alloué ici", strlen("alloué ici"))))
			buf_add(dst, p, l);
		p += l;
		len -= l;
	}
}

/* titre de cause sans « — fichier:ligne » si l'aide est à 0 */
static void	title_print(const char *t, char *dst, size_t n)
{
	const char	*w;

	w = strstr(t, " — ");
	if (g_opt.help < 1 && w)
		snprintf(dst, n, "%.*s", (int)(w - t), t);
	else
		snprintf(dst, n, "%s", t);
}

#define H_LEAK "un malloc n'est jamais free : libère le buffer APRÈS l'avoir écrit, sur tous les chemins (retours anticipés et erreurs compris)"
#define H_MEM "accès mémoire invalide : teste les retours de malloc / les pointeurs NULL avant de les utiliser (regarde la ligne indiquée)"
#define H_OVF "tu écris au-delà du bloc alloué : malloc trop petit (+1 pour le '\\0' oublié ?) ou boucle qui dépasse d'un cran"
#define H_RET "ft_printf doit renvoyer le nombre d'octets écrits, ou -1 si write échoue (et s'arrêter tout de suite)"
#define H_LOOP "boucle infinie : vérifie que l'index avance dans TOUS les cas (conversion inconnue, '%' final)"
#define H_DFREE "le même pointeur est libéré deux fois : mets-le à NULL après free ou clarifie qui possède le bloc"
#define H_UAF "utilisation après free : ne relis/réécris pas un buffer déjà libéré"
#define H_MALLOC "malloc a échoué mais ton code utilise quand même le résultat : ajoute `if (!ptr) return (-1);` juste après CHAQUE malloc/ft_itoa/ft_strdup"
#define H_LONE "après un '%', vérifie que le caractère suivant n'est pas '\\0' avant de le lire/avancer, sinon tu dépasses la fin du format"
#define H_UB "comportement indéfini (-INT_MIN, décalage, dépassement signé...) : passe par un unsigned ou un long"
#define H_VG "lecture non initialisée ou accès invalide : initialise le buffer et vérifie les tailles"

static const char	*trim_(const char *s)
{
	while (*s == ' ' || *s == '\t')
		s++;
	return (s);
}

static int	starts_(const char *s, const char *pre)
{
	return (!strncmp(trim_(s), pre, strlen(pre)));
}

static int	split_lines(const char *msg, t_ln *l)
{
	int			n;
	const char	*p;
	const char	*e;
	size_t		len;
	size_t		i;
	size_t		j;

	n = 0;
	p = msg;
	while (*p && n < MAX_LN)
	{
		e = strchr(p, '\n');
		len = e ? (size_t)(e - p) : strlen(p);
		l[n].o = p;
		l[n].ol = len;
		i = 0;
		j = 0;
		while (i < len && j + 1 < sizeof(l[n].c))
		{
			if (p[i] == '\033')
			{
				while (i < len && p[i] != 'm')
					i++;
				i++;
				continue ;
			}
			l[n].c[j++] = p[i++];
		}
		l[n].c[j] = '\0';
		l[n].out = starts_(l[n].c, "attendu :") || starts_(l[n].c, "obtenu")
			|| starts_(l[n].c, "sortie identique") || starts_(l[n].c, "retour attendu")
			|| starts_(l[n].c, "(1re différence");
		l[n].noise = strstr(l[n].c, "LSan :") || strstr(l[n].c, "erreur mémoire signalée à la sortie")
			|| strstr(l[n].c, "alloué ici :") || strstr(l[n].c, "(aucune frame")
			|| starts_(l[n].c, "→ ligne du bug");
		n++;
		p += len + (e != NULL);
	}
	return (n);
}

static int	has_sub(t_ln *l, int n, const char *sub)
{
	int	i;

	i = -1;
	while (++i < n)
		if (strstr(l[i].c, sub))
			return (1);
	return (0);
}

static int	find_starts(t_ln *l, int n, const char *pre)
{
	int	i;

	i = -1;
	while (++i < n)
		if (starts_(l[i].c, pre))
			return (i);
	return (-1);
}

/* copie s en coupant au premier délimiteur trouvé après 'skip' octets */
static void	copy_cut(char *dst, size_t dn, const char *s, size_t skip)
{
	static const char	*const delims[] = {" (", ":", " of size", " at 0x", NULL};
	size_t				len;
	const char			*q;
	int					i;

	len = strlen(s);
	i = -1;
	while (delims[++i])
	{
		q = strstr(s + (skip < len ? skip : len), delims[i]);
		if (q && (size_t)(q - s) < len)
			len = (size_t)(q - s);
	}
	if (len >= dn)
		len = dn - 1;
	memcpy(dst, s, len);
	dst[len] = '\0';
}

static void	find_where(t_ln *l, int n, char *w, size_t wn)
{
	int			i;
	const char	*q;
	char		*oct;

	w[0] = '\0';
	i = -1;
	while (++i < n)
	{
		q = strstr(l[i].c, "→ ligne du bug : ");
		if (q)
		{
			snprintf(w, wn, "%s", q + strlen("→ ligne du bug : "));
			break ;
		}
	}
	if (!w[0])
	{
		i = -1;
		while (++i < n)
		{
			q = strstr(l[i].c, "→ alloué ici : ");
			if (q && strncmp(q + strlen("→ alloué ici : "), "??", 2))
			{
				snprintf(w, wn, "%s", q + strlen("→ alloué ici : "));
				oct = strstr(w, " (");
				while (oct && strstr(oct + 1, " ("))
					oct = strstr(oct + 1, " (");
				if (oct && strstr(oct, "octet"))
					*oct = '\0';
				break ;
			}
		}
	}
	oct = strstr(w, "  (");
	if (oct)
		memmove(oct + 1, oct + 2, strlen(oct + 1));
}

static void	ck_add(t_ck *ck, int *k, const char *kind, const char *title,
		const char *where, const char *hint, int cls)
{
	if (*k >= 4)
		return ;
	snprintf(ck[*k].key, sizeof(ck[*k].key), "%s|%s", kind, where);
	if (where[0])
		snprintf(ck[*k].title, sizeof(ck[*k].title), "%s — %s", title, where);
	else
		snprintf(ck[*k].title, sizeof(ck[*k].title), "%s", title);
	ck[*k].hint = hint;
	ck[*k].cls = cls;
	(*k)++;
}

static const char	*pick_hint(t_ln *l, int n, const char *title, const char *def)
{
	if (has_sub(l, n, "renvoie NULL"))
		return (H_MALLOC);
	if (has_sub(l, n, "au-delà du '\\0'"))
		return (H_LONE);
	if (strstr(title, "out of bounds") || strstr(title, "buffer-overflow"))
		return (H_OVF);
	if (strstr(title, "use-after-free"))
		return (H_UAF);
	if (strstr(title, "double-free"))
		return (H_DFREE);
	return (def);
}

static int	extract_causes(t_ln *l, int n, t_ck *ck)
{
	char	where[200];
	char	title[160];
	int		k;
	int		a;
	int		u;
	int		v;

	k = 0;
	find_where(l, n, where, sizeof(where));
	if (has_sub(l, n, "TIMEOUT"))
		ck_add(ck, &k, "timeout", "TIMEOUT (boucle infinie ?)", "", H_LOOP, 0);
	a = find_starts(l, n, "ASan :");
	u = find_starts(l, n, "UBSan :");
	v = find_starts(l, n, "valgrind :");
	if (a >= 0)
	{
		copy_cut(title, sizeof(title), trim_(l[a].c), 7);
		ck_add(ck, &k, title, title, where, pick_hint(l, n, title, H_MEM), 0);
	}
	if (u >= 0)
	{
		copy_cut(title, sizeof(title), trim_(l[u].c), 8);
		if (!strncmp(title, "UBSan : runtime error", 21))
		{
			const char *rt = strstr(l[u].c, "runtime error: ");
			char	tmp[160];

			copy_cut(tmp, sizeof(tmp), rt ? rt + 15 : "?", 0);
			snprintf(title, sizeof(title), "UBSan : %s", tmp);
			if (rt && strstr(rt, " of type"))
			{
				const char *t = strstr(rt + 15, " of type");
				snprintf(title, sizeof(title), "UBSan : %.*s", (int)(t - rt - 15), rt + 15);
			}
		}
		ck_add(ck, &k, title, title, where,
			pick_hint(l, n, title, strstr(title, "null pointer") ? H_MEM : H_UB), 0);
	}
	if (v >= 0)
	{
		copy_cut(title, sizeof(title), trim_(l[v].c), 11);
		ck_add(ck, &k, title, title, where, pick_hint(l, n, title, H_VG), 0);
	}
	if (a < 0 && u < 0 && v < 0 && find_starts(l, n, "CRASH") >= 0)
	{
		copy_cut(title, sizeof(title), trim_(l[find_starts(l, n, "CRASH")].c), 5);
		ck_add(ck, &k, "crash", title, where, pick_hint(l, n, title, H_MEM), 0);
	}
	if (has_sub(l, n, "fuite :") || has_sub(l, n, "LSan :"))
		ck_add(ck, &k, "fuite", "fuite mémoire", where, H_LEAK, 0);
	if (has_sub(l, n, "débordement de tas"))
		ck_add(ck, &k, "ovf", "débordement de tas", where, H_OVF, 0);
	if (has_sub(l, n, "double free"))
		ck_add(ck, &k, "dfree", "double free", where, H_DFREE, 0);
	if (has_sub(l, n, "écriture après free"))
		ck_add(ck, &k, "uaf", "écriture après free", where, H_UAF, 0);
	if (has_sub(l, n, "(aucun)"))
		return (k ? k : (ck_add(ck, &k, "crash", "plantage", where, H_MEM, 0), k));
	if (has_sub(l, n, "valeur de retour fausse") || has_sub(l, n, "retour attendu"))
		ck_add(ck, &k, "ret", "valeur de retour fausse", "", H_RET, 1);
	if (find_starts(l, n, "attendu :") >= 0)
		ck_add(ck, &k, "out", "sortie différente de printf", "", NULL, 1);
	if (!k)
	{
		copy_cut(title, sizeof(title), n ? trim_(l[0].c) : "échec", 0);
		ck_add(ck, &k, "autre", title[0] ? title : "échec", "", NULL, 0);
	}
	return (k);
}

static void	snippet(t_buf *b, t_ln *l, int n, int cls)
{
	int	i;

	i = -1;
	while (++i < n)
	{
		if (cls == 1 ? !l[i].out : (l[i].out || l[i].noise))
			continue ;
		if (!l[i].c[0])
			continue ;
		buf_add(b, "    ", 4);
		buf_add(b, l[i].o, l[i].ol);
		buf_add(b, "\n", 1);
	}
}

static void	cause_add(const char *file, int line, const char *desc, const char *msg)
{
	t_ln		l[MAX_LN];
	t_ck		ck[4];
	int			n;
	int			k;
	int			i;
	int			j;
	t_cause		*c;

	if (desc && !strncmp(desc, "arrêt du lot", strlen("arrêt du lot")))
		return ;
	n = split_lines(msg ? msg : "", l);
	k = extract_causes(l, n, ck);
	i = -1;
	while (++i < k)
	{
		j = 0;
		while (j < g_ncause && strcmp(g_causes[j].key, ck[i].key))
			j++;
		if (j == g_ncause)
		{
			if (g_ncause >= MAX_CAUSE)
			{
				g_cause_over++;
				continue ;
			}
			g_ncause++;
			memset(&g_causes[j], 0, sizeof(t_cause));
			snprintf(g_causes[j].key, sizeof(g_causes[j].key), "%s", ck[i].key);
			snprintf(g_causes[j].title, sizeof(g_causes[j].title), "%s", ck[i].title);
			g_causes[j].hint = ck[i].hint;
		}
		c = &g_causes[j];
		c->count++;
		if (c->nex < 2)
		{
			snprintf(c->exhead[c->nex], sizeof(c->exhead[0]),
				"        ex %d : " C_CYA "%.200s" C_RST "  " C_DIM "[%s:%d]" C_RST "\n",
				c->nex + 1, desc ? desc : "?", file ? file : "?", line);
			snippet(&c->ex[c->nex], l, n, ck[i].cls);
			c->nex++;
		}
		else if (c->nmore < 4)
			snprintf(c->more[c->nmore++], sizeof(c->more[0]), "%.100s", desc ? desc : "?");
	}
}

static void	gcause_merge(t_cause *c)
{
	int	j;

	j = 0;
	while (j < g_ngc && strcmp(g_gc[j].key, c->key))
		j++;
	if (j == g_ngc)
	{
		if (g_ngc >= MAX_GCAUSE)
			return ;
		g_ngc++;
		memset(&g_gc[j], 0, sizeof(t_gcause));
		snprintf(g_gc[j].key, sizeof(g_gc[j].key), "%s", c->key);
		snprintf(g_gc[j].title, sizeof(g_gc[j].title), "%s", c->title);
		g_gc[j].hint = c->hint;
	}
	g_gc[j].count += c->count;
}

static int	gcause_seen(t_cause *c)
{
	int	j;

	j = -1;
	while (++j < g_ngc)
		if (!strcmp(g_gc[j].key, c->key))
			return (1);
	return (0);
}

static void	cause_render(t_cause *c)
{
	char	t[260];
	int		i;

	title_print(c->title, t, sizeof(t));
	if (gcause_seen(c))
	{
		dprintf(g_out, "      " C_RED "▸ %s" C_RST C_DIM "   (%d cas)  [même cause que plus haut]"
			C_RST "\n", t, c->count);
		return ;
	}
	dprintf(g_out, "      " C_RED "▸ %s" C_RST C_DIM "   (%d cas)" C_RST "\n", t, c->count);
	i = -1;
	while (++i < c->nex)
	{
		dprintf(g_out, "%s", c->exhead[i]);
		if (c->ex[i].len)
			(void)!write(g_out, c->ex[i].p, c->ex[i].len);
	}
	if (c->nmore)
	{
		dprintf(g_out, C_DIM "        aussi :");
		i = -1;
		while (++i < c->nmore)
			dprintf(g_out, "  %s", c->more[i]);
		dprintf(g_out, "%s" C_RST "\n", c->count > 2 + c->nmore ? "  …" : "");
	}
	if (c->hint && g_opt.help >= 2)
		dprintf(g_out, "        " C_RST "→ conseil : %s\n", c->hint);
}

static void	causes_flush(void)
{
	int	idx[MAX_CAUSE];
	int	i;
	int	j;
	int	t;

	i = -1;
	while (++i < g_ncause)
		idx[i] = i;
	i = 0;
	while (++i < g_ncause)
	{
		t = idx[i];
		j = i - 1;
		while (j >= 0 && g_causes[idx[j]].count < g_causes[t].count)
		{
			idx[j + 1] = idx[j];
			j--;
		}
		idx[j + 1] = t;
	}
	i = -1;
	while (++i < g_ncause)
	{
		{
			const char	*f[6];
			char		cnt[16];
			char		t[260];
			const char	*w;

			w = strchr(g_causes[idx[i]].key, '|');
			snprintf(cnt, sizeof(cnt), "%d", g_causes[idx[i]].count);
			snprintf(t, sizeof(t), "%s", g_causes[idx[i]].title);
			if (strstr(t, " — "))
				*strstr(t, " — ") = '\0';
			f[0] = "K";
			f[1] = g_cat.key;
			f[2] = t;
			f[3] = w ? w + 1 : "";
			f[4] = cnt;
			f[5] = g_causes[idx[i]].hint ? g_causes[idx[i]].hint : "";
			m_line(6, f);
		}
		if (i < MAX_SHOW)
			cause_render(&g_causes[idx[i]]);
		gcause_merge(&g_causes[idx[i]]);
	}
	if (g_ncause > MAX_SHOW || g_cause_over)
		dprintf(g_out, C_DIM "      … %d autre(s) cause(s) non détaillée(s) (-v pour tout voir)" C_RST "\n",
			g_ncause - MAX_SHOW + g_cause_over);
}

static void	causes_reset(void)
{
	int	i;

	i = -1;
	while (++i < g_ncause)
	{
		free(g_causes[i].ex[0].p);
		free(g_causes[i].ex[1].p);
	}
	memset(g_causes, 0, sizeof(g_causes));
	g_ncause = 0;
	g_cause_over = 0;
}

static void	causes_summary(void)
{
	int	idx[MAX_GCAUSE];
	int	i;
	int	j;
	int	t;
	int	rank;

	if (!g_ngc)
		return ;
	i = -1;
	while (++i < g_ngc)
		idx[i] = i;
	i = 0;
	while (++i < g_ngc)
	{
		t = idx[i];
		j = i - 1;
		while (j >= 0 && g_gc[idx[j]].count < g_gc[t].count)
		{
			idx[j + 1] = idx[j];
			j--;
		}
		idx[j + 1] = t;
	}
	dprintf(g_out, "\n  " C_BLU "À corriger, par ordre d'impact" C_RST "\n");
	rank = 0;
	while (rank < g_ngc && rank < 6)
	{
		t_gcause *g = &g_gc[idx[rank]];
		char	tt[260];

		title_print(g->title, tt, sizeof(tt));
		dprintf(g_out, "   %d. " C_RED "%s" C_RST C_DIM "  (%d cas)" C_RST "\n",
			rank + 1, tt, g->count);
		if (g->hint && g_opt.help >= 2)
			dprintf(g_out, "      " C_RST "→ %s\n", g->hint);
		rank++;
	}
	if (g_ngc > 6)
		dprintf(g_out, C_DIM "   … et %d autre(s) cause(s)" C_RST "\n", g_ngc - 6);
	dprintf(g_out, C_DIM "   (--only <conv> -v : détail complet de chaque test)" C_RST "\n");
}

void	tdf_category_end(void)
{
	const char	*col;

	if (!g_cat_open)
		return ;
	g_cat_open = 0;
	col = g_cat.ko ? C_RED : (g_cat.warn ? C_YEL : C_GRN);
	dprintf(g_out, "  %s%-3s" C_RST " %s%*s %s%5d/%-5d" C_RST,
		col, g_cat.ko ? "KO" : (g_cat.warn ? "~~" : "OK"), g_cat.title,
		pad_width(g_cat.title, 50), "",
		col, g_cat.ok + g_cat.warn, g_cat.ok + g_cat.ko + g_cat.warn);
	if (g_cat.ko)
		dprintf(g_out, C_RED "  %d KO" C_RST, g_cat.ko);
	if (g_cat.warn)
		dprintf(g_out, C_YEL "  %d avert." C_RST, g_cat.warn);
	dprintf(g_out, "\n");
	if (g_ncause)
		causes_flush();
	if (g_cat.details.len)
		(void)!write(g_out, g_cat.details.p, g_cat.details.len);
	if (!g_ncause && g_cat.shown_ko < g_cat.ko)
		dprintf(g_out, C_DIM "      … %d autre(s) KO non détaillé(s) (utilise --only %s -v)\n" C_RST,
			g_cat.ko - g_cat.shown_ko, g_cat.key);
	if (g_cat.shown_warn < g_cat.warn)
		dprintf(g_out, C_DIM "      … %d autre(s) avertissement(s) non détaillé(s) (-v pour tout voir)\n" C_RST,
			g_cat.warn - g_cat.shown_warn);
	{
		const char	*f[6];
		char		a[16];
		char		b2[16];
		char		c2[16];

		snprintf(a, sizeof(a), "%d", g_cat.ok);
		snprintf(b2, sizeof(b2), "%d", g_cat.ko);
		snprintf(c2, sizeof(c2), "%d", g_cat.warn);
		f[0] = "C";
		f[1] = g_cat.key;
		f[2] = g_cat.title;
		f[3] = a;
		f[4] = b2;
		f[5] = c2;
		m_line(6, f);
	}
	g_tot_ok += g_cat.ok;
	g_tot_ko += g_cat.ko;
	g_tot_warn += g_cat.warn;
	g_ncat++;
}

void	tdf_category(const char *key, const char *title)
{
	tdf_category_end();
	free(g_cat.details.p);
	causes_reset();
	memset(&g_cat, 0, sizeof(g_cat));
	snprintf(g_cat.key, sizeof(g_cat.key), "%s", key);
	snprintf(g_cat.title, sizeof(g_cat.title), "%s", title);
	g_cat_open = tdf_selected(key);
}

int	tdf_summary(void)
{
	tdf_category_end();
	dprintf(g_out, "\n  " C_BLU "Tests dynamiques" C_RST " [%s] : ", TDF_MODE);
	dprintf(g_out, C_GRN "%d OK" C_RST ", %s%d KO" C_RST ", %s%d avertissement(s)" C_RST "\n",
		g_tot_ok, g_tot_ko ? C_RED : C_GRN, g_tot_ko,
		g_tot_warn ? C_YEL : C_GRN, g_tot_warn);
	causes_summary();
	{
		const char	*f[5];
		char		a[16];
		char		b2[16];
		char		c2[16];

		snprintf(a, sizeof(a), "%d", g_tot_ok);
		snprintf(b2, sizeof(b2), "%d", g_tot_ko);
		snprintf(c2, sizeof(c2), "%d", g_tot_warn);
		f[0] = "T";
		f[1] = TDF_MODE;
		f[2] = a;
		f[3] = b2;
		f[4] = c2;
		m_line(5, f);
	}
	return (g_tot_ko ? 1 : 0);
}

/* Ajoute un résultat. msg = détail déjà formaté (peut être vide). */
static void	record(int ko, int warn, const char *file, int line,
		const char *desc, t_buf *msg)
{
	int	show;

	if (!g_cat_open)
		return ;
	if (!ko && !warn)
	{
		g_cat.ok++;
		return ;
	}
	if (ko)
		g_cat.ko++;
	else
		g_cat.warn++;
	{
		const char	*f[6];
		char		wh[300];

		snprintf(wh, sizeof(wh), "%s:%d", file ? file : "?", line);
		f[0] = "R";
		f[1] = g_cat.key;
		f[2] = ko ? "KO" : "WARN";
		f[3] = wh;
		f[4] = desc ? desc : "";
		f[5] = msg && msg->p ? msg->p : "";
		m_line(6, f);
	}
	if (ko && !g_opt.verbose)
	{
		cause_add(file, line, desc, msg ? msg->p : "");
		return ;
	}
	show = g_opt.verbose || (ko ? g_cat.shown_ko < 12 : g_cat.shown_warn < 6);
	if (!show)
		return ;
	if (ko)
		g_cat.shown_ko++;
	else
		g_cat.shown_warn++;
	bprintf(&g_cat.details, "      %s%s" C_RST " %s:%d  " C_CYA "%s" C_RST "\n",
		ko ? C_RED : C_YEL, ko ? "✗ KO  " : "⚠ AVERT", file, line, desc);
	if (msg && msg->len)
		help_filter(&g_cat.details, msg->p, msg->len);
}

void	tdf_ok(void)
{
	record(0, 0, NULL, 0, NULL, NULL);
}

void	tdf_result(int ko, int warn, const char *file, int line, const char *desc,
			const char *fmt, ...)
{
	t_buf	msg;
	char	tmp[4096];
	va_list	ap;

	memset(&msg, 0, sizeof(msg));
	if (fmt && *fmt)
	{
		va_start(ap, fmt);
		vsnprintf(tmp, sizeof(tmp), fmt, ap);
		va_end(ap);
		bprintf(&msg, "          %s\n", tmp);
	}
	record(ko, warn, file, line, desc, &msg);
	free(msg.p);
}

/* ------------------------------------------------------------------ */
/* préparation                                                          */
/* ------------------------------------------------------------------ */
static int	mktmp(void)
{
	FILE	*f;
	int		fd;

	f = tmpfile();
	if (!f)
	{
		perror("tmpfile");
		exit(2);
	}
	fd = dup(fileno(f));
	fclose(f);
	return (fd);
}

/*
** Arrêt propre. Le gestionnaire de signal ne fait que poser un drapeau (rien
** d'autre n'est sûr dans un gestionnaire) ; les boucles d'attente appellent
** tdf_quit_check(), qui tue le test en cours et rend la main sans laisser de
** processus orphelin. Un 2e signal non traité force la sortie (_exit).
*/
volatile sig_atomic_t	g_quit_requested;

static void	on_quit(int s)
{
	if (g_quit_requested)
		_exit(128 + s);
	g_quit_requested = s;
}

void	tdf_quit_check(pid_t child)
{
	int	s;

	s = g_quit_requested;
	if (!s)
		return ;
	if (child > 0)
	{
		kill(child, SIGKILL);
		waitpid(child, NULL, 0);
	}
	if (g_out >= 0 && isatty(g_out))
		dprintf(g_out, "\r\033[K\n  " C_YEL "arrêt demandé" C_RST
			" : test en cours tué, rien n'est laissé en route\n");
	signal(s, SIG_DFL);
	raise(s);
	_exit(128 + s);
}

/*
** Un lot long (10 000 appels de stress) tourne dans un seul fils : sans ceci,
** l'écran restait figé plusieurs secondes. Dix fois par seconde au plus, on
** redessine une barre (mode texte) et on écrit une ligne « P » que le jeu lit.
*/
static double	now(void);

void	tdf_progress(const char *label, int cur, int total, int end)
{
	static double	next;
	static int		shown;
	const char		*f[4];
	char			a[16];
	char			b[16];
	char			bar[128];
	int				i;

	if (total < 500 || (!end && now() < next))
		return ;
	next = now() + 0.1;
	if (cur > total || end)
		cur = total;
	snprintf(a, sizeof(a), "%d", cur);
	snprintf(b, sizeof(b), "%d", total);
	f[0] = "P";
	f[1] = label;
	f[2] = a;
	f[3] = b;
	if (!end || shown)
		m_line(4, f);
	if (isatty(g_out) && !end)
	{
		bar[0] = '\0';
		for (i = 0; i < 24; i++)
			strcat(bar, i < (int)((long)cur * 24 / total) ? "▰" : "▱");
		dprintf(g_out, "\r  " C_CYA "%s" C_RST " %3d %%  %s %d/%d  " C_DIM "(Ctrl+C : arrêter)" C_RST "\033[K",
			bar, (int)((long)cur * 100 / total), label, cur, total);
	}
	else if (isatty(g_out) && shown)
		dprintf(g_out, "\r\033[K");
	shown = !end;
}

/* un fils de test : signaux par défaut, et il meurt avec son père (Linux) */
void	tdf_in_child(void)
{
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGHUP, SIG_DFL);
#ifdef __linux__
	prctl(PR_SET_PDEATHSIG, SIGKILL);
	if (getppid() == 1)
		_exit(0);
#endif
}

void	tdf_init(void)
{
	struct sigaction	sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_quit;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
	m_open();
	g_out = dup(1);
	setvbuf(stdout, NULL, _IONBF, 0);
	g_expfd = mktmp();
	g_errfd = mktmp();
	g_devfull = open("/dev/full", O_WRONLY);
	g_shm = mmap(NULL, sizeof(t_shm), PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (g_shm == MAP_FAILED)
	{
		perror("mmap");
		exit(2);
	}
	memset(g_shm, 0, sizeof(t_shm));
	tdf_track_reset();
	signal(SIGPIPE, SIG_IGN);
}

static void	read_fd_all(int fd, t_buf *b, size_t max)
{
	char	tmp[65536];
	ssize_t	r;

	buf_reset(b);
	lseek(fd, 0, SEEK_SET);
	while ((r = read(fd, tmp, sizeof(tmp))) > 0)
	{
		if (b->len + (size_t)r > max)
			r = (ssize_t)(max - b->len);
		if (r > 0)
			buf_add(b, tmp, (size_t)r);
		if (b->len >= max)
			break ;
	}
	if (!b->p)
		buf_add(b, "", 0);
}

/* ------------------------------------------------------------------ */
/* déroulé d'un test                                                    */
/* ------------------------------------------------------------------ */
int	tdf_begin(const char *file, int line, const char *fmt, const char *args,
		int sev)
{
	static char	desc[512];
	const char	*s;

	tdf_quit_check(0);
	if (!g_cat_open)
		return (0);
	s = strrchr(file, '/');
	g_cur_file = s ? s + 1 : file;
	g_cur_line = line;
	if (args && *args)
		snprintf(desc, sizeof(desc), "ft_printf(%s, %s)", fmt, args);
	else
		snprintf(desc, sizeof(desc), "ft_printf(%s)", fmt);
	if (strlen(desc) > 150)
		strcpy(desc + 140, " …)");
	g_cur_desc = desc;
	g_cur_sev = sev;
	return (1);
}

const char	*tdf_cur_desc(void)
{
	return (g_cur_desc);
}

void	tdf_cur_where(const char **file, int *line)
{
	*file = g_cur_file;
	*line = g_cur_line;
}

/* Le vrai printf écrit dans un fichier temporaire (ou fd fermé, /dev/full). */
void	tdf_exp_start(void)
{
	fflush(stdout);
	if (ftruncate(g_expfd, 0) < 0)
		perror("ftruncate");
	lseek(g_expfd, 0, SEEK_SET);
	g_saved1 = dup(1);
	if (g_child_redirect == 0)
		dup2(g_expfd, 1);
	else if (g_child_redirect == 1)
		close(1);
	else
		dup2(g_devfull, 1);
}

void	tdf_exp_end(int ret)
{
	fflush(stdout);
	clearerr(stdout);
	dup2(g_saved1, 1);
	close(g_saved1);
	g_exp_ret = ret;
	if (g_child_redirect == 0)
		read_fd_all(g_expfd, &g_exp, MAX_OUT);
	else
		buf_reset(&g_exp);
	if (!g_exp.p)
		buf_add(&g_exp, "", 0);
}

static double	now(void)
{
	struct timespec	ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec + ts.tv_nsec / 1e9);
}

/* Attend la fin du fils en lisant sa sortie (pas de limite de taille). */
static void	collect(pid_t pid, int rfd, double timeout)
{
	double			deadline;
	struct pollfd	pfd;
	char			tmp[65536];
	ssize_t			r;
	int				ms;

	deadline = now() + timeout;
	buf_reset(&g_run.out);
	if (!g_run.out.p)
		buf_add(&g_run.out, "", 0);
	g_run.timed_out = 0;
	while (rfd >= 0)
	{
		tdf_quit_check(pid);
		ms = (int)((deadline - now()) * 1000);
		if (ms <= 0)
			break ;
		pfd.fd = rfd;
		pfd.events = POLLIN;
		if (poll(&pfd, 1, ms > 100 ? 100 : ms) <= 0)
			continue ;
		r = read(rfd, tmp, sizeof(tmp));
		if (r <= 0)
			break ;
		buf_add(&g_run.out, tmp, (size_t)r);
		if (g_run.out.len > MAX_OUT)
			break ;
	}
	if (rfd >= 0)
		close(rfd);
	while (waitpid(pid, &g_run.status, WNOHANG) == 0)
	{
		tdf_quit_check(pid);
		if (now() > deadline || g_run.out.len > MAX_OUT)
		{
			kill(pid, SIGKILL);
			waitpid(pid, &g_run.status, 0);
			g_run.timed_out = 1;
			break ;
		}
		usleep(200);
	}
	read_fd_all(g_errfd, &g_run.err, 1 << 20);
}

int	tdf_child_start(void)
{
	int		po[2];
	pid_t	pid;

	tdf_shm_clear_mem();
	if (pipe(po) < 0)
	{
		perror("pipe");
		exit(2);
	}
	if (ftruncate(g_errfd, 0) < 0)
		perror("ftruncate");
	lseek(g_errfd, 0, SEEK_SET);
	fflush(NULL);
	pid = fork();
	if (pid < 0)
	{
		perror("fork");
		exit(2);
	}
	if (pid == 0)
	{
		tdf_in_child();
		close(po[0]);
		dup2(g_errfd, 2);
		if (g_child_redirect == 0)
			dup2(po[1], 1);
		else if (g_child_redirect == 1)
			close(1);
		else
			dup2(g_devfull, 1);
		close(po[1]);
		tdf_track_reset();
		tdf_install_segv();
		return (1);
	}
	close(po[1]);
	g_run.pid = pid;
	collect(pid, po[0], g_opt.timeout);
	g_run.ret = g_shm->ret;
	g_run.returned = g_shm->returned;
	return (0);
}

void	tdf_child_end(int ret)
{
	volatile int	sink;

	if (ret < 0)
		sink = 1;
	else
		sink = 2;
	(void)sink;
	g_shm->ret = ret;
	g_shm->returned = 1;
	exit(0);
}

/* Place le format juste avant une page PROT_NONE : lire après le '\0' = SIGSEGV. */
const char	*tdf_edge(const char *fmt)
{
	static char	*page;
	static long	psz;
	size_t		len;
	size_t		need;

	if (!fmt)
		return (NULL);
	len = strlen(fmt) + 1;
	if (!psz)
		psz = sysconf(_SC_PAGESIZE);
	need = ((len + psz - 1) / psz) * psz;
	page = mmap(NULL, need + psz, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (page == MAP_FAILED)
		return (fmt);
	mprotect(page + need, psz, PROT_NONE);
	memcpy(page + need - len, fmt, len);
	return (page + need - len);
}

/* ------------------------------------------------------------------ */
/* analyse des plantages et rapports de sanitizers / valgrind           */
/* ------------------------------------------------------------------ */
static const char	*g_srcs[512];
static int			g_nsrcs;

static void	load_srcs(void)
{
	static int	loaded;
	FILE		*f;
	char		line[4096];
	size_t		l;

	if (loaded++ || !g_opt.srclist)
		return ;
	f = fopen(g_opt.srclist, "r");
	if (!f)
		return ;
	while (g_nsrcs < 512 && fgets(line, sizeof(line), f))
	{
		l = strlen(line);
		while (l && (line[l - 1] == '\n' || line[l - 1] == '\r'))
			line[--l] = '\0';
		if (l)
			g_srcs[g_nsrcs++] = strdup(line);
	}
	fclose(f);
}

/* Le chemin appartient-il aux sources de l'étudiant ? Renvoie le chemin relatif. */
static const char	*student_path(const char *path, size_t len)
{
	int			i;
	const char	*base;
	size_t		bl;
	const char	*pb;
	size_t		plen;

	load_srcs();
	pb = path + len;
	while (pb > path && pb[-1] != '/')
		pb--;
	plen = (size_t)(path + len - pb);
	i = -1;
	while (++i < g_nsrcs)
	{
		if (strlen(g_srcs[i]) == len && !strncmp(g_srcs[i], path, len))
			return (g_opt.project && !strncmp(g_srcs[i], g_opt.project,
					strlen(g_opt.project))
				? g_srcs[i] + strlen(g_opt.project) + 1 : g_srcs[i]);
		base = strrchr(g_srcs[i], '/');
		base = base ? base + 1 : g_srcs[i];
		bl = strlen(base);
		if (bl == plen && !strncmp(base, pb, plen) && pb == path)
			return (g_opt.project && !strncmp(g_srcs[i], g_opt.project,
					strlen(g_opt.project))
				? g_srcs[i] + strlen(g_opt.project) + 1 : g_srcs[i]);
	}
	return (NULL);
}

/* ASan : "#3 0x4f in ft_putnbr /chemin/ft_utils.c:42:7" */
static int	asan_student_frame(const char *start, const char *end, t_buf *msg)
{
	const char	*l;
	const char	*e;
	const char	*in;
	const char	*sp;
	const char	*col;
	char		fn[256];
	const char	*rel;

	l = start;
	while (l < end)
	{
		e = memchr(l, '\n', (size_t)(end - l));
		if (!e)
			e = end;
		in = strstr(l, " in ");
		if (in && in < e && strstr(l, "#") && strstr(l, "#") < in)
		{
			sp = memchr(in + 4, ' ', (size_t)(e - in - 4));
			if (sp)
			{
				snprintf(fn, sizeof(fn), "%.*s", (int)(sp - in - 4), in + 4);
				col = memchr(sp + 1, ':', (size_t)(e - sp - 1));
				if (col && (rel = student_path(sp + 1, (size_t)(col - sp - 1))))
				{
					const char	*c2 = memchr(col + 1, ':', (size_t)(e - col - 1));

					bprintf(msg, "          " C_BLU "→ ligne du bug : %s:%.*s  (dans %s)"
						C_RST "\n", rel, (int)((c2 ? c2 : e) - col - 1), col + 1, fn);
					return (1);
				}
			}
		}
		l = e + 1;
	}
	return (0);
}

static void	explain_sanitizer(t_buf *msg)
{
	const char	*s;
	const char	*e;
	const char	*p;
	const char	*end;
	size_t		n;

	s = g_run.err.p;
	if (!s || !*s)
		return ;
	end = s + g_run.err.len;
	if ((p = strstr(s, "runtime error:")))
	{
		const char	*rt;
		const char	*c1;
		const char	*c2;
		const char	*rel;

		rt = p;
		e = strchr(rt, '\n');
		n = e ? (size_t)(e - rt) : strlen(rt);
		while (p > s && p[-1] != '\n')
			p--;
		c1 = memchr(p, ':', (size_t)(rt - p));
		rel = c1 ? student_path(p, (size_t)(c1 - p)) : NULL;
		bprintf(msg, "          " C_RED "UBSan" C_RST " : %.*s\n", (int)n, rt);
		if (rel)
		{
			c2 = memchr(c1 + 1, ':', (size_t)(rt - c1 - 1));
			bprintf(msg, "          " C_BLU "→ ligne du bug : %s:%.*s" C_RST "\n", rel,
				(int)(c2 ? c2 - c1 - 1 : 0), c1 + 1);
		}
		else
			asan_student_frame(rt, end, msg);
		return ;
	}
	if ((p = strstr(s, "ERROR: AddressSanitizer: ")))
	{
		const char	*acc;
		const char	*on;

		p += 25;
		e = strchr(p, '\n');
		on = strstr(p, " on ");
		if (!on || (e && on > e))
			on = e;
		acc = strstr(p, "READ of size");
		if (!acc)
			acc = strstr(p, "WRITE of size");
		if (acc)
		{
			const char	*at = strstr(acc, " at ");
			size_t		alen = strcspn(acc, "\n");

			if (at && (size_t)(at - acc) < alen)
				alen = (size_t)(at - acc);
			bprintf(msg, "          " C_RED "ASan" C_RST " : %.*s (%.*s)\n",
				(int)(on ? on - p : (long)strlen(p)), p, (int)alen, acc);
		}
		else
			bprintf(msg, "          " C_RED "ASan" C_RST " : %.*s\n",
				(int)(on ? on - p : (long)strlen(p)), p);
		if (!asan_student_frame(p, end, msg))
			bprintf(msg, "          " C_DIM "(aucune frame dans tes sources : "
				"bug provoqué via un argument ?)" C_RST "\n");
		return ;
	}
	if ((p = strstr(s, "ERROR: LeakSanitizer")))
	{
		p = strstr(p, "leak of");
		while (p > s && p[-1] != '\n')
			p--;
		e = p ? strchr(p, '\n') : NULL;
		if (p)
			bprintf(msg, "          " C_RED "LSan" C_RST " : %.*s\n",
				(int)(e ? e - p : (long)strlen(p)), p);
		if (p)
			asan_student_frame(p, end, msg);
		return ;
	}
	if ((p = strstr(s, "free(): ")) || (p = strstr(s, "double free"))
		|| (p = strstr(s, "stack smashing")) || (p = strstr(s, "malloc(): "))
		|| (p = strstr(s, "corrupted")) || (p = strstr(s, "munmap_chunk")))
	{
		e = strchr(p, '\n');
		bprintf(msg, "          glibc : %.*s\n", (int)(e ? e - p : (long)strlen(p)), p);
	}
}

/* valgrind : lit le journal du fils (--log-file=dir/vg.%p) */
static int	explain_valgrind(t_buf *msg)
{
	char		path[4096];
	int			fd;
	t_buf		log;
	const char	*l;
	const char	*e;
	const char	*p;
	int			nerr;
	int			want_frame;
	const char	*paren;
	const char	*colon;
	const char	*rel;

	if (!g_opt.vglogdir)
		return (0);
	snprintf(path, sizeof(path), "%s/vg.%d", g_opt.vglogdir, (int)g_run.pid);
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (0);
	memset(&log, 0, sizeof(log));
	read_fd_all(fd, &log, 1 << 20);
	close(fd);
	unlink(path);
	nerr = 0;
	want_frame = 0;
	l = log.p;
	while (l && *l && nerr < 4)
	{
		e = strchr(l, '\n');
		if (!e)
			e = l + strlen(l);
		p = l;
		while (p < e && *p == '=')
			p++;
		while (p < e && *p != '=')
			p++;
		while (p < e && (*p == '=' || *p == ' '))
			p++;
		if (want_frame && (!strncmp(p, "at 0x", 5) || !strncmp(p, "by 0x", 5)))
		{
			paren = memchr(p, '(', (size_t)(e - p));
			colon = paren ? memchr(paren, ':', (size_t)(e - paren)) : NULL;
			if (paren && colon && (rel = student_path(paren + 1, (size_t)(colon - paren - 1))))
			{
				bprintf(msg, "          " C_BLU "→ ligne du bug : %s:%.*s" C_RST "\n",
					rel, (int)(e - colon - 2), colon + 1);
				want_frame = 0;
			}
		}
		else if (p < e && (strstr(p, "Invalid ") == p || strstr(p, "Conditional jump") == p
			|| strstr(p, "Use of uninitialised") == p || strstr(p, "Syscall param") == p
			|| strstr(p, "Mismatched free") == p || strstr(p, "Source and destination") == p
			|| ((strstr(p, "definitely lost") || strstr(p, "indirectly lost")
			|| strstr(p, "possibly lost")) && strstr(p, "blocks") && strstr(p, " are ")
			&& strstr(p, " are ") < e)))
		{
			bprintf(msg, "          " C_RED "valgrind" C_RST " : %.*s\n", (int)(e - p), p);
			want_frame = 1;
			nerr++;
		}
		l = *e ? e + 1 : e;
	}
	free(log.p);
	return (nerr);
}

void	tdf_symbolize(uintptr_t off, char *dst, size_t n)
{
	char		cmd[8192];
	FILE		*f;
	char		fn[512];
	char		loc[2048];
	const char	*rel;
	char		*c;

	snprintf(dst, n, "0x%lx", (unsigned long)off);
	if (!g_opt.exe)
		return ;
	snprintf(cmd, sizeof(cmd), "addr2line -f -e '%s' 0x%lx 2>/dev/null",
		g_opt.exe, (unsigned long)(off ? off - 1 : 0));
	f = popen(cmd, "r");
	if (!f)
		return ;
	if (fgets(fn, sizeof(fn), f) && fgets(loc, sizeof(loc), f))
	{
		fn[strcspn(fn, "\n")] = '\0';
		loc[strcspn(loc, "\n ")] = '\0';
		c = strrchr(loc, ':');
		rel = c ? student_path(loc, (size_t)(c - loc)) : NULL;
		if (rel)
			snprintf(dst, n, "%s:%s (dans %s)", rel, c + 1, fn);
		else
			snprintf(dst, n, "%s (dans %s)", loc, fn);
	}
	pclose(f);
}

/* Problèmes relevés par le traceur d'allocations (tous modes). */
void	tdf_memreport(t_buf *msg, int *ko, int *warn)
{
	char	where[2600];
	int		i;

	if (g_shm->nleak)
	{
		*ko = 1;
		bprintf(msg, "          " C_RED "fuite" C_RST " : %d bloc(s), %zu octet(s) "
			"encore alloué(s) après le retour de ft_printf\n",
			g_shm->nleak, g_shm->leak_bytes);
		i = -1;
		while (++i < g_shm->nleak && i < 3)
		{
			tdf_symbolize(g_shm->leak[i].off, where, sizeof(where));
			bprintf(msg, "          " C_BLU "→ alloué ici : %s" C_RST " (%zu octets)\n",
				where, g_shm->leak[i].size);
		}
	}
	if (g_shm->novf)
	{
		*ko = 1;
		tdf_symbolize(g_shm->ovf.off, where, sizeof(where));
		bprintf(msg, "          " C_RED "débordement de tas" C_RST " : écriture après "
			"la fin d'un bloc de %zu octets\n          " C_BLU "→ bloc alloué ici : %s"
			C_RST "  (taille trop petite ?)\n", g_shm->ovf.size, where);
	}
	if (g_shm->ndfree)
	{
		*ko = 1;
		tdf_symbolize(g_shm->dfree.off, where, sizeof(where));
		bprintf(msg, "          " C_RED "double free" C_RST "\n          " C_BLU
			"→ second free ici : %s" C_RST "\n", where);
	}
	if (g_shm->nbadfree)
	{
		*ko = 1;
		tdf_symbolize(g_shm->badfree.off, where, sizeof(where));
		bprintf(msg, "          " C_RED "free invalide" C_RST " (pointeur jamais "
			"alloué par ft_printf)\n          " C_BLU "→ ici : %s" C_RST "\n", where);
	}
	if (g_shm->nuaf)
	{
		*ko = 1;
		tdf_symbolize(g_shm->uaf.off, where, sizeof(where));
		bprintf(msg, "          " C_RED "écriture après free" C_RST " dans un bloc de "
			"%zu octets\n          " C_BLU "→ bloc alloué ici : %s" C_RST "\n",
			g_shm->uaf.size, where);
	}
	if (g_shm->nover)
	{
		*warn = 1;
		tdf_symbolize(g_shm->over[0].off, where, sizeof(where));
		bprintf(msg, "          " C_YEL "allocation trop grande" C_RST " : %zu octets "
			"alloués, %zu utilisés\n          " C_BLU "→ ici : %s" C_RST "\n",
			g_shm->over[0].size, g_shm->over[0].used, where);
	}
	if (g_shm->nforbid)
	{
		*warn = 1;
		bprintf(msg, "          " C_YEL "calloc/realloc appelé" C_RST " (fonctions "
			"non autorisées par le sujet)\n");
	}
}

static void	segv_details(t_buf *msg)
{
	char	where[2600];

	if (g_shm->crash_kind == 1)
		bprintf(msg, "          accès à l'adresse %#lx : déréférencement de NULL\n",
			(unsigned long)g_shm->crash_addr);
	else if (g_shm->crash_kind == 2 || g_shm->crash_kind == 3)
	{
		tdf_symbolize(g_shm->ovf.off, where, sizeof(where));
		bprintf(msg, "          " C_RED "%s" C_RST " (bloc de %zu octets alloué ici : %s)\n",
			g_shm->crash_kind == 2 ? "use-after-free" : "débordement de tas",
			g_shm->crash_blk, where);
	}
	if (g_shm->crash_off)
	{
		tdf_symbolize(g_shm->crash_off, where, sizeof(where));
		bprintf(msg, "          " C_BLU "→ ligne du bug : %s" C_RST "\n", where);
	}
}

static const char	*signame(int sig)
{
	if (sig == SIGSEGV)
		return ("SIGSEGV (segfault)");
	if (sig == SIGBUS)
		return ("SIGBUS");
	if (sig == SIGABRT)
		return ("SIGABRT (abort : double free, corruption, assert...)");
	if (sig == SIGFPE)
		return ("SIGFPE (division par zéro ?)");
	if (sig == SIGKILL)
		return ("SIGKILL");
	if (sig == SIGILL)
		return ("SIGILL");
	return (strsignal(sig));
}

/* Décrit l'état anormal du fils (crash, timeout, sanitizer). 1 si anormal. */
static int	crash_desc(t_buf *msg)
{
	int	code;

	if (g_run.timed_out)
	{
		bprintf(msg, "          " C_RED "TIMEOUT" C_RST " (> %.1fs : boucle infinie ?)\n",
			g_opt.timeout);
		return (1);
	}
	if (WIFSIGNALED(g_run.status))
	{
		bprintf(msg, "          " C_RED "CRASH" C_RST " : %s\n",
			signame(WTERMSIG(g_run.status)));
		if (g_shm->crashed)
			segv_details(msg);
		explain_sanitizer(msg);
		explain_valgrind(msg);
		return (1);
	}
	code = WIFEXITED(g_run.status) ? WEXITSTATUS(g_run.status) : 0;
	if (code != 0 || (g_run.err.len && (strstr(g_run.err.p, "runtime error:")
		|| strstr(g_run.err.p, "Sanitizer"))))
	{
		if (!g_run.returned)
			bprintf(msg, "          " C_RED "CRASH" C_RST " (le programme s'est arrêté "
				"pendant ft_printf, code %d)\n", code);
		else
			bprintf(msg, "          " C_RED "erreur mémoire" C_RST " signalée à la "
				"sortie (code %d)\n", code);
		explain_sanitizer(msg);
		explain_valgrind(msg);
		return (1);
	}
	if (TDF_VALGRIND)
		explain_valgrind(msg);
	return (0);
}

int	tdf_explain_crash(t_buf *msg)
{
	return (crash_desc(msg));
}

void	tdf_record_cur(int ko, int warn, t_buf *msg)
{
	record(ko, warn, g_cur_file, g_cur_line, g_cur_desc, msg);
}

void	tdf_record_at(int ko, int warn, const char *file, int line,
			const char *desc, t_buf *msg)
{
	const char	*s;

	s = strrchr(file, '/');
	record(ko, warn, s ? s + 1 : file, line, desc, msg);
}

void	tdf_bprintf(t_buf *b, const char *fmt, ...)
{
	char	tmp[4096];
	va_list	ap;
	int		n;

	va_start(ap, fmt);
	n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
	va_end(ap);
	if (n > (int)sizeof(tmp) - 1)
		n = sizeof(tmp) - 1;
	if (n > 0)
		buf_add(b, tmp, (size_t)n);
}

int	tdf_errfd(void)
{
	return (g_errfd);
}

void	tdf_collect_batch(pid_t pid)
{
	g_run.pid = pid;
	read_fd_all(g_errfd, &g_run.err, 1 << 20);
}

static void	show_diff(t_buf *msg)
{
	size_t	i;
	size_t	from;

	i = 0;
	while (i < g_exp.len && i < g_run.out.len && g_exp.p[i] == g_run.out.p[i])
		i++;
	from = 0;
	if (i > 60 && (g_exp.len > 120 || g_run.out.len > 120))
	{
		from = i - 20;
		bprintf(msg, "          (1re différence à l'octet %zu, affichage à partir de %zu)\n",
			i, from);
	}
	bprintf(msg, "          attendu : ");
	tdf_escape(msg, g_exp.p + from, g_exp.len - from, 120);
	bprintf(msg, "  ret=%d\n          obtenu  : ", g_exp_ret);
	tdf_escape(msg, g_run.out.p + from, g_run.out.len > from ? g_run.out.len - from : 0, 120);
	if (g_run.returned)
		bprintf(msg, "  ret=%d", g_run.ret);
	else
		bprintf(msg, "  ret=(aucun)");
	bprintf(msg, "\n");
}

/* Fin d'un test standard : verdict. */
void	tdf_finish(void)
{
	t_buf	msg;
	int		ko;
	int		warn;
	int		bad_out;
	int		crash;

	memset(&msg, 0, sizeof(msg));
	ko = 0;
	warn = 0;
	crash = crash_desc(&msg);
	if (g_child_redirect == 0)
		bad_out = g_exp.len != g_run.out.len
			|| memcmp(g_exp.p, g_run.out.p, g_exp.len) != 0;
	else
		bad_out = 0;
	if (!g_run.returned || g_run.ret != g_exp_ret)
		bad_out = 1;
	if (bad_out && !crash && g_run.returned && g_exp.len == g_run.out.len
		&& !memcmp(g_exp.p, g_run.out.p, g_exp.len))
		bprintf(&msg, "          sortie identique mais " C_RED "valeur de retour fausse"
			C_RST " : attendu %d, obtenu %d\n", g_exp_ret, g_run.ret);
	else if (bad_out && g_child_redirect != 0)
		bprintf(&msg, "          retour attendu %d (comme printf), obtenu %s%d\n",
			g_exp_ret, g_run.returned ? "" : "(aucun) ", g_run.ret);
	else if (bad_out)
		show_diff(&msg);
	tdf_memreport(&msg, &ko, &warn);
	if (crash || bad_out)
		ko = 1;
	if (g_cur_sev == SEV_LONE && (crash || ko > 1))
		bprintf(&msg, "          " C_BLU "→ ft_printf lit probablement au-delà du '\\0' "
			"final du format (le format est collé à une page protégée)" C_RST "\n");
	if (g_cur_sev == SEV_LONE && !crash && !g_shm->nleak && !g_shm->novf
		&& !g_shm->ndfree && !g_shm->nbadfree && !g_shm->nuaf && ko)
	{
		ko = 0;
		warn = 1;
		bprintf(&msg, "          " C_DIM "(comportement indéfini : simple avertissement ; "
			"glibc renvoie -1 sans rien écrire)" C_RST "\n");
	}
	if (g_cur_sev == SEV_UB && ko)
	{
		ko = 0;
		warn = 1;
		bprintf(&msg, "          " C_DIM "(comportement indéfini : simple avertissement)"
			C_RST "\n");
	}
	record(ko, warn, g_cur_file, g_cur_line, g_cur_desc, &msg);
	free(msg.p);
	g_child_redirect = 0;
}
