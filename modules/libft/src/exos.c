#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include "libft.h"

#define MAX_SHOWN 6

static int	g_checks;
static int	g_fails;

static void	check(int ok, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));

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

static int	g_inj_on;
static int	g_inj_cnt;
static int	g_inj_at;

void	*tdf_malloc_s(size_t n)
{
	if (g_inj_on && ++g_inj_cnt == g_inj_at)
		return (NULL);
	return (malloc(n));
}

void	tdf_free_s(void *p)
{
	free(p);
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

static char	*xdup(const char *s)
{
	size_t	n;
	char	*p;

	n = strlen(s) + 1;
	p = malloc(n);
	memcpy(p, s, n);
	return (p);
}

static const char	*show(const char *s)
{
	static char	buf[4][120];
	static int	w;
	char		*o;

	o = buf[w++ & 3];
	if (!s)
		return ("(null)");
	snprintf(o, 120, "\"%.100s\"", s);
	return (o);
}

#if defined(X_15) || defined(X_16) || defined(X_17) || defined(X_18) \
	|| defined(X_19) || defined(X_20) || defined(X_21)
static int	g_live;
static int	g_del;

static int	*mkint(int v)
{
	int	*p;

	p = malloc(sizeof(int));
	*p = v;
	g_live++;
	return (p);
}

static void	del_int(void *p)
{
	g_del++;
	g_live--;
	free(p);
}

static t_list	*mklist(const int *v, int n)
{
	t_list	*head;
	t_list	*cur;
	t_list	*node;
	int		i;

	head = NULL;
	cur = NULL;
	for (i = 0; i < n; i++)
	{
		node = malloc(sizeof(t_list));
		node->content = mkint(v[i]);
		node->next = NULL;
		if (cur)
			cur->next = node;
		else
			head = node;
		cur = node;
	}
	return (head);
}

static int	list_eq(t_list *l, const int *v, int n)
{
	int	i;

	for (i = 0; i < n; i++)
	{
		if (!l || !l->content || *(int *)l->content != v[i])
			return (0);
		l = l->next;
	}
	return (l == NULL);
}

static void	list_str(t_list *l, char *out, size_t sz)
{
	size_t	k;
	int		guard;

	k = snprintf(out, sz, "[");
	guard = 0;
	while (l && k < sz - 16 && guard++ < 40)
	{
		k += snprintf(out + k, sz - k, "%d%s", l->content ? *(int *)l->content : -999,
				l->next ? "," : "");
		l = l->next;
	}
	snprintf(out + k, sz - k, "%s]", l ? "..." : "");
}

static void	freelist(t_list *l)
{
	t_list	*nx;
	int		guard;

	guard = 0;
	while (l && guard++ < 100000)
	{
		nx = l->next;
		free(l->content);
		g_live--;
		free(l);
		l = nx;
	}
}

static int	cmp_int(const void *a, const void *b)
{
	return (*(const int *)a - *(const int *)b);
}
#endif

#ifdef X_01
char	*ft_strrev(char *s);

static void	run_test(void)
{
	const char	*in[] = {"", "a", "ab", "abc", "Hello, World!", "racecar", "12345678"};
	char		*s;
	char		want[64];
	size_t		i;
	size_t		n;
	size_t		k;

	for (k = 0; k < sizeof(in) / sizeof(in[0]); k++)
	{
		s = xdup(in[k]);
		n = strlen(in[k]);
		for (i = 0; i < n; i++)
			want[i] = in[k][n - 1 - i];
		want[n] = '\0';
		check(ft_strrev(s) == s, "ft_strrev(%s) doit renvoyer s lui-même", show(in[k]));
		check(strcmp(s, want) == 0, "ft_strrev(%s) = %s, attendu %s", show(in[k]), show(s), show(want));
		free(s);
	}
}
#endif

#ifdef X_02
size_t	ft_strspn(const char *s, const char *accept);

static void	run_test(void)
{
	const char	*t[][2] = {{"", "abc"}, {"abc", ""}, {"aaab", "a"}, {"hello", "leh"},
		{"   x", " "}, {"123abc", "0123456789"}, {"zzz", "z"}, {"\x80\x81z", "\x81\x80"},
		{"abc", "xyz"}};
	size_t		k;

	for (k = 0; k < sizeof(t) / sizeof(t[0]); k++)
		check(ft_strspn(t[k][0], t[k][1]) == strspn(t[k][0], t[k][1]),
			"ft_strspn(%s, %s) = %zu, attendu %zu", show(t[k][0]), show(t[k][1]),
			ft_strspn(t[k][0], t[k][1]), strspn(t[k][0], t[k][1]));
}
#endif

#ifdef X_03
void	*ft_memrchr(const void *s, int c, size_t n);

static void	run_test(void)
{
	const char	buf[] = "abcabc\0xa\x80z";
	size_t		n;
	int			c;
	const char	*want;
	const char	*got;
	char		*small;

	for (n = 0; n <= sizeof(buf); n++)
	{
		for (c = -1; c < 260; c += 1)
		{
			want = NULL;
			for (size_t i = n; i > 0; i--)
				if ((unsigned char)buf[i - 1] == (unsigned char)c)
				{
					want = buf + i - 1;
					break ;
				}
			got = ft_memrchr(buf, c, n);
			if (got != want)
			{
				check(0, "ft_memrchr(buf, %d, %zu) = buf+%ld, attendu %s%ld", c, n,
					got ? (long)(got - buf) : -1L, want ? "buf+" : "NULL ", want ? (long)(want - buf) : 0L);
				return ;
			}
		}
	}
	check(1, "ok");
	small = malloc(1);
	small[0] = 'q';
	check(ft_memrchr(small, 'q', 0) == NULL, "ft_memrchr(s, c, 0) doit renvoyer NULL sans rien lire");
	free(small);
}
#endif

#ifdef X_04
int	ft_strcasecmp(const char *s1, const char *s2);

static int	sgn(int x)
{
	return ((x > 0) - (x < 0));
}

static void	run_test(void)
{
	const char	*t[][2] = {{"", ""}, {"abc", "ABC"}, {"Hello", "hELLO"}, {"abc", "abd"},
		{"abd", "ABC"}, {"a", ""}, {"", "a"}, {"Zebra", "apple"}, {"[", "a"},
		{"\x80", "a"}, {"a\x80", "A\x80"}, {"same", "same"}};
	size_t		k;

	for (k = 0; k < sizeof(t) / sizeof(t[0]); k++)
		check(sgn(ft_strcasecmp(t[k][0], t[k][1])) == sgn(strcasecmp(t[k][0], t[k][1])),
			"ft_strcasecmp(%s, %s) = %d, le signe attendu est %d", show(t[k][0]), show(t[k][1]),
			ft_strcasecmp(t[k][0], t[k][1]), sgn(strcasecmp(t[k][0], t[k][1])));
}
#endif

#ifdef X_05
void	ft_memswap(void *a, void *b, size_t n);

static void	run_test(void)
{
	unsigned char	a[40];
	unsigned char	b[40];
	unsigned char	a0[40];
	unsigned char	b0[40];
	size_t			n;
	size_t			i;
	int				x;
	int				y;
	double			d;
	double			e;

	for (n = 0; n <= 32; n++)
	{
		for (i = 0; i < 40; i++)
		{
			a[i] = (unsigned char)(i * 7 + 1);
			b[i] = (unsigned char)(255 - i * 3);
		}
		memcpy(a0, a, 40);
		memcpy(b0, b, 40);
		ft_memswap(a, b, n);
		check(memcmp(a, b0, n) == 0 && memcmp(b, a0, n) == 0,
			"ft_memswap(a, b, %zu) : les %zu premiers octets doivent être échangés", n, n);
		check(memcmp(a + n, a0 + n, 40 - n) == 0 && memcmp(b + n, b0 + n, 40 - n) == 0,
			"ft_memswap(a, b, %zu) a touché des octets après le %zu-ième", n, n);
	}
	x = 42;
	y = -7;
	ft_memswap(&x, &y, sizeof(int));
	check(x == -7 && y == 42, "échanger deux int : x = %d, y = %d", x, y);
	d = 3.5;
	e = -0.25;
	ft_memswap(&d, &e, sizeof(double));
	check(d == -0.25 && e == 3.5, "échanger deux double : %g et %g", d, e);
}
#endif

#ifdef X_06
int	ft_bitcount(unsigned int n);

static void	run_test(void)
{
	unsigned int	v[] = {0, 1, 2, 3, 7, 8, 255, 256, 1023, 0x80000000u, 0xFFFFFFFFu,
		0xAAAAAAAAu, 12345678u, 4294967294u};
	size_t			k;

	for (k = 0; k < sizeof(v) / sizeof(v[0]); k++)
		check(ft_bitcount(v[k]) == __builtin_popcount(v[k]), "ft_bitcount(%u) = %d, attendu %d",
			v[k], ft_bitcount(v[k]), __builtin_popcount(v[k]));
}
#endif

#ifdef X_07
int	ft_atoi_base(const char *str, const char *base);

static void	run_test(void)
{
	struct s_t { const char *s; const char *b; int want; } t[] = {
		{"42", "0123456789", 42}, {"-42", "0123456789", -42}, {"  \t\n+101", "01", 5},
		{"ff", "0123456789abcdef", 255}, {"-7f", "0123456789abcdef", -127},
		{"poneyvif", "poneyvif", 342391}, {"zz", "0123456789", 0}, {"12xyz", "0123456789", 12},
		{"10", "0", 0}, {"10", "", 0}, {"10", "011", 0}, {"10", "01+", 0}, {"10", "0 1", 0},
		{"10", "-01", 0}, {"2147483647", "0123456789", 2147483647},
		{"-2147483648", "0123456789", -2147483648}, {"", "01", 0}, {"--1", "01", 0},
		{"0", "0123456789abcdef", 0}, {"  ba", "ab", 2}};
	size_t	k;
	int		got;

	for (k = 0; k < sizeof(t) / sizeof(t[0]); k++)
	{
		got = ft_atoi_base(t[k].s, t[k].b);
		check(got == t[k].want, "ft_atoi_base(%s, %s) = %d, attendu %d", show(t[k].s),
			show(t[k].b), got, t[k].want);
	}
}
#endif

#ifdef X_08
char	*ft_strndup(const char *s, size_t n);

static void	run_test(void)
{
	const char	*s = "bonjour";
	size_t		n;
	char		*r;
	int			k;

	for (n = 0; n <= 10; n++)
	{
		r = ft_strndup(s, n);
		check(r && strlen(r) == (n < 7 ? n : 7) && strncmp(r, s, n) == 0,
			"ft_strndup(\"bonjour\", %zu) = %s", n, show(r));
		check(r != s, "ft_strndup doit renvoyer une nouvelle zone");
		if (r)
			r[n < 7 ? n : 7] = '\0';
		free(r);
	}
	r = ft_strndup("", 5);
	check(r && r[0] == '\0', "ft_strndup(\"\", 5) = %s", show(r));
	free(r);
	for (k = 1; k <= 2; k++)
	{
		inject_on(k);
		r = ft_strndup("abc", 2);
		if (inject_off())
			check(r == NULL, "si malloc échoue, ft_strndup doit renvoyer NULL");
		free(r);
	}
}
#endif

#ifdef X_09
char	*ft_itoa_base(long n, const char *base);

static void	run_test(void)
{
	struct s_t { long n; const char *b; const char *want; } t[] = {
		{0, "0123456789", "0"}, {42, "0123456789", "42"}, {-42, "0123456789", "-42"},
		{255, "0123456789abcdef", "ff"}, {-255, "0123456789ABCDEF", "-FF"}, {5, "01", "101"},
		{LONG_MAX, "0123456789", "9223372036854775807"},
		{LONG_MIN, "0123456789", "-9223372036854775808"},
		{342391, "poneyvif", "oneyvif"}, {7, "0", NULL}, {7, "", NULL}, {7, "aa", NULL},
		{7, "a-b", NULL}, {7, "a+b", NULL}};
	size_t	k;
	char	*r;
	int		i;

	for (k = 0; k < sizeof(t) / sizeof(t[0]); k++)
	{
		r = ft_itoa_base(t[k].n, t[k].b);
		if (t[k].want)
			check(r && strcmp(r, t[k].want) == 0, "ft_itoa_base(%ld, %s) = %s, attendu %s",
				t[k].n, show(t[k].b), show(r), show(t[k].want));
		else
			check(r == NULL, "ft_itoa_base(%ld, %s) : base invalide, attendu NULL", t[k].n,
				show(t[k].b));
		free(r);
	}
	{
		char	bin[70];

		bin[0] = '-';
		bin[1] = '1';
		memset(bin + 2, '0', 63);
		bin[65] = '\0';
		r = ft_itoa_base(LONG_MIN, "01");
		check(r && strcmp(r, bin) == 0, "ft_itoa_base(LONG_MIN, \"01\") = %s", show(r));
		free(r);
	}
	for (i = 1; i <= 3; i++)
	{
		inject_on(i);
		r = ft_itoa_base(-1234, "0123456789");
		if (inject_off())
			check(r == NULL, "si malloc n°%d échoue, ft_itoa_base doit renvoyer NULL", i);
		free(r);
	}
}
#endif

#if defined(X_10) || defined(X_11) || defined(X_14)
static void	free_tab(char **t)
{
	size_t	i;

	if (!t)
		return ;
	for (i = 0; t[i]; i++)
		free(t[i]);
	free(t);
}

static int	tab_is(char **t, const char **want)
{
	size_t	i;

	if (!t)
		return (0);
	for (i = 0; want[i]; i++)
		if (!t[i] || strcmp(t[i], want[i]))
			return (0);
	return (t[i] == NULL);
}
#endif

#ifdef X_10
char	**ft_tabdup(char **tab);

static void	run_test(void)
{
	char		*src[] = {"un", "", "trois", "quatre!", NULL};
	char		*empty[] = {NULL};
	const char	*want[] = {"un", "", "trois", "quatre!", NULL};
	char		**r;
	size_t		i;
	int			k;

	r = ft_tabdup(src);
	check(tab_is(r, want), "ft_tabdup : copie différente de l'original");
	if (r)
		for (i = 0; src[i]; i++)
			check(r[i] != src[i], "ft_tabdup : la chaîne %zu doit être une nouvelle copie", i);
	check(r != src, "ft_tabdup doit renvoyer un nouveau tableau");
	free_tab(r);
	r = ft_tabdup(empty);
	check(r && r[0] == NULL, "ft_tabdup d'un tableau vide doit renvoyer {NULL}");
	free_tab(r);
	check(ft_tabdup(NULL) == NULL, "ft_tabdup(NULL) doit renvoyer NULL");
	for (k = 1; k <= 6; k++)
	{
		inject_on(k);
		r = ft_tabdup(src);
		if (inject_off())
			check(r == NULL, "si malloc n°%d échoue, ft_tabdup doit tout libérer et renvoyer NULL", k);
		else
		{
			free_tab(r);
			break ;
		}
		free_tab(r);
	}
}
#endif

#ifdef X_11
char	**ft_split_set(char const *s, char const *set);

static void	run_test(void)
{
	struct s_t { const char *s; const char *set; const char *w[8]; } t[] = {
		{"a,b;c", ",;", {"a", "b", "c", NULL}},
		{",,;hello;;world,,", ",;", {"hello", "world", NULL}},
		{"", ",", {NULL}}, {",;,;", ",;", {NULL}}, {"pas de separateur", "", {"pas de separateur", NULL}},
		{" tab\tet  espaces ", " \t", {"tab", "et", "espaces", NULL}},
		{"x", "x", {NULL}}, {"axbxc", "x", {"a", "b", "c", NULL}}};
	size_t	k;
	char	**r;
	int		i;

	for (k = 0; k < sizeof(t) / sizeof(t[0]); k++)
	{
		r = ft_split_set(t[k].s, t[k].set);
		check(tab_is(r, t[k].w), "ft_split_set(%s, %s) : mauvais découpage", show(t[k].s),
			show(t[k].set));
		free_tab(r);
	}
	for (i = 1; i <= 5; i++)
	{
		inject_on(i);
		r = ft_split_set("aa,bb;cc", ",;");
		if (inject_off())
			check(r == NULL, "si malloc n°%d échoue, ft_split_set doit tout libérer et renvoyer NULL", i);
		else
		{
			free_tab(r);
			break ;
		}
		free_tab(r);
	}
}
#endif

#ifdef X_12
char	*ft_str_replace(const char *s, const char *old, const char *new);

static void	run_test(void)
{
	struct s_t { const char *s; const char *o; const char *n; const char *w; } t[] = {
		{"le chat et le chien", "le", "un", "un chat et un chien"},
		{"aaaa", "aa", "b", "bb"}, {"aaa", "aa", "b", "ba"}, {"abc", "x", "yyy", "abc"},
		{"abc", "abc", "", ""}, {"", "a", "b", ""}, {"xox", "o", "OOO", "xOOOx"},
		{"abc", "", "zz", "abc"}, {"banane", "an", "", "be"}};
	size_t	k;
	char	*r;
	int		i;

	for (k = 0; k < sizeof(t) / sizeof(t[0]); k++)
	{
		r = ft_str_replace(t[k].s, t[k].o, t[k].n);
		check(r && strcmp(r, t[k].w) == 0, "ft_str_replace(%s, %s, %s) = %s, attendu %s",
			show(t[k].s), show(t[k].o), show(t[k].n), show(r), show(t[k].w));
		check(r != t[k].s, "ft_str_replace doit renvoyer une nouvelle chaîne");
		free(r);
	}
	for (i = 1; i <= 2; i++)
	{
		inject_on(i);
		r = ft_str_replace("abcabc", "b", "XX");
		if (inject_off())
			check(r == NULL, "si malloc échoue, ft_str_replace doit renvoyer NULL");
		free(r);
	}
}
#endif

#ifdef X_13
void	ft_putnbr_base_fd(long n, const char *base, int fd);

static void	run_test(void)
{
	struct s_t { long n; const char *b; const char *w; } t[] = {
		{0, "0123456789", "0"}, {-42, "0123456789", "-42"}, {255, "0123456789abcdef", "ff"},
		{5, "01", "101"}, {LONG_MIN, "0123456789", "-9223372036854775808"},
		{LONG_MAX, "0123456789ABCDEF", "7FFFFFFFFFFFFFFF"}, {9, "0", ""}, {9, "aba", ""},
		{9, "0+1", ""}, {342391, "poneyvif", "oneyvif"}};
	size_t	k;
	int		p[2];
	char	buf[200];
	ssize_t	r;

	for (k = 0; k < sizeof(t) / sizeof(t[0]); k++)
	{
		if (pipe(p) != 0)
			return ;
		ft_putnbr_base_fd(t[k].n, t[k].b, p[1]);
		close(p[1]);
		r = read(p[0], buf, sizeof(buf) - 1);
		close(p[0]);
		buf[r < 0 ? 0 : r] = '\0';
		check(strcmp(buf, t[k].w) == 0, "ft_putnbr_base_fd(%ld, %s) a écrit %s, attendu %s",
			t[k].n, show(t[k].b), show(buf), show(t[k].w));
	}
}
#endif

#ifdef X_14
void	ft_sort_strs(char **tab, int (*cmp)(const char *, const char *));

static int	by_len(const char *a, const char *b)
{
	return ((int)strlen(a) - (int)strlen(b));
}

static int	by_alpha(const char *a, const char *b)
{
	return (strcmp(a, b));
}

static void	run_test(void)
{
	char		*t1[] = {"poire", "kiwi", "abricot", "fraise", "banane", NULL};
	const char	*w1[] = {"abricot", "banane", "fraise", "kiwi", "poire", NULL};
	char		*t2[] = {"ccc", "a", "bbbb", "dd", NULL};
	const char	*w2[] = {"a", "dd", "ccc", "bbbb", NULL};
	char		*t3[] = {NULL};
	char		*t4[] = {"seul", NULL};
	char		*p0;

	p0 = t1[0];
	ft_sort_strs(t1, by_alpha);
	check(tab_is(t1, w1), "ft_sort_strs(ordre alphabétique) : mauvais ordre");
	check(t1[4] == p0, "ft_sort_strs doit échanger les pointeurs, pas recopier les chaînes");
	ft_sort_strs(t2, by_len);
	check(tab_is(t2, w2), "ft_sort_strs(par longueur) : mauvais ordre");
	ft_sort_strs(t3, by_alpha);
	check(t3[0] == NULL, "ft_sort_strs sur un tableau vide");
	ft_sort_strs(t4, by_alpha);
	check(strcmp(t4[0], "seul") == 0, "ft_sort_strs sur un seul élément");
}
#endif

#ifdef X_15
t_list	*ft_lstget(t_list *lst, size_t index);

static void	run_test(void)
{
	int		v[] = {10, 20, 30, 40};
	t_list	*l;
	t_list	*r;
	size_t	i;

	l = mklist(v, 4);
	for (i = 0; i < 4; i++)
	{
		r = ft_lstget(l, i);
		check(r && *(int *)r->content == v[i], "ft_lstget(liste, %zu) doit renvoyer le maillon %d", i, v[i]);
	}
	check(ft_lstget(l, 4) == NULL, "ft_lstget hors de la liste doit renvoyer NULL");
	check(ft_lstget(l, (size_t)-1) == NULL, "ft_lstget(liste, SIZE_MAX) doit renvoyer NULL");
	check(ft_lstget(NULL, 0) == NULL, "ft_lstget(NULL, 0) doit renvoyer NULL");
	freelist(l);
}
#endif

#ifdef X_16
void	ft_lstreverse(t_list **lst);

static void	run_test(void)
{
	int		v[] = {1, 2, 3, 4, 5};
	int		w[] = {5, 4, 3, 2, 1};
	t_list	*l;
	t_list	*first;
	t_list	*last;
	char	s[200];
	int		n;
	int		i;

	for (n = 0; n <= 5; n++)
	{
		l = mklist(v, n);
		first = l;
		ft_lstreverse(&l);
		list_str(l, s, sizeof(s));
		check(list_eq(l, w + 5 - n, n), "ft_lstreverse d'une liste de %d : obtenu %s", n, s);
		if (n > 0 && list_eq(l, w + 5 - n, n))
		{
			last = l;
			for (i = 0; i < n - 1; i++)
				last = last->next;
			check(last == first, "ft_lstreverse doit réutiliser les maillons, pas en créer");
		}
		freelist(l);
	}
	ft_lstreverse(NULL);
	check(1, "ft_lstreverse(NULL) ne plante pas");
}
#endif

#ifdef X_17
t_list	*ft_lstfind(t_list *lst, const void *ref, int (*cmp)(const void *, const void *));

static void	run_test(void)
{
	int		v[] = {4, 8, 15, 16, 23, 42, 15};
	int		ref;
	t_list	*l;
	t_list	*r;

	l = mklist(v, 7);
	ref = 15;
	r = ft_lstfind(l, &ref, cmp_int);
	check(r == l->next->next, "ft_lstfind doit renvoyer le PREMIER maillon égal à 15");
	ref = 4;
	check(ft_lstfind(l, &ref, cmp_int) == l, "ft_lstfind doit trouver le premier maillon");
	ref = 42;
	r = ft_lstfind(l, &ref, cmp_int);
	check(r && *(int *)r->content == 42, "ft_lstfind doit trouver 42");
	ref = 99;
	check(ft_lstfind(l, &ref, cmp_int) == NULL, "ft_lstfind sans résultat doit renvoyer NULL");
	check(ft_lstfind(NULL, &ref, cmp_int) == NULL, "ft_lstfind(NULL, ...) doit renvoyer NULL");
	freelist(l);
}
#endif

#ifdef X_18
void	ft_lstremove_if(t_list **lst, const void *ref,
			int (*cmp)(const void *, const void *), void (*del)(void *));

static void	run_test(void)
{
	struct s_t { int v[8]; int n; int ref; int w[8]; int wn; } t[] = {
		{{1, 2, 3, 2, 4}, 5, 2, {1, 3, 4}, 3}, {{2, 2, 2}, 3, 2, {0}, 0},
		{{2, 1, 2}, 3, 2, {1}, 1}, {{1, 3}, 2, 2, {1, 3}, 2}, {{0}, 0, 2, {0}, 0},
		{{5, 5, 6, 5}, 4, 5, {6}, 1}};
	size_t	k;
	t_list	*l;
	char	s[200];
	int		before;

	for (k = 0; k < sizeof(t) / sizeof(t[0]); k++)
	{
		l = mklist(t[k].v, t[k].n);
		before = g_live;
		g_del = 0;
		ft_lstremove_if(&l, &t[k].ref, cmp_int, del_int);
		list_str(l, s, sizeof(s));
		check(list_eq(l, t[k].w, t[k].wn), "ft_lstremove_if(test %zu, ref %d) : obtenu %s", k + 1,
			t[k].ref, s);
		check(g_del == t[k].n - t[k].wn, "ft_lstremove_if doit appeler del sur chaque maillon retiré (%d appels, attendu %d)",
			g_del, t[k].n - t[k].wn);
		(void)before;
		freelist(l);
	}
}
#endif

#ifdef X_19
void	ft_lstsort(t_list **lst, int (*cmp)(const void *, const void *));

static void	run_test(void)
{
	struct s_t { int v[10]; int n; } t[] = {
		{{3, 1, 2}, 3}, {{5, 4, 3, 2, 1}, 5}, {{1, 2, 3, 4}, 4}, {{7}, 1}, {{0}, 0},
		{{2, 9, 2, -4, 0, 9, 1}, 7}, {{-1, -5, 10, 3, 3, 8, -2, 0, 6, 1}, 10}};
	size_t	k;
	int		w[10];
	t_list	*l;
	char	s[200];
	int		i;
	int		j;
	int		x;

	for (k = 0; k < sizeof(t) / sizeof(t[0]); k++)
	{
		memcpy(w, t[k].v, sizeof(w));
		for (i = 1; i < t[k].n; i++)
			for (j = i; j > 0 && w[j - 1] > w[j]; j--)
			{
				x = w[j];
				w[j] = w[j - 1];
				w[j - 1] = x;
			}
		l = mklist(t[k].v, t[k].n);
		ft_lstsort(&l, cmp_int);
		list_str(l, s, sizeof(s));
		check(list_eq(l, w, t[k].n), "ft_lstsort (test %zu) : obtenu %s", k + 1, s);
		freelist(l);
	}
}
#endif

#ifdef X_20
void	**ft_lst_to_array(t_list *lst);

static void	run_test(void)
{
	int		v[] = {7, 8, 9};
	t_list	*l;
	void	**a;
	int		i;

	l = mklist(v, 3);
	a = ft_lst_to_array(l);
	check(a != NULL, "ft_lst_to_array ne doit pas renvoyer NULL");
	if (a)
	{
		check(a[0] == l->content && a[1] == l->next->content && a[2] == l->next->next->content,
			"le tableau doit contenir les pointeurs content dans l'ordre (pas des copies)");
		check(a[3] == NULL, "le tableau doit finir par NULL");
	}
	free(a);
	a = ft_lst_to_array(NULL);
	check(a && a[0] == NULL, "ft_lst_to_array(NULL) doit renvoyer un tableau {NULL}");
	free(a);
	for (i = 1; i <= 1; i++)
	{
		inject_on(i);
		a = ft_lst_to_array(l);
		if (inject_off())
			check(a == NULL, "si malloc échoue, ft_lst_to_array doit renvoyer NULL");
		free(a);
	}
	freelist(l);
}
#endif

#ifdef X_21
void	ft_lstrotate(t_list **lst, int n);

static void	run_test(void)
{
	int		v[] = {1, 2, 3, 4, 5};
	struct s_t { int n; int w[5]; } t[] = {
		{0, {1, 2, 3, 4, 5}}, {1, {2, 3, 4, 5, 1}}, {2, {3, 4, 5, 1, 2}}, {5, {1, 2, 3, 4, 5}},
		{7, {3, 4, 5, 1, 2}}, {-1, {5, 1, 2, 3, 4}}, {-6, {5, 1, 2, 3, 4}},
		{INT_MIN, {3, 4, 5, 1, 2}}, {INT_MAX, {3, 4, 5, 1, 2}}};
	size_t	k;
	t_list	*l;
	char	s[200];

	for (k = 0; k < sizeof(t) / sizeof(t[0]); k++)
	{
		l = mklist(v, 5);
		ft_lstrotate(&l, t[k].n);
		list_str(l, s, sizeof(s));
		check(list_eq(l, t[k].w, 5), "ft_lstrotate([1,2,3,4,5], %d) : obtenu %s", t[k].n, s);
		freelist(l);
	}
	l = NULL;
	ft_lstrotate(&l, 3);
	check(l == NULL, "ft_lstrotate sur une liste vide doit la laisser vide");
	l = mklist(v, 1);
	ft_lstrotate(&l, -4);
	check(list_eq(l, v, 1), "ft_lstrotate sur un seul maillon");
	freelist(l);
	ft_lstrotate(NULL, 1);
	check(1, "ft_lstrotate(NULL, 1) ne plante pas");
}
#endif

int	main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	run_test();
	printf("@@ %d %d\n", g_checks, g_fails);
	return (g_fails != 0);
}
