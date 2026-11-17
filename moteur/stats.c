#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include "fous.h"

#define SMAXITEM 120
#define SMAXRUN 3000
#define SMAXKIND 10
#define SMAXX 3000
#define SGARDE 49
#define SVUE 50

typedef struct s_sitem
{
	char	mod[12];
	char	name[40];
	int		fails;
	long	last;
	char	kind[4][16];
	int		kn[4];
	int		nk;
}	t_sitem;

typedef struct s_sdata
{
	int		runs[4];
	long	first;
	long	last;
	int		days;
	int		streak;
	int		recent;
	int		before;
	int		nratio;
	int		best;
	float	ratio[SMAXRUN];
	long	day[SMAXRUN];
	long	rt[SMAXRUN];
	char	rmod[SMAXRUN];
	short	rok[SMAXRUN];
	short	rtot[SMAXRUN];
	t_sitem	it[SMAXITEM];
	int		nit;
	char	kind[SMAXKIND][16];
	int		kn[SMAXKIND];
	int		nk;
	long	xt[SMAXX];
	long	xv[SMAXX];
	int		nx;
}	t_sdata;

static FILE		*g_sf;
static char		g_smod[12];
static long		g_st;
static t_sdata	g_sd;
static int		g_w = 80;

static void	stats_path(char *b, size_t n)
{
	snprintf(b, n, "%s/stats.txt", g_savedir);
}

static void	stats_trim(const char *path)
{
	FILE	*f;
	char	*buf;
	char	**ln;
	char	vu[8][12];
	char	m[12];
	char	tmpf[640];
	long	size;
	long	t;
	long	cut;
	int		n;
	int		nr;
	int		nv;
	int		ok;
	int		tot;
	int		dx;
	int		i;
	int		k;

	f = fopen(path, "rb");
	if (!f)
		return ;
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	rewind(f);
	buf = size >= 0 ? malloc(size + 1) : NULL;
	if (!buf || fread(buf, 1, size, f) != (size_t)size)
	{
		free(buf);
		fclose(f);
		return ;
	}
	fclose(f);
	buf[size] = '\0';
	for (n = 1, i = 0; i < size; i++)
		n += buf[i] == '\n';
	ln = malloc(sizeof(char *) * n);
	if (!ln)
	{
		free(buf);
		return ;
	}
	for (n = 0, nr = 0, i = 0; i <= size; n++)
	{
		ln[n] = buf + i;
		while (i < size && buf[i] != '\n')
			i++;
		buf[i++] = '\0';
		nr += ln[n][0] == 'R';
	}
	cut = 0;
	for (k = 0, i = 0; nr > SGARDE && i < n; i++)
		if (ln[i][0] == 'R' && ++k == nr - SGARDE + 1 && sscanf(ln[i], "R %ld", &cut) != 1)
			cut = 0;
	snprintf(tmpf, sizeof(tmpf), "%s.tmp", path);
	f = cut ? fopen(tmpf, "wb") : NULL;
	if (f)
	{
		nv = 0;
		dx = -1;
		for (i = 0; i < n; i++)
		{
			if (sscanf(ln[i], "V %ld %11s", &t, m) == 2
				|| (sscanf(ln[i], "R %ld %11s %d %d", &t, m, &ok, &tot) == 4 && tot > 0 && ok == tot && t < cut))
			{
				for (k = 0; k < nv && strcmp(vu[k], m); k++)
					;
				if (k == nv && nv < 8)
				{
					snprintf(vu[nv++], 12, "%s", m);
					fprintf(f, "V %ld %s\n", t, m);
				}
			}
			else if (sscanf(ln[i], "X %ld", &t) == 1 && t < cut)
				dx = i;
		}
		if (dx >= 0)
			fprintf(f, "%s\n", ln[dx]);
		for (i = 0; i < n; i++)
			if (ln[i][0] && ln[i][0] != 'V' && sscanf(ln[i] + 1, " %ld", &t) == 1 && t >= cut)
				fprintf(f, "%s\n", ln[i]);
		fclose(f);
		rename(tmpf, path);
		chmod(path, 0600);
	}
	free(ln);
	free(buf);
}

void	tdf_stats_xp(long xp)
{
	static int	lu;
	static long	dernier = -1;
	FILE		*f;
	char		path[620];
	char		l[200];
	long		t;
	long		v;

	if (!g_savedir || xp <= 0)
		return ;
	stats_path(path, sizeof(path));
	if (!lu)
	{
		lu = 1;
		f = fopen(path, "r");
		while (f && fgets(l, sizeof(l), f))
			if (sscanf(l, "X %ld %ld", &t, &v) == 2)
				dernier = v;
		if (f)
			fclose(f);
	}
	if (xp == dernier)
		return ;
	f = fopen(path, "a");
	if (!f)
		return ;
	fprintf(f, "X %ld %ld\n", (long)time(NULL), xp);
	fclose(f);
	chmod(path, 0600);
	dernier = xp;
}

static void	stats_word(char *dst, size_t n, const char *src)
{
	size_t	i;

	for (i = 0; src && src[i] && i + 1 < n; i++)
		dst[i] = (src[i] == ' ' || src[i] == '\t' || src[i] == '\n') ? '_' : src[i];
	dst[i] = '\0';
	if (!i && n > 1)
	{
		dst[0] = '?';
		dst[1] = '\0';
	}
}

void	tdf_stats_begin(const char *module)
{
	char	path[620];

	stats_path(path, sizeof(path));
	stats_trim(path);
	g_sf = fopen(path, "a");
	if (!g_sf)
		return ;
	chmod(path, 0600);
	stats_word(g_smod, sizeof(g_smod), module);
	g_st = (long)time(NULL);
}

void	tdf_stats_fail(const char *name, const char *kind)
{
	char	a[40];
	char	b[16];

	if (!g_sf)
		return ;
	stats_word(a, sizeof(a), name);
	stats_word(b, sizeof(b), kind);
	fprintf(g_sf, "K %ld %s %s %s\n", g_st, g_smod, a, b);
}

void	tdf_stats_end(int ok, int total)
{
	if (!g_sf)
		return ;
	fprintf(g_sf, "R %ld %s %d %d\n", g_st, g_smod, ok, total);
	fclose(g_sf);
	g_sf = NULL;
}

#ifndef TDF_STATS_SEUL

static long	day_of(long t)
{
	time_t		tt;
	struct tm	tmv;

	tt = (time_t)t;
	tmv = *localtime(&tt);
	tmv.tm_hour = 12;
	tmv.tm_min = 0;
	tmv.tm_sec = 0;
	return ((long)mktime(&tmv) / 86400);
}

static void	kind_add(char kind[][16], int *kn, int *nk, int max, const char *k)
{
	int	i;

	for (i = 0; i < *nk; i++)
	{
		if (!strcmp(kind[i], k))
		{
			kn[i]++;
			return ;
		}
	}
	if (*nk >= max)
		return ;
	snprintf(kind[*nk], 16, "%s", k);
	kn[*nk] = 1;
	(*nk)++;
}

static void	item_add(long t, const char *mod, const char *name, const char *kind)
{
	t_sitem	*it;
	int		i;

	for (i = 0; i < g_sd.nit; i++)
		if (!strcmp(g_sd.it[i].name, name) && !strcmp(g_sd.it[i].mod, mod))
			break ;
	if (i == g_sd.nit)
	{
		if (g_sd.nit >= SMAXITEM)
			return ;
		it = &g_sd.it[g_sd.nit++];
		memset(it, 0, sizeof(*it));
		snprintf(it->mod, sizeof(it->mod), "%s", mod);
		snprintf(it->name, sizeof(it->name), "%s", name);
	}
	it = &g_sd.it[i];
	it->fails++;
	if (t > it->last)
		it->last = t;
	kind_add(it->kind, it->kn, &it->nk, 4, kind);
	kind_add(g_sd.kind, g_sd.kn, &g_sd.nk, SMAXKIND, kind);
}

static int	item_cmp(const void *a, const void *b)
{
	const t_sitem	*x = a;
	const t_sitem	*y = b;

	if (x->fails != y->fails)
		return (y->fails - x->fails);
	return (y->last > x->last) - (y->last < x->last);
}

static void	stats_window(void)
{
	double	a;
	double	b;
	int		n;
	int		i;
	int		w;

	n = g_sd.nratio;
	w = n >= 20 ? 10 : n / 2;
	g_sd.recent = -1;
	g_sd.before = -1;
	if (n == 0)
		return ;
	if (w == 0)
		w = 1;
	a = 0;
	for (i = n - w; i < n; i++)
		a += g_sd.ratio[i];
	g_sd.recent = (int)(100.0 * a / w + 0.5);
	if (n - 2 * w < 0)
		return ;
	b = 0;
	for (i = n - 2 * w; i < n - w; i++)
		b += g_sd.ratio[i];
	g_sd.before = (int)(100.0 * b / w + 0.5);
}

static void	stats_days(void)
{
	long	today;
	long	prev;
	int		i;

	g_sd.days = 0;
	g_sd.streak = 0;
	g_sd.best = 0;
	prev = -1;
	today = 0;
	for (i = 0; i < g_sd.nratio; i++)
	{
		if (g_sd.day[i] != prev)
		{
			g_sd.days++;
			today = (prev >= 0 && g_sd.day[i] - prev == 1) ? today + 1 : 1;
			if (today > g_sd.best)
				g_sd.best = (int)today;
		}
		prev = g_sd.day[i];
	}
	if (!g_sd.nratio)
		return ;
	today = day_of((long)time(NULL));
	prev = g_sd.day[g_sd.nratio - 1];
	if (today - prev > 1)
		return ;
	g_sd.streak = 1;
	for (i = g_sd.nratio - 2; i >= 0; i--)
	{
		if (g_sd.day[i] == prev)
			continue ;
		if (prev - g_sd.day[i] != 1)
			break ;
		g_sd.streak++;
		prev = g_sd.day[i];
	}
}

static void	stats_load(void)
{
	FILE	*f;
	char	path[620];
	char	line[200];
	char	mod[12];
	char	name[40];
	char	kind[16];
	long	t;
	int		ok;
	int		tot;

	memset(&g_sd, 0, sizeof(g_sd));
	stats_path(path, sizeof(path));
	f = fopen(path, "r");
	while (f && fgets(line, sizeof(line), f))
	{
		if (sscanf(line, "R %ld %11s %d %d", &t, mod, &ok, &tot) == 4 && tot > 0)
		{
			g_sd.runs[!strcmp(mod, "printf") ? 1 : !strcmp(mod, "exam") ? 2 : !strcmp(mod, "gnl") ? 3 : 0]++;
			if (!g_sd.first || t < g_sd.first)
				g_sd.first = t;
			if (t > g_sd.last)
				g_sd.last = t;
			if (g_sd.nratio == SMAXRUN)
			{
				memmove(g_sd.ratio, g_sd.ratio + 1, sizeof(float) * (SMAXRUN - 1));
				memmove(g_sd.day, g_sd.day + 1, sizeof(long) * (SMAXRUN - 1));
				memmove(g_sd.rt, g_sd.rt + 1, sizeof(long) * (SMAXRUN - 1));
				memmove(g_sd.rmod, g_sd.rmod + 1, SMAXRUN - 1);
				memmove(g_sd.rok, g_sd.rok + 1, sizeof(short) * (SMAXRUN - 1));
				memmove(g_sd.rtot, g_sd.rtot + 1, sizeof(short) * (SMAXRUN - 1));
				g_sd.nratio--;
			}
			g_sd.ratio[g_sd.nratio] = (float)ok / tot;
			g_sd.rt[g_sd.nratio] = t;
			g_sd.rmod[g_sd.nratio] = !strcmp(mod, "printf") ? 1 : !strcmp(mod, "exam") ? 2 : !strcmp(mod, "gnl") ? 3 : 0;
			g_sd.rok[g_sd.nratio] = (short)ok;
			g_sd.rtot[g_sd.nratio] = (short)tot;
			g_sd.day[g_sd.nratio++] = day_of(t);
		}
		else if (sscanf(line, "K %ld %11s %39s %15s", &t, mod, name, kind) == 4)
			item_add(t, mod, name, kind);
		else if (sscanf(line, "X %ld %d", &t, &ok) == 2)
		{
			if (g_sd.nx == SMAXX)
			{
				memmove(g_sd.xt, g_sd.xt + 1, sizeof(long) * (SMAXX - 1));
				memmove(g_sd.xv, g_sd.xv + 1, sizeof(long) * (SMAXX - 1));
				g_sd.nx--;
			}
			g_sd.xt[g_sd.nx] = t;
			g_sd.xv[g_sd.nx++] = ok;
		}
	}
	if (f)
		fclose(f);
	qsort(g_sd.it, g_sd.nit, sizeof(t_sitem), item_cmp);
	stats_window();
	stats_days();
}

static void	save_key(const char *key, char *out, size_t n)
{
	FILE	*f;
	char	path[620];
	char	line[9000];
	size_t	kl;

	out[0] = '\0';
	snprintf(path, sizeof(path), "%s/save.txt", g_savedir);
	f = fopen(path, "r");
	if (!f)
		return ;
	kl = strlen(key);
	while (fgets(line, sizeof(line), f))
	{
		if (!strncmp(line, key, kl) && line[kl] == '=')
		{
			line[strcspn(line, "\r\n")] = '\0';
			snprintf(out, n, "%s", line + kl + 1);
			break ;
		}
	}
	fclose(f);
}

static int	ones(const char *s)
{
	int	n;

	for (n = 0; *s; s++)
		n += *s == '1';
	return (n);
}

static void	date_of(long t, char *b, size_t n)
{
	time_t	tt;

	tt = (time_t)t;
	if (!t || !strftime(b, n, "%d/%m/%Y", localtime(&tt)))
		snprintf(b, n, "-");
}

static void	ago(long t, char *b, size_t n)
{
	long	d;

	d = day_of((long)time(NULL)) - day_of(t);
	if (d <= 0)
		snprintf(b, n, "%s", L("aujourd'hui", "today"));
	else if (d == 1)
		snprintf(b, n, "%s", L("hier", "yesterday"));
	else
		snprintf(b, n, L("il y a %ld j", "%ld d ago"), d);
}

static void	line_bar(int x, int y, const char *label, int v, int max)
{
	char	tmp[80];

	text_at(x, y, theme()->text, label);
	bar(x + 22, y, 10, max ? (double)v / max : 0, theme()->accent, theme()->accent2);
	snprintf(tmp, sizeof(tmp), "%d/%d", v, max);
	text_at(x + 33, y, v == max ? rgb(120, 230, 150) : theme()->dim, tmp);
}

static int	draw_progress(int x, int y)
{
	char	v[300];
	char	tmp[160];
	int		done;
	int		n;
	int		big;

	done = 0;
	text_at(x, y++, theme()->accent2, L("PROGRESSION", "PROGRESS"));
	snprintf(tmp, sizeof(tmp), L("Niveau %d · %ld XP · %ld Octets", "Level %d · %ld XP · %ld Octets"),
		level_of(g_save.xp), g_save.xp, g_save.octets);
	text_at(x, y++, theme()->text, tmp);
	snprintf(tmp, sizeof(tmp), L("Rang : %s · prochain rang au niveau %d", "Rank: %s · next rank at level %d"),
		tdf_rang(level_of(g_save.xp), v, sizeof(v)), (level_of(g_save.xp) - 1) / 10 * 10 + 11);
	text_at(x, y++, theme()->accent2, tmp);
	n = ones(g_save.cleared);
	done += n;
	line_bar(x, y++, L("Libft · fonctions", "Libft · functions"), n, NFN);
	n = g_save.boss[0] + g_save.boss[1] + g_save.boss[2];
	done += n;
	line_bar(x, y++, L("Libft · boss", "Libft · bosses"), n, 3);
	n = ones(g_save.exo);
	done += n;
	line_bar(x, y++, L("Libft · entraînement", "Libft · training"), n, NEXO);
	save_key("printf.chains", v, sizeof(v));
	n = ones(v);
	done += n;
	line_bar(x, y++, L("printf · chaînes", "printf · chains"), n, 9);
	save_key("printf.big", v, sizeof(v));
	big = atoi(v) != 0;
	done += big;
	line_bar(x, y++, L("printf · ft_printf", "printf · ft_printf"), big, 1);
	save_key("printf.dlc", v, sizeof(v));
	n = ones(v);
	done += n;
	line_bar(x, y++, L("printf · flags", "printf · flags"), n, 6);
	save_key("printf.exo", v, sizeof(v));
	n = ones(v);
	done += n;
	line_bar(x, y++, L("printf · entraînement", "printf · training"), n, 16);
	n = NFN + 3 + NEXO + 9 + 1 + 6 + 16;
	text_at(x, y, theme()->accent, L("Avancement global", "Overall progress"));
	bar(x + 22, y, 10, (double)done / n, rgb(120, 230, 150), rgb(255, 215, 0));
	snprintf(tmp, sizeof(tmp), "%d %%", (int)(100.0 * done / n + 0.5));
	text_at(x + 33, y++, theme()->text, tmp);
	return (y);
}

static void	draw_activity(int x, int y)
{
	char	v[300];
	char	tmp[200];
	char	d1[24];
	char	d2[24];
	int		ach;

	text_at(x, y++, theme()->accent2, L("ACTIVITÉ", "ACTIVITY"));
	save_key("printf.runs", v, sizeof(v));
	snprintf(tmp, sizeof(tmp), L("Lancements : Libft %d · printf %d", "Runs: Libft %d · printf %d"),
		g_save.runs, atoi(v));
	text_at(x, y++, theme()->text, tmp);
	snprintf(tmp, sizeof(tmp), L("Jours actifs : %d · série : %d j", "Active days: %d · streak: %d d"),
		g_sd.days, g_sd.streak);
	text_at(x, y++, theme()->text, tmp);
	date_of(g_sd.first, d1, sizeof(d1));
	date_of(g_sd.last, d2, sizeof(d2));
	snprintf(tmp, sizeof(tmp), L("Suivi : du %s au %s", "Tracked: %s to %s"), d1, d2);
	text_at(x, y++, theme()->dim, tmp);
	if (g_sd.recent < 0)
		snprintf(tmp, sizeof(tmp), "%s", L("Réussite récente : -", "Recent success: -"));
	else if (g_sd.before < 0)
		snprintf(tmp, sizeof(tmp), L("Réussite récente : %d %%", "Recent success: %d %%"), g_sd.recent);
	else
		snprintf(tmp, sizeof(tmp), L("Réussite récente : %d %%  %s %+d", "Recent success: %d %%  %s %+d"),
			g_sd.recent, g_sd.recent >= g_sd.before ? "▲" : "▼", g_sd.recent - g_sd.before);
	text_at(x, y++, g_sd.recent >= g_sd.before ? rgb(120, 230, 150) : rgb(255, 150, 90), tmp);
	save_key("printf.ach", v, sizeof(v));
	ach = ones(g_save.ach) + ones(v);
	snprintf(tmp, sizeof(tmp), L("Succès : %d · Badges : %d/%d", "Achievements: %d · Badges: %d/%d"),
		ach, ones(g_save.badges), NBADGE);
	text_at(x, y++, theme()->text, tmp);
	snprintf(tmp, sizeof(tmp), L("Objets : %d · Titres : %d", "Items: %d · Titles: %d"),
		ones(g_save.items), ones(g_save.titles));
	text_at(x, y++, theme()->text, tmp);
	snprintf(tmp, sizeof(tmp), L("Plantages du menu évités : %d", "Menu crashes avoided: %d"), g_save.crashes);
	text_at(x, y++, theme()->dim, tmp);
}

static const char	*g_lf[NFN] = {
	"isalpha", "isdigit", "isalnum", "isascii", "isprint", "strlen",
	"memset", "bzero", "memcpy", "memmove", "strlcpy", "strlcat",
	"toupper", "tolower", "strchr", "strrchr", "strncmp", "memchr",
	"memcmp", "strnstr", "atoi", "calloc", "strdup",
	"substr", "strjoin", "strtrim", "split", "itoa", "strmapi", "striteri",
	"putchar_fd", "putstr_fd", "putendl_fd", "putnbr_fd",
	"lstnew", "lstadd_front", "lstsize", "lstlast", "lstadd_back",
	"lstdelone", "lstclear", "lstiter", "lstmap"
};

static t_rgb	vert(void)
{
	return (rgb(120, 230, 150));
}

static t_rgb	orange(void)
{
	return (rgb(255, 150, 90));
}

static void	title(int x, int y, const char *s)
{
	text_at(x, y, theme()->accent2, s);
}

static void	spark(int x, int y, int w, int mod, const char *label)
{
	static const char	*b[8] = {"▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"};
	int					idx[200];
	int					n;
	int					i;
	int					k;
	float				r;

	n = 0;
	for (i = g_sd.nratio - 1; i >= 0 && n < w && n < 200; i--)
		if (mod < 0 || g_sd.rmod[i] == mod)
			idx[n++] = i;
	text_at(x, y, theme()->text, label);
	if (!n)
	{
		text_at(x, y + 1, theme()->dim, L("pas encore de lancement", "no run yet"));
		return ;
	}
	at(x, y + 1);
	for (k = n - 1; k >= 0; k--)
	{
		r = g_sd.ratio[idx[k]];
		fg(r >= 0.999f ? vert() : r >= 0.75f ? rgb(255, 215, 0) : orange());
		outf("%s", b[(int)(r * 7.0f + 0.5f)]);
	}
	text_at(x, y + 2, theme()->dim, L("ancien  →  récent  (hauteur : part de réussite)", "older  →  newer  (height: success share)"));
}

static int	weak(int x, int y, int maxy, int mod, int max)
{
	static const char	*mods[4] = {"libft", "printf", "exam", "gnl"};
	char				tmp[220];
	char				when[40];
	char				nom[48];
	t_sitem				*it;
	t_rgb				c;
	int					i;
	int					k;
	int					best;
	int					shown;

	title(x, y++, L("CE QUI ÉCHOUE LE PLUS", "WHAT FAILS MOST"));
	shown = 0;
	for (i = 0; i < g_sd.nit && y < maxy && shown < max; i++)
	{
		it = &g_sd.it[i];
		if (mod >= 0 && strcmp(it->mod, mods[mod]))
			continue ;
		if (!strcmp(it->name, "niveau"))
			continue ;
		best = 0;
		for (k = 1; k < it->nk; k++)
			if (it->kn[k] > it->kn[best])
				best = k;
		ago(it->last, when, sizeof(when));
		snprintf(nom, sizeof(nom), "%s", it->name);
		for (k = 0; !strcmp(it->mod, "gnl") && nom[k]; k++)
			if (nom[k] == '_')
				nom[k] = ' ';
		c = shown < 3 ? orange() : theme()->text;
		if (mod < 0)
		{
			text_clip(x, y, c, it->mod, 7);
			text_clip(x + 8, y, c, nom, 30);
			snprintf(tmp, sizeof(tmp), L("%3d échec(s) · %s", "%3d failure(s) · %s"), it->fails, when);
			text_at(x + 40, y++, c, tmp);
		}
		else
		{
			text_clip(x, y, c, nom, 34);
			snprintf(tmp, sizeof(tmp), L("%3d échec(s) · surtout %-7s · %s", "%3d failure(s) · mostly %-7s · %s"),
				it->fails, it->kind[best], when);
			text_at(x + 36, y++, c, tmp);
		}
		shown++;
	}
	if (!shown)
		text_at(x, y++, theme()->dim, L("Aucun échec enregistré.", "No failure recorded."));
	return (y);
}

static void	page_resume(int x, int y, int maxy)
{
	char	tmp[200];
	int		yy;

	yy = draw_progress(x, y);
	draw_activity(x + g_w / 2, y);
	snprintf(tmp, sizeof(tmp), L("Plus longue série : %d j · lancements suivis : %d", "Longest streak: %d d · tracked runs: %d"),
		g_sd.best, g_sd.nratio);
	text_at(x + g_w / 2, y + 9, theme()->text, tmp);
	spark(x, yy + 2, g_w - 4, -1, L("TES DERNIERS LANCEMENTS (tous modules)", "YOUR LATEST RUNS (all modules)"));
	weak(x, yy + 7, maxy, -1, 8);
}

static void	page_libft(int x, int y, int maxy)
{
	static const int	part[4] = {0, 23, 34, 43};
	static const char	*pn[3] = {"Partie 1 · libc", "Partie 2 · ajouts", "Partie 3 · listes"};
	char				tmp[400];
	size_t				n;
	int					p;
	int					i;
	int					c;
	int					yy;

	title(x, y++, L("FONCTIONS VALIDÉES (au moins une fois)", "FUNCTIONS CLEARED (at least once)"));
	for (p = 0; p < 3; p++)
	{
		for (c = 0, i = part[p]; i < part[p + 1]; i++)
			c += g_save.cleared[i] == '1';
		line_bar(x, y++, pn[p], c, part[p + 1] - part[p]);
	}
	line_bar(x, y++, L("Boss battus", "Bosses beaten"), g_save.boss[0] + g_save.boss[1] + g_save.boss[2], 3);
	line_bar(x, y++, L("Entraînement", "Training"), ones(g_save.exo), NEXO);
	y++;
	title(x, y++, L("PAS ENCORE VALIDÉES", "NOT CLEARED YET"));
	n = 0;
	tmp[0] = '\0';
	for (i = 0; i < NFN; i++)
		if (g_save.cleared[i] != '1' && n < sizeof(tmp) - 20)
			n += snprintf(tmp + n, sizeof(tmp) - n, "%sft_%s", n ? ", " : "", g_lf[i]);
	yy = text_wrap(x, y, y + 4, g_w / 2 - 4, n ? orange() : vert(), n ? tmp : L("Toutes ! Bravo.", "All of them! Well done."));
	y = (yy > y ? yy : y + 1) + 1;
	spark(x + g_w / 2, 4, g_w / 2 - 4, 0, L("TES LANCEMENTS LIBFT", "YOUR LIBFT RUNS"));
	weak(x, y, maxy, 0, 8);
}

static void	page_printf(int x, int y, int maxy)
{
	static const char	*conv[9] = {"%c", "%s", "%p", "%d", "%i", "%u", "%x", "%X", "%%"};
	static const char	*flags[6] = {"-", "0", "largeur", ".", "#", "+ et espace"};
	char				v[300];
	char				tmp[200];
	int					i;

	title(x, y++, L("CONVERSIONS (chaînes brisées)", "CONVERSIONS (chains broken)"));
	save_key("printf.chains", v, sizeof(v));
	for (i = 0; i < 9; i++)
	{
		if (i == 0 || i == 5)
			at(x, y++);
		fg(v[i] == '1' ? vert() : theme()->dim);
		outf("%s %s   ", v[i] == '1' ? "✓" : "·", conv[i]);
	}
	save_key("printf.big", v, sizeof(v));
	text_at(x, y++, atoi(v) ? vert() : theme()->dim, atoi(v) ? L("✓ ft_printf vaincu", "✓ ft_printf beaten") : L("· ft_printf pas encore vaincu", "· ft_printf not beaten yet"));
	y++;
	title(x, y++, L("DLC · FLAGS", "DLC · FLAGS"));
	save_key("printf.dlc", v, sizeof(v));
	for (i = 0; i < 6; i++)
	{
		if (i == 0 || i == 4)
			at(x, y++);
		fg(v[i] == '1' ? vert() : theme()->dim);
		outf("%s %s   ", v[i] == '1' ? "✓" : "·", flags[i]);
	}
	save_key("printf.exo", v, sizeof(v));
	line_bar(x, y++, L("Entraînement", "Training"), ones(v), 16);
	save_key("printf.runs", v, sizeof(v));
	snprintf(tmp, sizeof(tmp), L("Lancements printf : %s", "printf runs: %s"), v[0] ? v : "0");
	text_at(x, y++, theme()->text, tmp);
	spark(x + g_w / 2, 4, g_w / 2 - 4, 1, L("TES LANCEMENTS PRINTF", "YOUR PRINTF RUNS"));
	weak(x, y + 1, maxy, 1, 8);
}

static int	bank_level(const char *name)
{
	FILE		*f;
	char		path[900];
	const char	*r;
	int			lv;

	r = getenv("TDF_RACINE");
	if (!r)
		return (0);
	snprintf(path, sizeof(path), "%s/banque/exam/%s/meta", r, name);
	f = fopen(path, "r");
	if (!f)
		return (0);
	lv = 0;
	while (fscanf(f, " niveau=%d", &lv) != 1 && fgetc(f) != EOF)
		;
	fclose(f);
	return (lv);
}

static void	bank_counts(int *tot)
{
	DIR				*d;
	struct dirent	*e;
	char			path[900];
	const char		*r;
	int				lv;

	memset(tot, 0, sizeof(int) * 5);
	r = getenv("TDF_RACINE");
	if (!r)
		return ;
	snprintf(path, sizeof(path), "%s/banque/exam", r);
	d = opendir(path);
	while (d && (e = readdir(d)))
	{
		if (e->d_name[0] == '.')
			continue ;
		lv = bank_level(e->d_name);
		if (lv >= 1 && lv <= 4)
			tot[lv]++;
	}
	if (d)
		closedir(d);
}

static void	page_exam(int x, int y, int maxy)
{
	char	v[2600];
	char	tmp[200];
	char	*tok;
	int		tot[5];
	int		got[5];
	int		lv;
	long	bt;

	title(x, y++, L("EXAM EN C", "C EXAM"));
	save_key("exam.runs", v, sizeof(v));
	snprintf(tmp, sizeof(tmp), L("Exams passés : %d", "Exams taken: %d"), atoi(v));
	text_at(x, y++, theme()->text, tmp);
	save_key("exam.passed", v, sizeof(v));
	snprintf(tmp, sizeof(tmp), L("Exams validés (100/100) : %d", "Exams passed (100/100): %d"), atoi(v));
	text_at(x, y++, atoi(v) ? vert() : theme()->text, tmp);
	save_key("exam.best", v, sizeof(v));
	snprintf(tmp, sizeof(tmp), L("Meilleure note : %d/100", "Best score: %d/100"), atoi(v));
	text_at(x, y++, theme()->text, tmp);
	save_key("exam.besttime", v, sizeof(v));
	bt = atol(v);
	if (bt > 0)
		snprintf(tmp, sizeof(tmp), L("Meilleur temps validé : %ld h %02ld min", "Best passing time: %ld h %02ld min"), bt / 3600, bt / 60 % 60);
	else
		snprintf(tmp, sizeof(tmp), "%s", L("Meilleur temps validé : -", "Best passing time: -"));
	text_at(x, y++, theme()->text, tmp);
	save_key("exam.cheats", v, sizeof(v));
	snprintf(tmp, sizeof(tmp), L("Cheats utilisés : %d", "Cheats used: %d"), atoi(v));
	text_at(x, y++, theme()->dim, tmp);
	y++;
	title(x, y++, L("EXERCICES RÉUSSIS AU MOINS UNE FOIS", "EXERCISES SOLVED AT LEAST ONCE"));
	bank_counts(tot);
	memset(got, 0, sizeof(got));
	save_key("exam.done", v, sizeof(v));
	for (tok = strtok(v, ","); tok; tok = strtok(NULL, ","))
	{
		lv = bank_level(tok);
		if (lv >= 1 && lv <= 4)
			got[lv]++;
	}
	for (lv = 1; lv <= 4; lv++)
	{
		snprintf(tmp, sizeof(tmp), L("Niveau %d", "Level %d"), lv);
		line_bar(x, y++, tmp, got[lv], tot[lv]);
	}
	spark(x + g_w / 2, 4, g_w / 2 - 4, 2, L("TES PASSAGES (corrections et exams)", "YOUR ATTEMPTS (gradings and exams)"));
	weak(x, y + 1, maxy, 2, 8);
}

static t_rgb	coul_mod(int m)
{
	static const int	c[4][3] = {{90, 200, 255}, {230, 120, 255}, {255, 215, 0}, {120, 230, 150}};

	return (rgb(c[m & 3][0], c[m & 3][1], c[m & 3][2]));
}

static const char	*nom_mod(int m)
{
	static const char	*n[4] = {"Libft", "ft_printf", "Exam", "get_next_line"};

	return (n[m & 3]);
}

static void	colonnes(int x, int y, int h, int n, const double *v, const t_rgb *c)
{
	static const char	*b[9] = {" ", "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"};
	int					k;
	int					r;
	int					q;

	for (k = 0; k < n; k++)
	{
		for (r = 0; r < h; r++)
		{
			q = (int)(v[k] * h * 8 + 0.5) - r * 8;
			if (r == 0 && q < 1 && v[k] > 0)
				q = 1;
			q = q < 0 ? 0 : q > 8 ? 8 : q;
			at(x + k, y + h - 1 - r);
			fg(c[k]);
			out(b[q]);
		}
	}
	at(x, y + h);
	fg(theme()->dim);
	for (k = 0; k < n; k++)
		out("▔");
}

static long	xp_a(int i)
{
	long	lim;
	long	v;
	int		k;

	lim = i + 1 < g_sd.nratio ? g_sd.rt[i + 1] : 0;
	v = -1;
	for (k = 0; k < g_sd.nx; k++)
		if (!lim || g_sd.xt[k] < lim)
			v = g_sd.xv[k];
	if (!lim && g_save.xp > v)
		v = g_save.xp;
	return (v);
}

static void	cadre_courbe(int x, int y, int h, const char *titre, const char *haut, const char *bas)
{
	title(x, y, titre);
	if (haut)
		text_at(x, y + 1, theme()->dim, haut);
	if (bas)
		text_at(x, y + h, theme()->dim, bas);
}

static void	page_courbes(int x, int y, int maxy)
{
	double	v[SVUE];
	t_rgb	c[SVUE];
	long	xp[SVUE];
	int		jour[SVUE];
	char	tmp[200];
	char	h1[16];
	int		cw;
	int		ch;
	int		n;
	int		d;
	int		k;
	int		x2;
	int		y2;
	long	lo;
	long	hi;
	long	d0;
	int		nj;
	int		mx;
	int		s;

	cw = g_w / 2 - 4;
	n = g_sd.nratio < SVUE ? g_sd.nratio : SVUE;
	if (n > cw - 6)
		n = cw - 6;
	if (n <= 0)
	{
		title(x, y, L("COURBES", "CHARTS"));
		text_at(x, y + 2, theme()->dim, L("Pas encore de tentative : lance des tests pour voir tes courbes.", "No attempt yet: run some tests to see your charts."));
		return ;
	}
	d = g_sd.nratio - n;
	ch = (maxy - y - 9) / 2;
	ch = ch < 3 ? 3 : ch > 9 ? 9 : ch;
	x2 = x + g_w / 2 + 1;
	y2 = y + ch + 4;
	for (k = 0; k < n; k++)
	{
		v[k] = g_sd.ratio[d + k];
		c[k] = coul_mod(g_sd.rmod[d + k]);
	}
	cadre_courbe(x, y, ch + 3, L("RÉUSSITE PAR TENTATIVE", "SUCCESS PER ATTEMPT"), "100%", NULL);
	text_at(x, y + ch, theme()->dim, "  0%");
	colonnes(x + 5, y + 1, ch, n, v, c);
	at(x + 5, y + ch + 2);
	for (k = 0; k < 4; k++)
	{
		fg(coul_mod(k));
		outf("■ %s  ", nom_mod(k));
	}
	lo = -1;
	hi = -1;
	for (k = 0; k < n; k++)
	{
		xp[k] = xp_a(d + k);
		if (xp[k] >= 0 && (lo < 0 || xp[k] < lo))
			lo = xp[k];
		if (xp[k] > hi)
			hi = xp[k];
	}
	for (k = 0; k < n; k++)
	{
		v[k] = xp[k] < 0 ? 0 : hi > lo ? 0.12 + 0.88 * (xp[k] - lo) / (double)(hi - lo) : 1;
		c[k] = theme()->accent2;
	}
	text_at(x + g_w - 2 - ulen(L("en cours d'amélioration · BETA", "being improved · BETA")), y - 1, rgb(255, 190, 70),
		L("en cours d'amélioration · BETA", "being improved · BETA"));
	title(x2, y, L("NIVEAU ET XP", "LEVEL AND XP"));
	colonnes(x2 + 5, y + 1, ch, n, v, c);
	if (lo >= 0)
		snprintf(tmp, sizeof(tmp), L("niveau %d → %d · %ld → %ld XP", "level %d → %d · %ld → %ld XP"), level_of(lo), level_of(hi), lo, hi);
	else
		snprintf(tmp, sizeof(tmp), "%s", L("XP pas encore enregistrée", "XP not recorded yet"));
	text_at(x2 + 5, y + ch + 2, theme()->dim, tmp);
	d0 = g_sd.day[d];
	nj = (int)(day_of((long)time(NULL)) - d0 + 1);
	if (nj > cw - 6)
	{
		d0 += nj - (cw - 6);
		nj = cw - 6;
	}
	if (nj > SVUE)
	{
		d0 += nj - SVUE;
		nj = SVUE;
	}
	memset(jour, 0, sizeof(jour));
	for (k = 0; k < g_sd.nratio; k++)
		if (g_sd.day[k] >= d0 && g_sd.day[k] < d0 + nj)
			jour[g_sd.day[k] - d0]++;
	for (mx = 1, k = 0; k < nj; k++)
		mx = jour[k] > mx ? jour[k] : mx;
	for (k = 0; k < nj; k++)
	{
		v[k] = (double)jour[k] / mx;
		c[k] = theme()->accent;
	}
	snprintf(h1, sizeof(h1), "%4d", mx);
	cadre_courbe(x, y2, ch + 3, L("ACTIVITÉ PAR JOUR", "ACTIVITY PER DAY"), h1, NULL);
	text_at(x, y2 + ch, theme()->dim, "   0");
	colonnes(x + 5, y2 + 1, ch, nj, v, c);
	snprintf(tmp, sizeof(tmp), L("%d jours · au plus %d tentatives en un jour", "%d days · at most %d attempts in a day"), nj, mx);
	text_at(x + 5, y2 + ch + 2, theme()->dim, tmp);
	for (s = 0, k = 0; k < n; k++)
	{
		if (g_sd.rmod[d + k] == 2)
			s += g_sd.rok[d + k];
		xp[k] = s;
	}
	for (k = 0; k < n; k++)
	{
		v[k] = s ? (double)xp[k] / s : 0;
		c[k] = rgb(255, 215, 0);
	}
	title(x2, y2, L("EXERCICES RÉUSSIS CUMULÉS", "EXERCISES PASSED, CUMULATED"));
	colonnes(x2 + 5, y2 + 1, ch, n, v, c);
	snprintf(tmp, sizeof(tmp), L("%d exercices d'exam et de la banque réussis", "%d exam and bank exercises passed"), s);
	text_at(x2 + 5, y2 + ch + 2, s ? theme()->dim : orange(), tmp);
	snprintf(tmp, sizeof(tmp), L("Axe horizontal : tes %d dernières tentatives, de la plus ancienne à la plus récente.", "Horizontal axis: your last %d attempts, from oldest to newest."), n);
	if (y2 + ch + 4 < maxy)
		text_at(x, y2 + ch + 4, theme()->dim, tmp);
}

static int	g_hsel;
static int	g_htop;

static int	echecs_de(int x, int y, int maxy, int w, long t, int mod)
{
	static const char	*mods[4] = {"libft", "printf", "exam", "gnl"};
	FILE				*f;
	char				path[620];
	char				l[200];
	char				m[12];
	char				nom[40];
	char				kind[16];
	char				tmp[120];
	long				tt;
	int					col;
	int					n;

	stats_path(path, sizeof(path));
	f = fopen(path, "r");
	n = 0;
	col = 0;
	while (f && fgets(l, sizeof(l), f))
	{
		if (sscanf(l, "K %ld %11s %39s %15s", &tt, m, nom, kind) != 4 || tt != t || strcmp(m, mods[mod & 3]))
			continue ;
		if (y >= maxy)
		{
			n++;
			continue ;
		}
		snprintf(tmp, sizeof(tmp), "✗ %s (%s)", nom, kind);
		text_clip(x + col * (w / 2), y, orange(), tmp, w / 2 - 1);
		col = w >= 44 ? !col : 0;
		if (!col)
			y++;
		n++;
	}
	if (f)
		fclose(f);
	return (n);
}

static void	page_histo(int x, int y, int maxy)
{
	char	tmp[200];
	char	d[40];
	time_t	tt;
	int		n;
	int		h;
	int		i;
	int		r;
	int		p;
	int		xd;
	int		wd;

	n = g_sd.nratio < SVUE ? g_sd.nratio : SVUE;
	snprintf(tmp, sizeof(tmp), L("HISTORIQUE · tes %d dernières tentatives", "HISTORY · your last %d attempts"), n);
	title(x, y++, tmp);
	y = text_wrap(x, y, y + 2, g_w - 2, rgb(255, 190, 70), L("⚠ Seules les 50 dernières tentatives sont gardées : au-delà, les plus anciennes sont supprimées.", "⚠ Only the last 50 attempts are kept: beyond that, the oldest ones are deleted.")) + 1;
	if (!n)
	{
		text_at(x, y, theme()->dim, L("Rien pour l'instant : lance des tests.", "Nothing yet: run some tests."));
		return ;
	}
	if (g_hsel >= n)
		g_hsel = n - 1;
	if (g_hsel < 0)
		g_hsel = 0;
	h = maxy - y;
	if (h < 3)
		h = 3;
	if (g_hsel < g_htop)
		g_htop = g_hsel;
	if (g_hsel >= g_htop + h)
		g_htop = g_hsel - h + 1;
	for (r = g_htop; r < n && r < g_htop + h; r++)
	{
		i = g_sd.nratio - 1 - r;
		tt = (time_t)g_sd.rt[i];
		strftime(d, sizeof(d), "%d/%m %H:%M", localtime(&tt));
		at(x, y + r - g_htop);
		if (r == g_hsel)
			bgc(mix(theme()->bg, theme()->accent, 0.22));
		fg(r == g_hsel ? theme()->text : theme()->dim);
		outf(" %s %s  ", r == g_hsel ? "▶" : " ", d);
		fg(coul_mod(g_sd.rmod[i]));
		outf("%-13s", nom_mod(g_sd.rmod[i]));
		fg(g_sd.ratio[i] >= 0.999f ? vert() : orange());
		outf("%3d/%-3d ", g_sd.rok[i], g_sd.rtot[i]);
		bgc(theme()->bg);
	}
	if (g_htop > 0)
		text_at(x + 36, y, theme()->dim, "▲");
	if (g_htop + h < n)
		text_at(x + 36, y + h - 1, theme()->dim, "▼");
	i = g_sd.nratio - 1 - g_hsel;
	xd = x + 40;
	wd = g_w - 42;
	tt = (time_t)g_sd.rt[i];
	strftime(d, sizeof(d), "%d/%m/%Y %H:%M", localtime(&tt));
	snprintf(tmp, sizeof(tmp), L("TENTATIVE %d SUR %d", "ATTEMPT %d OF %d"), n - g_hsel, n);
	title(xd, y, tmp);
	text_at(xd, y + 1, theme()->dim, d);
	text_at(xd, y + 2, coul_mod(g_sd.rmod[i]), nom_mod(g_sd.rmod[i]));
	snprintf(tmp, sizeof(tmp), L("Score : %d/%d · %d %%", "Score: %d/%d · %d %%"), g_sd.rok[i], g_sd.rtot[i], (int)(g_sd.ratio[i] * 100 + 0.5f));
	text_at(xd, y + 4, theme()->text, tmp);
	bar(xd, y + 5, wd < 40 ? wd : 40, g_sd.ratio[i], g_sd.ratio[i] >= 0.999f ? vert() : orange(), g_sd.ratio[i] >= 0.999f ? vert() : rgb(255, 215, 0));
	for (p = i - 1; p >= 0 && g_sd.rmod[p] != g_sd.rmod[i]; p--)
		;
	if (p < 0)
		snprintf(tmp, sizeof(tmp), "%s", L("Première tentative gardée pour ce projet.", "First attempt kept for this project."));
	else if (g_sd.rok[i] == g_sd.rok[p])
		snprintf(tmp, sizeof(tmp), "%s", L("Même score que la tentative d'avant.", "Same score as the attempt before."));
	else
		snprintf(tmp, sizeof(tmp), L("%+d par rapport à la tentative d'avant.", "%+d compared to the attempt before."), g_sd.rok[i] - g_sd.rok[p]);
	text_at(xd, y + 6, p >= 0 && g_sd.rok[i] < g_sd.rok[p] ? orange() : vert(), tmp);
	title(xd, y + 8, L("CE QUI A ÉCHOUÉ", "WHAT FAILED"));
	if (g_sd.rok[i] >= g_sd.rtot[i])
		text_at(xd, y + 9, vert(), L("Rien : tout est passé.", "Nothing: everything passed."));
	else if (!echecs_de(xd, y + 9, maxy, wd, g_sd.rt[i], g_sd.rmod[i]))
		text_at(xd, y + 9, theme()->dim, L("Détail pas enregistré pour cette tentative.", "No detail recorded for this attempt."));
}

static void	page_gnl(int x, int y, int maxy)
{
	char	tmp[200];
	char	d[40];
	char	b[64];
	char	p[700];
	FILE	*f;
	time_t	tt;
	int		i;
	int		n;
	int		der;
	int		best;
	int		nok;

	title(x, y++, L("GET_NEXT_LINE · LE PUITS", "GET_NEXT_LINE · THE WELL"));
	for (n = 0, der = -1, best = -1, i = 0; i < g_sd.nratio; i++)
	{
		if (g_sd.rmod[i] != 3)
			continue ;
		n++;
		der = i;
		if (best < 0 || g_sd.ratio[i] > g_sd.ratio[best])
			best = i;
	}
	snprintf(tmp, sizeof(tmp), L("Passages gardés : %d", "Runs kept: %d"), n);
	text_at(x, y++, theme()->text, tmp);
	if (der >= 0)
	{
		tt = (time_t)g_sd.rt[der];
		strftime(d, sizeof(d), "%d/%m/%Y %H:%M", localtime(&tt));
		snprintf(tmp, sizeof(tmp), L("Dernier passage : %d/%d · %s", "Last run: %d/%d · %s"), g_sd.rok[der], g_sd.rtot[der], d);
		text_at(x, y++, g_sd.ratio[der] >= 0.999f ? vert() : orange(), tmp);
		snprintf(tmp, sizeof(tmp), L("Meilleur passage : %d/%d", "Best run: %d/%d"), g_sd.rok[best], g_sd.rtot[best]);
		text_at(x, y++, theme()->text, tmp);
	}
	else
		text_at(x, y++, theme()->dim, L("Pas encore de passage : teste ton get_next_line depuis « Tester mon projet ».",
				"No run yet: test your get_next_line from Test my project."));
	text_at(x, y++, tdf_stats_valide("gnl") ? vert() : theme()->dim, tdf_stats_valide("gnl")
		? L("✓ validé au moins une fois sans échec", "✓ passed at least once without failure")
		: L("· pas encore validé sans échec", "· not passed without failure yet"));
	y++;
	snprintf(p, sizeof(p), "%s/puits.txt", g_savedir);
	b[0] = '\0';
	f = fopen(p, "r");
	if (f && fgets(b, sizeof(b), f))
		b[strcspn(b, "\r\n")] = '\0';
	if (f)
		fclose(f);
	if (b[0])
	{
		for (nok = 0, i = 0; b[i]; i++)
			nok += b[i] == 'O';
		snprintf(tmp, sizeof(tmp), L("LE DERNIER PUITS · %d anneaux sur 22", "THE LAST WELL · %d rings out of 22"), nok);
		title(x, y++, tmp);
		at(x, y++);
		for (i = 0; b[i] && i < 22; i++)
		{
			fg(b[i] == 'O' ? rgb(118, 222, 238) : rgb(220, 70, 60));
			out("● ");
		}
		text_at(x, y++, theme()->dim, L("bleu : le test passe · rouge : il échoue", "blue: the test passes · red: it fails"));
		y++;
	}
	spark(x + g_w / 2, 4, g_w / 2 - 4, 3, L("TES PASSAGES GET_NEXT_LINE", "YOUR GET_NEXT_LINE RUNS"));
	weak(x, y, maxy, 3, 8);
}

typedef struct s_det
{
	char	nom[64];
	char	cle[40];
	int		etat;
	t_sitem	*it;
}	t_det;

static t_det	g_det[200];
static int		g_ndet;
static int		g_dfocus;
static int		g_dsel;
static int		g_dtop;

static t_sitem	*det_item(const char *mod, const char *cle)
{
	int	i;

	for (i = 0; i < g_sd.nit; i++)
		if (!strcmp(g_sd.it[i].mod, mod) && !strcmp(g_sd.it[i].name, cle))
			return (&g_sd.it[i]);
	return (NULL);
}

static void	det_ajout(const char *mod, const char *nom, const char *cle, int etat)
{
	int	i;

	for (i = 0; i < g_ndet; i++)
		if (!strcmp(g_det[i].cle, cle))
			return ;
	if (g_ndet >= 200)
		return ;
	snprintf(g_det[g_ndet].nom, sizeof(g_det[0].nom), "%s", nom);
	snprintf(g_det[g_ndet].cle, sizeof(g_det[0].cle), "%s", cle);
	g_det[g_ndet].etat = etat;
	g_det[g_ndet].it = det_item(mod, cle);
	g_ndet++;
}

static void	det_cle(const char *nom, char *cle, size_t n)
{
	size_t	i;

	for (i = 0; nom[i] && i + 1 < n; i++)
		cle[i] = (nom[i] == ' ' || nom[i] == '\t') ? '_' : nom[i];
	cle[i] = '\0';
}

static void	det_charger(int mod)
{
	static const char	*mods[4] = {"libft", "printf", "exam", "gnl"};
	char				v[2600];
	char				l[1200];
	char				cle[40];
	char				nom[64];
	char				p[700];
	char				*c;
	char				*tok;
	FILE				*f;
	int					i;

	g_ndet = 0;
	if (mod == 0)
		for (i = 0; i < NFN; i++)
		{
			snprintf(nom, sizeof(nom), "ft_%s", g_lf[i]);
			det_ajout("libft", nom, g_lf[i], g_save.cleared[i] == '1');
		}
	if (mod == 3)
	{
		snprintf(p, sizeof(p), "%s/derniere/gnl.res", g_savedir);
		f = fopen(p, "r");
		while (f && fgets(l, sizeof(l), f))
		{
			l[strcspn(l, "\r\n")] = '\0';
			if (strncmp(l, "L\t", 2) || !(c = strchr(l + 2, '\t')))
				continue ;
			*c++ = '\0';
			tok = strchr(c, '\t');
			if (tok)
				*tok = '\0';
			det_cle(c, cle, sizeof(cle));
			det_ajout("gnl", c, cle, !strcmp(l + 2, "OK") || !strcmp(l + 2, "NOTE") || !strcmp(l + 2, "INFO"));
		}
		if (f)
			fclose(f);
	}
	if (mod == 2)
	{
		save_key("exam.done", v, sizeof(v));
		for (tok = strtok(v, ","); tok; tok = strtok(NULL, ","))
			det_ajout("exam", tok, tok, 1);
	}
	for (i = 0; i < g_sd.nit; i++)
		if (!strcmp(g_sd.it[i].mod, mods[mod]) && strcmp(g_sd.it[i].name, "niveau"))
		{
			snprintf(nom, sizeof(nom), "%s", g_sd.it[i].name);
			for (c = nom; mod == 3 && *c; c++)
				if (*c == '_')
					*c = ' ';
			det_ajout(mods[mod], nom, g_sd.it[i].name, -1);
		}
}

static void	page_details(int x, int y, int maxy, int mod)
{
	static const char	*titres[4] = {"LIBFT", "FT_PRINTF", "EXAM", "GET_NEXT_LINE"};
	char				tmp[200];
	char				d[40];
	t_det				*e;
	time_t				tt;
	int					h;
	int					i;
	int					k;
	int					xd;
	int					yd;

	det_charger(mod);
	snprintf(tmp, sizeof(tmp), L("DÉTAILS · %s · %d élément(s)", "DETAILS · %s · %d item(s)"), titres[mod], g_ndet);
	title(x, y++, tmp);
	y++;
	if (!g_ndet)
	{
		text_at(x, y, theme()->dim, L("Rien à détailler pour l'instant : lance des tests.", "Nothing to detail yet: run some tests."));
		return ;
	}
	if (g_dsel >= g_ndet)
		g_dsel = g_ndet - 1;
	h = maxy - y;
	if (h < 3)
		h = 3;
	if (g_dsel < g_dtop)
		g_dtop = g_dsel;
	if (g_dsel >= g_dtop + h)
		g_dtop = g_dsel - h + 1;
	for (i = g_dtop; i < g_ndet && i < g_dtop + h; i++)
	{
		e = &g_det[i];
		at(x, y + i - g_dtop);
		if (i == g_dsel)
			bgc(mix(theme()->bg, theme()->accent, 0.22));
		fg(e->etat == 1 ? vert() : e->etat == 0 ? orange() : theme()->dim);
		outf(" %s %s ", i == g_dsel ? "▶" : " ", e->etat == 1 ? "✓" : e->etat == 0 ? "✗" : "·");
		bgc(theme()->bg);
		text_clip(x + 6, y + i - g_dtop, i == g_dsel ? theme()->accent2 : theme()->text, e->nom, 30);
		if (e->it)
		{
			snprintf(tmp, sizeof(tmp), L("%d échec(s)", "%d failure(s)"), e->it->fails);
			text_at(x + 38, y + i - g_dtop, e->it->fails > 2 ? orange() : theme()->dim, tmp);
		}
	}
	e = &g_det[g_dsel];
	xd = x + g_w / 2 + 4;
	yd = y;
	text_clip(xd, yd++, theme()->accent2, e->nom, g_w / 2 - 6);
	yd++;
	if (mod == 0)
		text_at(xd, yd++, e->etat == 1 ? vert() : orange(), e->etat == 1
			? L("✓ validée au moins une fois", "✓ cleared at least once")
			: L("✗ pas encore validée", "✗ not cleared yet"));
	else if (mod == 3 && e->etat >= 0)
		text_at(xd, yd++, e->etat == 1 ? vert() : orange(), e->etat == 1
			? L("✓ passe au dernier test", "✓ passes in the last test")
			: L("✗ échoue au dernier test", "✗ fails in the last test"));
	else if (mod == 2 && e->etat == 1)
		text_at(xd, yd++, vert(), L("✓ réussi au moins une fois", "✓ passed at least once"));
	if (!e->it)
	{
		text_at(xd, yd++, theme()->dim, L("Aucun échec enregistré dans tes 50 dernières tentatives.",
				"No failure recorded in your last 50 attempts."));
		return ;
	}
	snprintf(tmp, sizeof(tmp), L("Échecs : %d dans tes 50 dernières tentatives", "Failures: %d in your last 50 attempts"), e->it->fails);
	text_at(xd, yd++, orange(), tmp);
	tt = (time_t)e->it->last;
	strftime(d, sizeof(d), "%d/%m/%Y %H:%M", localtime(&tt));
	ago(e->it->last, tmp, sizeof(tmp));
	snprintf(tmp + strlen(tmp), sizeof(tmp) - strlen(tmp), " · %s", d);
	text_at(xd, yd++, theme()->text, L("Dernier échec :", "Last failure:"));
	text_at(xd + 2, yd++, theme()->dim, tmp);
	yd++;
	text_at(xd, yd++, theme()->text, L("Par type d'échec :", "By kind of failure:"));
	for (k = 0; k < e->it->nk; k++)
	{
		snprintf(tmp, sizeof(tmp), "  %-10s %d", e->it->kind[k], e->it->kn[k]);
		text_at(xd, yd++, theme()->dim, tmp);
	}
	if (mod == 0 && e->etat != 1)
		text_at(xd, yd + 1, rgb(255, 215, 0), L("Première validation : +20 ◈ et +40 XP", "First clear: +20 ◈ and +40 XP"));
}

void	tdf_screen_stats(void)
{
	static int	page;
	const char	*noms[7];
	int			x;
	int			i;
	int			k;

	noms[0] = L("Résumé", "Summary");
	noms[1] = L("Courbes", "Charts");
	noms[2] = "Libft";
	noms[3] = "ft_printf";
	noms[4] = "get_next_line";
	noms[5] = "Exam";
	noms[6] = L("Historique", "History");
	save_write();
	stats_load();
	g_hsel = 0;
	g_htop = 0;
	g_dfocus = 0;
	cls();
	while (1)
	{
		g_w = term_cols() - 8;
		if (g_w > 140)
			g_w = 140;
		if (g_w < 80)
			g_w = 80;
		x = (term_cols() - g_w) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		frame();
		at(x, 1);
		for (i = 0; i < 7; i++)
		{
			if (i == page)
			{
				bgc(mix(theme()->bg, theme()->accent, 0.3));
				fg(theme()->accent2);
			}
			else
				fg(theme()->dim);
			outf(" %s ", noms[i]);
			bgc(theme()->bg);
			if (i == 4 && tdf_nouveau("3.2"))
				tdf_badge_new();
			outf("  ");
		}
		if (g_dfocus && page >= 2 && page <= 5)
			page_details(x, 4, term_rows() - 3, page == 2 ? 0 : page == 3 ? 1 : page == 4 ? 3 : 2);
		else if (page == 0)
			page_resume(x, 4, term_rows() - 3);
		else if (page == 1)
			page_courbes(x, 4, term_rows() - 3);
		else if (page == 2)
			page_libft(x, 4, term_rows() - 3);
		else if (page == 3)
			page_printf(x, 4, term_rows() - 3);
		else if (page == 4)
			page_gnl(x, 4, term_rows() - 3);
		else if (page == 5)
			page_exam(x, 4, term_rows() - 3);
		else
			page_histo(x, 4, term_rows() - 3);
		if (page == 6)
			foot_line(x, L("▲ ▼ choisir une tentative   ◀ ▶ changer de page   Échap : retour", "▲ ▼ pick an attempt   ◀ ▶ change page   Esc: back"));
		else if (page >= 2 && page <= 5 && g_dfocus)
			foot_line(x, L("▲ ▼ choisir   ◀ ▶ changer de page   Échap : retour au résumé", "▲ ▼ pick   ◀ ▶ change page   Esc: back to the summary"));
		else if (page >= 2 && page <= 5)
		{
			foot_line(x, L("▼ voir les détails   ◀ ▶ changer de page   Échap : retour", "▼ see the details   ◀ ▶ change page   Esc: back"));
			if (tdf_nouveau("3.2"))
			{
				at(x + ulen(L("▼ voir les détails   ◀ ▶ changer de page   Échap : retour", "▼ see the details   ◀ ▶ change page   Esc: back")) + 2, term_rows() - 1);
				tdf_badge_new();
			}
		}
		else
			foot_line(x, L("◀ ▶ changer de page   Échap : retour", "◀ ▶ change page   Esc: back"));
		flush();
		k = key_dir(key_read(200));
		if (page >= 2 && page <= 5 && g_dfocus && (k == 1000 || k == 1001 || k == 27))
		{
			if (k == 27 || (k == 1000 && g_dsel == 0))
				g_dfocus = 0;
			else if (k == 1000)
				g_dsel--;
			else
				g_dsel++;
			cls();
			continue ;
		}
		if (page >= 2 && page <= 5 && !g_dfocus && k == 1001)
		{
			g_dfocus = 1;
			g_dsel = 0;
			g_dtop = 0;
			cls();
			continue ;
		}
		if (k == 1002 || k == 1003)
			g_dfocus = 0;
		if (k == 1003)
			page = (page + 6) % 7;
		else if (k == 1002)
			page = (page + 1) % 7;
		else if (k == 1000 && page == 6)
			g_hsel--;
		else if (k == 1001 && page == 6)
			g_hsel++;
		else if (k >= '1' && k <= '7')
			page = k - '1';
		else if (key_quit(k) || k == 127 || k == 8 || k == '\r' || k == '\n')
			break ;
		if (k > 0)
			cls();
	}
	cls();
}

#endif
