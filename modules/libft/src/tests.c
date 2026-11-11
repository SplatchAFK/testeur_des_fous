#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include "libft.h"

/*
** Moteur de tests du testeur des fous.
** Chaque fonction est testée dans son propre binaire (-DT_nom).
** Ta libft est compilée avec -include tdf_hook.h : chacun de tes malloc et
** free passe par le traqueur ci-dessous, qui connaît la taille de chaque
** bloc, son propriétaire et le cas de test en cours. Ça marche partout
** (Linux, macOS, avec ou sans sanitizers), sans --wrap ni dlsym.
*/

#define MAX_SHOWN 8
#define MAX_WARN 4
#define COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))

static int	g_checks;
static int	g_fails;
static int	g_warns;
static char	g_case[200] = "début du test";
static long	g_case_seq;

static void	check(int ok, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));
static void	warn(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));
static void	setcase(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));

static void	check(int ok, const char *fmt, ...)
{
	va_list	ap;

	g_checks++;
	if (ok)
		return ;
	g_fails++;
	if (g_fails > MAX_SHOWN)
		return ;
	printf("  ko| ");
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
}

static void	warn(const char *fmt, ...)
{
	va_list	ap;

	g_warns++;
	if (g_warns > MAX_WARN)
		return ;
	printf("  wn| ");
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
}

static void	setcase(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vsnprintf(g_case, sizeof(g_case), fmt, ap);
	va_end(ap);
	g_case_seq++;
}

/* ------------------------------------------------------------------ */
/* Traqueur de mémoire                                                  */
/* ------------------------------------------------------------------ */

#define TSZ (1 << 18)
#define OWN_STU 1
#define OWN_TST 2
#define TOMB ((void *)1)

typedef struct s_blk
{
	void	*p;
	size_t	n;
	int		own;
	int		lab;
}	t_blk;

static t_blk	g_tab[TSZ];
static char		**g_labs;
static int		g_nlab;
static int		g_caplab;
static long		g_lab_seq = -1;
static int		g_stu_may_free;
static int		g_badfree_shown;
static int		g_inj_on;
static int		g_inj_cnt;
static int		g_inj_at;

static size_t	hsh(const void *p)
{
	return ((size_t)(((uintptr_t)p >> 4) * 2654435761u) & (TSZ - 1));
}

static int	blk_find(const void *p)
{
	size_t	i;
	size_t	k;

	i = hsh(p);
	for (k = 0; k < TSZ; k++)
	{
		if (g_tab[i].p == NULL)
			return (-1);
		if (g_tab[i].p == p)
			return ((int)i);
		i = (i + 1) & (TSZ - 1);
	}
	return (-1);
}

static int	lab_now(void)
{
	char	**nw;

	if (g_lab_seq == g_case_seq && g_nlab > 0)
		return (g_nlab - 1);
	if (g_nlab == g_caplab)
	{
		g_caplab = g_caplab ? g_caplab * 2 : 256;
		nw = realloc(g_labs, g_caplab * sizeof(char *));
		if (!nw)
			return (-1);
		g_labs = nw;
	}
	g_labs[g_nlab] = strdup(g_case);
	g_lab_seq = g_case_seq;
	return (g_nlab++);
}

static void	blk_add(void *p, size_t n, int own)
{
	size_t	i;
	int		tomb;

	tomb = -1;
	i = hsh(p);
	while (g_tab[i].p != NULL)
	{
		if (g_tab[i].p == p)
		{
			tomb = (int)i;
			break ;
		}
		if (g_tab[i].p == TOMB && tomb < 0)
			tomb = (int)i;
		i = (i + 1) & (TSZ - 1);
	}
	if (tomb >= 0)
		i = (size_t)tomb;
	g_tab[i].p = p;
	g_tab[i].n = n;
	g_tab[i].own = own;
	g_tab[i].lab = own == OWN_STU ? lab_now() : -1;
}

/* ton malloc */
void	*tdf_malloc_s(size_t n)
{
	void	*p;

	if (g_inj_on && ++g_inj_cnt == g_inj_at)
		return (NULL);
	p = malloc(n);
	if (p)
		blk_add(p, n, OWN_STU);
	return (p);
}

static void	bad_free(const char *who, const void *p)
{
	g_fails++;
	g_checks++;
	if (g_badfree_shown++ >= 2 || g_fails > MAX_SHOWN)
		return ;
	if (*who == 's')
		printf("  ko| free(%p) dans ta fonction : ce bloc n'est pas alloué"
			" (déjà libéré ? jamais alloué ?)  · cas : %s\n", p, g_case);
	else
		printf("  ko| le pointeur %p rendu par ta fonction n'est plus un bloc"
			" malloc vivant (libéré par toi ? chaîne littérale ? adresse dans"
			" l'argument ?)  · cas : %s\n", p, g_case);
}

/* ton free */
void	tdf_free_s(void *p)
{
	int	i;

	if (p == NULL)
		return ;
	i = blk_find(p);
	if (i < 0)
	{
		bad_free("s", p);
		return ;
	}
	if (g_tab[i].own == OWN_TST && !g_stu_may_free)
	{
		g_fails++;
		g_checks++;
		if (g_fails <= MAX_SHOWN)
			printf("  ko| ta fonction a libéré une mémoire qui ne lui appartient"
				" pas (un de ses arguments ?)  · cas : %s\n", g_case);
	}
	g_tab[i].p = TOMB;
	free(p);
}

/* malloc et free du testeur */
static void	*tst_malloc(size_t n)
{
	void	*p;

	p = malloc(n ? n : 1);
	if (!p)
	{
		printf("  ko| le testeur n'a plus de mémoire\n");
		exit(3);
	}
	blk_add(p, n, OWN_TST);
	return (p);
}

static void	tst_free(void *p)
{
	int	i;

	if (p == NULL)
		return ;
	i = blk_find(p);
	if (i < 0)
	{
		bad_free("t", p);
		return ;
	}
	g_tab[i].p = TOMB;
	free(p);
}

#define free(p) tst_free(p)

/* le bloc est-il encore vivant (alloué et pas libéré) ? */
static int	tst_alive(const void *p)
{
	return (p != NULL && blk_find(p) >= 0);
}

/* taille exacte du bloc rendu : trop petit ou trop grand = faux */
static int	size_is(const void *p, size_t want, const char *what)
{
	int	i;

	if (p == NULL)
		return (1);
	i = blk_find(p);
	if (i < 0 || g_tab[i].own != OWN_STU)
	{
		check(0, "%s : le résultat n'a pas été alloué avec malloc  · cas : %s",
			what, g_case);
		return (0);
	}
	if (g_tab[i].n < want)
	{
		check(0, "%s : malloc(%zu) au lieu de malloc(%zu), trop petit  · cas : %s",
			what, g_tab[i].n, want, g_case);
		return (0);
	}
	if (g_tab[i].n > want)
	{
		check(0, "%s : malloc(%zu) au lieu de malloc(%zu), trop grand (tripouille"
			" : MKO)  · cas : %s", what, g_tab[i].n, want, g_case);
		return (0);
	}
	return (1);
}

static void	inject_on(int k)
{
	g_inj_cnt = 0;
	g_inj_at = k;
	g_inj_on = 1;
}

static int	inject_off(void)
{
	g_inj_on = 0;
	return (g_inj_cnt >= g_inj_at);
}

/* blocs alloués par ta fonction et jamais libérés */
static void	leak_report(void)
{
	size_t	i;
	long	nb;
	long	bytes;
	int		shown[4];
	int		ns;
	int		k;
	int		l;

	nb = 0;
	bytes = 0;
	ns = 0;
	for (i = 0; i < TSZ; i++)
	{
		if (g_tab[i].p == NULL || g_tab[i].p == TOMB || g_tab[i].own != OWN_STU)
			continue ;
		nb++;
		bytes += (long)g_tab[i].n;
		l = g_tab[i].lab;
		for (k = 0; k < ns && shown[k] != l; k++)
			;
		if (k == ns && ns < 3)
		{
			shown[ns++] = l;
			printf("  lk| %zu octets jamais libérés, alloués pendant : %s\n",
				g_tab[i].n, l >= 0 ? g_labs[l] : "?");
		}
	}
	if (nb)
		printf("@@L %ld %ld\n", nb, bytes);
}

/* ------------------------------------------------------------------ */
/* Crash, boucle infinie : on dit quel cas tournait                     */
/* ------------------------------------------------------------------ */

static void	say_case(const char *tag)
{
	char	buf[300];
	int		n;

	n = snprintf(buf, sizeof(buf), "\n@@%s %s\n", tag, g_case);
	if (n > 0)
		(void)!write(1, buf, (size_t)n < sizeof(buf) ? (size_t)n : sizeof(buf));
}

static void	on_death(void)
{
	say_case("C");
}

static void	on_sig(int s)
{
	if (s == SIGALRM)
	{
		say_case("T");
		_exit(124);
	}
	say_case("C");
	signal(s, SIG_DFL);
	raise(s);
}

void	__sanitizer_set_death_callback(void (*cb)(void)) __attribute__((weak));

static void	guard_init(void)
{
	const char	*t;
	int			sec;

	if (__sanitizer_set_death_callback)
		__sanitizer_set_death_callback(on_death);
	signal(SIGSEGV, on_sig);
	signal(SIGBUS, on_sig);
	signal(SIGFPE, on_sig);
	signal(SIGILL, on_sig);
	signal(SIGABRT, on_sig);
	signal(SIGALRM, on_sig);
	t = getenv("TDF_TIMEOUT");
	sec = t ? atoi(t) : 20;
	alarm(sec > 0 ? (unsigned)sec : 20);
}

/* ------------------------------------------------------------------ */
/* Outils                                                               */
/* ------------------------------------------------------------------ */

static const char	*show(const void *p, size_t n)
{
	static char			buf[8][220];
	static int			w;
	const unsigned char	*s;
	char				*o;
	size_t				i;
	size_t				k;

	o = buf[w++ & 7];
	if (p == NULL)
		return (strcpy(o, "(null)"));
	s = p;
	k = 0;
	for (i = 0; i < n && k < 180; i++)
	{
		if (s[i] == '\n')
			k += sprintf(o + k, "\\n");
		else if (s[i] == '\t')
			k += sprintf(o + k, "\\t");
		else if (s[i] == '\\')
			k += sprintf(o + k, "\\\\");
		else if (s[i] < 32 || s[i] > 126)
			k += sprintf(o + k, "\\x%02x", s[i]);
		else
			o[k++] = s[i];
	}
	if (i < n)
		k += sprintf(o + k, "...");
	o[k] = '\0';
	return (o);
}
#define S(x) show((x), (x) ? strlen(x) : 0)

static int	sgn(long x)
{
	return ((x > 0) - (x < 0));
}

static void	*xalloc(size_t n)
{
	return (tst_malloc(n));
}

static char	*xdup(const char *s)
{
	size_t	n;
	char	*p;

	n = strlen(s) + 1;
	p = tst_malloc(n);
	memcpy(p, s, n);
	return (p);
}

static void	*xbytes(const void *s, size_t n)
{
	void	*p;

	p = tst_malloc(n);
	memcpy(p, s, n);
	return (p);
}

/* ------------------------------------------------------------------ */
/* Partie 1                                                             */
/* ------------------------------------------------------------------ */

/* La libc renvoie « non nul » pour vrai (souvent 1024, 2048...) :
** seul le vrai/faux compte, comme pour tripouille et la moulinette. */
static void	ctype_bool(const char *n, int (*mine)(int), int (*real)(int))
{
	int	c;
	int	a;
	int	b;

	for (c = -1; c <= 255; c++)
	{
		setcase("%s(%d)", n, c);
		a = mine(c);
		b = real(c);
		check((a != 0) == (b != 0), "%s(%d) = %d, la libc dit %s", n, c, a,
			b ? "vrai (non nul)" : "faux (0)");
	}
}

static void	ctype_conv(const char *n, int (*mine)(int), int (*real)(int))
{
	int	c;

	for (c = -1; c <= 255; c++)
	{
		setcase("%s(%d)", n, c);
		check(mine(c) == real(c), "%s(%d) = %d, la libc dit %d", n, c,
			mine(c), real(c));
	}
}

#define REAL(f) static int r_##f(int c) { return (f(c)); }
REAL(isalpha) REAL(isdigit) REAL(isalnum) REAL(isascii) REAL(isprint)
REAL(toupper) REAL(tolower)

#ifdef T_isalpha
static void	run_test(void) { ctype_bool("ft_isalpha", ft_isalpha, r_isalpha); }
#endif
#ifdef T_isdigit
static void	run_test(void) { ctype_bool("ft_isdigit", ft_isdigit, r_isdigit); }
#endif
#ifdef T_isalnum
static void	run_test(void) { ctype_bool("ft_isalnum", ft_isalnum, r_isalnum); }
#endif
#ifdef T_isascii
static void	run_test(void) { ctype_bool("ft_isascii", ft_isascii, r_isascii); }
#endif
#ifdef T_isprint
static void	run_test(void) { ctype_bool("ft_isprint", ft_isprint, r_isprint); }
#endif
#ifdef T_toupper
static void	run_test(void) { ctype_conv("ft_toupper", ft_toupper, r_toupper); }
#endif
#ifdef T_tolower
static void	run_test(void) { ctype_conv("ft_tolower", ft_tolower, r_tolower); }
#endif

#ifdef T_strlen
static void	run_test(void)
{
	static const char	*tab[] = {"", "a", "ab", "hello world", "  42\n\t",
		"\xff\xfe\x80", "tab\there", "ab\0cd"};
	char				*p;
	size_t				i;
	int					k;

	for (k = 0; k < COUNT(tab); k++)
	{
		p = xdup(tab[k]);
		setcase("strlen(\"%s\")", show(p, strlen(p)));
		check(ft_strlen(p) == strlen(p), "strlen(\"%s\") = %zu, attendu %zu",
			show(p, strlen(p)), ft_strlen(p), strlen(p));
		free(p);
	}
	for (i = 0; i < 70; i++)
	{
		p = xalloc(i + 1);
		memset(p, 'x', i);
		p[i] = '\0';
		setcase("strlen de %zu x", i);
		check(ft_strlen(p) == i, "strlen de %zu x = %zu", i, ft_strlen(p));
		free(p);
	}
	p = xalloc((1 << 20) + 1);
	memset(p, 'y', 1 << 20);
	p[1 << 20] = '\0';
	setcase("strlen d'une chaîne de 1 Mio");
	check(ft_strlen(p) == (1 << 20), "strlen d'une chaîne de 1 Mio = %zu",
		ft_strlen(p));
	free(p);
}
#endif

#if defined(T_strchr) || defined(T_strrchr)
static char	*r_strchr(const char *s, int c) { return (strchr(s, c)); }
static char	*r_strrchr(const char *s, int c) { return (strrchr(s, c)); }

static void	chr_loop(const char *n, char *(*mine)(const char *, int),
		char *(*real)(const char *, int))
{
	static const char	*tab[] = {"", "a", "aaa", "hello", "hello world",
		"42 Paris\n", "abcabcabc", "\xff\xfe\x80", "x\x7f", "a b  c"};
	char				*p;
	char				*a;
	char				*b;
	int					k;
	int					c;

	for (k = 0; k < COUNT(tab); k++)
	{
		p = xdup(tab[k]);
		for (c = -300; c <= 600; c++)
		{
			setcase("%s(\"%s\", %d)", n, S(p), c);
			a = mine(p, c);
			b = real(p, c);
			check(a == b, "%s(\"%s\", %d) -> %s, la libc -> %s", n, S(p), c,
				a ? show(a, strlen(a)) : "NULL", b ? show(b, strlen(b)) : "NULL");
		}
		free(p);
	}
}
#endif
#ifdef T_strchr
static void	run_test(void) { chr_loop("ft_strchr", ft_strchr, r_strchr); }
#endif
#ifdef T_strrchr
static void	run_test(void) { chr_loop("ft_strrchr", ft_strrchr, r_strrchr); }
#endif

#ifdef T_strncmp
static void	run_test(void)
{
	static const char	*tab[] = {"", "a", "abc", "abd", "abcd", "ABC", "\xff",
		"\x80", "a\xff", "test\x80", "test", "Hello", "hello", "1234", "1235"};
	static const size_t	ns[] = {0, 1, 2, 3, 4, 5, 6, 7, 100, SIZE_MAX};
	char				*x;
	char				*y;
	int					i;
	int					j;
	int					k;

	for (i = 0; i < COUNT(tab); i++)
		for (j = 0; j < COUNT(tab); j++)
		{
			x = xdup(tab[i]);
			y = xdup(tab[j]);
			for (k = 0; k < COUNT(ns); k++)
			{
				setcase("strncmp(\"%s\", \"%s\", %zu)", S(x), S(y), ns[k]);
				check(sgn(ft_strncmp(x, y, ns[k])) == sgn(strncmp(x, y, ns[k])),
					"strncmp(\"%s\", \"%s\", %zu) = %d, la libc %d (le signe"
					" doit être le même, octets en unsigned char)",
					S(x), S(y), ns[k], ft_strncmp(x, y, ns[k]),
					strncmp(x, y, ns[k]));
			}
			free(x);
			free(y);
		}
}
#endif

#ifdef T_memset
static void	run_test(void)
{
	static const int	vals[] = {0, 'A', 127, 128, 255, 256 + 66, -1, INT_MAX,
		INT_MIN};
	unsigned char		a[40];
	unsigned char		b[40];
	unsigned char		*p;
	void				*ret;
	size_t				off;
	size_t				n;
	int					v;

	for (v = 0; v < COUNT(vals); v++)
	{
		for (off = 0; off < 6; off += 5)
			for (n = 0; n + off <= 40; n++)
			{
				setcase("memset(buf+%zu, %d, %zu)", off, vals[v], n);
				memset(a, '#', 40);
				memset(b, '#', 40);
				ret = ft_memset(a + off, vals[v], n);
				memset(b + off, vals[v], n);
				check(ret == a + off, "memset c=%d n=%zu : ne renvoie pas dest",
					vals[v], n);
				check(memcmp(a, b, 40) == 0,
					"memset(buf+%zu, %d, %zu) : mauvais octets", off, vals[v], n);
			}
		for (n = 0; n <= 33; n++)
		{
			setcase("memset sur un bloc de %zu octets pile", n);
			p = xalloc(n);
			ret = ft_memset(p, vals[v], n);
			check(ret == p, "memset bloc exact n=%zu : ne renvoie pas dest", n);
			free(p);
		}
	}
	setcase("memset de 100000 octets");
	p = xalloc(100000);
	ft_memset(p, 'm', 100000);
	for (n = 0; n < 100000 && p[n] == 'm'; n++)
		;
	check(n == 100000, "memset de 100000 octets : octet %zu faux", n);
	free(p);
}
#endif

#ifdef T_bzero
static void	run_test(void)
{
	unsigned char	a[40];
	unsigned char	*p;
	size_t			off;
	size_t			n;
	size_t			i;
	int				bad;

	for (off = 0; off < 6; off += 5)
		for (n = 0; n + off <= 40; n++)
		{
			setcase("bzero(buf+%zu, %zu)", off, n);
			memset(a, '#', 40);
			ft_bzero(a + off, n);
			bad = 0;
			for (i = 0; i < 40; i++)
				if ((i >= off && i < off + n && a[i] != 0)
					|| ((i < off || i >= off + n) && a[i] != '#'))
					bad = 1;
			check(!bad, "bzero(buf+%zu, %zu) : pas mis à zéro ou écrit à côté",
				off, n);
		}
	for (n = 0; n <= 33; n++)
	{
		setcase("bzero sur un bloc de %zu octets pile", n);
		p = xalloc(n);
		memset(p, 0x55, n);
		ft_bzero(p, n);
		for (i = 0; i < n; i++)
			if (p[i] != 0)
				break ;
		check(i == n, "bzero bloc exact n=%zu : octet %zu pas à zéro", n, i);
		free(p);
	}
}
#endif

#ifdef T_memcpy
static void	run_test(void)
{
	unsigned char	pat[64];
	unsigned char	big[100];
	unsigned char	*src;
	unsigned char	*dst;
	void			*ret;
	size_t			n;
	size_t			i;

	for (i = 0; i < 64; i++)
		pat[i] = (unsigned char)(i * 7 + (i % 5 == 0 ? 0 : 3));
	for (n = 0; n <= 64; n++)
	{
		setcase("memcpy de %zu octets entre blocs exacts", n);
		src = xbytes(pat, n);
		dst = xalloc(n);
		memset(dst, 0x55, n);
		ret = ft_memcpy(dst, src, n);
		check(ret == dst, "memcpy n=%zu : ne renvoie pas dest", n);
		check(memcmp(dst, pat, n) == 0,
			"memcpy n=%zu : mauvais octets (arrêt sur un '\\0' ?)", n);
		free(src);
		free(dst);
		setcase("memcpy de %zu octets au milieu d'un tampon", n);
		memset(big, 0x55, 100);
		src = xbytes(pat, n);
		ft_memcpy(big + 16, src, n);
		check(memcmp(big + 16, pat, n) == 0, "memcpy au milieu, n=%zu", n);
		for (i = 0; i < 100; i++)
			if ((i < 16 || i >= 16 + n) && big[i] != 0x55)
				break ;
		check(i == 100, "memcpy n=%zu : a touché l'octet %zu hors zone", n, i);
		free(src);
	}
}
#endif

#ifdef T_memmove
static void	run_test(void)
{
	static const size_t	ns[] = {0, 1, 2, 3, 7, 8, 16, 17, 24};
	unsigned char		a[40];
	unsigned char		b[40];
	unsigned char		*s;
	unsigned char		*d;
	unsigned char		*x;
	unsigned char		*y;
	void				*ret;
	size_t				i;
	size_t				si;
	size_t				di;
	int					k;

	for (si = 0; si < 16; si++)
		for (di = 0; di < 16; di++)
			for (k = 0; k < COUNT(ns); k++)
			{
				if (si + ns[k] > 40 || di + ns[k] > 40)
					continue ;
				setcase("memmove(buf+%zu, buf+%zu, %zu)", di, si, ns[k]);
				for (i = 0; i < 40; i++)
					a[i] = b[i] = (unsigned char)(i + 1);
				ret = ft_memmove(a + di, a + si, ns[k]);
				memmove(b + di, b + si, ns[k]);
				check(ret == a + di, "memmove dst+%zu src+%zu n=%zu : retour",
					di, si, ns[k]);
				check(memcmp(a, b, 40) == 0,
					"memmove dst+%zu src+%zu n=%zu : mauvais octets (sens de copie)",
					di, si, ns[k]);
			}
	for (i = 0; i <= 40; i++)
	{
		setcase("memmove entre blocs exacts, n=%zu", i);
		for (si = 0; si < 40; si++)
			a[si] = (unsigned char)(si * 3);
		s = xbytes(a, i);
		d = xalloc(i);
		ft_memmove(d, s, i);
		check(memcmp(d, a, i) == 0, "memmove entre blocs exacts n=%zu", i);
		free(s);
		free(d);
	}
	x = xalloc(5000);
	y = xalloc(5000);
	for (k = 0; k < 2; k++)
	{
		setcase("memmove de 4000 octets qui se chevauchent (%s)",
			k ? "vers la gauche" : "vers la droite");
		for (i = 0; i < 5000; i++)
			x[i] = y[i] = (unsigned char)(i * 13 + 7);
		ft_memmove(k ? x : x + 997, k ? x + 997 : x, 4000);
		memmove(k ? y : y + 997, k ? y + 997 : y, 4000);
		check(memcmp(x, y, 5000) == 0,
			"memmove de 4000 octets qui se chevauchent (%s) : mauvais octets",
			k ? "vers la gauche" : "vers la droite");
	}
	free(x);
	free(y);
}
#endif

#ifdef T_memchr
static void	run_test(void)
{
	static const unsigned char	data[20] = {'h', 'e', 'l', 'l', 'o', 0, 'w',
		'o', 'r', 0xff, 'l', 'd', 0x80, '!', 'a', 'b', 'c', 'a', 0x7f, 0};
	unsigned char				*p;
	size_t						n;
	int							c;

	for (n = 0; n <= 20; n++)
	{
		p = xbytes(data, n);
		for (c = -300; c <= 600; c++)
		{
			setcase("memchr(buf, %d, %zu)", c, n);
			check(ft_memchr(p, c, n) == memchr(p, c, n),
				"memchr(buf, %d, %zu) = %p, la libc %p", c, n, ft_memchr(p, c, n),
				memchr(p, c, n));
		}
		free(p);
	}
}
#endif

#ifdef T_memcmp
static void	run_test(void)
{
	static const unsigned char	a[][9] = {"abcdefgh", "abcdefgh", "abc\0defg",
		"\xff" "bcdefgh", "a\200cdefgh", "abcdefgh", "\0\0\0\0\0\0\0\1",
		"abcdefgh", "\x7f" "bcdefgh", "\x01\x02\x03\x04\x05\x06\x07\x08"};
	static const unsigned char	b[][9] = {"abcdefgh", "abcdefgX", "abc\0dezg",
		"\x01" "bcdefgh", "a\0cdefgh", "Abcdefgh", "\0\0\0\0\0\0\0\2",
		"abcdefg", "\x80" "bcdefgh", "\x01\x02\x03\x04\x05\x06\x07\xff"};
	unsigned char				*x;
	unsigned char				*y;
	size_t						n;
	int							i;

	for (i = 0; i < COUNT(a); i++)
		for (n = 0; n <= 8; n++)
		{
			setcase("memcmp paire %d, n=%zu", i, n);
			x = xbytes(a[i], n);
			y = xbytes(b[i], n);
			check(sgn(ft_memcmp(x, y, n)) == sgn(memcmp(x, y, n)),
				"memcmp paire %d n=%zu = %d, la libc %d (même signe, octets en"
				" unsigned char)", i, n, ft_memcmp(x, y, n), memcmp(x, y, n));
			free(x);
			free(y);
		}
}
#endif

#if defined(T_strlcpy) || defined(T_strlcat)
static size_t	ref_strlcpy(char *d, const char *s, size_t size)
{
	size_t	l;
	size_t	c;

	l = strlen(s);
	if (size)
	{
		c = (l >= size) ? size - 1 : l;
		memcpy(d, s, c);
		d[c] = '\0';
	}
	return (l);
}

static size_t	ref_strlcat(char *d, const char *s, size_t size)
{
	size_t	dl;
	size_t	sl;
	size_t	c;

	dl = 0;
	while (dl < size && d[dl])
		dl++;
	sl = strlen(s);
	if (dl == size)
		return (size + sl);
	c = (sl > size - dl - 1) ? size - dl - 1 : sl;
	memcpy(d + dl, s, c);
	d[dl + c] = '\0';
	return (dl + sl);
}
#endif

#ifdef T_strlcpy
static void	run_test(void)
{
	static const char	*srcs[] = {"", "a", "ab", "hello", "abcdefg", "\xff\x80z"};
	char				*s;
	char				*d;
	char				*r;
	size_t				size;
	size_t				a;
	size_t				b;
	int					k;

	for (k = 0; k < COUNT(srcs); k++)
		for (size = 0; size <= 10; size++)
		{
			setcase("strlcpy(dst[%zu], \"%s\", %zu)", size, S(srcs[k]), size);
			s = xdup(srcs[k]);
			d = xalloc(size);
			r = xalloc(size);
			memset(d, 'X', size);
			memset(r, 'X', size);
			a = ft_strlcpy(d, s, size);
			b = ref_strlcpy(r, s, size);
			check(a == b, "strlcpy(dst, \"%s\", %zu) renvoie %zu, attendu %zu",
				S(s), size, a, b);
			check(memcmp(d, r, size) == 0,
				"strlcpy(dst, \"%s\", %zu) : dst=\"%s\" attendu \"%s\"", S(s),
				size, show(d, size), show(r, size));
			free(s);
			free(d);
			free(r);
		}
	setcase("strlcpy(dst[10], \"hello\", SIZE_MAX)");
	s = xdup("hello");
	d = xalloc(10);
	memset(d, 'A', 10);
	a = ft_strlcpy(d, s, SIZE_MAX);
	check(a == 5 && strcmp(d, "hello") == 0 && d[6] == 'A',
		"strlcpy(dst, \"hello\", SIZE_MAX) : copie fausse ou écrit trop loin");
	free(s);
	free(d);
	setcase("strlcpy d'une chaîne de 5000 caractères");
	s = xalloc(5001);
	memset(s, 'q', 5000);
	s[5000] = '\0';
	d = xalloc(5001);
	check(ft_strlcpy(d, s, 5001) == 5000 && strcmp(d, s) == 0,
		"strlcpy d'une chaîne de 5000 caractères");
	free(s);
	free(d);
}
#endif

#ifdef T_strlcat
static void	run_test(void)
{
	size_t	size;
	size_t	dl;
	size_t	sl;
	size_t	cap;
	size_t	a;
	size_t	b;
	char	*d;
	char	*r;
	char	*s;

	for (size = 0; size <= 10; size++)
		for (dl = 0; dl <= 12; dl++)
			for (sl = 0; sl <= 6; sl++)
			{
				setcase("strlcat(dst[longueur %zu], src de %zu, %zu)", dl, sl, size);
				cap = (dl >= size ? dl : size) + 1;
				d = xalloc(cap);
				r = xalloc(cap);
				memset(d, 'X', cap);
				memset(d, 'D', dl);
				d[dl] = '\0';
				memcpy(r, d, cap);
				s = xalloc(sl + 1);
				memcpy(s, (size + dl) % 2 ? "a\xff\x80" "bcd" : "abcdef", sl);
				s[sl] = '\0';
				if (dl && size % 3 == 0)
					memset(d, 0xE9, dl);
				memcpy(r, d, cap);
				a = ft_strlcat(d, s, size);
				b = ref_strlcat(r, s, size);
				check(a == b,
					"strlcat(dst[longueur %zu], \"%s\", %zu) renvoie %zu, attendu %zu%s",
					dl, S(s), size, a, b,
					dl >= size ? " (dst plus long que size : renvoyer size + len(src),"
					" ne rien écrire)" : "");
				check(memcmp(d, r, cap) == 0,
					"strlcat(dst[longueur %zu], \"%s\", %zu) : dst=\"%s\" attendu \"%s\"",
					dl, S(s), size, show(d, cap), show(r, cap));
				free(d);
				free(r);
				free(s);
			}
}
#endif

#ifdef T_strnstr
static char	*ref_strnstr(const char *h, const char *n, size_t len)
{
	size_t	nl;
	size_t	i;

	nl = strlen(n);
	if (nl == 0)
		return ((char *)h);
	for (i = 0; h[i] && i + nl <= len; i++)
		if (strncmp(h + i, n, nl) == 0)
			return ((char *)h + i);
	return (NULL);
}

static void	run_test(void)
{
	static const char	*hs[] = {"", "a", "aaab", "abcabcd", "hello world",
		"lorem ipsum dolor sit amet", "mississippi", "ab\xff\x80" "cd\xff"};
	static const char	*ns[] = {"", "a", "aa", "aab", "abcd", "ss", "issip",
		"ppi", "world", "xyz", "mississippi", "mississippix", "o w", "\xff",
		"\x80" "cd"};
	char				*h;
	char				*n;
	char				*a;
	char				*b;
	size_t				len;
	int					i;
	int					j;

	for (i = 0; i < COUNT(hs); i++)
		for (j = 0; j < COUNT(ns); j++)
		{
			h = xdup(hs[i]);
			n = xdup(ns[j]);
			for (len = 0; len <= strlen(h) + 2; len++)
			{
				setcase("strnstr(\"%s\", \"%s\", %zu)", S(h), S(n), len);
				a = ft_strnstr(h, n, len);
				b = ref_strnstr(h, n, len);
				check(a == b, "strnstr(\"%s\", \"%s\", %zu) -> position %ld,"
					" attendu %ld (-1 = NULL)", S(h), S(n), len,
					a ? (long)(a - h) : -1L, b ? (long)(b - h) : -1L);
			}
			setcase("strnstr(\"%s\", \"%s\", SIZE_MAX)", S(h), S(n));
			a = ft_strnstr(h, n, SIZE_MAX);
			b = ref_strnstr(h, n, SIZE_MAX);
			check(a == b, "strnstr(\"%s\", \"%s\", SIZE_MAX) diffère", S(h), S(n));
			free(h);
			free(n);
		}
}
#endif

#ifdef T_atoi
static void	run_test(void)
{
	static const char	*tab[] = {"0", "42", "-42", "+42", "   42",
		" \t\n\v\f\r42", "2147483647", "-2147483648", "42abc", "abc", "",
		"--42", "+-42", "-+42", "  -0012", "4 2", " + 42", "-", "+", "  -",
		"0000", "-0", "+0", "\t-2147483648xyz", "9", "-9", "0x1A", "1e5",
		"\v42", "\f-42", " 12 34", "00000000000000000042",
		"-00000000000000002147483648", "2147483647abc", "\xff" "42",
		"42\xff", " ---1", "1-1", "2147483646", "-2147483647", "\x1b" "42",
		"\x80" "5", " \t+7", "12\n34", "-1", "1000000000", "-999999999"};
	char				*p;
	int					i;

	for (i = 0; i < COUNT(tab); i++)
	{
		p = xdup(tab[i]);
		setcase("atoi(\"%s\")", S(p));
		check(ft_atoi(p) == atoi(p), "atoi(\"%s\") = %d, la libc %d", S(p),
			ft_atoi(p), atoi(p));
		free(p);
	}
}
#endif

#ifdef T_calloc
static void	expect_null(size_t a, size_t b, const char *txt)
{
	void	*p;

	setcase("calloc(%s)", txt);
	p = ft_calloc(a, b);
	check(p == NULL, "calloc(%s) doit renvoyer NULL (dépassement de size_t)",
		txt);
	free(p);
}

static void	run_test(void)
{
	static const size_t	pairs[][2] = {{1, 1}, {3, 7}, {250, 4}, {1000, 1000},
		{1, 65536}, {65536, 1}, {2, 2}, {7, 1}};
	static const size_t	zeros[][2] = {{0, 0}, {0, 8}, {8, 0}, {0, SIZE_MAX},
		{SIZE_MAX, 0}, {0, (size_t)-5}, {(size_t)-5, 0}};
	unsigned char		*p;
	size_t				i;
	size_t				a;
	size_t				b;
	int					k;

	for (k = 0; k < 3 * COUNT(pairs); k++)
	{
		a = pairs[k % COUNT(pairs)][0];
		b = pairs[k % COUNT(pairs)][1];
		setcase("calloc(%zu, %zu)", a, b);
		p = ft_calloc(a, b);
		check(p != NULL, "calloc(%zu, %zu) renvoie NULL", a, b);
		if (p == NULL)
			continue ;
		size_is(p, a * b, "calloc");
		for (i = 0; i < a * b; i++)
			if (p[i] != 0)
				break ;
		check(i == a * b, "calloc(%zu, %zu) : l'octet %zu n'est pas à zéro"
			" (mémoire recyclée sale ?)", a, b, i);
		memset(p, 0xAB, a * b);
		free(p);
	}
	for (k = 0; k < COUNT(zeros); k++)
	{
		setcase("calloc(%zu, %zu)", zeros[k][0], zeros[k][1]);
		p = ft_calloc(zeros[k][0], zeros[k][1]);
		if (p == NULL)
			warn("calloc(%zu, %zu) renvoie NULL : autorisé par POSIX, mais la"
				" libc de Linux renvoie un pointeur libérable", zeros[k][0],
				zeros[k][1]);
		free(p);
	}
	expect_null(SIZE_MAX, 2, "SIZE_MAX, 2");
	expect_null(2, SIZE_MAX, "2, SIZE_MAX");
	expect_null(SIZE_MAX, SIZE_MAX, "SIZE_MAX, SIZE_MAX");
	expect_null(SIZE_MAX / 2 + 1, 2, "SIZE_MAX/2+1, 2 : le produit fait 0");
	expect_null(2, SIZE_MAX / 2 + 1, "2, SIZE_MAX/2+1 : le produit fait 0");
	expect_null((size_t)1 << 32, (size_t)1 << 32, "2^32, 2^32 : le produit fait 0");
	expect_null(SIZE_MAX / 2, 3, "SIZE_MAX/2, 3");
	setcase("calloc(INT_MAX, INT_MAX)");
	p = ft_calloc(INT_MAX, INT_MAX);
	check(p == NULL, "calloc(INT_MAX, INT_MAX) doit renvoyer NULL");
	free(p);
	setcase("calloc(4, 4) avec malloc en échec");
	inject_on(1);
	p = ft_calloc(4, 4);
	if (inject_off())
		check(p == NULL, "calloc doit renvoyer NULL quand malloc échoue");
	free(p);
}
#endif

#ifdef T_strdup
static void	run_test(void)
{
	static const char	*tab[] = {"", "a", "hello", "hello world\n\t42",
		"\xff\x80\x7f", "coucou"};
	char				*s;
	char				*d;
	char				*big;
	int					i;

	for (i = 0; i < COUNT(tab); i++)
	{
		s = xdup(tab[i]);
		setcase("strdup(\"%s\")", S(s));
		d = ft_strdup(s);
		check(d != NULL, "strdup(\"%s\") renvoie NULL", S(s));
		if (d)
		{
			check(strcmp(d, s) == 0, "strdup(\"%s\") = \"%s\"", S(s),
				show(d, strlen(d)));
			check(d != s, "strdup(\"%s\") renvoie le même pointeur", S(s));
			size_is(d, strlen(s) + 1, "strdup");
			free(d);
		}
		free(s);
	}
	setcase("strdup de 100000 caractères");
	big = xalloc(100001);
	memset(big, 'z', 100000);
	big[100000] = '\0';
	d = ft_strdup(big);
	check(d && strcmp(d, big) == 0, "strdup de 100000 caractères");
	free(d);
	free(big);
	setcase("strdup(\"fail me\") avec malloc en échec");
	s = xdup("fail me");
	inject_on(1);
	d = ft_strdup(s);
	if (inject_off())
		check(d == NULL, "strdup doit renvoyer NULL quand malloc échoue");
	free(d);
	free(s);
}
#endif

/* ------------------------------------------------------------------ */
/* Partie 2                                                             */
/* ------------------------------------------------------------------ */

#ifdef T_substr
static void	run_test(void)
{
	static const char		*tab[] = {"", "a", "hello", "hello world, 42!",
		"tripouille", "BONJOUR LES HARICOTS !", "\xff\x80" "abc\xfe"};
	static const size_t		lens[] = {SIZE_MAX, SIZE_MAX - 1, 1000, 42000000};
	static const unsigned	starts[] = {UINT_MAX, UINT_MAX - 1, 1000};
	char					*s;
	char					*r;
	char					exp[64];
	size_t					sl;
	size_t					st;
	size_t					ln;
	size_t					el;
	int						k;

	for (k = 0; k < COUNT(tab); k++)
	{
		s = xdup(tab[k]);
		sl = strlen(s);
		for (st = 0; st <= sl + 2; st++)
			for (ln = 0; ln <= sl + 2; ln++)
			{
				setcase("substr(\"%s\", %zu, %zu)", S(s), st, ln);
				r = ft_substr(s, (unsigned int)st, ln);
				el = st >= sl ? 0 : (ln > sl - st ? sl - st : ln);
				if (el)
					memcpy(exp, s + st, el);
				exp[el] = '\0';
				check(r != NULL && strcmp(r, exp) == 0,
					"substr(\"%s\", %zu, %zu) = %s, attendu \"%s\"", S(s), st,
					ln, r ? show(r, strlen(r)) : "NULL", exp);
				if (r && strcmp(r, exp) == 0)
					size_is(r, el + 1, "substr");
				free(r);
			}
		for (st = 0; st < (size_t)COUNT(starts); st++)
			for (ln = 0; ln < (size_t)COUNT(lens); ln++)
			{
				setcase("substr(\"%s\", %u, %zu)", S(s), starts[st], lens[ln]);
				r = ft_substr(s, starts[st], lens[ln]);
				check(r != NULL && *r == '\0',
					"substr(\"%s\", %u, %zu) doit être une chaîne vide", S(s),
					starts[st], lens[ln]);
				if (r && *r == '\0')
					size_is(r, 1, "substr");
				free(r);
				setcase("substr(\"%s\", 0, %zu)", S(s), lens[ln]);
				r = ft_substr(s, 0, lens[ln]);
				check(r != NULL && strcmp(r, s) == 0,
					"substr(\"%s\", 0, %zu) doit être la chaîne entière", S(s),
					lens[ln]);
				if (r && strcmp(r, s) == 0)
					size_is(r, sl + 1, "substr");
				free(r);
			}
		free(s);
	}
	s = xdup("fail me");
	for (k = 1; k < 4; k++)
	{
		setcase("substr(\"fail me\", 1, 3) avec malloc n°%d en échec", k);
		inject_on(k);
		r = ft_substr(s, 1, 3);
		if (!inject_off())
		{
			free(r);
			break ;
		}
		check(r == NULL, "substr doit renvoyer NULL quand malloc n°%d échoue", k);
		free(r);
	}
	free(s);
}
#endif

#ifdef T_strjoin
static void	run_test(void)
{
	static const char	*tab[] = {"", "a", "hello", "world !", "\xff\x80",
		"tripouille", "42"};
	char				*x;
	char				*y;
	char				*r;
	char				*e;
	char				*big;
	int					i;
	int					j;

	for (i = 0; i < COUNT(tab); i++)
		for (j = 0; j < COUNT(tab); j++)
		{
			x = xdup(tab[i]);
			y = xdup(tab[j]);
			setcase("strjoin(\"%s\", \"%s\")", S(x), S(y));
			r = ft_strjoin(x, y);
			e = xalloc(strlen(x) + strlen(y) + 1);
			strcpy(e, x);
			strcat(e, y);
			check(r != NULL && strcmp(r, e) == 0,
				"strjoin(\"%s\", \"%s\") = %s, attendu \"%s\"", S(x), S(y),
				r ? show(r, strlen(r)) : "NULL", show(e, strlen(e)));
			check(r != x && r != y, "strjoin renvoie un de ses arguments");
			check(strcmp(x, tab[i]) == 0 && strcmp(y, tab[j]) == 0,
				"strjoin a modifié un argument");
			if (r && r != x && r != y)
				size_is(r, strlen(e) + 1, "strjoin");
			free(e);
			if (r != x && r != y)
				free(r);
			free(x);
			free(y);
		}
	setcase("strjoin de 2 x 50000 caractères");
	big = xalloc(50001);
	memset(big, 'b', 50000);
	big[50000] = '\0';
	r = ft_strjoin(big, big);
	check(r && strlen(r) == 100000 && r[99999] == 'b', "strjoin de 2 x 50000");
	free(r);
	free(big);
	x = xdup("left");
	y = xdup("right");
	for (i = 1; i < 5; i++)
	{
		setcase("strjoin(\"left\", \"right\") avec malloc n°%d en échec", i);
		inject_on(i);
		r = ft_strjoin(x, y);
		if (inject_off())
			check(r == NULL, "strjoin doit renvoyer NULL quand malloc échoue");
		free(r);
	}
	free(x);
	free(y);
}
#endif

#ifdef T_strtrim
static char	*ref_trim(const char *s, const char *set)
{
	size_t	a;
	size_t	b;
	char	*r;

	a = 0;
	while (s[a] && strchr(set, s[a]))
		a++;
	b = strlen(s);
	while (b > a && strchr(set, s[b - 1]))
		b--;
	r = xalloc(b - a + 1);
	memcpy(r, s + a, b - a);
	r[b - a] = '\0';
	return (r);
}

static void	run_test(void)
{
	static const char	*sets[] = {"", " ", "a", "ab", "ba ", "abab", "xyz"};
	static const char	*hand[][3] = {{"  abc  ", " ", "abc"},
	{"\t\n  hello \n\t", " \n\t", "hello"}, {"abcba", "ab", "c"},
	{"xxhixx", "x", "hi"}, {"hello", "xyz", "hello"}, {"xxxx", "x", ""},
	{"", "x", ""}, {"a", "a", ""}, {"hello", "", "hello"},
	{"   a   b   ", " ", "a   b"}, {"1234abc4321", "4321", "abc"},
	{"   xxxtripouille", " x", "tripouille"}, {"tripouille   xxx", " x",
		"tripouille"}, {"   xxx   xxx", " x", ""}, {"", "", ""},
	{"abcdba", "acb", "d"}, {"ababa", "a", "bab"},
	{"\xff\xfehello\xfe\xff", "\xff\xfe", "hello"}, {"a\x80" "b", "\x80", "a\x80" "b"},
	{"\x80\x80", "\x80", ""}, {"hi\xe9", " ", "hi\xe9"}};
	static const char	alpha[] = "ab ";
	char				*s;
	char				*t;
	char				*r;
	char				*e;
	int					i;
	int					len;
	int					n;
	int					m;
	int					cnt;

	for (i = 0; i < COUNT(hand); i++)
	{
		s = xdup(hand[i][0]);
		t = xdup(hand[i][1]);
		setcase("strtrim(\"%s\", \"%s\")", S(s), S(t));
		r = ft_strtrim(s, t);
		check(r != NULL && strcmp(r, hand[i][2]) == 0,
			"strtrim(\"%s\", \"%s\") = %s, attendu \"%s\"", S(s), S(t),
			r ? show(r, strlen(r)) : "NULL", hand[i][2]);
		if (r && strcmp(r, hand[i][2]) == 0)
			size_is(r, strlen(r) + 1, "strtrim");
		free(r);
		free(s);
		free(t);
	}
	for (len = 0; len <= 5; len++)
	{
		cnt = 1;
		for (i = 0; i < len; i++)
			cnt *= 3;
		for (n = 0; n < cnt; n++)
		{
			s = xalloc(len + 1);
			for (i = 0, m = n; i < len; i++, m /= 3)
				s[i] = alpha[m % 3];
			s[len] = '\0';
			for (i = 0; i < COUNT(sets); i++)
			{
				t = xdup(sets[i]);
				setcase("strtrim(\"%s\", \"%s\")", S(s), S(t));
				r = ft_strtrim(s, t);
				e = ref_trim(s, t);
				check(r != NULL && strcmp(r, e) == 0,
					"strtrim(\"%s\", \"%s\") = %s, attendu \"%s\"", S(s), S(t),
					r ? show(r, strlen(r)) : "NULL", e);
				free(r);
				free(e);
				free(t);
			}
			free(s);
		}
	}
	s = xdup("  fail me  ");
	t = xdup(" ");
	for (i = 1; i < 4; i++)
	{
		setcase("strtrim(\"  fail me  \", \" \") avec malloc n°%d en échec", i);
		inject_on(i);
		r = ft_strtrim(s, t);
		if (inject_off())
			check(r == NULL, "strtrim doit renvoyer NULL quand malloc échoue");
		free(r);
	}
	free(s);
	free(t);
}
#endif

#ifdef T_split
typedef struct s_scase
{
	const char	*s;
	char		c;
	const char	*exp[16];
}	t_scase;

static void	free_split(char **r, int max)
{
	int	i;

	for (i = 0; i < max && r[i]; i++)
		free(r[i]);
	free(r);
}

static void	run_test(void)
{
	static const t_scase	cases[] = {
	{"hello world", ' ', {"hello", "world"}},
	{"  hello   world  ", ' ', {"hello", "world"}},
	{"abc", ' ', {"abc"}}, {"", ' ', {NULL}}, {"     ", ' ', {NULL}},
	{"a", 'a', {NULL}}, {"aaa", 'a', {NULL}}, {"a", ' ', {"a"}},
	{"a b c d e", ' ', {"a", "b", "c", "d", "e"}},
	{"  a", ' ', {"a"}}, {"a  ", ' ', {"a"}}, {" a ", ' ', {"a"}},
	{"a,b,,c,", ',', {"a", "b", "c"}}, {",,,", ',', {NULL}},
	{"abc", '\0', {"abc"}}, {"", '\0', {NULL}},
	{"tab\tsep\tok", '\t', {"tab", "sep", "ok"}},
	{"line1\nline2\n\nline3", '\n', {"line1", "line2", "line3"}},
	{"xAxxBxxxCxxxxD", 'x', {"A", "B", "C", "D"}},
	{"a\xff" "b", '\xff', {"a", "b"}},
	{"x\x01y z", ' ', {"x\x01y", "z"}}, {"\xe9t\xe9 b", ' ', {"\xe9t\xe9", "b"}},
	{"a+b,c+d", ',', {"a+b", "c+d"}}, {"a\tb c\nd", ' ', {"a\tb", "c\nd"}},
	{"\x01\x02 \x03", ' ', {"\x01\x02", "\x03"}}, {"a\xff" "b\xffc", 'b', {"a\xff", "\xffc"}},
	{"  tripouille  42  ", ' ', {"tripouille", "42"}},
	{"--1-2--3---4----5-----42", '-', {"1", "2", "3", "4", "5", "42"}},
	{"lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed non risus.",
		' ', {"lorem", "ipsum", "dolor", "sit", "amet,", "consectetur",
			"adipiscing", "elit.", "Sed", "non", "risus."}}};
	char					**r;
	char					*in;
	char					*big;
	char					c;
	int						k;
	int						i;
	int						ne;

	for (k = 0; k < COUNT(cases); k++)
	{
		in = xdup(cases[k].s);
		c = cases[k].c >= 32 ? cases[k].c : '?';
		setcase("split(\"%s\", '%c')", S(in), c);
		r = ft_split(in, cases[k].c);
		ne = 0;
		while (cases[k].exp[ne])
			ne++;
		check(r != NULL, "split(\"%s\", '%c') renvoie NULL", S(in), c);
		if (r)
		{
			size_is(r, (ne + 1) * sizeof(char *), "split (le tableau)");
			for (i = 0; i < ne; i++)
			{
				if (r[i] == NULL)
				{
					check(0, "split(\"%s\", '%c') : %d mots seulement, attendu %d",
						S(in), c, i, ne);
					break ;
				}
				check(strcmp(r[i], cases[k].exp[i]) == 0,
					"split(\"%s\", '%c')[%d] = \"%s\", attendu \"%s\"", S(in),
					c, i, show(r[i], strlen(r[i])), cases[k].exp[i]);
				size_is(r[i], strlen(r[i]) + 1, "split (un mot)");
			}
			if (i == ne)
				check(r[ne] == NULL,
					"split(\"%s\", '%c') : tableau pas terminé par NULL après %d mots",
					S(in), c, ne);
			free_split(r, i == ne ? ne : i);
		}
		free(in);
	}
	setcase("split de 1000 mots d'une lettre");
	big = xalloc(2001);
	for (i = 0; i < 1000; i++)
	{
		big[2 * i] = 'w';
		big[2 * i + 1] = ' ';
	}
	big[2000] = '\0';
	r = ft_split(big, ' ');
	for (i = 0; r && i < 1000 && r[i]; i++)
		;
	check(r && i == 1000 && r[1000] == NULL, "split de 1000 mots d'une lettre");
	if (r)
		free_split(r, i);
	free(big);
	in = xdup("ab cd ef gh");
	for (k = 1; k < 12; k++)
	{
		setcase("split(\"ab cd ef gh\", ' ') avec malloc n°%d en échec", k);
		inject_on(k);
		r = ft_split(in, ' ');
		if (inject_off())
			check(r == NULL,
				"split doit renvoyer NULL quand malloc n°%d échoue (et libérer"
				" ce qui était déjà alloué)", k);
		else
		{
			free_split(r, 4);
			break ;
		}
	}
	free(in);
}
#endif

#ifdef T_itoa
static void	itoa_one(int n)
{
	char	exp[32];
	char	*r;

	setcase("itoa(%d)", n);
	snprintf(exp, sizeof(exp), "%d", n);
	r = ft_itoa(n);
	check(r != NULL && strcmp(r, exp) == 0, "itoa(%d) = %s, attendu \"%s\"",
		n, r ? show(r, strlen(r)) : "NULL", exp);
	if (r && strcmp(r, exp) == 0)
		size_is(r, strlen(exp) + 1, "itoa");
	free(r);
}

static void	run_test(void)
{
	unsigned int	seed;
	long			p;
	int				n;
	int				i;
	char			*r;

	for (n = -1100; n <= 1100; n++)
		itoa_one(n);
	itoa_one(INT_MIN);
	itoa_one(INT_MIN + 1);
	itoa_one(INT_MAX);
	itoa_one(INT_MAX - 1);
	for (p = 1; p <= 1000000000; p *= 10)
	{
		itoa_one((int)p);
		itoa_one((int)p - 1);
		itoa_one((int)p + 1);
		itoa_one(-(int)p);
		itoa_one(-(int)p - 1);
		itoa_one(-(int)p + 1);
	}
	seed = 12345;
	for (i = 0; i < 20000; i++)
	{
		seed = seed * 1103515245u + 12345u;
		itoa_one((int)seed);
	}
	for (i = 1; i < 4; i++)
	{
		setcase("itoa(-42) avec malloc n°%d en échec", i);
		inject_on(i);
		r = ft_itoa(-42);
		if (inject_off())
			check(r == NULL, "itoa doit renvoyer NULL quand malloc échoue");
		free(r);
	}
}
#endif

#if defined(T_strmapi) || defined(T_striteri)
static unsigned int	g_calls;
static unsigned int	g_order_ok;

static void	order_reset(void)
{
	g_calls = 0;
	g_order_ok = 1;
}

static void	order_note(unsigned int i)
{
	if (i != g_calls)
		g_order_ok = 0;
	g_calls++;
}
#endif

#ifdef T_strmapi
static char	f_add(unsigned int i, char c)
{
	order_note(i);
	return ((char)(c + i));
}

static char	f_upper(unsigned int i, char c)
{
	order_note(i);
	return ((char)toupper((unsigned char)c));
}

static char	f_zero_at_1(unsigned int i, char c)
{
	order_note(i);
	return (i == 1 ? '\0' : c);
}

static void	run_test(void)
{
	char	*s;
	char	*r;
	char	*big;
	size_t	i;

	setcase("strmapi(\"abc\", c + i)");
	s = xdup("abc");
	order_reset();
	r = ft_strmapi(s, f_add);
	check(r != NULL && strcmp(r, "ace") == 0, "strmapi(\"abc\", c+i) = %s",
		r ? show(r, strlen(r)) : "NULL");
	check(g_calls == 3 && g_order_ok, "f doit être appelée 3 fois avec i=0,1,2"
		" dans l'ordre (appelée %u fois)", g_calls);
	check(strcmp(s, "abc") == 0, "strmapi a modifié la chaîne source");
	if (r)
		size_is(r, 4, "strmapi");
	free(r);
	free(s);
	setcase("strmapi(\"\", f)");
	s = xdup("");
	order_reset();
	r = ft_strmapi(s, f_add);
	check(r != NULL && *r == '\0' && g_calls == 0,
		"strmapi sur \"\" doit renvoyer \"\" sans appeler f");
	if (r)
		size_is(r, 1, "strmapi");
	free(r);
	free(s);
	setcase("strmapi(\"Hello, World 42\\xff\", toupper)");
	s = xdup("Hello, World 42\xff");
	order_reset();
	r = ft_strmapi(s, f_upper);
	check(r != NULL && strcmp(r, "HELLO, WORLD 42\xff") == 0,
		"strmapi(toupper) = %s", r ? show(r, strlen(r)) : "NULL");
	free(r);
	free(s);
	setcase("strmapi(\"abcd\", f qui renvoie '\\0' en 1)");
	s = xdup("abcd");
	order_reset();
	r = ft_strmapi(s, f_zero_at_1);
	check(r != NULL && memcmp(r, "a\0cd\0", 5) == 0 && g_calls == 4,
		"strmapi doit appeler f pour chaque indice même si f renvoie '\\0'"
		" (appels=%u)", g_calls);
	free(r);
	free(s);
	setcase("strmapi sur 5000 caractères");
	big = xalloc(5001);
	memset(big, 'a', 5000);
	big[5000] = '\0';
	order_reset();
	r = ft_strmapi(big, f_add);
	for (i = 0; r && i < 5000; i++)
		if (r[i] != (char)('a' + (unsigned int)i))
			break ;
	check(r && i == 5000 && r[5000] == '\0' && g_calls == 5000 && g_order_ok,
		"strmapi sur 5000 caractères (raté à l'indice %zu)", i);
	free(r);
	free(big);
	setcase("strmapi(\"fail me\") avec malloc en échec");
	s = xdup("fail me");
	inject_on(1);
	r = ft_strmapi(s, f_upper);
	if (inject_off())
		check(r == NULL, "strmapi doit renvoyer NULL quand malloc échoue");
	free(r);
	free(s);
}
#endif

#ifdef T_striteri
static char	*g_base;
static int	g_ptr_ok;

static void	f_add(unsigned int i, char *c)
{
	order_note(i);
	if (c != g_base + i)
		g_ptr_ok = 0;
	*c = (char)(*c + i);
}

static void	f_upper(unsigned int i, char *c)
{
	order_note(i);
	*c = (char)toupper((unsigned char)*c);
}

static void	run_test(void)
{
	char	*s;
	char	*big;
	size_t	i;

	setcase("striteri(\"abc\", c + i)");
	s = xdup("abc");
	g_base = s;
	g_ptr_ok = 1;
	order_reset();
	ft_striteri(s, f_add);
	check(strcmp(s, "ace") == 0, "striteri(\"abc\", c+i) = \"%s\"",
		show(s, strlen(s)));
	check(g_calls == 3 && g_order_ok, "f doit être appelée 3 fois avec i=0,1,2"
		" dans l'ordre (appelée %u fois)", g_calls);
	check(g_ptr_ok, "f doit recevoir l'ADRESSE de s[i], pas une copie");
	free(s);
	setcase("striteri(\"\", f)");
	s = xdup("");
	order_reset();
	ft_striteri(s, f_upper);
	check(g_calls == 0, "striteri sur \"\" ne doit pas appeler f (appelée %u"
		" fois)", g_calls);
	free(s);
	setcase("striteri(\"Hello, World 42\", toupper)");
	s = xdup("Hello, World 42");
	ft_striteri(s, f_upper);
	check(strcmp(s, "HELLO, WORLD 42") == 0, "striteri(toupper) = \"%s\"",
		show(s, strlen(s)));
	free(s);
	setcase("striteri(\"\\xe9t\\xe9 \\xff\", toupper)");
	s = xdup("\xe9t\xe9 \xff");
	order_reset();
	ft_striteri(s, f_upper);
	check(g_calls == 5 && strcmp(s, "\xe9T\xe9 \xff") == 0,
		"striteri doit aller jusqu'au '\\0' même après un octet >= 0x80"
		" (appelée %u fois sur 5)", g_calls);
	free(s);
	setcase("striteri sur 5000 caractères");
	big = xalloc(5001);
	memset(big, 'a', 5000);
	big[5000] = '\0';
	g_base = big;
	g_ptr_ok = 1;
	order_reset();
	ft_striteri(big, f_add);
	for (i = 0; i < 5000; i++)
		if (big[i] != (char)('a' + (unsigned int)i))
			break ;
	check(i == 5000 && g_calls == 5000 && g_order_ok && g_ptr_ok,
		"striteri sur 5000 caractères (raté à l'indice %zu)", i);
	free(big);
}
#endif

#if defined(T_putchar_fd) || defined(T_putstr_fd) || defined(T_putendl_fd) \
	|| defined(T_putnbr_fd)
static char	g_cap_path[64];

static int	cap_start(void)
{
	const char	*d;
	int			fd;

	d = getenv("TMPDIR");
	snprintf(g_cap_path, sizeof(g_cap_path), "%s/tdf_cap_XXXXXX",
		d && *d && strlen(d) < 40 ? d : "/tmp");
	fd = mkstemp(g_cap_path);
	return (fd);
}

static size_t	cap_end(int fd, char *buf, size_t max)
{
	ssize_t	r;

	lseek(fd, 0, SEEK_SET);
	r = read(fd, buf, max);
	close(fd);
	unlink(g_cap_path);
	return (r < 0 ? 0 : (size_t)r);
}
#endif

#ifdef T_putchar_fd
static void	run_test(void)
{
	static char	buf[512];
	size_t		got;
	int			fd;
	int			c;

	setcase("putchar_fd('a', fd)");
	fd = cap_start();
	ft_putchar_fd('a', fd);
	got = cap_end(fd, buf, sizeof(buf));
	check(got == 1 && buf[0] == 'a', "putchar_fd('a') : %zu octets écrits", got);
	setcase("putchar_fd('\\n', fd)");
	fd = cap_start();
	ft_putchar_fd('\n', fd);
	got = cap_end(fd, buf, sizeof(buf));
	check(got == 1 && buf[0] == '\n', "putchar_fd('\\n') : %zu octets écrits",
		got);
	setcase("putchar_fd('\\0', fd)");
	fd = cap_start();
	ft_putchar_fd('\0', fd);
	got = cap_end(fd, buf, sizeof(buf));
	check(got == 1 && buf[0] == '\0', "putchar_fd('\\0') doit écrire 1 octet"
		" (%zu écrits)", got);
	setcase("putchar_fd des 256 valeurs d'octet");
	fd = cap_start();
	for (c = 0; c < 256; c++)
		ft_putchar_fd((char)c, fd);
	got = cap_end(fd, buf, sizeof(buf));
	for (c = 0; c < 256 && got == 256; c++)
		if ((unsigned char)buf[c] != c)
			break ;
	check(got == 256 && c == 256, "putchar_fd des 256 valeurs d'octet (%zu"
		" octets reçus, premier faux : %d)", got, c);
	setcase("putchar_fd('x', -1)");
	ft_putchar_fd('x', -1);
	check(1, "survit à un fd invalide");
}
#endif

#ifdef T_putstr_fd
static void	run_test(void)
{
	static const char	*tab[] = {"", "a", "hello", "hello\n", "tab\there",
		"\xff\x80 high", "ab\0cd"};
	static char			buf[200100];
	char				*s;
	char				*big;
	size_t				got;
	int					fd;
	int					i;

	for (i = 0; i < COUNT(tab); i++)
	{
		s = xdup(tab[i]);
		setcase("putstr_fd(\"%s\", fd)", S(s));
		fd = cap_start();
		ft_putstr_fd(s, fd);
		got = cap_end(fd, buf, sizeof(buf));
		check(got == strlen(s) && memcmp(buf, s, got) == 0,
			"putstr_fd(\"%s\") : %zu octets écrits, attendu %zu", S(s), got,
			strlen(s));
		free(s);
	}
	setcase("putstr_fd de 150000 caractères");
	big = xalloc(150001);
	memset(big, 'k', 150000);
	big[150000] = '\0';
	fd = cap_start();
	ft_putstr_fd(big, fd);
	got = cap_end(fd, buf, sizeof(buf));
	check(got == 150000 && memcmp(buf, big, got) == 0,
		"putstr_fd de 150000 caractères : %zu octets écrits", got);
	free(big);
	setcase("putstr_fd(\"x\", -1)");
	ft_putstr_fd("x", -1);
}
#endif

#ifdef T_putendl_fd
static void	run_test(void)
{
	static const char	*tab[] = {"", "a", "hello", "hello\n", "\xff\x80 high"};
	static char			buf[256];
	char				exp[64];
	char				*s;
	size_t				got;
	int					fd;
	int					i;

	for (i = 0; i < COUNT(tab); i++)
	{
		s = xdup(tab[i]);
		setcase("putendl_fd(\"%s\", fd)", S(s));
		fd = cap_start();
		ft_putendl_fd(s, fd);
		got = cap_end(fd, buf, sizeof(buf));
		snprintf(exp, sizeof(exp), "%s\n", tab[i]);
		check(got == strlen(exp) && memcmp(buf, exp, got) == 0,
			"putendl_fd(\"%s\") : reçu \"%s\" (%zu octets), attendu \"%s\"",
			S(s), show(buf, got), got, show(exp, strlen(exp)));
		free(s);
	}
	setcase("putendl_fd(\"x\", -1)");
	ft_putendl_fd("x", -1);
}
#endif

#ifdef T_putnbr_fd
static void	putnbr_one(int n)
{
	static char	buf[64];
	char		exp[32];
	size_t		got;
	int			fd;

	setcase("putnbr_fd(%d, fd)", n);
	fd = cap_start();
	ft_putnbr_fd(n, fd);
	got = cap_end(fd, buf, sizeof(buf));
	snprintf(exp, sizeof(exp), "%d", n);
	check(got == strlen(exp) && memcmp(buf, exp, got) == 0,
		"putnbr_fd(%d) : reçu \"%s\", attendu \"%s\"", n, show(buf, got), exp);
}

static void	run_test(void)
{
	unsigned int	seed;
	int				n;
	int				i;

	for (n = -1100; n <= 1100; n++)
		putnbr_one(n);
	putnbr_one(INT_MIN);
	putnbr_one(INT_MIN + 1);
	putnbr_one(INT_MAX);
	putnbr_one(INT_MAX - 1);
	putnbr_one(1000000000);
	putnbr_one(-1000000000);
	seed = 777;
	for (i = 0; i < 2000; i++)
	{
		seed = seed * 1103515245u + 12345u;
		putnbr_one((int)seed);
	}
	setcase("putnbr_fd(42, -1)");
	ft_putnbr_fd(42, -1);
}
#endif

/* ------------------------------------------------------------------ */
/* Partie 3                                                             */
/* ------------------------------------------------------------------ */

#if defined(T_lstnew) || defined(T_lstadd_front) || defined(T_lstsize) \
	|| defined(T_lstlast) || defined(T_lstadd_back) || defined(T_lstdelone) \
	|| defined(T_lstclear) || defined(T_lstiter) || defined(T_lstmap)
static int	g_del_count;
static int	g_del_order[64];
static int	g_live;

static int	*mkint(int v)
{
	int	*p;

	p = tst_malloc(sizeof(int));
	*p = v;
	g_live++;
	return (p);
}

static void	del_int(void *p)
{
	if (g_del_count < 64)
		g_del_order[g_del_count] = *(int *)p;
	g_del_count++;
	g_live--;
	free(p);
}

static void	del_noop(void *p)
{
	(void)p;
	g_del_count++;
}

static t_list	*mknode(void *content)
{
	t_list	*n;

	n = tst_malloc(sizeof(t_list));
	n->content = content;
	n->next = NULL;
	return (n);
}

static t_list	*mklist(int n)
{
	t_list	*head;
	t_list	*cur;
	int		i;

	head = NULL;
	cur = NULL;
	for (i = 1; i <= n; i++)
	{
		if (!head)
			head = cur = mknode(mkint(i));
		else
		{
			cur->next = mknode(mkint(i));
			cur = cur->next;
		}
	}
	return (head);
}

static void	freelist(t_list *l, int free_content)
{
	t_list	*nx;

	while (l)
	{
		nx = l->next;
		if (free_content)
		{
			free(l->content);
			g_live--;
		}
		free(l);
		l = nx;
	}
}

static int	list_is(t_list *l, int n, int mul)
{
	int	i;

	for (i = 1; i <= n; i++)
	{
		if (l == NULL || l->content == NULL || *(int *)l->content != i * mul)
			return (0);
		l = l->next;
	}
	return (l == NULL);
}
#endif

#ifdef T_lstnew
static void	run_test(void)
{
	int		x;
	t_list	*n;
	int		i;

	setcase("lstnew(&x)");
	x = 42;
	n = ft_lstnew(&x);
	check(n != NULL, "lstnew renvoie NULL");
	if (n)
	{
		check(n->content == &x, "lstnew : content doit être le pointeur donné"
			" (pas une copie)");
		check(n->next == NULL, "lstnew : next doit être NULL");
		size_is(n, sizeof(t_list), "lstnew");
		free(n);
	}
	setcase("lstnew(NULL)");
	n = ft_lstnew(NULL);
	check(n != NULL && n->content == NULL && n->next == NULL,
		"lstnew(NULL) doit donner un maillon valide avec content NULL");
	free(n);
	setcase("lstnew((void *)42)");
	n = ft_lstnew((void *)42);
	check(n != NULL && n->content == (void *)42 && n->next == NULL,
		"lstnew((void *)42) : content doit valoir 42 tel quel");
	free(n);
	for (i = 1; i < 3; i++)
	{
		setcase("lstnew(&x) avec malloc n°%d en échec", i);
		inject_on(i);
		n = ft_lstnew(&x);
		if (inject_off())
			check(n == NULL, "lstnew doit renvoyer NULL quand malloc échoue");
		free(n);
	}
}
#endif

#ifdef T_lstadd_front
static void	run_test(void)
{
	t_list	*l;
	t_list	*a;
	t_list	*b;
	t_list	*c;

	setcase("lstadd_front sur une liste vide");
	l = NULL;
	a = mknode(NULL);
	ft_lstadd_front(&l, a);
	check(l == a && a->next == NULL, "lstadd_front sur une liste vide"
		" (*lst == NULL)");
	setcase("lstadd_front sur une liste d'un maillon");
	b = mknode(NULL);
	ft_lstadd_front(&l, b);
	check(l == b && b->next == a && a->next == NULL,
		"lstadd_front : la nouvelle tête doit pointer vers l'ancienne");
	setcase("lstadd_front sur une liste de deux maillons");
	c = mknode(NULL);
	ft_lstadd_front(&l, c);
	check(l == c && c->next == b && b->next == a && a->next == NULL,
		"lstadd_front : chaîne de trois");
	free(a);
	free(b);
	free(c);
}
#endif

#ifdef T_lstsize
static void	run_test(void)
{
	t_list	*l;
	int		n;

	setcase("lstsize(NULL)");
	check((long)ft_lstsize(NULL) == 0, "lstsize(NULL) doit valoir 0 (reçu %ld)",
		(long)ft_lstsize(NULL));
	for (n = 1; n <= 50; n++)
	{
		setcase("lstsize d'une liste de %d maillons", n);
		l = mklist(n);
		check((long)ft_lstsize(l) == n, "lstsize d'une liste de %d maillons = %ld",
			n, (long)ft_lstsize(l));
		freelist(l, 1);
	}
	setcase("lstsize d'une liste de 10000 maillons");
	l = mklist(10000);
	check((long)ft_lstsize(l) == 10000, "lstsize de 10000 maillons = %ld",
		(long)ft_lstsize(l));
	freelist(l, 1);
}
#endif

#ifdef T_lstlast
static void	run_test(void)
{
	t_list	*l;
	t_list	*t;
	int		n;

	setcase("lstlast(NULL)");
	check(ft_lstlast(NULL) == NULL, "lstlast(NULL) doit valoir NULL");
	for (n = 1; n <= 20; n++)
	{
		setcase("lstlast d'une liste de %d maillons", n);
		l = mklist(n);
		for (t = l; t->next; t = t->next)
			;
		check(ft_lstlast(l) == t, "lstlast de %d maillons : mauvais maillon", n);
		freelist(l, 1);
	}
}
#endif

#ifdef T_lstadd_back
static void	run_test(void)
{
	t_list	*l;
	t_list	*n;
	int		i;

	setcase("lstadd_back sur une liste vide");
	l = NULL;
	n = mknode(mkint(1));
	ft_lstadd_back(&l, n);
	check(l == n && n->next == NULL, "lstadd_back sur une liste vide"
		" (*lst == NULL)");
	for (i = 2; i <= 30; i++)
	{
		setcase("lstadd_back du maillon %d", i);
		ft_lstadd_back(&l, mknode(mkint(i)));
	}
	check(list_is(l, 30, 1), "lstadd_back x30 : l'ordre doit être 1..30,"
		" terminé par NULL");
	freelist(l, 1);
}
#endif

#ifdef T_lstdelone
static void	run_test(void)
{
	t_list	*l;
	t_list	*second;

	g_stu_may_free = 1;
	setcase("lstdelone sur le 1er maillon d'une liste de 2");
	l = mklist(2);
	second = l->next;
	g_del_count = 0;
	ft_lstdelone(l, del_int);
	check(g_del_count == 1 && g_del_order[0] == 1,
		"lstdelone : del doit être appelée une fois sur content (appelée %d fois)",
		g_del_count);
	check(!tst_alive(l), "lstdelone doit libérer le maillon avec free (fuite"
		" de 16 octets par appel)");
	if (tst_alive(l))
		free(l);
	check(second->next == NULL && *(int *)second->content == 2,
		"lstdelone ne doit pas toucher aux maillons suivants");
	freelist(second, 1);
	setcase("lstdelone sur un maillon au content NULL");
	l = mknode(NULL);
	g_del_count = 0;
	ft_lstdelone(l, del_noop);
	check(g_del_count == 1, "lstdelone avec un content NULL : del appelée %d"
		" fois", g_del_count);
	if (tst_alive(l))
		free(l);
}
#endif

#ifdef T_lstclear
static void	run_test(void)
{
	t_list	*l;
	int		i;

	t_list	*nodes[10];
	t_list	*t;
	int		alive;

	g_stu_may_free = 1;
	setcase("lstclear d'une liste de 10 maillons");
	l = mklist(10);
	for (t = l, i = 0; t && i < 10; t = t->next, i++)
		nodes[i] = t;
	g_del_count = 0;
	ft_lstclear(&l, del_int);
	for (alive = 0, i = 0; i < 10; i++)
		alive += tst_alive(nodes[i]);
	check(alive == 0, "lstclear : %d maillon(s) sur 10 jamais libérés avec"
		" free", alive);
	for (i = 0; i < 10; i++)
		if (tst_alive(nodes[i]))
			free(nodes[i]);
	check(l == NULL, "lstclear : *lst doit valoir NULL après");
	check(g_del_count == 10, "lstclear : del appelée %d fois, attendu 10",
		g_del_count);
	for (i = 0; i < 10 && i < g_del_count; i++)
		if (g_del_order[i] < 1 || g_del_order[i] > 10)
			break ;
	check(i == 10 || g_del_count != 10, "lstclear : del a reçu autre chose"
		" que les content de la liste");
	setcase("lstclear d'un seul maillon");
	l = mklist(1);
	g_del_count = 0;
	ft_lstclear(&l, del_int);
	check(l == NULL && g_del_count == 1, "lstclear sur un seul maillon");
	setcase("lstclear d'une liste vide");
	l = NULL;
	g_del_count = 0;
	ft_lstclear(&l, del_int);
	check(l == NULL && g_del_count == 0, "lstclear sur une liste vide");
	setcase("lstclear d'une liste de 10000 maillons");
	l = mklist(10000);
	g_del_count = 0;
	ft_lstclear(&l, del_int);
	check(l == NULL && g_del_count == 10000,
		"lstclear de 10000 maillons : del appelée %d fois", g_del_count);
}
#endif

#ifdef T_lstiter
static int	g_iter_n;

static void	f_inc(void *p)
{
	g_iter_n++;
	(*(int *)p)++;
}

static void	run_test(void)
{
	t_list	*l;
	t_list	*e;
	t_list	*t;
	int		i;

	setcase("lstiter(liste de 8, +1)");
	l = mklist(8);
	g_iter_n = 0;
	ft_lstiter(l, f_inc);
	check(g_iter_n == 8, "lstiter : f appelée %d fois, attendu 8", g_iter_n);
	for (t = l, i = 2; t && *(int *)t->content == i; t = t->next, i++)
		;
	check(t == NULL && i == 10, "lstiter : f doit recevoir node->content"
		" (chaque valeur +1)");
	freelist(l, 1);
	setcase("lstiter(NULL, f)");
	e = NULL;
	g_iter_n = 0;
	ft_lstiter(e, f_inc);
	check(g_iter_n == 0, "lstiter sur NULL ne doit rien faire");
}
#endif

#ifdef T_lstmap
static void	*f_double(void *p)
{
	return (mkint(*(int *)p * 2));
}

static void	run_test(void)
{
	t_list	*l;
	t_list	*m;
	t_list	*a;
	t_list	*b;
	int		k;
	int		base;

	g_stu_may_free = 1;
	setcase("lstmap(liste 1..7, x2)");
	l = mklist(7);
	m = ft_lstmap(l, f_double, del_int);
	check(list_is(m, 7, 2), "lstmap : la nouvelle liste doit contenir 2,4,..,14"
		" dans l'ordre");
	check(list_is(l, 7, 1), "lstmap ne doit pas modifier la liste d'origine");
	for (a = l, b = m; a && b; a = a->next, b = b->next)
		if (a == b || a->content == b->content)
			break ;
	check(a == NULL && b == NULL, "lstmap : les maillons doivent être neufs");
	freelist(m, 1);
	freelist(l, 1);
	setcase("lstmap(NULL, f, del)");
	check(ft_lstmap(NULL, f_double, del_int) == NULL, "lstmap(NULL) doit valoir"
		" NULL");
	l = mklist(5);
	base = g_live;
	for (k = 1; k <= 6; k++)
	{
		setcase("lstmap(liste de 5) avec malloc n°%d en échec", k);
		g_del_count = 0;
		inject_on(k);
		m = ft_lstmap(l, f_double, del_int);
		if (inject_off())
		{
			check(m == NULL, "lstmap doit renvoyer NULL quand malloc n°%d échoue",
				k);
			check(g_live == base,
				"lstmap, malloc n°%d en échec : %d content(s) créés par f jamais"
				" passés à del", k, g_live - base);
			g_live = base;
		}
		else
		{
			freelist(m, 1);
			break ;
		}
	}
	check(list_is(l, 5, 1), "lstmap en échec a abîmé la liste d'origine");
	freelist(l, 1);
}
#endif

int	main(void)
{
	struct timeval	a;
	struct timeval	b;

	setvbuf(stdout, NULL, _IONBF, 0);
	guard_init();
	gettimeofday(&a, NULL);
	run_test();
	gettimeofday(&b, NULL);
	alarm(0);
	setcase("fin du test");
	leak_report();
	printf("@@ %d %d %d %ld\n", g_checks, g_fails, g_warns,
		(b.tv_sec - a.tv_sec) * 1000L + (b.tv_usec - a.tv_usec) / 1000L);
	return (g_fails != 0);
}
