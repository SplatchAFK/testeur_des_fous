#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "fous.h"

extern const int			g_fou_grand_w;
extern const int			g_fou_grand_h;
extern const char *const	g_fou_grand_txt[];
extern const unsigned char	g_fou_grand_rgb[][48][3];
extern const int			g_fou_petit_w;
extern const int			g_fou_petit_h;
extern const char *const	g_fou_petit_txt[];
extern const unsigned char	g_fou_petit_rgb[][30][3];

typedef struct s_lettre
{
	char		c;
	const char	*l[5];
}	t_lettre;

static const t_lettre	g_police[] = {
	{'A', {" ### ", "#   #", "#####", "#   #", "#   #"}},
	{'D', {"#### ", "#   #", "#   #", "#   #", "#### "}},
	{'E', {"####", "#   ", "### ", "#   ", "####"}},
	{'F', {"####", "#   ", "### ", "#   ", "#   "}},
	{'H', {"#   #", "#   #", "#####", "#   #", "#   #"}},
	{'L', {"#   ", "#   ", "#   ", "#   ", "####"}},
	{'M', {"#   #", "## ##", "# # #", "#   #", "#   #"}},
	{'O', {" ### ", "#   #", "#   #", "#   #", " ### "}},
	{'R', {"#### ", "#   #", "#### ", "#  # ", "#   #"}},
	{'S', {" ####", "#    ", " ### ", "    #", "#### "}},
	{'T', {"#####", "  #  ", "  #  ", "  #  ", "  #  "}},
	{'U', {"#   #", "#   #", "#   #", "#   #", " ### "}},
	{'X', {"#   #", " # # ", "  #  ", " # # ", "#   #"}},
	{' ', {"  ", "  ", "  ", "  ", "  "}},
	{0, {0, 0, 0, 0, 0}}
};

typedef struct s_ebuf
{
	char	d[262144];
	size_t	n;
	int		r;
	int		g;
	int		b;
}	t_ebuf;

static t_ebuf	g_eb;

static void	eb_put(const char *s, size_t n)
{
	if (g_eb.n + n >= sizeof(g_eb.d))
		return ;
	memcpy(g_eb.d + g_eb.n, s, n);
	g_eb.n += n;
}

static void	eb_col(int r, int g, int b)
{
	char	t[40];
	int		n;
	int		v;

	if (r == g_eb.r && g == g_eb.g && b == g_eb.b)
		return ;
	g_eb.r = r;
	g_eb.g = g;
	g_eb.b = b;
	if (getenv("NO_COLOR"))
		return ;
	if (entete_24bits())
		n = snprintf(t, sizeof(t), "\033[38;2;%d;%d;%dm", r, g, b);
	else
	{
		v = 16 + 36 * (r * 5 / 255) + 6 * (g * 5 / 255) + b * 5 / 255;
		n = snprintf(t, sizeof(t), "\033[38;5;%dm", v);
	}
	eb_put(t, n);
}

int	entete_24bits(void)
{
	const char	*c;
	const char	*t;

	c = getenv("COLORTERM");
	t = getenv("TERM");
	if (c && (strstr(c, "truecolor") || strstr(c, "24bit")))
		return (1);
	if (getenv("VTE_VERSION") || getenv("WT_SESSION"))
		return (1);
	return (t && (strstr(t, "kitty") || strstr(t, "direct") || strstr(t, "foot")));
}

static int	utf8(void)
{
	const char	*l;

	l = getenv("LC_ALL");
	if (!l || !*l)
		l = getenv("LC_CTYPE");
	if (!l || !*l)
		l = getenv("LANG");
	return (l && (strstr(l, "UTF-8") || strstr(l, "utf8") || strstr(l, "UTF8") || strstr(l, "utf-8")));
}

void	entete_taille(int *cols, int *rows)
{
	struct winsize	w;

	*cols = 80;
	*rows = 24;
	if (ioctl(1, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
	{
		*cols = w.ws_col;
		*rows = w.ws_row;
	}
}

static const t_lettre	*lettre(char c)
{
	int	i;

	for (i = 0; g_police[i].c; i++)
		if (g_police[i].c == c)
			return (&g_police[i]);
	return (&g_police[13]);
}

static int	largeur_mot(const char *s)
{
	int	w;

	w = 0;
	for (; *s; s++)
		w += (int)strlen(lettre(*s)->l[0]) + 1;
	return (w > 0 ? w - 1 : 0);
}

typedef struct s_page
{
	int	ech;
	int	lignes;
	int	fou;
	int	tw;
	int	th;
	int	fw;
	int	fh;
	int	w;
	int	h;
}	t_page;

static int	largeur_ligne(const char *l1, const char *l2)
{
	return (largeur_mot(l1) + 4 + largeur_mot(l2));
}

static void	mise_en_page(const char *l1, const char *l2, int reste, t_page *pg)
{
	t_page	c;
	int		cols;
	int		rows;
	int		best;

	entete_taille(&cols, &rows);
	memset(pg, 0, sizeof(*pg));
	best = -1;
	for (c.ech = 2; c.ech >= 1; c.ech--)
		for (c.lignes = 1; c.lignes <= 2; c.lignes++)
			for (c.fou = 2; c.fou >= 0; c.fou--)
			{
				c.tw = c.ech * (c.lignes == 1 ? largeur_ligne(l1, l2)
						: (largeur_mot(l1) > largeur_mot(l2) ? largeur_mot(l1) : largeur_mot(l2)));
				c.th = c.ech * (c.lignes == 1 ? 5 : 11);
				c.fw = c.fou == 2 ? g_fou_grand_w : (c.fou == 1 ? g_fou_petit_w : 0);
				c.fh = c.fou == 2 ? g_fou_grand_h : (c.fou == 1 ? g_fou_petit_h : 0);
				c.w = c.tw + (c.fou ? 4 + c.fw : 0);
				c.h = c.th > c.fh ? c.th : c.fh;
				if (c.w > cols - 2 || c.h > rows - reste)
					continue ;
				if (c.ech * 1000000 + c.w * c.h > best)
				{
					best = c.ech * 1000000 + c.w * c.h;
					*pg = c;
				}
			}
	if (best < 0 && rows - reste >= 2 && cols >= 4)
	{
		pg->h = 2;
		pg->w = (int)strlen(l1) + 1 + (int)strlen(l2);
	}
}

int	entete_hauteur(const char *l1, const char *l2, int reste)
{
	t_page	pg;

	mise_en_page(l1, l2, reste, &pg);
	return (pg.h);
}

static void	pixel(int x, int y, double phase, int r, int g, int b)
{
	double	p;
	double	k;

	p = (x + y * 0.6) - phase;
	k = 0.85 * exp(-(p * p) / 40.0) * 0.75;
	eb_col((int)(r + (255 - r) * k), (int)(g + (255 - g) * k), (int)(b + (255 - b) * k));
}

static void	aller(int y, int x)
{
	char	t[32];

	eb_put(t, snprintf(t, sizeof(t), "\033[%d;%dH", y, x));
}

static void	titre(const char *mot, int x0, int y0, int ech, int tw, double phase)
{
	const t_lettre	*l;
	const char		*s;
	int				r;
	int				x;
	int				i;
	int				u;
	int				e;

	u = utf8();
	for (r = 0; r < 5 * ech; r++)
	{
		aller(y0 + r, x0 + 1);
		x = 0;
		for (s = mot; *s; s++)
		{
			l = lettre(*s);
			for (i = 0; l->l[r / ech][i]; i++)
				for (e = 0; e < ech; e++, x++)
				{
					if (l->l[r / ech][i] == '#')
					{
						pixel(x0 + x, y0 + r, phase, 100 + 60 * x / (tw ? tw : 1), 200, 238 - 40 * x / (tw ? tw : 1));
						eb_put(u ? "\xe2\x96\x88" : "#", u ? 3 : 1);
					}
					else
						eb_put(" ", 1);
				}
			for (e = 0; e < ech; e++, x++)
				eb_put(" ", 1);
		}
	}
}

double	entete_phase(long ms, const char *l1, const char *l2)
{
	int		cols;
	int		rows;
	double	cycle;

	entete_taille(&cols, &rows);
	(void)l1;
	(void)l2;
	cycle = cols + 90.0;
	return (fmod(ms / 22.0, cycle) - 40.0);
}

void	entete_dessiner(const char *l1, const char *l2, long ms, int y0, int reste)
{
	const unsigned char	*rgb;
	const char *const	*txt;
	t_page				pg;
	int					cols;
	int					rows;
	int					ty;
	int					fy;
	int					r;
	int					c;
	int					x0;
	double				phase;

	entete_taille(&cols, &rows);
	mise_en_page(l1, l2, reste, &pg);
	if (!pg.h)
		return ;
	x0 = (cols - pg.w) / 2;
	if (x0 < 1)
		x0 = 1;
	g_eb.n = 0;
	g_eb.r = -1;
	eb_put("\0337\033[?25l", 8);
	if (!pg.ech)
	{
		aller(y0, x0 + 1);
		eb_col(118, 222, 238);
		eb_put(l1, strlen(l1));
		eb_put(" ", 1);
		eb_put(l2, strlen(l2));
	}
	else
	{
		phase = entete_phase(ms, l1, l2);
		ty = y0 + (pg.h - pg.th) / 2;
		if (pg.lignes == 1)
		{
			titre(l1, x0, ty, pg.ech, pg.tw, phase);
			titre(l2, x0 + pg.ech * (largeur_mot(l1) + 4), ty, pg.ech, pg.tw, phase);
		}
		else
		{
			titre(l1, x0, ty, pg.ech, pg.tw, phase);
			titre(l2, x0, ty + 6 * pg.ech, pg.ech, pg.tw, phase);
		}
		txt = pg.fou == 2 ? g_fou_grand_txt : g_fou_petit_txt;
		rgb = pg.fou == 2 ? &g_fou_grand_rgb[0][0][0] : &g_fou_petit_rgb[0][0][0];
		fy = y0 + (pg.h - pg.fh) / 2;
		for (r = 0; pg.fou && r < pg.fh; r++)
		{
			aller(fy + r, x0 + pg.tw + 4 + 1);
			for (c = 0; c < pg.fw; c++)
			{
				if (txt[r][c] == ' ')
				{
					eb_put(" ", 1);
					continue ;
				}
				pixel(x0 + pg.tw + 4 + c, r, phase, rgb[(r * pg.fw + c) * 3], rgb[(r * pg.fw + c) * 3 + 1],
					rgb[(r * pg.fw + c) * 3 + 2]);
				eb_put(&txt[r][c], 1);
			}
		}
	}
	eb_put("\033[0m\0338", 6);
	g_eb.r = -1;
	fflush(stdout);
	(void)!write(1, g_eb.d, g_eb.n);
}
