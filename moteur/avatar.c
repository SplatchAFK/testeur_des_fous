#include <math.h>
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

int	item_find(const char *id)
{
	int	i;

	for (i = 0; i < g_nitems; i++)
		if (!strcmp(g_items[i].id, id))
			return (i);
	return (-1);
}

t_rgb	item_color(int item, int col)
{
	if (item < 0)
		return (rgb(255, 255, 255));
	if (col <= 0 || col >= 10)
		return (g_items[item].base);
	return (g_swatch[col]);
}

void	look_default(t_look *lk)
{
	static const char	*def[NSLOT] = {"fond_aucun", "cape_classique",
		"jambes_jambieres", "armure_cotte", "casque_rond", "yeux_points",
		"deco_aucun", "arme_epee", "main_bouclier", "familier_aucun", "aura_aucune"};
	int					i;

	for (i = 0; i < NSLOT; i++)
	{
		lk->eq[i] = item_find(def[i]);
		lk->col[i] = 0;
	}
}

static void	aura(t_canvas *c, int x, int y, int item, t_rgb col, long t)
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
		if (!strcmp(id, "aura_flammes"))
		{
			cv_px(c, px, py, mix(rgb(255, 230, 80), rgb(255, 60, 20), ph));
			cv_px(c, px, py + 1, mix(rgb(255, 160, 40), rgb(120, 20, 10), ph));
		}
		else if (!strcmp(id, "aura_bulles"))
		{
			cv_px(c, px, py, mix(rgb(200, 240, 255), col, 0.3));
			if (k % 3 == 0)
				cv_px(c, px + 1, py, rgb(255, 255, 255));
		}
		else if (!strcmp(id, "aura_notes"))
		{
			cv_px(c, px, py, col);
			cv_px(c, px + 1, py - 1, col);
			cv_px(c, px + 1, py - 2, col);
		}
		else if (!strcmp(id, "aura_coeurs"))
		{
			cv_px(c, px, py, rgb(255, 90, 150));
			cv_px(c, px + 2, py, rgb(255, 90, 150));
			cv_px(c, px + 1, py + 1, rgb(255, 90, 150));
		}
		else if (!strcmp(id, "aura_binaire"))
			cv_px(c, px, (y + (int)(ph * 30)), (k % 2) ? rgb(80, 255, 120) : rgb(30, 140, 60));
		else if (!strcmp(id, "aura_flocons"))
			cv_px(c, px + (int)(2 * sin(ph * 6 + k)), y + (int)(ph * 30), rgb(240, 250, 255));
		else
			cv_px(c, px, py, (k + t / 120) % 3 ? col : rgb(255, 255, 255));
	}
}

void	avatar_draw(t_canvas *c, int x, int y, const t_look *lk, long t)
{
	static const int	order[] = {SL_FOND, SL_AURA, SL_CAPE, SL_JAMBES, SL_ARMURE,
		SL_CASQUE, SL_YEUX, SL_DECO, SL_ARME, SL_MAIN, SL_FAMILIER};
	int					k;
	int					it;
	int					bob;
	t_rgb				eye;

	eye = item_color(lk->eq[SL_YEUX], lk->col[SL_YEUX]);
	bob = (t / 600) % 2;
	for (k = 0; k < NSLOT; k++)
	{
		it = lk->eq[order[k]];
		if (it < 0)
			continue ;
		if (order[k] == SL_AURA)
		{
			aura(c, x, y, it, item_color(it, lk->col[SL_AURA]), t);
			continue ;
		}
		cv_spr(c, &g_items[it].spr, x,
			y + ((order[k] == SL_FOND || order[k] == SL_FAMILIER) ? 0 : bob),
			item_color(it, lk->col[order[k]]), eye, 0);
	}
}
