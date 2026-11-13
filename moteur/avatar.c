#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

t_rgb	pal(char c, t_rgb tint, t_rgb eye)
{
	const char	*p;

	if (c == '1')
		return (tint);
	if (c == '2')
		return (mix(tint, rgb(0, 0, 0), 0.45));
	if (c == '3')
		return (mix(tint, rgb(255, 255, 255), 0.45));
	if (c == 'E')
		return (eye);
	p = strchr(g_pal_keys, c);
	if (!p)
		return (rgb(255, 0, 255));
	return (g_pal_vals[p - g_pal_keys]);
}

void	cv_init(t_canvas *c, int w, int h)
{
	c->w = w;
	c->h = h;
	c->px = calloc((size_t)w * h, sizeof(t_rgb));
	c->a = calloc((size_t)w * h, 1);
}

void	cv_free(t_canvas *c)
{
	free(c->px);
	free(c->a);
	c->px = NULL;
	c->a = NULL;
}

void	cv_clear(t_canvas *c)
{
	memset(c->a, 0, (size_t)c->w * c->h);
}

void	cv_px(t_canvas *c, int x, int y, t_rgb col)
{
	if (x < 0 || y < 0 || x >= c->w || y >= c->h)
		return ;
	c->px[y * c->w + x] = col;
	c->a[y * c->w + x] = 1;
}

void	cv_spr(t_canvas *c, const t_spr *s, int x, int y, t_rgb tint, t_rgb eye,
		double flash)
{
	int		i;
	int		j;
	char	ch;
	t_rgb	col;

	if (!s->rows)
		return ;
	for (j = 0; j < s->h; j++)
	{
		for (i = 0; i < s->w && s->rows[j][i]; i++)
		{
			ch = s->rows[j][i];
			if (ch == '.')
				continue ;
			col = pal(ch, tint, eye);
			if (flash > 0)
				col = mix(col, rgb(255, 255, 255), flash);
			cv_px(c, x + s->ox + i, y + s->oy + j, col);
		}
	}
}

int	cv_row(const t_canvas *c, int row, t_rgb bg, char *buf, size_t sz)
{
	int		x;
	int		k;
	int		top;
	int		bot;
	t_rgb	a;
	t_rgb	b;
	size_t	n;

	n = 0;
	for (x = 0; x < c->w && n + 64 < sz; x++)
	{
		top = (row * 2) * c->w + x;
		bot = (row * 2 + 1) * c->w + x;
		a = c->a[top] ? c->px[top] : bg;
		b = (row * 2 + 1 < c->h && c->a[bot]) ? c->px[bot] : bg;
		k = color_seq(buf + n, a, 0);
		n += k;
		k = color_seq(buf + n, b, 1);
		n += k;
		memcpy(buf + n, "▀", 3);
		n += 3;
	}
	buf[n] = '\0';
	return ((int)n);
}

void	cv_print(const t_canvas *c, int sx, int sy, t_rgb bg)
{
	static char	buf[1 << 16];
	int			r;

	for (r = 0; r < (c->h + 1) / 2; r++)
	{
		cv_row(c, r, bg, buf, sizeof(buf));
		at(sx, sy + r);
		out(buf);
	}
	bgc(theme()->bg);
}

t_rgb	item_color(int item, int col)
{
	if (item < 0)
		return (rgb(255, 255, 255));
	if (col <= 0 || col >= 10)
		return (g_items[item].base);
	return (g_swatch[col]);
}

static void	aura(t_canvas *c, int x, int y, int item, long t)
{
	const char	*id;
	int			k;
	int			px;
	int			py;
	double		ph;
	unsigned	h;

	if (item < 0)
		return ;
	id = g_items[item].id;
	if (!strcmp(id, "aura_aucune"))
		return ;
	for (k = 0; k < 14; k++)
	{
		h = (unsigned)k * 2654435761u;
		ph = ((t / 40 + (long)(h % 997)) % 60) / 60.0;
		px = x + 2 + (int)(h % 24);
		py = y + 30 - (int)(ph * 30);
		if (!strcmp(id, "aura_braises"))
		{
			cv_px(c, px, py, mix(rgb(255, 210, 120), rgb(200, 70, 30), ph));
			if (k % 2)
				cv_px(c, px, py + 1, mix(rgb(234, 142, 72), rgb(90, 30, 14), ph));
		}
		else if (!strcmp(id, "aura_brume"))
			cv_px(c, px + (int)(2 * sin(ph * 6 + k)), y + 28 + (int)(3 * sin(ph * 6.28 + k)),
				mix(rgb(200, 214, 222), rgb(60, 70, 84), ph));
		else
			cv_px(c, px + (int)(sin(ph * 6 + k)), py, mix(rgb(160, 240, 250), rgb(60, 140, 160), ph));
	}
}

typedef struct s_avcache
{
	unsigned long	cle;
	int				ok;
	int				w;
	int				h;
	unsigned char	*fond;
	unsigned char	*fig;
	unsigned char	*grand;
}	t_avcache;

static t_avcache	g_av;

static void	empiler(unsigned char *dst, const t_pic *p, int w, int h)
{
	int		i;
	double	a;

	if (!p || p->w != w || p->h != h)
		return ;
	for (i = 0; i < w * h; i++)
	{
		a = p->a[i] / 255.0;
		if (a <= 0)
			continue ;
		dst[i * 4] = dst[i * 4] + (p->rgb[i * 3] - dst[i * 4]) * a;
		dst[i * 4 + 1] = dst[i * 4 + 1] + (p->rgb[i * 3 + 1] - dst[i * 4 + 1]) * a;
		dst[i * 4 + 2] = dst[i * 4 + 2] + (p->rgb[i * 3 + 2] - dst[i * 4 + 2]) * a;
		dst[i * 4 + 3] = dst[i * 4 + 3] + (255 - dst[i * 4 + 3]) * a;
	}
}

static unsigned char	*reduire_av(const unsigned char *src, int sw, int sh, int w, int h)
{
	unsigned char	*o;
	int				x;
	int				y;
	int				u;
	int				v;
	double			acc[4];
	int				n;

	if (!(o = calloc((size_t)w * h, 4)))
		return (NULL);
	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
		{
			acc[0] = 0;
			acc[1] = 0;
			acc[2] = 0;
			acc[3] = 0;
			n = 0;
			for (v = y * sh / h; v < (y + 1) * sh / h; v++)
				for (u = x * sw / w; u < (x + 1) * sw / w; u++)
				{
					acc[0] += src[(v * sw + u) * 4] * (double)src[(v * sw + u) * 4 + 3];
					acc[1] += src[(v * sw + u) * 4 + 1] * (double)src[(v * sw + u) * 4 + 3];
					acc[2] += src[(v * sw + u) * 4 + 2] * (double)src[(v * sw + u) * 4 + 3];
					acc[3] += src[(v * sw + u) * 4 + 3];
					n++;
				}
			if (!n || acc[3] <= 0)
				continue ;
			o[(y * w + x) * 4] = (unsigned char)fmin(255, acc[0] / acc[3] * 1.12);
			o[(y * w + x) * 4 + 1] = (unsigned char)fmin(255, acc[1] / acc[3] * 1.12);
			o[(y * w + x) * 4 + 2] = (unsigned char)fmin(255, acc[2] / acc[3] * 1.12);
			o[(y * w + x) * 4 + 3] = (unsigned char)(acc[3] / n);
		}
	return (o);
}

static int	composer_av(const t_look *lk)
{
	static const int	ordre[] = {SL_CAPE, SL_JAMBES, SL_ARMURE, SL_CASQUE, SL_YEUX, SL_DECO,
		SL_ARME, SL_MAIN, SL_FAMILIER};
	unsigned long		cle;
	const t_pic			*p;
	unsigned char		*fig;
	unsigned char		*fd;
	char				nom[64];
	int					k;
	int					it;
	int					w;
	int					h;

	cle = 5381;
	for (k = 0; k < NSLOT; k++)
		cle = cle * 33 + (unsigned long)(lk->eq[k] + 2);
	if (g_av.cle == cle && g_av.fig)
		return (g_av.ok);
	g_av.cle = cle;
	free(g_av.fond);
	free(g_av.fig);
	free(g_av.grand);
	g_av.fond = NULL;
	g_av.fig = NULL;
	g_av.grand = NULL;
	g_av.ok = 0;
	if (!(p = art_pic("av_casque_cabosse")))
		return (0);
	w = p->w;
	h = p->h;
	fig = calloc((size_t)w * h, 4);
	fd = calloc((size_t)w * h, 4);
	if (!fig || !fd)
	{
		free(fig);
		free(fd);
		return (0);
	}
	it = lk->eq[SL_FOND];
	if (it >= 0 && strcmp(g_items[it].id, "fond_aucun"))
	{
		snprintf(nom, sizeof(nom), "av_%s", g_items[it].id);
		empiler(fd, art_pic(nom), w, h);
	}
	for (k = 0; k < (int)(sizeof(ordre) / sizeof(ordre[0])); k++)
	{
		it = lk->eq[ordre[k]];
		if (it < 0 || strstr(g_items[it].id, "_aucun"))
			continue ;
		snprintf(nom, sizeof(nom), "av_%s", g_items[it].id);
		empiler(fig, art_pic(nom), w, h);
	}
	g_av.w = w;
	g_av.h = h;
	g_av.grand = malloc((size_t)w * h * 3);
	if (g_av.grand)
		for (k = 0; k < w * h; k++)
		{
			it = fig[k * 4 + 3];
			g_av.grand[k * 3] = (fd[k * 4] * (255 - it) + fig[k * 4] * it) / 255 + (fd[k * 4 + 3] ? 0 : (12 * (255 - it)) / 255);
			g_av.grand[k * 3 + 1] = (fd[k * 4 + 1] * (255 - it) + fig[k * 4 + 1] * it) / 255 + (fd[k * 4 + 3] ? 0 : (16 * (255 - it)) / 255);
			g_av.grand[k * 3 + 2] = (fd[k * 4 + 2] * (255 - it) + fig[k * 4 + 2] * it) / 255 + (fd[k * 4 + 3] ? 0 : (26 * (255 - it)) / 255);
		}
	g_av.fond = reduire_av(fd, w, h, 28, 32);
	g_av.fig = reduire_av(fig, w, h, 23, 32);
	free(fig);
	free(fd);
	g_av.ok = g_av.fond && g_av.fig;
	return (g_av.ok);
}

static void	poser(t_canvas *c, const unsigned char *s, int w, int h, int x, int y, int seuil)
{
	int	i;
	int	j;

	for (j = 0; j < h; j++)
		for (i = 0; i < w; i++)
			if (s[(j * w + i) * 4 + 3] >= seuil)
				cv_px(c, x + i, y + j, rgb(s[(j * w + i) * 4], s[(j * w + i) * 4 + 1], s[(j * w + i) * 4 + 2]));
}

void	avatar_draw(t_canvas *c, int x, int y, const t_look *lk, long t)
{
	int	bob;

	bob = (t / 600) % 2;
	if (!composer_av(lk))
		return ;
	poser(c, g_av.fond, 28, 32, x, y, 100);
	aura(c, x, y, lk->eq[SL_AURA], t);
	poser(c, g_av.fig, 23, 32, x + 2, y + bob, 110);
}
