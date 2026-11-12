#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

static unsigned char	*g_fo;
static unsigned char	*g_fc;
static int				g_fw;
static int				g_fh;
static int				g_cx0;
static int				g_cy0;
static int				g_cw;
static int				g_ch;
static unsigned long	g_fsum;

static void	empiler_fo(const t_pic *p)
{
	int		i;
	double	a;

	if (!p || p->w != g_fw || p->h != g_fh)
		return ;
	for (i = 0; i < g_fw * g_fh; i++)
	{
		if (p->a[i] < 6)
			continue ;
		a = p->a[i] / 255.0;
		g_fo[i * 3] = g_fo[i * 3] + (p->rgb[i * 3] - g_fo[i * 3]) * a;
		g_fo[i * 3 + 1] = g_fo[i * 3 + 1] + (p->rgb[i * 3 + 1] - g_fo[i * 3 + 1]) * a;
		g_fo[i * 3 + 2] = g_fo[i * 3 + 2] + (p->rgb[i * 3 + 2] - g_fo[i * 3 + 2]) * a;
	}
}

static int	composer_fo(const int *etat)
{
	const t_pic		*b;
	unsigned long	s;
	char			nom[48];
	int				i;
	int				libre;
	int				y;

	if (!(b = art_pic("fonderie_fond")))
		return (0);
	s = 7;
	for (i = 0; i < 9; i++)
		s = s * 3 + (etat[i] != 0);
	if (g_fo && s == g_fsum)
		return (1);
	if (!g_fo || g_fw != b->w || g_fh != b->h)
	{
		free(g_fo);
		free(g_fc);
		g_fw = b->w;
		g_fh = b->h;
		g_cx0 = g_fw * 32 / 100;
		g_cy0 = g_fh * 30 / 100;
		g_cw = g_fw * 36 / 100;
		g_ch = g_fh * 66 / 100;
		g_fo = malloc((size_t)g_fw * g_fh * 3);
		g_fc = malloc((size_t)g_cw * g_ch * 3);
		if (!g_fo || !g_fc)
			return (0);
	}
	g_fsum = s;
	memcpy(g_fo, b->rgb, (size_t)g_fw * g_fh * 3);
	libre = 1;
	for (i = 0; i < 9; i++)
	{
		snprintf(nom, sizeof(nom), etat[i] ? "fonderie_brisee_%d" : "fonderie_chaine_%d", i);
		empiler_fo(art_pic(nom));
		libre &= etat[i] != 0;
	}
	empiler_fo(art_pic(libre ? "fonderie_cage_ouverte" : "fonderie_cage"));
	for (y = 0; y < g_ch; y++)
		memcpy(g_fc + (size_t)y * g_cw * 3, g_fo + ((size_t)(g_cy0 + y) * g_fw + g_cx0) * 3, (size_t)g_cw * 3);
	return (1);
}

void	fonderie_dessiner(const int *etat, int x, int y, int h)
{
	int	p;
	int	w;

	p = rendu_palier();
	if (p == 3 || !composer_fo(etat))
		return ;
	if (p == 0)
		rendu_fenetre(g_fo, g_fw, g_fh);
	w = (int)(h * 2.0 * g_cw / g_ch + 0.5);
	if (h < 6 || x < 0 || x + w > term_cols())
		return ;
	rendu_image_terminal(g_fc, g_cw, g_ch, x, y, w, h);
}
