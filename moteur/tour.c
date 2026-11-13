#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

static const float	g_fen[43][2] = {
	{0.5854f, 0.3192f}, {0.6158f, 0.3215f}, {0.6410f, 0.3152f},
	{0.5870f, 0.3475f}, {0.6139f, 0.3464f}, {0.6433f, 0.3499f},
	{0.5883f, 0.3848f}, {0.6048f, 0.3836f}, {0.6293f, 0.3927f},
	{0.6473f, 0.3934f}, {0.5846f, 0.4235f}, {0.6060f, 0.4187f},
	{0.6328f, 0.4257f}, {0.6558f, 0.4282f}, {0.5888f, 0.4547f},
	{0.6042f, 0.4633f}, {0.6349f, 0.4551f}, {0.6504f, 0.4588f},
	{0.5874f, 0.4883f}, {0.6090f, 0.4950f}, {0.6294f, 0.4974f},
	{0.6601f, 0.4929f}, {0.5897f, 0.5362f}, {0.6079f, 0.5301f},
	{0.6344f, 0.5247f}, {0.6538f, 0.5346f}, {0.5844f, 0.5657f},
	{0.6127f, 0.5736f}, {0.6351f, 0.5687f}, {0.6553f, 0.5640f},
	{0.5855f, 0.6029f}, {0.6064f, 0.6047f}, {0.6370f, 0.5968f},
	{0.6594f, 0.6006f}, {0.5823f, 0.6423f}, {0.6083f, 0.6373f},
	{0.6367f, 0.6308f}, {0.6646f, 0.6302f}, {0.5810f, 0.6778f},
	{0.6049f, 0.6768f}, {0.6216f, 0.6778f}, {0.6430f, 0.6798f},
	{0.6622f, 0.6667f}
};

static unsigned char	*g_tour;
static unsigned char	*g_crop;
static int				g_tw;
static int				g_th;
static int				g_cx0;
static int				g_cy0;
static int				g_cw;
static int				g_ch;
static unsigned long	g_sum;

static void	teinte(unsigned char *q, t_rgb c, double k)
{
	if (k <= 0)
		return ;
	if (k > 1)
		k = 1;
	q[0] = q[0] + (c.r - q[0]) * k;
	q[1] = q[1] + (c.g - q[1]) * k;
	q[2] = q[2] + (c.b - q[2]) * k;
}

static void	fenetre(unsigned char *p, double cx, double cy, double s, int etat)
{
	int		x;
	int		y;
	double	rx;
	double	ry;
	double	d;
	t_rgb	c;

	rx = 3.2 * s;
	ry = 5.6 * s;
	c = etat == 1 ? rgb(118, 222, 238) : rgb(150, 70, 60);
	for (y = (int)(cy - ry * 3); y <= (int)(cy + ry * 3); y++)
		for (x = (int)(cx - rx * 3); x <= (int)(cx + rx * 3); x++)
		{
			if (x < 0 || y < 0 || x >= g_tw || y >= g_th)
				continue ;
			d = ((x - cx) * (x - cx)) / (rx * rx) + ((y - cy) * (y - cy)) / (ry * ry);
			if (etat == 1 && d < 1)
				teinte(p + ((size_t)y * g_tw + x) * 3, mix(c, rgb(220, 250, 255), 0.25 * (1 - d)), 0.92);
			else if (etat == 1 && d < 9)
				teinte(p + ((size_t)y * g_tw + x) * 3, c, 0.45 * (1 - sqrt(d) / 3) * (1 - sqrt(d) / 3));
			else if (etat == 2 && d < 1 && fabs((x - cx) - (y - cy) * 0.35) < 0.7 * s)
				teinte(p + ((size_t)y * g_tw + x) * 3, rgb(140, 170, 190), 0.8);
			else if (etat == 2 && d < 2.2)
				teinte(p + ((size_t)y * g_tw + x) * 3, c, 0.18 * (1 - d / 2.2));
		}
}

static int	charger(void)
{
	const t_pic	*b;

	if (g_tour)
		return (1);
	b = art_pic("tour_eteinte");
	if (!b || b->w < 100)
		return (0);
	g_tw = b->w;
	g_th = b->h;
	g_tour = malloc((size_t)g_tw * g_th * 3);
	g_cx0 = (int)(g_tw * 0.53);
	g_cy0 = 0;
	g_cw = (int)(g_tw * 0.22);
	g_ch = (int)(g_th * 0.80);
	g_crop = malloc((size_t)g_cw * g_ch * 3);
	return (g_tour && g_crop);
}

static void	composer(const int *etat, int n)
{
	const t_pic		*b;
	unsigned long	s;
	double			k;
	int				i;
	int				y;

	s = 5381;
	for (i = 0; i < n; i++)
		s = s * 33 + (unsigned long)etat[i] + 1;
	if (s == g_sum)
		return ;
	g_sum = s;
	b = art_pic("tour_eteinte");
	memcpy(g_tour, b->rgb, (size_t)g_tw * g_th * 3);
	k = g_tw / 1040.0;
	for (i = 0; i < n && i < 43; i++)
		if (etat[i])
			fenetre(g_tour, g_fen[42 - i][0] * g_tw, g_fen[42 - i][1] * g_th, k, etat[i]);
	for (y = 0; y < g_ch; y++)
		memcpy(g_crop + (size_t)y * g_cw * 3, g_tour + ((size_t)(g_cy0 + y) * g_tw + g_cx0) * 3, (size_t)g_cw * 3);
}

void	tour_dessiner(const int *etat, int n, int x, int y, int h)
{
	int	p;
	int	w;

	p = rendu_palier();
	if (p == 3 || !charger())
		return ;
	composer(etat, n);
	w = (int)(h * 2.0 * g_cw / g_ch + 0.5);
	if (h < 6 || x < 0 || x + w > term_cols())
		return ;
	rendu_image_terminal(g_crop, g_cw, g_ch, x, y, w, h);
}

int	tour_largeur(int h)
{
	if (!charger())
		return (0);
	return ((int)(h * 2.0 * g_cw / g_ch + 0.5));
}
