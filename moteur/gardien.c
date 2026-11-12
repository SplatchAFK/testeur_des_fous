#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

typedef struct s_gdef
{
	const char		*img;
	int				x;
	int				y;
	int				ney;
	int				ey[5][2];
	unsigned char	t[3];
	double			sway;
}	t_gdef;

static const t_gdef	g_gdef[3] = {
	{"gardien_memoire", 312, 113, 2, {{432, 151}, {451, 150}}, {150, 255, 190}, 0.4},
	{"gardien_tisseur", 343, 98, 2, {{495, 151}, {509, 153}}, {255, 220, 120}, 0.6},
	{"gardien_hydre", 308, 103, 5, {{345, 182}, {394, 136}, {455, 122}, {514, 151}, {563, 204}}, {150, 230, 255}, 1.6},
};

static unsigned char	*g_img;
static int				g_w;
static int				g_h;
static unsigned char	*g_spr[3];
static int				g_sw[3];
static int				g_sh[3];

static void	melange(unsigned char *q, int r, int g, int b, double k)
{
	if (k <= 0)
		return ;
	if (k > 1)
		k = 1;
	q[0] = q[0] + (r - q[0]) * k;
	q[1] = q[1] + (g - q[1]) * k;
	q[2] = q[2] + (b - q[2]) * k;
}

static void	coller(const t_pic *s, int ox, int oy)
{
	int				x;
	int				y;
	int				tx;
	int				ty;
	unsigned char	a;

	for (y = 0; y < s->h; y++)
	{
		ty = oy + y;
		if (ty < 0 || ty >= g_h)
			continue ;
		for (x = 0; x < s->w; x++)
		{
			tx = ox + x;
			a = s->a[(size_t)y * s->w + x];
			if (tx < 0 || tx >= g_w || a < 8)
				continue ;
			melange(g_img + ((size_t)ty * g_w + tx) * 3, s->rgb[((size_t)y * s->w + x) * 3],
				s->rgb[((size_t)y * s->w + x) * 3 + 1], s->rgb[((size_t)y * s->w + x) * 3 + 2], a / 255.0);
		}
	}
}

static void	halo(double cx, double cy, double r, const unsigned char *c, double k)
{
	int		x;
	int		y;
	double	d;

	for (y = (int)(cy - r * 2); y <= (int)(cy + r * 2); y++)
		for (x = (int)(cx - r * 2); x <= (int)(cx + r * 2); x++)
		{
			if (x < 0 || y < 0 || x >= g_w || y >= g_h)
				continue ;
			d = ((x - cx) * (x - cx) + (y - cy) * (y - cy)) / (r * r);
			if (d < 4)
				melange(g_img + ((size_t)y * g_w + x) * 3, c[0], c[1], c[2], k * (1 - d / 4) * (1 - d / 4));
		}
}

static unsigned int	hache(unsigned int v)
{
	v ^= v >> 16;
	v *= 0x7feb352dU;
	v ^= v >> 15;
	v *= 0x846ca68bU;
	return (v ^ (v >> 16));
}

const unsigned char	*gardien_image(int p, long ms, int *w, int *h)
{
	static const unsigned char	ame[3] = {118, 222, 238};
	const t_pic					*bg;
	const t_pic					*fg;
	double						t;
	double						dx;
	double						dy;
	double						pu;
	int							i;

	if (p < 0 || p > 2 || !(bg = art_pic("salle_gardiens")) || !(fg = art_pic(g_gdef[p].img)))
		return (NULL);
	if (!g_img || g_w != bg->w || g_h != bg->h)
	{
		free(g_img);
		g_w = bg->w;
		g_h = bg->h;
		g_img = malloc((size_t)g_w * g_h * 3);
		if (!g_img)
			return (NULL);
	}
	memcpy(g_img, bg->rgb, (size_t)g_w * g_h * 3);
	t = ms / 1000.0 * 2 * 3.14159265358979 / 1.9;
	dy = sin(t) * 2.2;
	dx = sin(t * 0.5) * g_gdef[p].sway;
	coller(fg, g_gdef[p].x + (int)lround(dx), g_gdef[p].y + (int)lround(dy));
	pu = 0.5 + 0.5 * sin(t * 2);
	for (i = 0; i < g_gdef[p].ney; i++)
		halo(g_gdef[p].ey[i][0] + dx, g_gdef[p].ey[i][1] + dy, 3 + 3 * pu, g_gdef[p].t, 0.35 + 0.45 * pu);
	for (i = 0; i < 46; i++)
		halo(hache(i * 3 + 1) % g_w + sin(ms / 900.0 + i) * 4,
			fmod(hache(i * 3 + 2) % g_h - ms / 1000.0 * (4 + hache(i * 3 + 3) % 9) + g_h * 100.0, g_h),
			1.2, ame, 0.35 + 0.3 * sin(ms / 300.0 + i));
	*w = g_w;
	*h = g_h;
	return (g_img);
}

static int	reduire(int p, int mw, int mh)
{
	const t_pic	*s;
	double		k;
	int			x;
	int			y;
	int			sx;
	int			sy;
	double		acc[4];
	int			n;
	int			u;
	int			v;

	if (g_spr[p])
		return (1);
	if (!(s = art_pic(g_gdef[p].img)))
		return (0);
	k = fmin((double)mw / s->w, (double)mh / s->h);
	g_sw[p] = (int)(s->w * k);
	g_sh[p] = (int)(s->h * k);
	if (!(g_spr[p] = malloc((size_t)g_sw[p] * g_sh[p] * 4)))
		return (0);
	for (y = 0; y < g_sh[p]; y++)
		for (x = 0; x < g_sw[p]; x++)
		{
			memset(acc, 0, sizeof(acc));
			n = 0;
			for (v = (int)(y / k); v < (int)((y + 1) / k) && v < s->h; v++)
				for (u = (int)(x / k); u < (int)((x + 1) / k) && u < s->w; u++)
				{
					sx = s->a[(size_t)v * s->w + u];
					sy = ((size_t)v * s->w + u) * 3;
					acc[0] += s->rgb[sy] * sx;
					acc[1] += s->rgb[sy + 1] * sx;
					acc[2] += s->rgb[sy + 2] * sx;
					acc[3] += sx;
					n++;
				}
			sx = ((y * g_sw[p]) + x) * 4;
			g_spr[p][sx + 3] = n ? (unsigned char)(acc[3] / n) : 0;
			for (u = 0; u < 3; u++)
				g_spr[p][sx + u] = acc[3] > 0 ? (unsigned char)fmin(255, acc[u] / acc[3] * 1.35) : 0;
		}
	return (1);
}

int	gardien_sprite(t_canvas *cv, int p, int x, int y, int mw, int mh, double flash)
{
	int				i;
	int				j;
	unsigned char	*q;
	t_rgb			c;

	if (p < 0 || p > 2 || !reduire(p, mw, mh))
		return (0);
	x += (mw - g_sw[p]) / 2;
	y += mh - g_sh[p];
	for (j = 0; j < g_sh[p]; j++)
		for (i = 0; i < g_sw[p]; i++)
		{
			q = g_spr[p] + ((size_t)j * g_sw[p] + i) * 4;
			if (q[3] < 110)
				continue ;
			c = rgb(q[0], q[1], q[2]);
			if (flash > 0)
				c = mix(c, rgb(255, 255, 255), flash > 1 ? 1 : flash);
			cv_px(cv, x + i, y + j, c);
		}
	return (1);
}

void	gardien_fenetre(int p, long ms)
{
	static long	dernier;
	const unsigned char	*px;
	int			w;
	int			h;

	if (rendu_palier() != 0 || (ms - dernier < 66 && ms >= dernier))
		return ;
	dernier = ms;
	if ((px = gardien_image(p, ms, &w, &h)))
		rendu_fenetre(px, w, h);
}
