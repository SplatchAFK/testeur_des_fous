#include <math.h>
#include <stdio.h>
#include <string.h>
#include "fous.h"

/*
** Lore de la version libft : Moulinette, la chatte de 42, est enfermée en
** haut de la Tour de la Libft. Les trois gardiens tiennent chacun une salle.
** Marvin, le chevalier du joueur, monte salle par salle pour la libérer.
**
** Les décors et Moulinette sont de vrais dessins (images PNG du dossier assets, faites par
** outils/dessins.py). Chaque scène fait 140 x 84 pixels, agrandie sans
** déformation sur les grands terminaux (voir stage_fit dans cine.c).
*/

#define CAT_W 24

static int	g_room;

static double	ease(double a, double b, double t)
{
	if (t <= 0)
		return (a);
	if (t >= 1)
		return (b);
	t = t * t * (3 - 2 * t);
	return (a + (b - a) * t);
}

static void	bubble(t_hd *d, const t_stage *s, double x, double y, const char *str)
{
	int	cx;
	int	cy;

	cx = stage_col(s, x);
	cy = stage_row(s, y);
	if (cy >= 2 && cy < d->ch && cx >= 0 && cx < d->cw)
		hd_bubble(d, cx, cy, str, rgb(30, 25, 45));
}

static void	center_text(t_hd *d, int y, t_rgb c, const char *s)
{
	t_rgb	bg;
	int		x;

	x = (d->cw - ulen(s)) / 2;
	if (x < 0)
		x = 0;
	if (y < 0 || y >= d->ch)
		return ;
	bg = d->px[(y * 2 + 1) * d->w + (x < d->w ? x : 0)];
	hd_text(d, x, y, c, bg, s);
}

/* Moulinette : 0 assise, 1 respire, 2 cligne, 3 contente, 4 surprise */
static int	cat_idle(double t)
{
	if (fmod(t, 3.3) > 3.05)
		return (2);
	return ((int)(t * 1.5) % 2);
}

static void	cat(t_hd *d, const t_stage *s, double x, double y, int frame, int zoom, int shadow)
{
	if (shadow)
		stage_shadow(d, s, x + CAT_W * zoom / 2.0, y + 21 * zoom, 9 * zoom, 0.5);
	stage_pic(d, s, "chat", CAT_W, frame, x, y, zoom);
}

/* une torche murale qui crépite ; (x, y) = haut de la flamme */
static void	torch(t_hd *d, const t_stage *s, double x, double y, double t, int seed)
{
	int	f;

	f = (int)(t * 9 + seed * 2) % 4;
	stage_glow(d, s, x + 4.5, y + 4, 26, rgb(255, 150, 60), 0.32 + 0.06 * sin(t * 13 + seed));
	stage_pic(d, s, "torche", 9, f, x, y, 1);
}

/* ---------------------------------------------------------------- la tour */

/*
** Le décor « tour » fait 140 x 260 : la caméra part du sommet (cage), descend
** au balcon des gardiens, puis au pied de la tour où attend Marvin.
*/
static int	sc_tower(t_hd *d, double t)
{
	t_stage	s;
	double	cam;
	int		p;
	int		lift;
	char	tmp[200];

	stage_fit(d, &s, 0);
	cam = t < 3.4 ? 0 : (t < 6.8 ? ease(0, 66, (t - 3.4) / 1.4) : ease(66, 176, (t - 6.8) / 1.4));
	stage_bg(d, &s, "tour", (int)cam, rgb(38, 18, 66), rgb(24, 40, 92));
	/* sommet : Moulinette dans sa cage */
	cat(d, &s, 58, 44 - cam, t > 0.6 && t < 3.3 ? 4 : cat_idle(t), 1, 0);
	lift = 0;
	stage_pic(d, &s, "cage", 28, lift, 56, 36 - cam, 1);
	/* balcon des trois gardiens */
	for (p = 0; p < 3; p++)
	{
		stage_shadow(d, &s, 28 + p * 42, 123 - cam, 12, 0.4);
		stage_boss(d, &s, p, 8 + p * 42, 87 - cam + (t > 4.7 && t < 6.8 ? -fabs(sin(t * 4 + p)) * 2 : 0), 1);
	}
	/* au pied de la tour : Marvin */
	stage_marvin(d, &s, 24, 208 - cam, t);
	if (t < 0.8)
		stage_fade(d, rgb(0, 0, 0), 1 - t / 0.8);
	if (t > 9.6)
		stage_fade(d, rgb(10, 6, 20), fmin(0.72, (t - 9.6) * 1.2));
	hd_present(d);
	if (t > 0.6 && t < 3.3)
		bubble(d, &s, 70, 38 - cam, L("Miaou ! Au secours !", "Meow! Help!"));
	if (t > 4.9 && t < 6.8)
		bubble(d, &s, 70, 87 - cam, L("Personne ne la libérera.\nSes tests sont à nous !",
				"Nobody will free her.\nHer tests are ours!"));
	if (t > 8.1 && t < 9.6)
		bubble(d, &s, 38, 208 - cam, L("Tiens bon, Moulinette.\nJ'arrive avec ma libft !",
				"Hold on, Moulinette.\nHere I come with my libft!"));
	if (t > 9.8)
	{
		center_text(d, d->ch / 2 - 1, rgb(255, 215, 120), L("L A   T O U R   D E   L A   L I B F T",
				"T H E   L I B F T   T O W E R"));
		snprintf(tmp, sizeof(tmp), "%s", L("3 salles · 3 gardiens · libère Moulinette",
				"3 rooms · 3 guardians · free Moulinette"));
		center_text(d, d->ch / 2 + 1, rgb(220, 220, 240), tmp);
	}
	return (0);
}

/* ---------------------------------------------------------------- une salle */

static int	sc_room(t_hd *d, double t)
{
	t_stage	s;
	double	k;
	char	tmp[160];

	stage_fit(d, &s, t > 0.9 && t < 1.15 ? (int)(sin(t * 80) * 1.5) : 0);
	stage_bg(d, &s, "salle", 0, rgb(16, 12, 26), rgb(40, 30, 50));
	torch(d, &s, 14, 20, t, 0);
	torch(d, &s, 116, 20, t, 1);
	k = fmin(1, t * 1.2);
	stage_glow(d, &s, 70, 44, 30, g_bosses[g_room].glow, 0.35 * k);
	stage_shadow(d, &s, 70, 62, 14, 0.5 * k);
	stage_boss(d, &s, g_room, 50, 25 + (1 - k) * 6, k);
	if (t < 0.4)
		stage_fade(d, rgb(0, 0, 0), 1 - t / 0.4);
	hd_present(d);
	snprintf(tmp, sizeof(tmp), L("S A L L E   %d", "R O O M   %d"), g_room + 1);
	center_text(d, 2, rgb(255, 215, 120), tmp);
	center_text(d, 4, g_bosses[g_room].glow, L(g_bosses[g_room].name, g_bosses[g_room].en));
	snprintf(tmp, sizeof(tmp), L("partie %d de la libft", "libft part %d"), g_room + 1);
	center_text(d, d->ch - 3, rgb(200, 200, 220), tmp);
	return (0);
}

/* ---------------------------------------------------------------- final */

/* les deux pattes de Moulinette qui arrivent sur toi */
static void	paws(t_hd *d, const t_stage *s, double k)
{
	int		side;
	int		i;
	double	x;
	double	y;

	for (side = 0; side < 2; side++)
	{
		x = side ? 104 : 36;
		y = 96 - 26 * k;
		hd_ellipse(d, s->ox + x * s->k, s->oy + (y + 1) * s->k, 17 * s->k, 15 * s->k, rgb(70, 72, 88));
		hd_ellipse(d, s->ox + x * s->k, s->oy + y * s->k, 16 * s->k, 14 * s->k, rgb(158, 160, 174));
		hd_ellipse(d, s->ox + (x - 3) * s->k, s->oy + (y - 4) * s->k, 9 * s->k, 6 * s->k, rgb(190, 192, 206));
		for (i = -1; i <= 1; i++)
			hd_ellipse(d, s->ox + (x + i * 6) * s->k, s->oy + (y - 8) * s->k, 2.6 * s->k, 2.2 * s->k,
				rgb(255, 160, 186));
		hd_ellipse(d, s->ox + x * s->k, s->oy + (y - 1) * s->k, 4.5 * s->k, 3.5 * s->k, rgb(255, 160, 186));
	}
}

static int	sc_finale(t_hd *d, double t)
{
	t_stage	s;
	double	k;
	int		i;
	int		z;

	if (t < 3.2)
	{
		stage_fit(d, &s, 0);
		stage_bg(d, &s, "sommet", 0, rgb(38, 18, 66), rgb(70, 50, 110));
		k = t < 2.2 ? 0 : (t - 2.2);
		z = 1 + (int)(k * 3);
		if (z == 1)
		{
			cat(d, &s, 58, 36, t < 0.5 ? 4 : 3, 1, 1);
			stage_pic(d, &s, "cage", 28, t < 0.4 ? 0 : (t > 1.3 ? 3 : (int)((t - 0.4) / 0.3)), 56,
				28 - fmax(0, t - 1.3) * 40, 1);
		}
		else
			cat(d, &s, 70 - 12 * z, 36 - 10 * (z - 1), 3, z, 0);
		if (t > 0.4 && t < 1.3)
			stage_sparks(d, &s, 70, 32, fmod(t - 0.4, 0.45), rgb(220, 220, 240), (int)(t * 3));
		hd_present(d);
		if (t > 0.3 && t < 2.0)
			bubble(d, &s, 70, 34, L("Marvin ! Tu es venu !", "Marvin! You came!"));
		return (0);
	}
	if (t < 5.8)
	{
		k = (t - 3.2) / 2.6;
		stage_fit(d, &s, t > 4.2 && t < 4.4 ? (int)(sin(t * 90) * 2) : 0);
		hd_vgrad(d, 0, d->h, rgb(255, 200, 150), rgb(255, 160, 120));
		stage_pic(d, &s, "chat_visage", 112, fmod(t, 1.6) > 1.45, 14, ease(60, 0, k * 2.5), 1);
		if (t > 4.2)
			paws(d, &s, fmin(1, (t - 4.2) * 2.2));
		hd_present(d);
		if (t > 3.6)
			center_text(d, 2, rgb(120, 40, 60), L("M I A O U  !", "M E O W  !"));
		return (0);
	}
	k = fmin(1, (t - 5.8) / 0.8);
	stage_fit(d, &s, 0);
	stage_bg(d, &s, "sommet", 0, rgb(38, 18, 66), rgb(70, 50, 110));
	stage_fade(d, rgb(255, 190, 150), 0.6 * (1 - k));
	for (i = 0; i < 22; i++)
	{
		double r = fmod(t * 0.3 + i * 0.137, 1.0);
		double x = fmod(i * 37.0, 140) + sin(t * 2 + i) * 3;
		double y = 84 * (1 - r);
		hd_ellipse(d, s.ox + (x - 1) * s.k, s.oy + y * s.k, 1.2 * s.k, 1.2 * s.k, rgb(255, 110, 150));
		hd_ellipse(d, s.ox + (x + 1) * s.k, s.oy + y * s.k, 1.2 * s.k, 1.2 * s.k, rgb(255, 110, 150));
		hd_ellipse(d, s.ox + x * s.k, s.oy + (y + 1.4) * s.k, 1.2 * s.k, 1.2 * s.k, rgb(255, 110, 150));
	}
	cat(d, &s, 46, 14 - fabs(sin(t * 4)) * 3, fmod(t, 2.0) < 1.6 ? 3 : 0, 2, 1);
	hd_present(d);
	center_text(d, 2, rgb(80, 230, 140), L("L I B F T   V A L I D É E", "L I B F T   V A L I D A T E D"));
	center_text(d, 4, rgb(240, 236, 250), L("Moulinette est libre. Merci, Marvin !", "Moulinette is free. Thank you, Marvin!"));
	return (0);
}

void	story_libft_intro(void)
{
	cine_play(sc_tower, 12.0);
}

void	story_room(int p)
{
	g_room = p;
	cine_play(sc_room, 2.4);
}

void	story_libft_finale(void)
{
	cine_play(sc_finale, 9.0);
}
