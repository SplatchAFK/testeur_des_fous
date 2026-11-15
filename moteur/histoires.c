#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

static char	g_anneaux[32];
static int	g_portes;

static double	lisse(double a, double b, double t)
{
	if (t <= 0)
		return (a);
	if (t >= 1)
		return (b);
	t = t * t * (3 - 2 * t);
	return (a + (b - a) * t);
}

static void	bulle_h(t_hd *d, const t_stage *s, double x, double y, const char *str)
{
	int	cx;
	int	cy;

	cx = stage_col(s, x);
	cy = stage_row(s, y);
	if (cy >= 2 && cy < d->ch && cx >= 0 && cx < d->cw)
		hd_bubble(d, cx, cy, str, rgb(30, 25, 45));
}

static void	texte_h(t_hd *d, int y, t_rgb c, const char *s)
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

static void	spx(t_hd *d, const t_stage *s, double x, double y, t_rgb c)
{
	hd_rect(d, s->ox + (int)floor(x) * s->k, s->oy + (int)floor(y) * s->k, s->k, s->k, c);
}

static void	sbl(t_hd *d, const t_stage *s, double x, double y, t_rgb c, double k)
{
	int	i;
	int	j;

	for (j = 0; j < s->k; j++)
		for (i = 0; i < s->k; i++)
			hd_blend(d, s->ox + (int)floor(x) * s->k + i, s->oy + (int)floor(y) * s->k + j, c, k);
}

static void	lanterne(t_hd *d, const t_stage *s, double x, double y, double lum, double t)
{
	if (lum > 0)
		stage_glow(d, s, x + 4.5, y + 7, 22, rgb(140, 230, 240), lum * (0.42 + 0.05 * sin(t * 7)));
	stage_pic(d, s, "lanterne", 9, lum > 0.3, x, y, 1);
}

static int	anneaux_profondeur(int *nok)
{
	int	prof;
	int	i;

	prof = -1;
	*nok = 0;
	for (i = 0; i < 22 && g_anneaux[i]; i++)
	{
		if (g_anneaux[i] == 'O')
			(*nok)++;
		if (g_anneaux[i] == 'O' && prof == i - 1)
			prof = i;
	}
	return (prof);
}

static double	anneau_y(int i)
{
	return (44 + i * 8);
}

static int	sc_puits(t_hd *d, double t)
{
	t_stage	s;
	char	tmp[200];
	double	ly;
	double	cible;
	double	cam;
	double	ax;
	double	ay;
	int		prof;
	int		nok;
	int		i;

	prof = anneaux_profondeur(&nok);
	cible = nok == 22 ? 226 : (prof >= 0 ? anneau_y(prof) - 6 : 26);
	ly = lisse(14, cible, (t - 1.2) / 5.5);
	cam = ly - 34;
	cam = cam < 0 ? 0 : (cam > 166 ? 166 : cam);
	stage_fit(d, &s, 0);
	stage_bg(d, &s, "puits_pixel", (int)cam, rgb(10, 12, 24), rgb(4, 8, 16));
	for (i = 0; i < 22; i++)
	{
		ax = i % 2 ? 106 : 32;
		ay = anneau_y(i) - cam;
		if (ay < -3 || ay > 87)
			continue ;
		if (g_anneaux[i] == 'O' && ly + 6 >= anneau_y(i) - 2)
		{
			stage_glow(d, &s, ax + 1, ay, 6, rgb(118, 222, 238), 0.55);
			spx(d, &s, ax, ay - 1, rgb(190, 250, 255));
			spx(d, &s, ax + 1, ay - 1, rgb(190, 250, 255));
			spx(d, &s, ax, ay, rgb(118, 222, 238));
			spx(d, &s, ax + 1, ay, rgb(118, 222, 238));
			spx(d, &s, ax, ay + 1, rgb(60, 150, 170));
			spx(d, &s, ax + 1, ay + 1, rgb(60, 150, 170));
		}
		else if (g_anneaux[i] == 'K' && ly + 6 >= anneau_y(i) - 2)
		{
			stage_glow(d, &s, ax + 1, ay, 5, rgb(220, 70, 60), 0.45 + 0.15 * sin(t * 6 + i));
			spx(d, &s, ax, ay - 1, rgb(250, 120, 100));
			spx(d, &s, ax + 1, ay, rgb(200, 60, 50));
			spx(d, &s, ax, ay + 1, rgb(120, 30, 30));
		}
		else
		{
			spx(d, &s, ax, ay - 1, rgb(70, 74, 86));
			spx(d, &s, ax + 1, ay - 1, rgb(70, 74, 86));
			spx(d, &s, ax, ay, rgb(40, 42, 52));
			spx(d, &s, ax + 1, ay, rgb(40, 42, 52));
		}
	}
	for (ay = 24 - cam; ay < ly - cam; ay += 1)
		if (ay >= 0)
			sbl(d, &s, 70, ay, ((int)ay % 3) ? rgb(150, 120, 80) : rgb(110, 86, 56), 1);
	lanterne(d, &s, 66, ly - cam, 1, t);
	if (t < 0.6)
		stage_fade(d, rgb(0, 0, 0), 1 - t / 0.6);
	if (t > 7.4)
		stage_fade(d, rgb(4, 6, 14), fmin(0.45, (t - 7.4) * 0.6));
	hd_present(d);
	texte_h(d, 2, rgb(150, 236, 244), L("L E   P U I T S", "T H E   W E L L"));
	if (t > 7.6)
	{
		snprintf(tmp, sizeof(tmp), L("%d anneaux sur 22 · la lanterne est descendue de %d ligne(s)",
				"%d rings out of 22 · the lantern went down %d line(s)"), nok, prof + 1);
		texte_h(d, 4, rgb(220, 222, 236), tmp);
		texte_h(d, d->ch - 3, rgb(240, 236, 250), nok == 22
			? L("Au fond, de l'eau calme. Les empreintes de Moulinette mènent vers une ouverture.",
				"At the bottom, calm water. Moulinette's paw prints lead to an opening.")
			: L("Le Puits ne se descend qu'une ligne à la fois. Les anneaux rouges ne tiennent pas encore.",
				"The Well goes down one line at a time. The red rings do not hold yet."));
	}
	return (0);
}

static int	sc_avant(t_hd *d, double t)
{
	t_stage	s;
	double	mx;
	double	k;

	stage_fit(d, &s, 0);
	stage_bg(d, &s, "epreuve_couloir", 0, rgb(10, 12, 22), rgb(18, 20, 34));
	k = 0.25 + 0.2 * sin(t * 2.2);
	stage_glow(d, &s, 70, 40, 16, rgb(90, 120, 170), t > 5.6 ? fmin(0.8, k + (t - 5.6) * 0.3) : k * 0.5);
	mx = t < 3.2 ? lisse(-30, 50, t / 3.2) : (t < 6 ? 50 : lisse(50, 56, (t - 6) / 1.5));
	if (t > 3.4)
		lanterne(d, &s, 86, 69, fmin(1, (t - 3.4) * 2), t);
	stage_marvin(d, &s, mx, 50 - (t > 6 ? (t - 6) * 3 : 0), t);
	if (t < 3.4)
		lanterne(d, &s, mx + 22, 64, 1, t);
	if (t < 0.6)
		stage_fade(d, rgb(0, 0, 0), 1 - t / 0.6);
	if (t > 7.6)
		stage_fade(d, rgb(0, 0, 0), fmin(1, (t - 7.6) / 1.2));
	hd_present(d);
	texte_h(d, 2, rgb(150, 236, 244), L("L ' É P R E U V E   D U   S I L E N C E", "T H E   T R I A L   O F   S I L E N C E"));
	if (t > 3.8 && t < 6.2)
		bulle_h(d, &s, 64, 48, L("Ici, personne ne m'aidera.\nJuste moi et mon code.", "Here, nobody will help me.\nJust me and my code."));
	if (t > 4)
		texte_h(d, d->ch - 3, rgb(240, 236, 250), L("Marvin pose sa lanterne et entre seul. Ici, personne ne t'aidera.",
				"Marvin puts down his lantern and goes in alone. Here, nobody will help you."));
	return (0);
}

static void	porte_ouverte(t_hd *d, const t_stage *s, int cx, double f, double t)
{
	int		x;
	int		y;
	double	e;
	double	lim;
	t_rgb	c;

	lim = cx - 9 + 18 * (1 - f);
	for (y = 16; y < 62; y++)
		for (x = cx - 9; x <= cx + 9; x++)
		{
			e = y >= 26 ? 0 : ((x - cx) * (x - cx)) / 81.0 + ((y - 26) * (y - 26)) / 100.0;
			if (e > 1)
				continue ;
			if (x < lim)
			{
				if (x >= lim - 2)
					spx(d, s, x, y, rgb(70, 52, 38));
				continue ;
			}
			c = mix(rgb(160, 236, 244), rgb(20, 40, 60), (y - 16) / 46.0);
			spx(d, s, x, y, mix(c, rgb(230, 255, 255), 0.15 + 0.1 * sin(t * 5 + y * 0.3)));
		}
	stage_glow(d, s, cx, 52, 18, rgb(130, 220, 236), 0.35 * f);
}

static int	sc_apres(t_hd *d, double t)
{
	t_stage	s;
	char	tmp[160];
	double	f;
	int		n;
	int		i;

	n = g_portes < 0 ? 0 : (g_portes > 4 ? 4 : g_portes);
	stage_fit(d, &s, 0);
	stage_bg(d, &s, "epreuve_portes", 0, rgb(10, 12, 22), rgb(18, 20, 34));
	for (i = 0; i < n; i++)
	{
		f = (t - 1.0 - i * 1.1) / 0.8;
		if (f > 0)
			porte_ouverte(d, &s, 25 + i * 30, f > 1 ? 1 : f, t);
	}
	stage_marvin(d, &s, 56, 50, t);
	if (n >= 4)
		lanterne(d, &s, 88, 69, t > 5.8 ? fmin(1, (t - 5.8) * 1.5) : 0, t);
	if (n >= 4 && t > 5.8)
		stage_sparks(d, &s, 92, 72, fmod(t - 5.8, 1.2), rgb(150, 236, 244), (int)(t * 2));
	if (t < 0.6)
		stage_fade(d, rgb(0, 0, 0), 1 - t / 0.6);
	hd_present(d);
	texte_h(d, 2, rgb(150, 236, 244), L("L ' É P R E U V E   D U   S I L E N C E", "T H E   T R I A L   O F   S I L E N C E"));
	snprintf(tmp, sizeof(tmp), L("%d porte(s) sur 4 se sont ouvertes", "%d door(s) out of 4 opened"), n);
	if (t > 0.8)
		texte_h(d, 4, n ? rgb(150, 236, 170) : rgb(200, 200, 220), tmp);
	if (t > 1.4 + n * 1.1)
		texte_h(d, d->ch - 3, rgb(240, 236, 250), n >= 4
			? L("Les quatre portes sont ouvertes. Dehors, la lanterne s'est rallumée toute seule.",
				"All four doors are open. Outside, the lantern lit itself again.")
			: L("Les portes encore fermées attendent la prochaine épreuve.",
				"The doors still closed are waiting for the next trial."));
	return (0);
}

static int	sc_fin(t_hd *d, double t)
{
	t_stage	s;
	double	v;
	int		f;
	int		i;

	stage_fit(d, &s, 0);
	stage_bg(d, &s, "mecanisme", 0, rgb(8, 12, 22), rgb(16, 18, 28));
	v = t < 3.6 ? 0 : (t - 3.6) * 2.5;
	f = (int)(v * (1 + v * 0.1));
	for (i = 0; i < 7; i++)
		stage_glow(d, &s, 20 + i * 18, 2, 8, rgb(110, 220, 230), t > 3.6 + i * 0.4 ? 0.5 : 0.08);
	stage_glow(d, &s, 70, 34, 34, rgb(255, 190, 90), t > 3.6 ? fmin(0.3, (t - 3.6) * 0.15) : 0.05);
	stage_pic(d, &s, "engrenage", 36, f % 4, 48, 14, 1);
	stage_pic(d, &s, "engrenage_petit", 24, 3 - f % 4, 80, 22, 1);
	stage_pic(d, &s, "engrenage_petit", 24, 3 - f % 4, 30, 12, 1);
	stage_shadow(d, &s, 36, 72, 9, 0.5);
	stage_pic(d, &s, "chat", 24, t > 4 && t < 6.6 ? 4 : (fmod(t, 3.3) > 3.05 ? 2 : (int)(t * 1.5) % 2), 24, 51, 1);
	stage_marvin(d, &s, t < 2.6 ? lisse(150, 84, t / 2.6) : 84, 50, t);
	if (t > 2.8)
		lanterne(d, &s, 116, 69, fmin(1, (t - 2.8) * 2), t);
	if (t < 0.6)
		stage_fade(d, rgb(0, 0, 0), 1 - t / 0.6);
	if (t > 9.6)
		stage_fade(d, rgb(4, 8, 16), fmin(0.78, (t - 9.6) * 0.8));
	hd_present(d);
	if (t < 9.6)
		texte_h(d, 2, rgb(255, 215, 120), L("L E   D E R N I E R   M É C A N I S M E", "T H E   L A S T   M E C H A N I S M"));
	if (t > 4 && t < 6.6)
		bulle_h(d, &s, 36, 50, L("Miaou ! Tout en bas, sous le Puits…\ntout se remet à tourner.",
				"Meow! Down here, under the Well…\neverything turns again."));
	if (t > 6.8 && t < 9.4)
		bulle_h(d, &s, 98, 49, L("Alors c'était ça,\nle cœur de la Cité.", "So that was it,\nthe heart of the City."));
	if (t > 9.8)
	{
		texte_h(d, d->ch / 2 - 1, rgb(255, 215, 120), L("F I N", "T H E   E N D"));
		texte_h(d, d->ch / 2 + 1, rgb(220, 222, 236), L("La Cité tourne à nouveau, grâce à ton code.",
				"The City turns again, thanks to your code."));
	}
	return (0);
}

static int	sc_secrete(t_hd *d, double t)
{
	t_stage			s;
	const t_pic		*p;
	unsigned char	*c;
	unsigned int	h;
	double			k;
	int				x;
	int				y;
	int				i;

	stage_fit(d, &s, 0);
	stage_bg(d, &s, "cite_nuit", 0, rgb(10, 8, 34), rgb(60, 40, 100));
	p = art_pic("cite_nuit");
	k = (t - 1.0) / 5.0;
	for (y = 0; p && k > 0 && y < 62 && y < p->h; y++)
		for (x = 0; x < 140 && x < p->w; x++)
		{
			c = p->rgb + ((size_t)y * p->w + x) * 3;
			if (c[0] != 20 || c[1] != 16 || c[2] != 36)
				continue ;
			h = (unsigned int)(x * 73856093u ^ y * 19349663u) % 1000;
			if (h < k * 1000)
				spx(d, &s, x, y, h % 3 ? rgb(255, 210, 110) : rgb(150, 236, 244));
		}
	if (k > 0)
		stage_glow(d, &s, 70, 40, 60, rgb(255, 190, 110), fmin(0.18, k * 0.18));
	stage_marvin(d, &s, 26, 30, t);
	stage_pic(d, &s, "chat", 24, fmod(t, 3.3) > 3.05 ? 2 : (int)(t * 1.5) % 2, 58, 40, 1);
	for (i = 0; i < 3; i++)
		stage_pic(d, &s, "peluches", 13, i, 88 + i * 14, 49 - fabs(sin(t * 3 + i)) * 2, 1);
	if (t < 0.6)
		stage_fade(d, rgb(0, 0, 0), 1 - t / 0.6);
	hd_present(d);
	if (t > 1.6 && t < 4.6)
		bulle_h(d, &s, 70, 39, L("Rrr… regarde, Marvin :\ntoute la Cité s'allume.", "Purr… look, Marvin:\nthe whole City is lighting up."));
	if (t > 4.8 && t < 7.4)
		bulle_h(d, &s, 106, 47, L("Et les trois gardiens…\nen peluche !", "And the three guardians…\nas plushies!"));
	if (t > 7.4)
	{
		texte_h(d, 2, rgb(255, 215, 120), L("F I N   S E C R È T E", "S E C R E T   E N D I N G"));
		texte_h(d, 4, rgb(240, 236, 250), L("Libft et ft_printf validés · la Cité s'allume pour toi",
				"Libft and ft_printf validated · the City lights up for you"));
	}
	return (0);
}

int	tdf_histoire(const char *nom, const char *arg)
{
	int	nok;

	if (!strcmp(nom, "puits"))
	{
		snprintf(g_anneaux, sizeof(g_anneaux), "%s", arg ? arg : "");
		cine_play(sc_puits, 13.0);
		anneaux_profondeur(&nok);
		if (nok == 22)
			cine_play(sc_fin, 13.0);
	}
	else if (!strcmp(nom, "avant"))
		cine_play(sc_avant, 9.0);
	else if (!strcmp(nom, "apres"))
	{
		g_portes = arg ? atoi(arg) : 0;
		cine_play(sc_apres, 9.5);
	}
	else if (!strcmp(nom, "fin"))
		cine_play(sc_fin, 13.0);
	else if (!strcmp(nom, "secrete"))
		cine_play(sc_secrete, 11.0);
	else
		return (0);
	return (1);
}
