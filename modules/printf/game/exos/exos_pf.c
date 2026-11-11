/*
** Tests des 16 niveaux d'entraînement du testeur des fous printf.
** Compilé avec le fichier du joueur et -DLEVEL=n par run.sh --exo.
** Sortie : lignes « D\tdétail » puis « X\tok|ko\tnombre_de_vérifications ».
** Aucune solution ici : seulement les cas et les résultats attendus.
*/
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wformat-truncation"
#include <limits.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int		ft_putchar_count(char c);
int		ft_putstr_count(char *s);
int		ft_putnbr_count(int n);
int		ft_putbase_count(unsigned long n, char *base);
int		ft_puthex_count(unsigned int n, int upper);
int		ft_putptr_count(void *p);
int		ft_is_conv(char c);
int		ft_count_conv(const char *fmt);
int		ft_sum(int n, ...);
int		ft_max(int n, ...);
char	*ft_strjoin_n(int n, ...);
int		ft_sum_twice(int n, ...);
int		ft_putstr_fd_count(int fd, char *s);
char	*ft_utoa_base(unsigned long n, const char *base);
int		ft_vprintf_mini(const char *fmt, va_list ap);
int		ft_dprintf_mini(int fd, const char *fmt, ...);

static int	g_checks;
static int	g_fails;
static int	g_pipe[2];
static int	g_save_fd;
static char	g_out[1 << 16];
static int	g_len;

static void	fail(const char *fmt, ...)
{
	va_list	ap;
	char	b[400];

	g_fails++;
	if (g_fails > 8)
		return ;
	va_start(ap, fmt);
	vsnprintf(b, sizeof(b), fmt, ap);
	va_end(ap);
	for (char *p = b; *p; p++)
		if (*p == '\n' || *p == '\t')
			*p = ' ';
	printf("D\t%s\n", b);
	fflush(stdout);
}

static void	cap_begin(void)
{
	fflush(stdout);
	if (pipe(g_pipe) != 0)
		exit(3);
	g_save_fd = dup(1);
	dup2(g_pipe[1], 1);
	close(g_pipe[1]);
}

static void	cap_end(void)
{
	ssize_t	r;

	dup2(g_save_fd, 1);
	close(g_save_fd);
	g_len = 0;
	while ((r = read(g_pipe[0], g_out + g_len, sizeof(g_out) - 1 - g_len)) > 0)
		g_len += r;
	g_out[g_len] = '\0';
	close(g_pipe[0]);
}

/* compare la sortie capturée et la valeur de retour */
static void	expect(const char *what, const char *exp, int exp_len, int ret, int exp_ret)
{
	g_checks++;
	if (g_len != exp_len || memcmp(g_out, exp, exp_len))
		fail("%s : sortie « %.*s », attendu « %.*s »", what, g_len > 60 ? 60 : g_len, g_out,
			exp_len > 60 ? 60 : exp_len, exp);
	else if (ret != exp_ret)
		fail("%s : renvoie %d, attendu %d", what, ret, exp_ret);
}

/* écrire vers une sortie fermée : write échoue, on attend -1 */
static int	closed_begin(void)
{
	int	fd;

	fflush(stdout);
	fd = dup(1);
	close(1);
	return (fd);
}

static void	closed_end(int fd)
{
	dup2(fd, 1);
	close(fd);
}

static void	ret_only(const char *what, int ret, int exp)
{
	g_checks++;
	if (ret != exp)
		fail("%s : renvoie %d, attendu %d", what, ret, exp);
}

/* copie la chaîne tout au bout d'un bloc : lire après le '\0' = erreur ASan */
static char	*edge(const char *s)
{
	char	*p;

	p = malloc(strlen(s) + 1);
	memcpy(p, s, strlen(s) + 1);
	return (p);
}

#if LEVEL == 15
static int	call_v(const char *fmt, ...)
{
	va_list	ap;
	int		r;

	va_start(ap, fmt);
	r = ft_vprintf_mini(fmt, ap);
	va_end(ap);
	return (r);
}
#endif

int	main(void)
{
	char	e[300];
	int		r;
	int		fd;

	(void)e;
	(void)r;
	(void)fd;
#if LEVEL == 1
	{
		const char	cs[] = {'a', '\0', '\n', (char)200, '~'};

		for (int i = 0; i < 5; i++)
		{
			cap_begin();
			r = ft_putchar_count(cs[i]);
			cap_end();
			snprintf(e, sizeof(e), "ft_putchar_count(%d)", (unsigned char)cs[i]);
			expect(e, &cs[i], 1, r, 1);
		}
		fd = closed_begin();
		r = ft_putchar_count('x');
		closed_end(fd);
		ret_only("ft_putchar_count avec la sortie fermée", r, -1);
	}
#elif LEVEL == 2
	{
		static char	big[3001];

		memset(big, 'z', 3000);
		cap_begin(); r = ft_putstr_count("hello"); cap_end(); expect("ft_putstr_count(\"hello\")", "hello", 5, r, 5);
		cap_begin(); r = ft_putstr_count(""); cap_end(); expect("ft_putstr_count(\"\")", "", 0, r, 0);
		cap_begin(); r = ft_putstr_count(NULL); cap_end(); expect("ft_putstr_count(NULL)", "(null)", 6, r, 6);
		cap_begin(); r = ft_putstr_count(big); cap_end(); expect("ft_putstr_count(3000 'z')", big, 3000, r, 3000);
		fd = closed_begin(); r = ft_putstr_count("abc"); closed_end(fd);
		ret_only("ft_putstr_count avec la sortie fermée", r, -1);
	}
#elif LEVEL == 3
	{
		const int	v[] = {0, -1, 42, -42, 1000, INT_MAX, INT_MIN, 7};

		for (int i = 0; i < 8; i++)
		{
			cap_begin();
			r = ft_putnbr_count(v[i]);
			cap_end();
			snprintf(e, sizeof(e), "%d", v[i]);
			char w[64];
			snprintf(w, sizeof(w), "ft_putnbr_count(%d)", v[i]);
			expect(w, e, (int)strlen(e), r, (int)strlen(e));
		}
	}
#elif LEVEL == 4
	{
		cap_begin(); r = ft_putbase_count(0, "01"); cap_end(); expect("ft_putbase_count(0, \"01\")", "0", 1, r, 1);
		cap_begin(); r = ft_putbase_count(5, "01"); cap_end(); expect("ft_putbase_count(5, \"01\")", "101", 3, r, 3);
		cap_begin(); r = ft_putbase_count(255, "0123456789abcdef"); cap_end(); expect("ft_putbase_count(255, hex)", "ff", 2, r, 2);
		cap_begin(); r = ft_putbase_count(8, "01234567"); cap_end(); expect("ft_putbase_count(8, octal)", "10", 2, r, 2);
		cap_begin(); r = ft_putbase_count(ULONG_MAX, "0123456789abcdef"); cap_end();
		snprintf(e, sizeof(e), "%lx", ULONG_MAX);
		expect("ft_putbase_count(ULONG_MAX, hex)", e, (int)strlen(e), r, (int)strlen(e));
		cap_begin(); r = ft_putbase_count(1234567890123UL, "0123456789"); cap_end();
		expect("ft_putbase_count(1234567890123, décimal)", "1234567890123", 13, r, 13);
	}
#elif LEVEL == 5
	{
		const unsigned int	v[] = {0, 255, 0xabc, UINT_MAX, 16};

		for (int i = 0; i < 5; i++)
			for (int up = 0; up < 2; up++)
			{
				cap_begin();
				r = ft_puthex_count(v[i], up);
				cap_end();
				snprintf(e, sizeof(e), up ? "%X" : "%x", v[i]);
				char w[64];
				snprintf(w, sizeof(w), "ft_puthex_count(%u, %d)", v[i], up);
				expect(w, e, (int)strlen(e), r, (int)strlen(e));
			}
	}
#elif LEVEL == 6
	{
		int		x;
		void	*ps[4];

		ps[0] = NULL;
		ps[1] = &x;
		ps[2] = (void *)1;
		ps[3] = (void *)ULONG_MAX;
		for (int i = 0; i < 4; i++)
		{
			cap_begin();
			r = ft_putptr_count(ps[i]);
			cap_end();
			snprintf(e, sizeof(e), "%p", ps[i]);
			char w[64];
			snprintf(w, sizeof(w), "ft_putptr_count(%s)", i == 0 ? "NULL" : e);
			expect(w, e, (int)strlen(e), r, (int)strlen(e));
		}
	}
#elif LEVEL == 7
	{
		const char	*yes = "cspdiuxX%";
		const char	*no = "aDyZ\0 -.#0";

		for (int i = 0; yes[i]; i++)
			ret_only((snprintf(e, sizeof(e), "ft_is_conv('%c')", yes[i]), e), ft_is_conv(yes[i]), 1);
		for (int i = 0; i < 10; i++)
			ret_only((snprintf(e, sizeof(e), "ft_is_conv(%d)", no[i]), e), ft_is_conv(no[i]), 0);
	}
#elif LEVEL == 8
	{
		const char	*f[] = {"", "abc", "%d", "%%", "%d%s%%", "100%", "%", "%%%", "%c et %x", "%%d"};
		const int	exp[] = {0, 0, 1, 1, 3, 0, 0, 1, 2, 1};
		char		*p;

		for (int i = 0; i < 10; i++)
		{
			p = edge(f[i]);
			snprintf(e, sizeof(e), "ft_count_conv(\"%s\")", f[i]);
			ret_only(e, ft_count_conv(p), exp[i]);
			free(p);
		}
	}
#elif LEVEL == 9
	{
		ret_only("ft_sum(0)", ft_sum(0), 0);
		ret_only("ft_sum(1, 5)", ft_sum(1, 5), 5);
		ret_only("ft_sum(3, 1, 2, 3)", ft_sum(3, 1, 2, 3), 6);
		ret_only("ft_sum(4, -1, -2, -3, 10)", ft_sum(4, -1, -2, -3, 10), 4);
		ret_only("ft_sum(10, 1 ... 10)", ft_sum(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10), 55);
	}
#elif LEVEL == 10
	{
		ret_only("ft_max(1, 7)", ft_max(1, 7), 7);
		ret_only("ft_max(3, -5, -2, -9)", ft_max(3, -5, -2, -9), -2);
		ret_only("ft_max(5, 1, 9, 3, 9, 2)", ft_max(5, 1, 9, 3, 9, 2), 9);
		ret_only("ft_max(2, INT_MIN, INT_MIN)", ft_max(2, INT_MIN, INT_MIN), INT_MIN);
		ret_only("ft_max(8, ... INT_MAX à la fin)", ft_max(8, 1, 2, 3, 4, 5, 6, 7, INT_MAX), INT_MAX);
	}
#elif LEVEL == 11
	{
		char	*s;

		s = ft_strjoin_n(0);
		g_checks++;
		if (!s || strcmp(s, ""))
			fail("ft_strjoin_n(0) : attendu \"\"");
		free(s);
		s = ft_strjoin_n(2, "ab", "cd");
		g_checks++;
		if (!s || strcmp(s, "abcd"))
			fail("ft_strjoin_n(2, \"ab\", \"cd\") : obtenu \"%s\", attendu \"abcd\"", s ? s : "NULL");
		free(s);
		s = ft_strjoin_n(4, "", "x", "", "yz");
		g_checks++;
		if (!s || strcmp(s, "xyz"))
			fail("ft_strjoin_n(4, \"\", \"x\", \"\", \"yz\") : obtenu \"%s\", attendu \"xyz\"", s ? s : "NULL");
		if (s && malloc_usable_size(s) != strlen(s) + 1)
			fail("ft_strjoin_n : %zu octets alloués pour %zu utiles (taille exacte attendue)",
				malloc_usable_size(s), strlen(s) + 1);
		free(s);
	}
#elif LEVEL == 12
	{
		ret_only("ft_sum_twice(3, 1, 2, 4)", ft_sum_twice(3, 1, 2, 4), 14);
		ret_only("ft_sum_twice(0)", ft_sum_twice(0), 0);
		ret_only("ft_sum_twice(4, 1, 3, 5, 7)", ft_sum_twice(4, 1, 3, 5, 7), 0);
		ret_only("ft_sum_twice(5, 2, 2, 2, 2, 2)", ft_sum_twice(5, 2, 2, 2, 2, 2), 50);
	}
#elif LEVEL == 13
	{
		int	p[2];
		char	b[64];
		ssize_t	n;

		if (pipe(p) != 0)
			return (3);
		r = ft_putstr_fd_count(p[1], "sur le fd");
		close(p[1]);
		n = read(p[0], b, sizeof(b) - 1);
		b[n > 0 ? n : 0] = '\0';
		close(p[0]);
		g_checks++;
		if (strcmp(b, "sur le fd") || r != 9)
			fail("ft_putstr_fd_count(pipe, \"sur le fd\") : obtenu « %s », renvoie %d", b, r);
		ret_only("ft_putstr_fd_count(fd fermé, \"x\")", ft_putstr_fd_count(p[1], "x"), -1);
		ret_only("ft_putstr_fd_count(-1, \"x\")", ft_putstr_fd_count(-1, "x"), -1);
	}
#elif LEVEL == 14
	{
		const unsigned long	v[] = {0, 42, 255, ULONG_MAX};
		const char			*b[] = {"0123456789", "0123456789", "0123456789abcdef", "01"};
		char				*s;
		char				w[200];

		for (int i = 0; i < 4; i++)
		{
			s = ft_utoa_base(v[i], b[i]);
			if (i == 0)
				snprintf(e, sizeof(e), "0");
			else if (i == 1)
				snprintf(e, sizeof(e), "42");
			else if (i == 2)
				snprintf(e, sizeof(e), "ff");
			else
				memset(e, '1', 64), e[64] = '\0';
			snprintf(w, sizeof(w), "ft_utoa_base(%lu, \"%s\")", v[i], b[i]);
			g_checks++;
			if (!s || strcmp(s, e))
				fail("%s : obtenu \"%s\", attendu \"%s\"", w, s ? s : "NULL", e);
			else if (malloc_usable_size(s) != strlen(s) + 1)
				fail("%s : %zu octets alloués pour %zu utiles (taille exacte attendue)", w, malloc_usable_size(s),
					strlen(s) + 1);
			free(s);
		}
	}
#elif LEVEL == 15
	{
		cap_begin(); r = call_v("a%db", 42); cap_end(); expect("vprintf_mini(\"a%db\", 42)", "a42b", 4, r, 4);
		cap_begin(); r = call_v("%s!", "hey"); cap_end(); expect("vprintf_mini(\"%s!\", \"hey\")", "hey!", 4, r, 4);
		cap_begin(); r = call_v("100%%"); cap_end(); expect("vprintf_mini(\"100%%\")", "100%", 4, r, 4);
		cap_begin(); r = call_v("%d %d %s", INT_MIN, 0, "x"); cap_end();
		snprintf(e, sizeof(e), "%d %d %s", INT_MIN, 0, "x");
		expect("vprintf_mini(\"%d %d %s\", INT_MIN, 0, \"x\")", e, (int)strlen(e), r, (int)strlen(e));
		cap_begin(); r = call_v("%s", (char *)NULL); cap_end(); expect("vprintf_mini(\"%s\", NULL)", "(null)", 6, r, 6);
	}
#elif LEVEL == 16
	{
		int		p[2];
		char	b[200];
		ssize_t	n;

		if (pipe(p) != 0)
			return (3);
		r = ft_dprintf_mini(p[1], "[%c|%s|%d]", 'z', "ok", -7);
		close(p[1]);
		n = read(p[0], b, sizeof(b) - 1);
		b[n > 0 ? n : 0] = '\0';
		close(p[0]);
		g_checks++;
		if (strcmp(b, "[z|ok|-7]") || r != 9)
			fail("ft_dprintf_mini(fd, \"[%%c|%%s|%%d]\", 'z', \"ok\", -7) : obtenu « %s », renvoie %d", b, r);
		ret_only("ft_dprintf_mini(fd fermé, \"xyz\")", ft_dprintf_mini(p[1], "xyz"), -1);
		ret_only("ft_dprintf_mini(-1, \"%d\", 1)", ft_dprintf_mini(-1, "%d", 1), -1);
	}
#endif
	printf("X\t%s\t%d\n", g_fails ? "ko" : "ok", g_checks);
	return (0);
}
