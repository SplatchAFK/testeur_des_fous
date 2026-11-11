#include "tdf_tests.h"
#include <stdlib.h>
#include <string.h>

static unsigned long long	g_rng;

static unsigned	rnd(void)
{
	g_rng ^= g_rng << 13;
	g_rng ^= g_rng >> 7;
	g_rng ^= g_rng << 17;
	return ((unsigned)(g_rng >> 11));
}

static unsigned	rndn(unsigned n)
{
	return (rnd() % n);
}

static char	g_s_long[1001];
static int	g_glob;
static const char	*g_strs[] = {
	"", "a", "hello", "Hello, World!", NULL, "é漢字🙂", g_s_long, "%d %s %%",
	"\t\n", " ", "(null)", "0x0", "-2147483648", "\x7f\x80\xff"
};
static const int	g_ints[] = {
	0, 1, -1, 9, 10, -10, 42, -42, 99, 100, 255, 256, 4095, 4096, 65535,
	123456789, 999999999, 1000000000, -1000000000, INT_MAX, INT_MAX - 1,
	INT_MIN, INT_MIN + 1
};

static void	add_text(char *f, size_t *l, size_t max)
{
	static const char	cs[] = "abcXYZ 0123456789 .,;:!?-_=+*/()[]{}<>|#@&~^'\"\\";
	unsigned			n;

	n = 1 + rndn(8);
	while (n-- && *l + 4 < max)
	{
		if (rndn(40) == 0)
		{
			memcpy(f + *l, "\xc3\xa9", 2);
			*l += 2;
		}
		else if (rndn(50) == 0)
			f[(*l)++] = '\n';
		else
			f[(*l)++] = cs[rndn(sizeof(cs) - 1)];
	}
}

static void	add_conv(t_bc *c, char *f, size_t *l, int *local)
{
	static const char	conv[] = "cspdiuxX";
	char				cv;
	int					a;

	cv = conv[rndn(8)];
	f[(*l)++] = '%';
	f[(*l)++] = cv;
	a = c->n++;
	if (cv == 'c')
	{
		c->k[a] = K_INT;
		c->v[a].i = rndn(4) ? (int)rndn(256) : g_ints[rndn(sizeof(g_ints) / sizeof(int))];
	}
	else if (cv == 'd' || cv == 'i')
	{
		c->k[a] = K_INT;
		c->v[a].i = rndn(2) ? (int)rnd() * (rndn(2) ? 1 : -1)
			: g_ints[rndn(sizeof(g_ints) / sizeof(int))];
	}
	else if (cv == 'u' || cv == 'x' || cv == 'X')
	{
		c->k[a] = K_UNS;
		c->v[a].u = rndn(2) ? rnd() * 2654435761u
			: (unsigned)g_ints[rndn(sizeof(g_ints) / sizeof(int))];
	}
	else if (cv == 's')
	{
		c->k[a] = K_STR;
		c->v[a].s = g_strs[rndn(sizeof(g_strs) / sizeof(char *))];
	}
	else
	{
		c->k[a] = K_PTR;
		a = (int)rndn(7);
		c->v[c->n - 1].p = a == 0 ? NULL : a == 1 ? (void *)local : a == 2
			? (void *)&g_glob : a == 3 ? (void *)g_s_long : a == 4
			? (void *)(unsigned long)-1L
			: (void *)(((unsigned long)rnd() << 32) | rnd());
	}
}

static void	gen_case(t_bc *c, int *local)
{
	size_t		l;
	unsigned	segs;

	memset(c, 0, sizeof(*c));
	l = 0;
	segs = rndn(9);
	while (segs-- && l < sizeof(c->fmt) - 20)
	{
		if (rndn(3) == 0)
			add_text(c->fmt, &l, sizeof(c->fmt));
		else if (rndn(8) == 0)
		{
			c->fmt[l++] = '%';
			c->fmt[l++] = '%';
		}
		else if (c->n < 4)
			add_conv(c, c->fmt, &l, local);
	}
	c->fmt[l] = '\0';
	c->sev = SEV_STRICT;
}

void	tdf_tests_stress(void)
{
	t_bc	*cases;
	int		i;
	int		local;
	char	title[96];

	memset(g_s_long, 'L', 1000);
	g_s_long[1000] = '\0';
	snprintf(title, sizeof(title), "stress aléatoire : %d appels, graine %u",
		g_opt.stress_n, g_opt.seed);
	tdf_category("stress", title);
	if (!tdf_selected("stress") || g_opt.stress_n <= 0)
		return ;
	g_rng = 0x9E3779B97F4A7C15ULL ^ ((unsigned long long)g_opt.seed * 0x100000001B3ULL);
	if (!g_rng)
		g_rng = 1;
	cases = calloc((size_t)g_opt.stress_n, sizeof(t_bc));
	i = -1;
	while (++i < g_opt.stress_n)
	{
		if ((i & 1023) == 0)
			tdf_quit_check(0);
		gen_case(&cases[i], &local);
	}
	tdf_batch(cases, g_opt.stress_n, __FILE__, __LINE__, "stress");
	free(cases);
}

#define F_MINUS 1
#define F_ZERO 2
#define F_HASH 4
#define F_PLUS 8
#define F_SPACE 16
#define F_WIDTH 32
#define F_PREC 64

static int	is_ub(int fl, int wp, char cv, int null_str)
{
	int	prec;

	prec = wp & 0xff;
	if (cv == '%')
		return (fl != 0 || wp != 0);
	if ((fl & F_HASH) && cv != 'x' && cv != 'X')
		return (1);
	if ((fl & F_ZERO) && (cv == 'c' || cv == 's' || cv == 'p'))
		return (1);
	if (prec && (cv == 'c' || cv == 'p'))
		return (1);
	if ((fl & (F_PLUS | F_SPACE)) && cv != 'd' && cv != 'i')
		return (1);
	if (null_str && prec)
		return (1);
	return (0);
}

static int	probe(const char *fmt, int v)
{
	int		ok;

	if (tdf_child_start())
	{
		tdf_track_arm();
		tdf_child_end(ft_printf(fmt, v));
	}
	tdf_exp_start();
	tdf_exp_end(printf(fmt, v));
	ok = !g_run.timed_out && g_run.returned && g_run.ret == g_exp_ret
		&& g_run.out.len == g_exp.len && !memcmp(g_run.out.p, g_exp.p, g_exp.len);
	return (ok);
}

static int	detect_features(char *names, size_t n)
{
	int	f;

	f = 0;
	if (probe("[%-5d]", 42) && probe("[%-3c]", 'a'))
		f |= F_MINUS;
	if (probe("[%05d]", -42) && probe("[%03u]", 7))
		f |= F_ZERO;
	if (probe("[%#x]", 42) && probe("[%#X]", 42))
		f |= F_HASH;
	if (probe("[%+d]", 42) && probe("[%+d]", -1))
		f |= F_PLUS;
	if (probe("[% d]", 42) && probe("[% i]", 0))
		f |= F_SPACE;
	if (probe("[%5d]", 42) && probe("[%3c]", 'z'))
		f |= F_WIDTH;
	if (probe("[%.3d]", 7) && probe("[%.0d]", 0))
		f |= F_PREC;
	snprintf(names, n, "%s%s%s%s%s%s%s", f & F_MINUS ? "- " : "", f & F_ZERO ? "0 " : "",
		f & F_PREC ? ". " : "", f & F_HASH ? "# " : "", f & F_PLUS ? "+ " : "",
		f & F_SPACE ? "esp " : "", f & F_WIDTH ? "largeur" : "");
	return (f);
}

static int	push(t_bc *c, int *n, int max, const char *fmt, int k, t_val v, int sev)
{
	if (*n >= max)
		return (0);
	memset(&c[*n], 0, sizeof(t_bc));
	snprintf(c[*n].fmt, sizeof(c[*n].fmt), "%s", fmt);
	c[*n].n = (k >= 0);
	c[*n].k[0] = k < 0 ? 0 : k;
	c[*n].v[0] = v;
	c[*n].sev = sev;
	(*n)++;
	return (1);
}

static void	gen_values(char cv, int *k, t_val *vals, int *nv, int *local)
{
	static const int	iv[] = {0, 1, -1, 7, 42, -42, 12345, INT_MAX, INT_MIN};
	static const char	*sv[] = {"", "a", "hello", "bonjour le monde", NULL};
	int					i;

	*nv = 0;
	if (cv == 'c')
	{
		*k = K_INT;
		vals[(*nv)++].i = 'a';
		vals[(*nv)++].i = 0;
		vals[(*nv)++].i = 255;
	}
	else if (cv == 's')
	{
		*k = K_STR;
		i = -1;
		while (++i < 5)
			vals[(*nv)++].s = sv[i];
	}
	else if (cv == 'p')
	{
		*k = K_PTR;
		vals[(*nv)++].p = NULL;
		vals[(*nv)++].p = (void *)1;
		vals[(*nv)++].p = (void *)local;
		vals[(*nv)++].p = (void *)-1L;
	}
	else if (cv == '%')
		*k = -1;
	else
	{
		*k = (cv == 'd' || cv == 'i') ? K_INT : K_UNS;
		i = -1;
		while (++i < 9)
			vals[(*nv)++].i = iv[i];
	}
	if (cv == '%')
		*nv = 1;
}

static void	bonus_one(const char *key, const char *title, const char **flags, int fl,
				const char **wd, const char **pr, const char *convs)
{
	char	fmt[64];
	t_bc	*cases;
	t_val	vals[16];
	int		n;
	int		k;
	int		nv;
	int		vi;
	int		w;
	int		p;
	int		ci;
	int		fi;
	int		local;

	cases = calloc(4000, sizeof(t_bc));
	if (!cases)
		return ;
	tdf_category(key, title);
	n = 0;
	for (fi = 0; flags[fi]; fi++)
	for (w = 0; wd[w]; w++)
		for (p = 0; pr[p]; p++)
			for (ci = 0; convs[ci]; ci++)
			{
				snprintf(fmt, sizeof(fmt), "[%%%s%s%s%c]", flags[fi], wd[w], pr[p], convs[ci]);
				gen_values(convs[ci], &k, vals, &nv, &local);
				for (vi = 0; vi < nv; vi++)
					push(cases, &n, 4000, fmt, k, vals[vi],
						is_ub(fl, (pr[p][0] ? 1 : 0) | (wd[w][0] ? 1 << 8 : 0), convs[ci],
							convs[ci] == 's' && !vals[vi].s) ? SEV_UB : SEV_STRICT);
			}
	tdf_batch(cases, n, __FILE__, __LINE__, key);
	free(cases);
}

static void	bonus_isolated(void)
{
	static const char	*w3[] = {"1", "5", "15", NULL};
	static const char	*w0[] = {"", NULL};
	static const char	*w2[] = {"", "6", NULL};
	static const char	*p0[] = {"", NULL};
	static const char	*pp[] = {".", ".0", ".1", ".5", ".15", NULL};

	static const char	*fm[] = {"-", NULL};
	static const char	*fz[] = {"0", NULL};
	static const char	*fn[] = {"", NULL};
	static const char	*fh[] = {"#", NULL};
	static const char	*fs[] = {"+", " ", NULL};

	bonus_one("bonus_minus", "DLC · flag - (le Gaucher)", fm, F_MINUS, w3, p0, "cspdiuxX");
	bonus_one("bonus_zero", "DLC · flag 0 (le Remplisseur)", fz, F_ZERO, w3, p0, "diuxX");
	bonus_one("bonus_width", "DLC · largeur (le Géant)", fn, F_WIDTH, w3, p0, "cspdiuxX");
	bonus_one("bonus_prec", "DLC · précision . (le Précis)", fn, F_PREC, w0, pp, "sdiuxX");
	bonus_one("bonus_hash", "DLC · flag # (l'Alterné)", fh, F_HASH, w2, p0, "xX");
	bonus_one("bonus_sign", "DLC · flags + et espace (le Signataire)", fs, F_PLUS | F_SPACE, w2, p0, "di");
}

void	tdf_tests_bonus(void)
{
	static const char	*fl[] = {"-", "0", "#", "+", " "};
	static const char	*wd[] = {"", "1", "5", "15"};
	static const char	*pr[] = {"", ".", ".0", ".1", ".5", ".15"};
	static const char	cv[] = "cspdiuxX%";
	char				feat[96];
	char				title[160];
	char				fmt[64];
	char				flags[8];
	int					f;
	int					m;
	int					w;
	int					p;
	int					ci;
	int					need;
	t_bc				*cases;
	int					n;
	int					k;
	t_val				vals[16];
	int					nv;
	int					vi;
	int					local;
	int					j;

	if (!tdf_selected("bonus") || g_opt.bonus < 0)
		return ;
	f = detect_features(feat, sizeof(feat));
	if (g_opt.bonus == 0 && !f)
	{
		tdf_category("bonus", "bonus non détecté (ignoré, sujet v1.1 : pas de bonus)");
		return ;
	}
	if (g_opt.bonus == 1)
		f = 127;
	snprintf(title, sizeof(title), "bonus : %s", g_opt.bonus == 1 ? "tout (--bonus)" : feat);
	tdf_category("bonus", title);
	cases = calloc(60000, sizeof(t_bc));
	n = 0;
	m = -1;
	while (++m < 32)
	{
		w = -1;
		while (++w < 4)
		{
			p = -1;
			while (++p < 6)
			{
				need = (m << 0) | (w ? F_WIDTH : 0) | (p ? F_PREC : 0);
				if ((need & f) != need)
					continue ;
				flags[0] = '\0';
				j = -1;
				while (++j < 5)
					if (m & (1 << j))
						strcat(flags, fl[j]);
				ci = -1;
				while (cv[++ci])
				{
					snprintf(fmt, sizeof(fmt), "[%%%s%s%s%c]", flags, wd[w], pr[p], cv[ci]);
					gen_values(cv[ci], &k, vals, &nv, &local);
					vi = -1;
					while (++vi < nv)
						push(cases, &n, 60000, fmt, k, vals[vi],
							is_ub(m, p | (w << 8), cv[ci], cv[ci] == 's' && !vals[vi].s)
							? SEV_UB : SEV_STRICT);
				}
			}
		}
	}
	tdf_batch(cases, n, __FILE__, __LINE__, "bonus");
	free(cases);
	if (g_opt.bonus == 1)
		bonus_isolated();
}
