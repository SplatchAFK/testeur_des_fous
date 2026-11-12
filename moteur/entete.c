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

static int	choix_fou(int cols, int rows, int tw, int reste)
{
	if (cols >= 2 + tw + 4 + g_fou_grand_w && rows >= g_fou_grand_h + reste)
		return (2);
	if (cols >= 2 + tw + 4 + g_fou_petit_w && rows >= g_fou_petit_h + reste)
		return (1);
	return (0);
}

int	entete_hauteur(const char *l1, const char *l2, int reste)
{
	int	cols;
	int	rows;
	int	tw;
	int	f;

	entete_taille(&cols, &rows);
	tw = largeur_mot(l1) > largeur_mot(l2) ? largeur_mot(l1) : largeur_mot(l2);
	if (cols < tw + 4 || rows < 11 + reste)
		return (0);
	f = choix_fou(cols, rows, tw, reste);
	if (f == 2)
		return (g_fou_grand_h);
	if (f == 1)
		return (g_fou_petit_h);
	return (11);
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

static void	titre(const char *mot, int x0, int y0, int ligne0, int tw, double phase)
{
	const t_lettre	*l;
	const char		*s;
	int				r;
	int				x;
	int				i;
	int				u;

	u = utf8();
	for (r = 0; r < 5; r++)
	{
		aller(y0 + r, x0 + 1);
		x = 0;
		for (s = mot; *s; s++)
		{
			l = lettre(*s);
			for (i = 0; l->l[r][i]; i++, x++)
			{
				if (l->l[r][i] == '#')
				{
					pixel(x0 + x, ligne0 + r, phase, 100 + 60 * x / (tw ? tw : 1), 200, 238 - 40 * x / (tw ? tw : 1));
					eb_put(u ? "\xe2\x96\x88" : "#", u ? 3 : 1);
				}
				else
					eb_put(" ", 1);
			}
			eb_put(" ", 1);
			x++;
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
	int					cols;
	int					rows;
	int					tw;
	int					f;
	int					fw;
	int					fh;
	int					ty;
	int					r;
	int					c;
	int					x0;
	double				phase;

	entete_taille(&cols, &rows);
	tw = largeur_mot(l1) > largeur_mot(l2) ? largeur_mot(l1) : largeur_mot(l2);
	if (cols < tw + 4 || rows < 11 + reste)
		return ;
	f = choix_fou(cols, rows, tw, reste);
	fw = f == 2 ? g_fou_grand_w : (f == 1 ? g_fou_petit_w : 0);
	fh = f == 2 ? g_fou_grand_h : (f == 1 ? g_fou_petit_h : 11);
	txt = f == 2 ? g_fou_grand_txt : g_fou_petit_txt;
	rgb = f == 2 ? &g_fou_grand_rgb[0][0][0] : &g_fou_petit_rgb[0][0][0];
	x0 = (cols - (tw + (fw ? 4 + fw : 0))) / 2;
	if (x0 < 1)
		x0 = 1;
	phase = entete_phase(ms, l1, l2);
	g_eb.n = 0;
	g_eb.r = -1;
	eb_put("\0337\033[?25l", 8);
	ty = y0 + (fh - 11) / 2;
	titre(l1, x0, ty, ty - y0, tw, phase);
	titre(l2, x0, ty + 6, ty - y0 + 6, tw, phase);
	for (r = 0; f && r < fh; r++)
	{
		aller(y0 + r, x0 + tw + 4 + 1);
		for (c = 0; c < fw; c++)
		{
			if (txt[r][c] == ' ')
			{
				eb_put(" ", 1);
				continue ;
			}
			pixel(x0 + tw + 4 + c, r, phase, rgb[(r * fw + c) * 3], rgb[(r * fw + c) * 3 + 1], rgb[(r * fw + c) * 3 + 2]);
			eb_put(&txt[r][c], 1);
		}
	}
	eb_put("\033[0m\0338", 6);
	g_eb.r = -1;
	fflush(stdout);
	(void)!write(1, g_eb.d, g_eb.n);
}
