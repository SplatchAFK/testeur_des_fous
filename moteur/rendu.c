#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include "fous.h"

t_caps	g_caps;

typedef struct s_shown
{
	int				on;
	int				want;
	int				x;
	int				y;
	int				w;
	int				h;
	int				gen;
	unsigned long	sum;
	unsigned char	*px;
	int				pw;
	int				ph;
}	t_shown;

static t_shown	g_sh;
static int		g_kitty_used;

static int	probe_byte(long fin)
{
	fd_set			s;
	struct timeval	tv;
	unsigned char	c;
	long			r;

	r = fin - now_ms();
	if (r <= 0)
		return (-1);
	FD_ZERO(&s);
	FD_SET(0, &s);
	tv.tv_sec = r / 1000;
	tv.tv_usec = (r % 1000) * 1000;
	if (select(1, &s, NULL, NULL, &tv) <= 0)
		return (-1);
	if (read(0, &c, 1) != 1)
		return (-1);
	return (c);
}

static void	probe_da1(const char *p)
{
	int	v;

	p += 3;
	while (*p && *p != 'c')
	{
		v = atoi(p);
		if (v == 4)
			g_caps.sixel = 1;
		while (*p >= '0' && *p <= '9')
			p++;
		if (*p == ';')
			p++;
		else if (*p != 'c')
			break ;
	}
}

static void	probe_term(void)
{
	static const char	q[] = "\033_Gi=31,s=1,v=1,a=q,t=d,f=24;AAAA\033\\\033[16t\033[14t\033[c";
	char				b[512];
	const char			*t;
	const char			*p;
	int					n;
	int					c;
	int					h;
	int					w;
	long				fin;

	t = getenv("TERM");
	if (!isatty(0) || !isatty(1) || getenv("TDF_SANS_SONDE")
		|| (t && (!strcmp(t, "linux") || !strcmp(t, "dumb") || !strncmp(t, "vt", 2))))
		return ;
	(void)!write(1, q, sizeof(q) - 1);
	n = 0;
	fin = now_ms() + 700;
	while (n < (int)sizeof(b) - 1 && (c = probe_byte(fin)) >= 0)
	{
		b[n++] = c;
		b[n] = 0;
		if (c == 'c' && strstr(b, "\033[?"))
			break ;
	}
	b[n] = 0;
	g_caps.sonde = n > 0;
	if (strstr(b, "\033_Gi=31;OK"))
		g_caps.kitty = 1;
	if ((p = strstr(b, "\033[?")))
		probe_da1(p);
	if ((p = strstr(b, "\033[6;")) && sscanf(p + 4, "%d;%d", &h, &w) == 2 && h > 0 && w > 0)
	{
		g_caps.cell_w = w;
		g_caps.cell_h = h;
	}
	if ((p = strstr(b, "\033[4;")) && sscanf(p + 4, "%d;%d", &h, &w) == 2 && h > 0 && w > 0)
	{
		g_caps.win_w = w;
		g_caps.win_h = h;
		if (!g_caps.cell_w && term_cols() > 0 && term_rows() > 0)
		{
			g_caps.cell_w = w / term_cols();
			g_caps.cell_h = h / term_rows();
		}
	}
}

static int	has_ci(const char *s, const char *w)
{
	size_t	n;

	if (!s)
		return (0);
	n = strlen(w);
	for (; *s; s++)
		if (!strncasecmp(s, w, n))
			return (1);
	return (0);
}

static void	probe_env(void)
{
	const char	*t;
	const char	*c;
	const char	*l;

	t = getenv("TERM");
	c = getenv("COLORTERM");
	if (has_ci(c, "truecolor") || has_ci(c, "24bit") || has_ci(t, "direct")
		|| (getenv("VTE_VERSION") && atoi(getenv("VTE_VERSION")) >= 3600)
		|| has_ci(t, "kitty") || has_ci(t, "foot") || has_ci(t, "wezterm")
		|| has_ci(getenv("TERM_PROGRAM"), "iTerm") || has_ci(getenv("TERM_PROGRAM"), "WezTerm")
		|| has_ci(getenv("TERM_PROGRAM"), "vscode") || getenv("WT_SESSION"))
		g_caps.couleurs = 2;
	else if (has_ci(t, "256"))
		g_caps.couleurs = 1;
	l = getenv("LC_ALL");
	if (!l || !*l)
		l = getenv("LC_CTYPE");
	if (!l || !*l)
		l = getenv("LANG");
	g_caps.utf8 = has_ci(l, "utf-8") || has_ci(l, "utf8");
	if (t && (!strcmp(t, "linux") || !strncmp(t, "vt", 2)))
		g_caps.utf8 = 0;
}

static void	caps_write(void)
{
	char	p[1024];
	FILE	*f;

	if (!getenv("HOME"))
		return ;
	snprintf(p, sizeof(p), "%s/.testeur_des_fous", getenv("HOME"));
	mkdir(p, 0700);
	snprintf(p, sizeof(p), "%s/.testeur_des_fous/debug", getenv("HOME"));
	mkdir(p, 0700);
	snprintf(p, sizeof(p), "%s/.testeur_des_fous/debug/rendu.txt", getenv("HOME"));
	if (!(f = fopen(p, "w")))
		return ;
	fprintf(f, "TERM=%s COLORTERM=%s VTE_VERSION=%s TERM_PROGRAM=%s\n", getenv("TERM") ? getenv("TERM") : "",
		getenv("COLORTERM") ? getenv("COLORTERM") : "", getenv("VTE_VERSION") ? getenv("VTE_VERSION") : "",
		getenv("TERM_PROGRAM") ? getenv("TERM_PROGRAM") : "");
	fprintf(f, "sonde=%d couleurs=%d utf8=%d sixel=%d kitty=%d cellule=%dx%d\n",
		g_caps.sonde, g_caps.couleurs, g_caps.utf8, g_caps.sixel, g_caps.kitty, g_caps.cell_w, g_caps.cell_h);
	fprintf(f, "palier=%d deploiement=%d\n", rendu_palier(), rendu_deploiement(NULL));
	fclose(f);
}

void	rendu_sonde(void)
{
	memset(&g_caps, 0, sizeof(g_caps));
	probe_env();
	probe_term();
	if (getenv("TDF_IMAGES") && *getenv("TDF_IMAGES"))
	{
		g_caps.kitty = !strcmp(getenv("TDF_IMAGES"), "kitty");
		g_caps.sixel = !strcmp(getenv("TDF_IMAGES"), "sixel");
	}
	caps_write();
}

int	rendu_dispo(int p)
{
	if (p == 0)
		return (0);
	if (p == 1)
		return (g_caps.kitty || g_caps.sixel);
	if (p == 2)
		return (g_caps.utf8);
	return (p == 3);
}

int	rendu_palier(void)
{
	int	p;

	p = g_save.cfg.rendu >= 1 && g_save.cfg.rendu <= 4 ? g_save.cfg.rendu - 1 : 0;
	while (p < 3 && !rendu_dispo(p))
		p++;
	return (p);
}

const char	*rendu_nom(int p)
{
	static const char	*n[4][2] = {{"fenêtre", "window"}, {"images terminal", "terminal images"},
		{"cellules fines", "fine cells"}, {"texte", "text"}};

	return (L(n[p & 3][0], n[p & 3][1]));
}

int	rendu_deploiement(int *det)
{
	int	v[5];
	int	p;
	int	i;
	int	s;

	p = rendu_palier();
	v[0] = g_caps.couleurs == 2 && !g_save.cfg.col256 ? 20 : (g_caps.couleurs ? 10 : 0);
	v[1] = g_caps.utf8 ? 15 : 0;
	v[2] = term_cols() >= 120 && term_rows() >= 36 ? 10 : 5;
	v[3] = p == 1 ? 45 : (p == 2 ? 30 : 0);
	v[4] = g_save.cfg.anim != 2 ? 10 : 0;
	s = 0;
	for (i = 0; i < 5; i++)
	{
		s += v[i];
		if (det)
			det[i] = v[i];
	}
	return (s);
}

static t_rgb	area(const unsigned char *px, int pw, int ph, double x0, double y0, double x1, double y1)
{
	int		x;
	int		y;
	int		a[4];
	long	s[3];
	long	n;

	a[0] = (int)x0;
	a[1] = (int)y0;
	a[2] = (int)x1;
	a[3] = (int)y1;
	if (a[2] <= a[0])
		a[2] = a[0] + 1;
	if (a[3] <= a[1])
		a[3] = a[1] + 1;
	s[0] = 0;
	s[1] = 0;
	s[2] = 0;
	n = 0;
	for (y = a[1]; y < a[3] && y < ph; y++)
		for (x = a[0]; x < a[2] && x < pw; x++)
		{
			s[0] += px[((size_t)y * pw + x) * 3];
			s[1] += px[((size_t)y * pw + x) * 3 + 1];
			s[2] += px[((size_t)y * pw + x) * 3 + 2];
			n++;
		}
	if (!n)
		return (rgb(0, 0, 0));
	return (rgb(s[0] / n, s[1] / n, s[2] / n));
}

static unsigned char	*scale(const unsigned char *px, int pw, int ph, int w, int h)
{
	unsigned char	*o;
	int				x;
	int				y;
	t_rgb			c;

	if (!(o = malloc((size_t)w * h * 3)))
		return (NULL);
	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
		{
			c = area(px, pw, ph, (double)x * pw / w, (double)y * ph / h,
					(double)(x + 1) * pw / w, (double)(y + 1) * ph / h);
			o[((size_t)y * w + x) * 3] = c.r;
			o[((size_t)y * w + x) * 3 + 1] = c.g;
			o[((size_t)y * w + x) * 3 + 2] = c.b;
		}
	return (o);
}

static void	cells(const unsigned char *px, int pw, int ph, int cx, int cy, int cw, int ch, int half)
{
	unsigned char	*s;
	int				x;
	int				y;
	unsigned char	*a;
	unsigned char	*b;

	if (!(s = scale(px, pw, ph, cw, half ? ch * 2 : ch)))
		return ;
	for (y = 0; y < ch; y++)
	{
		at(cx, cy + y);
		for (x = 0; x < cw; x++)
		{
			a = s + ((size_t)(half ? y * 2 : y) * cw + x) * 3;
			b = half ? s + ((size_t)(y * 2 + 1) * cw + x) * 3 : a;
			bgc(rgb(b[0], b[1], b[2]));
			if (half)
			{
				fg(rgb(a[0], a[1], a[2]));
				out("▀");
			}
			else
				out(" ");
		}
	}
	free(s);
	bgc(theme()->bg);
}

typedef struct s_buf
{
	char	*d;
	size_t	n;
	size_t	cap;
}	t_buf;

static void	bput(t_buf *b, const char *s, size_t n)
{
	char	*d;

	if (b->n + n + 1 > b->cap)
	{
		b->cap = (b->n + n + 1) * 2;
		if (!(d = realloc(b->d, b->cap)))
			return ;
		b->d = d;
	}
	memcpy(b->d + b->n, s, n);
	b->n += n;
}

static void	bputs(t_buf *b, const char *s)
{
	bput(b, s, strlen(s));
}

static void	bsend(t_buf *b)
{
	size_t	o;

	for (o = 0; o < b->n; o += 32768)
		term_raw(b->d + o, b->n - o < 32768 ? b->n - o : 32768);
	free(b->d);
	b->d = NULL;
	b->n = 0;
	b->cap = 0;
}

static void	b64(t_buf *b, const unsigned char *s, size_t n)
{
	static const char	t[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char				q[4];
	size_t				i;
	unsigned long		v;

	for (i = 0; i < n; i += 3)
	{
		v = (unsigned long)s[i] << 16;
		if (i + 1 < n)
			v |= (unsigned long)s[i + 1] << 8;
		if (i + 2 < n)
			v |= s[i + 2];
		q[0] = t[(v >> 18) & 63];
		q[1] = t[(v >> 12) & 63];
		q[2] = i + 1 < n ? t[(v >> 6) & 63] : '=';
		q[3] = i + 2 < n ? t[v & 63] : '=';
		bput(b, q, 4);
	}
}

static void	kitty_send(const unsigned char *s, int w, int h, int cx, int cy, int cw, int ch)
{
	t_buf	e;
	t_buf	b;
	char	hd[160];
	size_t	o;
	size_t	k;

	memset(&e, 0, sizeof(e));
	memset(&b, 0, sizeof(b));
	b64(&e, s, (size_t)w * h * 3);
	snprintf(hd, sizeof(hd), "\033_Ga=d,d=I,i=4242,q=2\033\\\033[%d;%dH", cy + 1, cx + 1);
	bputs(&b, hd);
	for (o = 0; o < e.n; o += 4096)
	{
		k = e.n - o < 4096 ? e.n - o : 4096;
		if (o == 0)
			snprintf(hd, sizeof(hd), "\033_Ga=T,f=24,s=%d,v=%d,c=%d,r=%d,i=4242,q=2,C=1,m=%d;",
				w, h, cw, ch, o + k < e.n);
		else
			snprintf(hd, sizeof(hd), "\033_Gm=%d;", o + k < e.n);
		bputs(&b, hd);
		bput(&b, e.d + o, k);
		bputs(&b, "\033\\");
	}
	free(e.d);
	bsend(&b);
	g_kitty_used = 1;
}

typedef struct s_pal
{
	int				n;
	int				c[256][3];
	short			map[32768];
}	t_pal;

static int	key15(const unsigned char *p)
{
	return ((p[0] >> 3) << 10 | (p[1] >> 3) << 5 | (p[2] >> 3));
}

static void	pal_build(t_pal *pl, const unsigned char *s, size_t n)
{
	static unsigned	cnt[32768];
	static long		sum[32768][3];
	int				i;
	int				j;
	int				best;
	int				k;
	long			d;
	long			bd;

	memset(cnt, 0, sizeof(cnt));
	memset(sum, 0, sizeof(sum));
	for (i = 0; (size_t)i < n; i++)
	{
		k = key15(s + (size_t)i * 3);
		cnt[k]++;
		sum[k][0] += s[(size_t)i * 3];
		sum[k][1] += s[(size_t)i * 3 + 1];
		sum[k][2] += s[(size_t)i * 3 + 2];
	}
	for (pl->n = 0; pl->n < 256; pl->n++)
	{
		best = -1;
		for (k = 0; k < 32768; k++)
			if (cnt[k] && (best < 0 || cnt[k] > cnt[best]))
				best = k;
		if (best < 0)
			break ;
		for (j = 0; j < 3; j++)
			pl->c[pl->n][j] = sum[best][j] / cnt[best];
		cnt[best] = 0;
	}
	for (k = 0; k < 32768; k++)
		pl->map[k] = -1;
	for (i = 0; (size_t)i < n; i++)
	{
		k = key15(s + (size_t)i * 3);
		if (pl->map[k] >= 0)
			continue ;
		bd = -1;
		for (j = 0; j < pl->n; j++)
		{
			d = (long)(pl->c[j][0] - s[(size_t)i * 3]) * (pl->c[j][0] - s[(size_t)i * 3])
				+ (long)(pl->c[j][1] - s[(size_t)i * 3 + 1]) * (pl->c[j][1] - s[(size_t)i * 3 + 1])
				+ (long)(pl->c[j][2] - s[(size_t)i * 3 + 2]) * (pl->c[j][2] - s[(size_t)i * 3 + 2]);
			if (bd < 0 || d < bd)
			{
				bd = d;
				pl->map[k] = j;
			}
		}
	}
}

static void	sixel_band(t_buf *b, const unsigned char *ix, int w, int h, int y0)
{
	char	used[256];
	char	t[32];
	int		c;
	int		x;
	int		k;
	int		run;
	int		cur;
	int		bits;

	memset(used, 0, sizeof(used));
	for (k = 0; k < 6 && y0 + k < h; k++)
		for (x = 0; x < w; x++)
			used[ix[(size_t)(y0 + k) * w + x]] = 1;
	for (c = 0; c < 256; c++)
	{
		if (!used[c])
			continue ;
		snprintf(t, sizeof(t), "#%d", c);
		bputs(b, t);
		run = 0;
		cur = -1;
		for (x = 0; x <= w; x++)
		{
			bits = -1;
			if (x < w)
			{
				bits = 0;
				for (k = 0; k < 6 && y0 + k < h; k++)
					if (ix[(size_t)(y0 + k) * w + x] == c)
						bits |= 1 << k;
			}
			if (bits == cur)
			{
				run++;
				continue ;
			}
			if (run > 3)
			{
				snprintf(t, sizeof(t), "!%d%c", run, 63 + cur);
				bputs(b, t);
			}
			else
				while (run-- > 0)
				{
					t[0] = 63 + cur;
					bput(b, t, 1);
				}
			cur = bits;
			run = 1;
		}
		bputs(b, "$");
	}
	bputs(b, "-");
}

static void	sixel_send(const unsigned char *s, int w, int h, int cx, int cy)
{
	unsigned char	*ix;
	t_pal			*pl;
	t_buf			b;
	char			t[64];
	size_t			i;
	int				c;

	ix = malloc((size_t)w * h);
	pl = malloc(sizeof(*pl));
	if (!ix || !pl)
	{
		free(ix);
		free(pl);
		return ;
	}
	pal_build(pl, s, (size_t)w * h);
	for (i = 0; i < (size_t)w * h; i++)
		ix[i] = pl->map[key15(s + i * 3)];
	memset(&b, 0, sizeof(b));
	snprintf(t, sizeof(t), "\033[%d;%dH\033P0;1;0q\"1;1;%d;%d", cy + 1, cx + 1, w, h);
	bputs(&b, t);
	for (c = 0; c < pl->n; c++)
	{
		snprintf(t, sizeof(t), "#%d;2;%d;%d;%d", c, pl->c[c][0] * 100 / 255,
			pl->c[c][1] * 100 / 255, pl->c[c][2] * 100 / 255);
		bputs(&b, t);
	}
	for (c = 0; c < h; c += 6)
		sixel_band(&b, ix, w, h, c);
	bputs(&b, "\033\\");
	free(ix);
	free(pl);
	bsend(&b);
}

static unsigned long	sum_of(const unsigned char *p, size_t n)
{
	unsigned long	h;
	size_t			i;

	h = 1469598103934665603UL;
	for (i = 0; i < n; i++)
		h = (h ^ p[i]) * 1099511628211UL;
	return (h);
}

void	rendu_image_terminal(const unsigned char *px, int pw, int ph, int cx, int cy, int cw, int ch)
{
	rendu_image(px, pw, ph, cx, cy, cw, ch);
}

void	rendu_image(const unsigned char *px, int pw, int ph, int cx, int cy, int cw, int ch)
{
	int				p;
	unsigned long	s;

	if (!px || pw <= 0 || ph <= 0 || cw <= 0 || ch <= 0)
		return ;
	p = rendu_palier();
	if (p != 1)
	{
		cells(px, pw, ph, cx, cy, cw, ch, p != 3);
		return ;
	}
	fill(cx, cy, cw, ch, theme()->bg);
	s = sum_of(px, (size_t)pw * ph * 3) ^ ((unsigned long)cx << 48 | (unsigned long)cy << 32
			| (unsigned long)cw << 16 | (unsigned long)ch);
	if (g_sh.px && g_sh.sum == s)
	{
		g_sh.want = 1;
		return ;
	}
	free(g_sh.px);
	g_sh.px = malloc((size_t)pw * ph * 3);
	if (!g_sh.px)
		return ;
	memcpy(g_sh.px, px, (size_t)pw * ph * 3);
	g_sh.pw = pw;
	g_sh.ph = ph;
	g_sh.x = cx;
	g_sh.y = cy;
	g_sh.w = cw;
	g_sh.h = ch;
	g_sh.sum = s;
	g_sh.on = 0;
	g_sh.want = 1;
}

void	rendu_frame(void)
{
	g_sh.want = 0;
}

static void	hide(void)
{
	if (g_kitty_used)
		term_raw("\033_Ga=d,d=A,q=2\033\\", 17);
	term_dirty(g_sh.x, g_sh.y, g_sh.w, g_sh.h);
	g_sh.on = 0;
}

void	rendu_apres_flush(void)
{
	unsigned char	*s;
	int				w;
	int				h;
	int				cw;
	int				ch;

	if (!g_sh.px)
		return ;
	if (!g_sh.want)
	{
		if (g_sh.on)
			hide();
		free(g_sh.px);
		g_sh.px = NULL;
		g_sh.sum = 0;
		return ;
	}
	if (g_sh.on && g_sh.gen == term_gen())
		return ;
	cw = g_caps.cell_w > 0 ? g_caps.cell_w : 10;
	ch = g_caps.cell_h > 0 ? g_caps.cell_h : 20;
	w = g_sh.w * cw;
	h = g_sh.h * ch;
	if (w > 2400 || h > 1600)
	{
		w = w / 2;
		h = h / 2;
	}
	if (!(s = scale(g_sh.px, g_sh.pw, g_sh.ph, w, h)))
		return ;
	if (g_caps.kitty)
		kitty_send(s, w, h, g_sh.x, g_sh.y, g_sh.w, g_sh.h);
	else
		sixel_send(s, w, h - h % 6, g_sh.x, g_sh.y);
	free(s);
	term_raw("\033[H", 3);
	g_sh.on = 1;
	g_sh.gen = term_gen();
}

void	rendu_fin(void)
{
	if (g_kitty_used)
		(void)!write(1, "\033_Ga=d,d=A,q=2\033\\", 17);
}

static void	put(unsigned char *p, int w, int h, int x, int y, t_rgb c, double k)
{
	unsigned char	*q;

	if (x < 0 || y < 0 || x >= w || y >= h)
		return ;
	q = p + ((size_t)y * w + x) * 3;
	q[0] = q[0] + (c.r - q[0]) * k;
	q[1] = q[1] + (c.g - q[1]) * k;
	q[2] = q[2] + (c.b - q[2]) * k;
}

static void	glow(unsigned char *p, int w, int h, double cx, double cy, double r, t_rgb c)
{
	int		x;
	int		y;
	double	d;

	for (y = cy - r; y <= cy + r; y++)
		for (x = cx - r; x <= cx + r; x++)
		{
			d = ((x - cx) * (x - cx) + (y - cy) * (y - cy)) / (r * r);
			if (d < 1)
				put(p, w, h, x, y, c, (1 - d) * (1 - d) * 0.85);
		}
}

static unsigned char	*apercu(int w, int h)
{
	unsigned char	*p;
	int				x;
	int				y;
	int				tw;
	unsigned		s;
	double			t;

	if (!(p = malloc((size_t)w * h * 3)))
		return (NULL);
	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
		{
			t = (double)y / h;
			p[((size_t)y * w + x) * 3] = 11 + 10 * t;
			p[((size_t)y * w + x) * 3 + 1] = 13 + 37 * t;
			p[((size_t)y * w + x) * 3 + 2] = 18 + 56 * t;
		}
	glow(p, w, h, w * 0.78, h * 0.92, h * 0.55, rgb(255, 140, 50));
	for (y = h * 0.86; y < h; y++)
		for (x = 0; x < w; x++)
			put(p, w, h, x, y, rgb(8, 9, 12), 1);
	tw = w / 7;
	for (y = h * 0.12; y < h * 0.86; y++)
		for (x = w / 2 - tw / 2 - (y > h * 0.6 ? (y - h * 0.6) / 4 : 0);
			x < w / 2 + tw / 2 + (y > h * 0.6 ? (y - h * 0.6) / 4 : 0); x++)
			put(p, w, h, x, y, rgb(16, 18, 24), 1);
	s = 7;
	for (y = h * 0.18; y < h * 0.8; y += h / 14)
		for (x = w / 2 - tw / 3; x < w / 2 + tw / 3; x += tw / 3)
		{
			s = s * 1103515245u + 12345u;
			if ((s >> 16) % 3)
				glow(p, w, h, x + tw / 6, y, h / 18.0, rgb(120, 230, 255));
		}
	glow(p, w, h, w / 2, h * 0.08, h / 12.0, rgb(230, 220, 120));
	return (p);
}

static void	ligne(int x, int y, const char *nom, int ok, const char *val, int pts)
{
	char	b[32];

	text_at(x, y, ok ? rgb(120, 230, 160) : rgb(255, 120, 120), ok ? "✓" : "✗");
	text_at(x + 2, y, theme()->text, nom);
	text_clip(x + 24, y, theme()->dim, val, 12);
	if (pts >= 0)
	{
		snprintf(b, sizeof(b), "+%d", pts);
		text_at(x + 37, y, theme()->accent, b);
	}
}

static const char	*terminal_nom(char *b, size_t n)
{
	if (getenv("VTE_VERSION"))
		snprintf(b, n, "VTE %d.%d", atoi(getenv("VTE_VERSION")) / 10000,
			atoi(getenv("VTE_VERSION")) / 100 % 100);
	else if (getenv("TERM_PROGRAM"))
		snprintf(b, n, "%s", getenv("TERM_PROGRAM"));
	else
		snprintf(b, n, "%s", getenv("TERM") ? getenv("TERM") : "?");
	return (b);
}

static void	conseil(int x, int y, int *v)
{
	const char	*s;

	s = NULL;
	if (!v[0])
		s = L("Couleurs : lance avec COLORTERM=truecolor si ton terminal les gère.",
				"Colors: run with COLORTERM=truecolor if your terminal supports it.");
	else if (!v[1])
		s = L("UTF-8 : lance avec LANG=fr_FR.UTF-8 (ou en_US.UTF-8).",
				"UTF-8: run with LANG=en_US.UTF-8.");
	else if (!rendu_dispo(1))
		s = L("Images : ce terminal n'affiche pas d'images, les cellules fines prennent le relais.",
				"Images: this terminal shows no images, fine cells take over.");
	else if (v[2] < 10)
		s = L("Taille : agrandis le terminal à 120 x 36 ou plus.",
				"Size: make the terminal 120 x 36 or bigger.");
	else if (!v[4])
		s = L("Animations : remets-les dans Paramètres.", "Animations: turn them back on in Settings.");
	if (s)
		text_wrap(x, y, y + 2, term_cols() - x - 2, theme()->dim, s);
}

void	screen_rendu(void)
{
	static unsigned char	*img;
	char					b[64];
	char					t[32];
	int						v[5];
	int						k;
	int						x;
	int						pw;
	int						d;

	if (!img)
		img = apercu(320, 180);
	cls();
	while (1)
	{
		frame();
		bgc(theme()->bg);
		x = 2;
		center(1, theme()->accent2, L("D É P L O I E M E N T", "D E P L O Y M E N T"));
		d = rendu_deploiement(v);
		snprintf(b, sizeof(b), "%s : %s", L("Terminal", "Terminal"), terminal_nom(t, sizeof(t)));
		text_at(x, 3, theme()->dim, b);
		ligne(x, 5, L("Couleurs 24 bits", "24-bit colors"), v[0] == 20,
			g_caps.couleurs == 2 ? (g_save.cfg.col256 ? "256 (choix)" : "24 bits") : (g_caps.couleurs ? "256" : "16"), v[0]);
		ligne(x, 6, L("Caractères UTF-8", "UTF-8 characters"), g_caps.utf8, g_caps.utf8 ? L("oui", "yes") : L("non", "no"), v[1]);
		snprintf(b, sizeof(b), "%d x %d", term_cols(), term_rows());
		ligne(x, 7, L("Taille du terminal", "Terminal size"), v[2] == 10, b, v[2]);
		ligne(x, 8, L("Images terminal", "Terminal images"), rendu_dispo(1),
			g_caps.kitty ? "kitty" : (g_caps.sixel ? "sixel" : L("non", "no")), rendu_palier() == 1 ? v[3] : -1);
		ligne(x, 9, L("Cellules fines", "Fine cells"), rendu_dispo(2),
			rendu_dispo(2) ? L("oui", "yes") : L("non", "no"), rendu_palier() == 2 ? v[3] : -1);
		ligne(x, 10, L("Animations", "Animations"), v[4] > 0, v[4] ? L("oui", "yes") : L("non", "no"), v[4]);
		snprintf(b, sizeof(b), "%s : %s%s", L("Palier", "Tier"), rendu_nom(rendu_palier()),
			g_save.cfg.rendu ? L(" (choisi)", " (chosen)") : " (auto)");
		text_at(x, 12, theme()->text, b);
		snprintf(b, sizeof(b), "%d %%", d);
		bar(x, 13, 30, d / 100.0, theme()->accent, theme()->dim);
		text_at(x + 32, 13, theme()->accent2, b);
		conseil(x, 15, v);
		pw = term_cols() - 44 - 2;
		d = pw * 9 / 32 > 12 ? 12 : pw * 9 / 32;
		if (pw >= 20)
			rendu_image(img, 320, 180, 44 + (pw - d * 32 / 9) / 2, 3, d * 32 / 9, d);
		foot_line(x, L("◀ ▶ changer le palier   Échap : retour", "◀ ▶ change tier   Esc: back"));
		flush();
		k = key_dir(key_read(120));
		if (key_quit(k) || k == 127 || k == 8 || k == '\r' || k == '\n')
			break ;
		if (k == 1002 || k == 1003)
		{
			x = k == 1002 ? 1 : 4;
			k = g_save.cfg.rendu;
			do
				k = (k + x) % 5;
			while (k && !rendu_dispo(k - 1));
			g_save.cfg.rendu = k;
			save_write();
			cls();
		}
	}
	cls();
}
