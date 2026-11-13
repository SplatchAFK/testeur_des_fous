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

static unsigned char	*g_spr[3];
static int				g_sw[3];
static int				g_sh[3];

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

