#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "fous.h"

#define CAT_W 24

static int		g_res[NCHAIN];
static int		g_nres;
static int		g_dlc;
static int		g_ph[NPHASE];
static int		g_secret;

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

static void	ctext(t_hd *d, int y, t_rgb c, const char *s)
{
	int	x;

	x = (d->cw - ulen(s)) / 2;
	if (x < 0)
		x = 0;
	if (y < 0 || y >= d->ch)
		return ;
	hd_text(d, x, y, c, d->px[(y * 2 + 1) * d->w + (x < d->w ? x : 0)], s);
}

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

static void	percent(t_hd *d, const t_stage *s, double x, double y, double r, t_rgb c)
{
	double	cx;
	double	cy;
	double	k;
	t_rgb	hole;

	cx = s->ox + x * s->k;
	cy = s->oy + y * s->k;
	r *= s->k;
	hole = mix(c, rgb(0, 0, 0), 0.75);
	hd_ellipse(d, cx - r * 0.62, cy - r * 0.62, r * 0.36, r * 0.36, c);
	hd_ellipse(d, cx - r * 0.62, cy - r * 0.62, r * 0.16, r * 0.16, hole);
	hd_ellipse(d, cx + r * 0.62, cy + r * 0.62, r * 0.36, r * 0.36, c);
	hd_ellipse(d, cx + r * 0.62, cy + r * 0.62, r * 0.16, r * 0.16, hole);
	for (k = -1; k <= 1; k += 0.5)
		hd_line(d, cx + r * 0.75 + k, cy - r, cx - r * 0.75 + k, cy + r, c);
}

static void	chain_line(t_hd *d, const t_stage *s, double x0, double y0, double x1, double y1, t_rgb c,
	double cut)
{
	double	n;
	double	i;
	double	x;
	double	y;
	t_rgb	sh;

	n = sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)) / 2.5;
	sh = mix(c, rgb(0, 0, 0), 0.45);
	for (i = 0; i <= n; i++)
	{
		if (cut > 0 && fabs(i / n - 0.5) < cut * 0.5)
			continue ;
		x = s->ox + (x0 + (x1 - x0) * i / n) * s->k;
		y = s->oy + (y0 + (y1 - y0) * i / n) * s->k;
		if ((int)i % 2)
		{
			hd_ellipse(d, x, y + s->k * 0.5, 1.2 * s->k, 0.7 * s->k, sh);
			hd_ellipse(d, x, y, 1.2 * s->k, 0.7 * s->k, c);
		}
		else
		{
			hd_ellipse(d, x, y + s->k * 0.5, 0.7 * s->k, 1.2 * s->k, sh);
			hd_ellipse(d, x, y, 0.7 * s->k, 1.2 * s->k, mix(c, rgb(255, 255, 255), 0.2));
		}
	}
}

static void	workshop(t_hd *d, const t_stage *s, double t, double speed)
{
	int	f;

	stage_bg(d, s, "machine", 0, rgb(22, 14, 34), rgb(46, 30, 58));
	f = (int)(t * 6 * speed);
	stage_pic(d, s, "engrenage", 36, f % 4, 2, 26, 1);
	stage_pic(d, s, "engrenage_petit", 24, 3 - f % 4, 30, 48, 1);
	stage_pic(d, s, "engrenage", 36, 3 - f % 4, 104, 18, 1);
	stage_pic(d, s, "engrenage_petit", 24, f % 4, 116, 44, 1);
}

static void	anchor(int i, int n, double *x, double *y)
{
	double	a;

	a = -M_PI * 0.98 + i * (M_PI * 0.9 / (n - 1));
	*x = 70 + cos(a) * 29;
	*y = 40 + sin(a) * 30;
}

static void	draw_chains(t_hd *d, const t_stage *s, int n, double t, int resolved)
{
	double	ax;
	double	ay;
	double	cut;
	int		i;
	t_rgb	c;

	for (i = 0; i < n; i++)
	{
		anchor(i, n, &ax, &ay);
		c = g_dlc ? g_dlcb[i].c : g_chains[i].c;
		cut = 0;
		if (i < resolved && g_res[i] == 1)
			cut = fmin(1, (t - (2.0 + i * 0.55)) * 3);
		else if (i < resolved && g_res[i] == 0)
			c = mix(c, rgb(255, 40, 60), 0.5 + 0.5 * sin(t * 9));
		else if (i < resolved)
			c = rgb(110, 110, 130);
		else
			c = mix(c, rgb(255, 255, 255), 0.2 + 0.2 * sin(t * 4 + i));
		if (cut < 1)
			chain_line(d, s, ax, ay, 70, 37, c, cut);
		hd_ellipse(d, s->ox + ax * s->k, s->oy + ay * s->k, 2.2 * s->k, 2.2 * s->k, rgb(70, 66, 84));
		hd_ellipse(d, s->ox + ax * s->k, s->oy + ay * s->k, 1.4 * s->k, 1.4 * s->k, rgb(170, 160, 186));
	}
}

static void	chain_labels(t_hd *d, const t_stage *s, int n, int resolved)
{
	char	tmp[40];
	double	ax;
	double	ay;
	int		i;
	int		x;
	int		y;
	t_rgb	c;

	for (i = 0; i < n; i++)
	{
		anchor(i, n, &ax, &ay);
		x = stage_col(s, ax) - 2;
		y = stage_row(s, ay) - 1;
		if (i < resolved)
			snprintf(tmp, sizeof(tmp), "%s %s", g_res[i] == 1 ? "✓" : (g_res[i] == 0 ? "✗" : "?"),
				g_dlc ? g_dlcb[i].conv : g_chains[i].conv);
		else
			snprintf(tmp, sizeof(tmp), "%s", g_dlc ? g_dlcb[i].conv : g_chains[i].conv);
		c = i < resolved ? (g_res[i] == 1 ? rgb(80, 230, 140) : (g_res[i] == 0 ? rgb(255, 90, 100)
					: rgb(150, 150, 170))) : (g_dlc ? g_dlcb[i].c : g_chains[i].c);
		if (y >= 0 && x >= 0 && y < d->ch)
			hd_text(d, x, y, c, d->px[(y * 2 + 1) * d->w + (x < d->w ? x : 0)], tmp);
	}
}

static int	sc_splash(t_hd *d, double t)
{
	t_stage	s;
	int		x;
	int		i;
	double	y;
	double	k;
	char	sub[64];

	stage_fit(d, &s, 0);
	stage_bg(d, &s, "scene_titre", 0, rgb(10, 4, 22), rgb(30, 10, 44));
	for (x = 0; x < d->w; x += 2)
	{
		k = fmod(t * (0.25 + (x * 7 % 13) / 26.0) + (x * 37 % 101) / 101.0, 1.3);
		y = k * d->h;
		for (i = 0; i < 9; i++)
			hd_blend(d, x, (int)y - i, mix(rgb(255, 80, 200), rgb(140, 90, 255), (x % 30) / 30.0),
				0.6 * (1 - i / 9.0));
	}
	stage_glow(d, &s, 70, 34, 34 + 4 * sin(t * 3), rgb(255, 80, 200), fmin(0.5, t * 0.4));
	if (t < 0.8)
		stage_fade(d, rgb(0, 0, 0), 1 - t / 0.8);
	hd_present(d);
	if (t > 0.6)
	{
		hd_big(d, (d->cw - big_width("LE TESTEUR")) / 2, d->ch / 2 - 8, "LE TESTEUR", t - 0.6,
			(t > 1.6 && t < 1.9) ? 1 : 2);
		hd_big(d, (d->cw - big_width("DES FOUS")) / 2, d->ch / 2 - 2, "DES FOUS", t - 1.3,
			(t > 2.0 && t < 2.3) ? 1 : 2);
	}
	if (t > 2.0)
	{
		snprintf(sub, sizeof(sub), "%.*s", (int)((t - 2.0) * 40), "·  V E R S I O N   P R I N T F  ·");
		ctext(d, d->ch / 2 + 5, rgb(255, 120, 220), sub);
	}
	if (t > 2.6)
		ctext(d, d->ch / 2 + 7, rgb(220, 210, 240), L("Créé et vibe-codé par Splatch  (login : romasant)",
				"Created and vibe-coded by Splatch  (login: romasant)"));
	if (t > 3.1)
		ctext(d, d->ch / 2 + 8, rgb(160, 150, 190), L("Pour les étudiants du cursus 42 Next · Paris et autres campus",
				"For 42 Next students · Paris and other campuses"));
	if (t > 3.6)
		ctext(d, d->ch / 2 + 10, rgb(140, 130, 170), "v" TDF_VERSION "  ·  " TDF_DATE);
	return (0);
}

static int	sc_previously(t_hd *d, double t)
{
	t_stage	s;
	double	cx;
	double	k;
	int		p;
	int		fr;

	stage_fit(d, &s, t > 4.2 && t < 4.6 ? (int)(sin(t * 70) * 1.5) : 0);
	stage_bg(d, &s, "scene_avant", 0, rgb(60, 44, 40), rgb(110, 84, 64));
	k = t < 4.2 ? fmin(0.38, t * 0.3) : fmax(0, 0.38 - (t - 4.2));
	for (p = 0; p < 3; p++)
		if (k > 0)
			stage_boss(d, &s, p, 30 + p * 30, 4 + sin(t * 1.5 + p) * 1.5, k);
	stage_marvin(d, &s, 76, 40, t);
	cx = 106 + (t > 4.4 ? sin(t * 40) * 1 : 0);
	fr = t > 4.4 ? 4 : (t > 1.0 && t < 4.2 ? 3 : cat_idle(t));
	cat(d, &s, cx, 50, fr, 1, 1);
	if (t > 4.2)
	{
		k = fmin(1, (t - 4.2) * 1.5);
		stage_glow(d, &s, 112, 18, 22 * k, rgb(255, 60, 180), 0.8 * k);
		percent(d, &s, 112, 18, 7 * k, rgb(255, 130, 225));
		if (t > 5.6)
			hd_line(d, s.ox + 112 * s.k, s.oy + 24 * s.k, s.ox + (cx + 12) * s.k, s.oy + 50 * s.k,
				mix(rgb(255, 90, 200), rgb(255, 255, 255), 0.5 + 0.5 * sin(t * 20)));
	}
	if (t < 0.8)
		stage_fade(d, rgb(0, 0, 0), 1 - t / 0.8);
	if (t > 6.8)
		stage_fade(d, rgb(255, 120, 220), (t - 6.8) * 1.4);
	hd_present(d);
	ctext(d, 2, rgb(255, 230, 190), L("Précédemment, dans la Libft…", "Previously, in the Libft…"));
	if (t > 1.0 && t < 4.2)
		ctext(d, 4, rgb(240, 220, 200), L("Les trois gardiens sont tombés. Moulinette est libre.",
				"The three guardians fell. Moulinette is free."));
	if (t > 4.4)
		ctext(d, 4, rgb(255, 160, 220), L("Mais à peine sortie de la tour, quelqu'un l'attendait…",
				"But she had barely left the tower when someone was waiting…"));
	if (t > 5.0 && t < 6.8)
		bubble(d, &s, cx + 12, 50, L("Miaou ?!", "Meow?!"));
	return (0);
}

static int	sc_intro(t_hd *d, double t)
{
	t_stage	s;
	double	mx;

	stage_fit(d, &s, 0);
	workshop(d, &s, t, 1);
	g_dlc = 0;
	draw_chains(d, &s, NCHAIN, t, 0);
	cat(d, &s, 58, 26, t > 5.9 && t < 7.4 ? 4 : cat_idle(t), 1, 0);
	stage_glow(d, &s, 124, 70, 18 + 2 * sin(t * 2), rgb(255, 60, 180), 0.45 + 0.15 * sin(t * 2));
	percent(d, &s, 124, 70, 6, rgb(255, 130, 225));
	mx = t < 7.4 ? -32 : ease(-32, 6, (t - 7.4) / 1.2);
	stage_marvin(d, &s, mx, 33, t);
	if (t < 1.2)
		stage_fade(d, rgb(0, 0, 0), 1 - t / 1.2);
	if (t > 10.4)
		stage_fade(d, rgb(8, 5, 16), fmin(0.7, (t - 10.4) * 1.2));
	hd_present(d);
	if (t > 2.0 && t < 10.4)
		chain_labels(d, &s, NCHAIN, 0);
	if (t > 2.6 && t < 5.8)
		bubble(d, &s, 124, 62, L("Moulinette est à moi.\nChaque chaîne est une de mes conversions.",
				"Moulinette is mine.\nEach chain is one of my conversions."));
	if (t > 5.9 && t < 7.4)
		bubble(d, &s, 70, 25, L("Miaou… (Marvin, vite !)", "Meow… (Marvin, hurry!)"));
	if (t > 8.6 && t < 10.4)
		bubble(d, &s, mx + 14, 33, L("Neuf chaînes, neuf conversions.\nOn commence.",
				"Nine chains, nine conversions.\nLet's begin."));
	if (t > 10.6)
	{
		ctext(d, d->ch / 2 - 1, rgb(255, 120, 220), L("L A   M A C H I N E   D E   L A   N O R M I N E T T E",
				"T H E   N O R M I N E T T E   M A C H I N E"));
		ctext(d, d->ch / 2 + 1, rgb(230, 220, 245), L("9 chaînes · 1 big boss · libère Moulinette",
				"9 chains · 1 big boss · free Moulinette"));
	}
	return (0);
}

static void	topbar(t_hd *d, int done, int n, int stage)
{
	char	tmp[200];
	int		i;
	int		k;

	k = snprintf(tmp, sizeof(tmp), "%s ", L("MINI-BOSS", "MINI-BOSSES"));
	for (i = 0; i < n; i++)
		k += snprintf(tmp + k, sizeof(tmp) - k, "%s", i < done ? "▰" : "▱");
	snprintf(tmp + k, sizeof(tmp) - k, " %d/%d   ─────   %s %s", done, n, stage ? "▶" : " ",
		L("BIG BOSS ft_printf", "BIG BOSS ft_printf"));
	ctext(d, 0, stage ? rgb(255, 120, 220) : rgb(230, 220, 245), tmp);
}

static int	sc_chains(t_hd *d, double t)
{
	t_stage	s;
	double	age;
	double	ax;
	double	ay;
	int		resolved;
	int		ok;
	int		i;
	int		shake;
	char	tmp[160];

	resolved = t < 2.0 ? 0 : (int)((t - 2.0) / 0.55) + 1;
	if (resolved > g_nres)
		resolved = g_nres;
	for (shake = 0, ok = 0, i = 0; i < resolved; i++)
	{
		ok += g_res[i] == 1;
		age = t - (2.0 + i * 0.55);
		if (age < 0.22)
			shake = (int)(sin(t * 90) * (g_res[i] == 1 ? 1 : 2));
	}
	stage_fit(d, &s, shake);
	workshop(d, &s, t, 1 + resolved * 0.3);
	draw_chains(d, &s, g_nres, t, resolved);
	if (resolved == g_nres && t > 2.0 + g_nres * 0.55)
		cat(d, &s, 58, 26 - (ok == g_nres ? fabs(sin(t * 5)) * 3 : 0), ok == g_nres ? 3 : cat_idle(t), 1, 0);
	else
		cat(d, &s, 58, 26, resolved ? 4 : cat_idle(t), 1, 0);
	for (i = 0; i < resolved; i++)
	{
		age = t - (2.0 + i * 0.55);
		anchor(i, g_nres, &ax, &ay);
		if (g_res[i] == 1)
			stage_sparks(d, &s, (ax + 70) / 2, (ay + 37) / 2, age, g_dlc ? g_dlcb[i].c : g_chains[i].c, i);
		if (g_res[i] == 0 && age < 0.3)
			stage_glow(d, &s, (ax + 70) / 2, (ay + 37) / 2, 12, rgb(255, 40, 60), 0.6 * (1 - age / 0.3));
		if (g_res[i] == 1 && age < 0.12)
			stage_fade(d, rgb(255, 255, 230), 0.25 * (1 - age / 0.12));
	}
	hd_present(d);
	chain_labels(d, &s, g_nres, resolved);
	topbar(d, ok, g_nres, 0);
	if (t < 2.0)
		ctext(d, d->ch - 3, rgb(230, 220, 245), g_dlc ? L("Les runes des flags s'éveillent…", "The flag runes awaken…")
			: L("Marvin attaque les 9 chaînes en même temps…", "Marvin strikes all 9 chains at once…"));
	if (resolved == g_nres && t > 2.0 + g_nres * 0.55)
	{
		snprintf(tmp, sizeof(tmp), L("%d/%d brisées", "%d/%d broken"), ok, g_nres);
		ctext(d, d->ch - 4, ok == g_nres ? rgb(80, 230, 140) : rgb(255, 200, 70), tmp);
		ctext(d, d->ch - 2, rgb(230, 220, 245), ok == g_nres ? (g_dlc ? L("Le monde des flags est à toi !",
					"The flag world is yours!") : L("Moulinette est libre !", "Moulinette is free!"))
			: L("Les chaînes rouges résistent : Résultats détaillés pour savoir pourquoi.",
				"The red chains hold: see Detailed results to know why."));
	}
	return (0);
}

static int	sc_freed(t_hd *d, double t)
{
	t_stage	s;
	double	k;

	stage_fit(d, &s, 0);
	workshop(d, &s, t, 0.4);
	stage_marvin(d, &s, 6, 33, t);
	k = fmin(1, t / 1.1);
	cat(d, &s, ease(58, 36, k), ease(26, 43, k) - sin(k * M_PI) * 18, k < 1 ? 3 : (fmod(t, 1.2) < 0.6 ? 3 : 0),
		1, k >= 1);
	if (k >= 1 && t < 1.6)
		stage_sparks(d, &s, 48, 62, t - 1.1, rgb(255, 210, 120), 3);
	hd_present(d);
	topbar(d, NCHAIN, NCHAIN, 0);
	if (t > 1.3)
		bubble(d, &s, 48, 43, L("Merci Marvin ! Attention… il arrive !",
				"Thank you Marvin! Watch out… here he comes!"));
	return (0);
}

static void	colossus(t_hd *d, const t_stage *s, double x, double y, double t, double rise, double flash,
	double fade)
{
	const t_pic		*p;
	int				fw;
	int				i;
	int				j;
	int				u;
	int				v;
	int				x0;
	int				y0;
	int				f;
	double			al;
	unsigned char	*c;

	p = art_pic("ft_printf");
	if (!p || fade <= 0)
		return ;
	fw = p->w / 3;
	f = (int)(t * 5) % 3;
	x0 = s->ox + (int)floor((x - fw / 2.0 + sin(t * 1.3) * 1.2) * s->k);
	y0 = s->oy + (int)floor((y - p->h + rise) * s->k);
	stage_glow(d, s, x, y - 52 + rise, 30, rgb(255, 60, 180), 0.3 * fade);
	for (j = 0; j < p->h - (int)rise; j++)
		for (i = 0; i < fw; i++)
		{
			al = p->a[(size_t)j * p->w + f * fw + i] / 255.0 * fade;
			if (al < 0.06)
				continue ;
			c = p->rgb + ((size_t)j * p->w + f * fw + i) * 3;
			for (v = 0; v < s->k; v++)
				for (u = 0; u < s->k; u++)
					hd_blend(d, x0 + i * s->k + u, y0 + j * s->k + v,
						mix(rgb(c[0], c[1], c[2]), rgb(255, 255, 255), flash), al);
		}
	if (fade > 0.5 && rise < 40)
	{
		stage_glow(d, s, x - fw / 2.0 + 25.5 + sin(t * 1.3) * 1.2, y - p->h + rise + 13.5, 4, rgb(255, 120, 230), 0.7);
		stage_glow(d, s, x - fw / 2.0 + 34.5 + sin(t * 1.3) * 1.2, y - p->h + rise + 13.5, 4, rgb(255, 120, 230), 0.7);
	}
}

static int	sc_preboss(t_hd *d, double t)
{
	t_stage	s;
	double	k;

	k = fmin(1, t / 2.5);
	stage_fit(d, &s, t < 2.5 ? (int)(sin(t * 60) * 1.2) : 0);
	workshop(d, &s, t, 3);
	stage_fade(d, rgb(30, 4, 30), k * 0.5);
	colossus(d, &s, 102, 82, t, (1 - ease(0, 1, k)) * 66, 0, fmin(1, k * 2));
	if (t < 2.5)
		stage_sparks(d, &s, 102, 80, fmod(t, 0.5), rgb(200, 120, 255), (int)(t * 2));
	stage_marvin(d, &s, 6, 33, t);
	cat(d, &s, 34, 43, 4, 1, 1);
	hd_present(d);
	topbar(d, NCHAIN, NCHAIN, 1);
	if (t > 2.6)
		bubble(d, &s, 76, 24, L("Je suis le format. Je vérifie TOUT :\nfuites, Norme, Makefile, README…",
				"I am the format. I check EVERYTHING:\nleaks, Norm, Makefile, README…"));
	if (t > 4.4)
		ctext(d, d->ch - 2, rgb(255, 120, 220), "f t _ p r i n t f");
	return (0);
}

static int	sc_bigboss(t_hd *d, double t)
{
	t_stage	s;
	double	hp;
	double	fl;
	double	ph_t;
	double	gone;
	int		ph;
	int		i;
	int		dmg;
	int		hit;
	char	tmp[200];

	ph = t < 0.6 ? -1 : (int)((t - 0.6) / 1.3);
	ph_t = t < 0.6 ? 0 : fmod(t - 0.6, 1.3);
	for (dmg = 0, i = 0; i <= ph && i < NPHASE; i++)
		dmg += g_ph[i];
	hp = 1 - dmg / (double)NPHASE;
	hit = ph >= 0 && ph < NPHASE && ph_t < 0.35;
	fl = hit && g_ph[ph] ? 0.45 * (1 - ph_t / 0.35) : 0;
	stage_fit(d, &s, hit && ph_t < 0.2 ? (int)(sin(t * 90) * 2) : 0);
	workshop(d, &s, t, 2);
	stage_fade(d, rgb(24, 4, 30), 0.55);
	gone = ph >= NPHASE && dmg == NPHASE ? fmin(1, (t - 0.6 - NPHASE * 1.3) / 2.0) : 0;
	colossus(d, &s, 102, 82, t, gone * 66, gone > 0 ? 0.3 : fl, 1 - gone * 0.6);
	stage_marvin(d, &s, 10, 34, t);
	if (hit && g_ph[ph])
	{
		hd_line(d, s.ox + 34 * s.k, s.oy + 48 * s.k, s.ox + 98 * s.k, s.oy + 44 * s.k, rgb(255, 255, 230));
		hd_line(d, s.ox + 34 * s.k, s.oy + 49 * s.k, s.ox + 98 * s.k, s.oy + 45 * s.k, rgb(255, 200, 120));
		stage_sparks(d, &s, 98, 44, ph_t, rgb(255, 220, 140), ph);
	}
	else if (hit)
	{
		stage_pic(d, &s, "tampon", 16, ph_t > 0.12, 16, ease(-24, 12, ph_t / 0.12), 1);
		stage_glow(d, &s, 24, 50, 22, rgb(255, 40, 60), 0.6 * (1 - ph_t / 0.35));
	}
	if (gone > 0)
		cat(d, &s, 40, 44, 3, 1, 1);
	hd_present(d);
	snprintf(tmp, sizeof(tmp), "ft_printf  ");
	for (i = 0; i < 20; i++)
		strcat(tmp, i < (int)(hp * 20 + 0.5) ? "█" : "░");
	ctext(d, 1, rgb(255, 120, 220), tmp);
	topbar(d, NCHAIN, NCHAIN, 1);
	for (i = 0; i <= ph && i < NPHASE; i++)
	{
		snprintf(tmp, sizeof(tmp), "%s %s", g_ph[i] ? "✓" : "✗", L(g_phases[i].fr, g_phases[i].en));
		hd_text(d, 2, 3 + i, g_ph[i] ? rgb(80, 230, 140) : rgb(255, 90, 100), d->px[((3 + i) * 2 + 1) * d->w + 2], tmp);
	}
	if (ph >= NPHASE)
		ctext(d, d->ch - 2, dmg == NPHASE ? rgb(80, 230, 140) : rgb(255, 200, 70), dmg == NPHASE
			? L("ft_printf est vaincu !", "ft_printf is defeated!")
			: L("ft_printf résiste : corrige les phases en rouge (Résultats détaillés).",
				"ft_printf holds: fix the red phases (Detailed results)."));
	return (0);
}

static int	sc_victory(t_hd *d, double t)
{
	t_stage	s;
	int		i;
	int		p;
	double	x;
	double	y;

	stage_fit(d, &s, 0);
	stage_bg(d, &s, "restaurant", 0, rgb(120, 70, 50), rgb(80, 46, 36));
	for (i = 0; i < 34; i++)
	{
		x = 92 + fmod(i * 7.3, 40);
		y = 8 + fmod(t * 34 + i * 11.7, 38);
		for (p = 0; p < 3 && y + p < 46; p++)
			hd_blend(d, s.ox + (int)((x - p * 0.4) * s.k), s.oy + (int)((y + p) * s.k), rgb(150, 180, 230), 0.7 - p * 0.2);
	}
	if (g_secret)
		for (p = 0; p < 3; p++)
			stage_pic(d, &s, "peluches", 13, p, 4 + p * 14, 50 - fabs(sin(t * 3 + p)) * 2, 1);
	cat(d, &s, 58, 41 + (fmod(t, 2.4) < 0.15 ? 1 : 0), t > 2.4 && t < 6.5 ? 3 : cat_idle(t), 1, 1);
	for (p = 0; p < 2; p++)
	{
		x = p ? 98 : 18;
		stage_pic(d, &s, "tasse", 0, 0, x, 64, 2);
		for (i = 0; i < 7; i++)
			stage_glow(d, &s, x + 9 + sin(t * 2 + i * 0.8 + p) * 2, 62 - fmod(t * 6 + i * 3, 20), 2.2,
				rgb(255, 255, 255), 0.25);
	}
	stage_glow(d, &s, 46, 16, 30, rgb(255, 210, 140), 0.12 + 0.03 * sin(t * 2));
	if (t < 1.0)
		stage_fade(d, rgb(0, 0, 0), 1 - t);
	hd_present(d);
	if (t > 1.5)
		ctext(d, 2, rgb(80, 230, 140), L("P R I N T F   V A L I D É", "P R I N T F   V A L I D A T E D"));
	if (t > 2.4 && t < 6.5)
		bubble(d, &s, 70, 41, g_secret ? L("Rrr… regarde, Marvin : les trois gardiens\nen peluche !",
				"Purr… look, Marvin: the three guardians\nas plushies!") : L("Rrr… merci, Marvin.", "Purr… thank you, Marvin."));
	if (t > 6.6 && g_secret)
		ctext(d, d->ch - 2, rgb(255, 215, 120), L("Fin secrète · Libft et Printf validés", "Secret ending · Libft and Printf validated"));
	return (0);
}

void	story_splash(void)
{
	cine_play(sc_splash, 5.5);
}

void	story_previously(void)
{
	cine_play(sc_previously, 7.5);
}

void	story_printf_intro(void)
{
	cine_play(sc_intro, 13.0);
}

int	story_chains(int dlc)
{
	int	i;
	int	ok;

	g_dlc = dlc;
	g_nres = dlc ? NDLC : NCHAIN;
	for (ok = 0, i = 0; i < g_nres; i++)
	{
		if (dlc)
			g_res[i] = g_pf.dlc[i] == '1';
		else
			g_res[i] = pf_cat(g_chains[i].key) ? pf_chain_ok(i) : -1;
		ok += g_res[i] == 1;
	}
	cine_play(sc_chains, 2.0 + g_nres * 0.55 + 2.5);
	return (ok);
}

void	story_freed(void)
{
	cine_play(sc_freed, 4.0);
}

void	story_preboss(void)
{
	cine_play(sc_preboss, 6.0);
}

int	story_bigboss(void)
{
	int	i;
	int	ok;

	for (ok = 0, i = 0; i < NPHASE; i++)
	{
		g_ph[i] = pf_phase_ok(i);
		ok += g_ph[i];
	}
	cine_play(sc_bigboss, 0.6 + NPHASE * 1.3 + 3.0);
	return (ok == NPHASE);
}

void	story_victory(int secret)
{
	pid_t	pid;
	int		st;

	g_secret = secret;
	if (secret && getenv("TDF_SCENE") && *getenv("TDF_SCENE"))
	{
		term_pause();
		st = -1;
		pid = fork();
		if (pid == 0)
		{
			execl(getenv("TDF_SCENE"), getenv("TDF_SCENE"), g_tester_dir, "secrete", (char *)NULL);
			_exit(127);
		}
		if (pid > 0)
			waitpid(pid, &st, 0);
		if (pid > 0 && WIFEXITED(st) && WEXITSTATUS(st) == 0)
		{
			term_resume();
			term_invalidate();
			return ;
		}
		term_resume();
		term_invalidate();
	}
	cine_play(sc_victory, secret ? 10.0 : 8.0);
}

void	story_dlc(void)
{
	story_chains(1);
}
