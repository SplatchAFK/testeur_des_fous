#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

/*
** Moteur de cinématiques « HD » : tout le terminal devient une image de
** term_cols() x 2 * term_rows() pixels (demi-blocs ▀, couleurs 24 bits).
** Seules les cases qui changent sont réécrites : une scène fluide reste
** légère même dans une grande fenêtre.
*/

static uint64_t	pack(t_rgb a, t_rgb b)
{
	return (((uint64_t)a.r << 40) | ((uint64_t)a.g << 32) | ((uint64_t)a.b << 24)
		| ((uint64_t)b.r << 16) | ((uint64_t)b.g << 8) | (uint64_t)b.b);
}

void	hd_begin(t_hd *d)
{
	d->cw = term_cols();
	d->ch = term_rows();
	d->w = d->cw;
	d->h = d->ch * 2;
	d->px = calloc((size_t)d->w * d->h, sizeof(t_rgb));
	d->prev = malloc((size_t)d->cw * d->ch * sizeof(*d->prev));
	memset(d->prev, 0xff, (size_t)d->cw * d->ch * sizeof(*d->prev));
	d->och = calloc((size_t)d->cw * d->ch, sizeof(*d->och));
	d->ofg = calloc((size_t)d->cw * d->ch, sizeof(t_rgb));
	d->obg = calloc((size_t)d->cw * d->ch, sizeof(t_rgb));
	d->ohas = calloc((size_t)d->cw * d->ch, 1);
	out("\033[?25l");
}

void	hd_end(t_hd *d)
{
	free(d->px);
	free(d->prev);
	free(d->och);
	free(d->ofg);
	free(d->obg);
	free(d->ohas);
	d->px = NULL;
	d->prev = NULL;
	bgc(theme()->bg);
	cls();
}

/* gardé pour compatibilité : le texte est maintenant fusionné à l'image */
void	hd_dirty(t_hd *d, int cx, int cy, int w, int h)
{
	(void)d;
	(void)cx;
	(void)cy;
	(void)w;
	(void)h;
}

static uint64_t	ovl_key(t_hd *d, int i)
{
	uint64_t	h;
	int			k;

	h = 0x9e3779b97f4a7c15ULL;
	for (k = 0; k < 8 && d->och[i][k]; k++)
		h = (h ^ (unsigned char)d->och[i][k]) * 1099511628211ULL;
	h ^= ((uint64_t)d->ofg[i].r << 16 | d->ofg[i].g << 8 | d->ofg[i].b) * 0x100000001b3ULL;
	h ^= ((uint64_t)d->obg[i].r << 40 | (uint64_t)d->obg[i].g << 32 | (uint64_t)d->obg[i].b << 24);
	return (h | 1ULL << 63);
}

/* une image : les pixels (demi-blocs) et, par-dessus, la couche de texte */
static void	present_real(t_hd *d)
{
	static char	b[32];
	int			x;
	int			y;
	int			lx;
	int			ly;
	t_rgb		a;
	t_rgb		c;
	uint64_t	k;
	uint64_t	lf;
	uint64_t	lb;

	lx = -9;
	ly = -9;
	lf = UINT64_MAX;
	lb = UINT64_MAX;
	for (y = 0; y < d->ch; y++)
		for (x = 0; x < d->cw; x++)
		{
			a = d->px[(y * 2) * d->w + x];
			c = d->px[(y * 2 + 1) * d->w + x];
			k = d->ohas[y * d->cw + x] ? ovl_key(d, y * d->cw + x) : pack(a, c) & ~(1ULL << 63);
			if (d->prev[y * d->cw + x] == k)
				continue ;
			d->prev[y * d->cw + x] = k;
			if (ly != y || lx != x)
				at(x, y);
			if (d->ohas[y * d->cw + x])
			{
				fg(d->ofg[y * d->cw + x]);
				bgc(d->obg[y * d->cw + x]);
				out(d->och[y * d->cw + x]);
				lf = UINT64_MAX;
				lb = UINT64_MAX;
				lx = x + 1;
				ly = y;
				continue ;
			}
			if (lf != (k >> 24))
				outn(b, color_seq(b, a, 0));
			if (lb != (k & 0xffffff))
				outn(b, color_seq(b, c, 1));
			lf = k >> 24;
			lb = k & 0xffffff;
			out("▀");
			lx = x + 1;
			ly = y;
		}
}

void	hd_present(t_hd *d)
{
	(void)d;
}

void	hd_px(t_hd *d, int x, int y, t_rgb c)
{
	if (x >= 0 && y >= 0 && x < d->w && y < d->h)
		d->px[y * d->w + x] = c;
}

void	hd_blend(t_hd *d, int x, int y, t_rgb c, double k)
{
	if (x >= 0 && y >= 0 && x < d->w && y < d->h && k > 0)
		d->px[y * d->w + x] = mix(d->px[y * d->w + x], c, k > 1 ? 1 : k);
}

void	hd_rect(t_hd *d, int x, int y, int w, int h, t_rgb c)
{
	int	i;
	int	j;

	for (j = y; j < y + h; j++)
		for (i = x; i < x + w; i++)
			hd_px(d, i, j, c);
}

void	hd_vgrad(t_hd *d, int y0, int y1, t_rgb top, t_rgb bot)
{
	int	x;
	int	y;

	for (y = y0; y < y1; y++)
		for (x = 0; x < d->w; x++)
			hd_px(d, x, y, mix(top, bot, (y - y0) / (double)(y1 - y0 > 1 ? y1 - y0 : 1)));
}

void	hd_ellipse(t_hd *d, double cx, double cy, double rx, double ry, t_rgb c)
{
	int		x;
	int		y;
	double	u;
	double	v;

	for (y = (int)(cy - ry) - 1; y <= (int)(cy + ry) + 1; y++)
		for (x = (int)(cx - rx) - 1; x <= (int)(cx + rx) + 1; x++)
		{
			u = (x + 0.5 - cx) / (rx > 0.1 ? rx : 0.1);
			v = (y + 0.5 - cy) / (ry > 0.1 ? ry : 0.1);
			if (u * u + v * v <= 1.0)
				hd_px(d, x, y, c);
		}
}

void	hd_glow(t_hd *d, double cx, double cy, double r, t_rgb c, double k)
{
	int		x;
	int		y;
	double	q;

	for (y = (int)(cy - r); y <= (int)(cy + r); y++)
		for (x = (int)(cx - r); x <= (int)(cx + r); x++)
		{
			q = 1 - sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy)) / r;
			if (q > 0)
				hd_blend(d, x, y, c, q * q * k);
		}
}

void	hd_line(t_hd *d, double x0, double y0, double x1, double y1, t_rgb c)
{
	double	n;
	double	i;

	n = fabs(x1 - x0) > fabs(y1 - y0) ? fabs(x1 - x0) : fabs(y1 - y0);
	if (n < 1)
		n = 1;
	for (i = 0; i <= n; i++)
		hd_px(d, (int)floor(x0 + (x1 - x0) * i / n), (int)floor(y0 + (y1 - y0) * i / n), c);
}

void	hd_tri(t_hd *d, double ax, double ay, double bx, double by, double cx, double cy, t_rgb c)
{
	int		x;
	int		y;
	double	w0;
	double	w1;
	double	w2;
	double	ar;

	ar = (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
	if (fabs(ar) < 1e-6)
		return ;
	for (y = (int)fmin(ay, fmin(by, cy)); y <= (int)fmax(ay, fmax(by, cy)); y++)
		for (x = (int)fmin(ax, fmin(bx, cx)); x <= (int)fmax(ax, fmax(bx, cx)); x++)
		{
			w0 = ((bx - x - 0.5) * (cy - y - 0.5) - (by - y - 0.5) * (cx - x - 0.5)) / ar;
			w1 = ((cx - x - 0.5) * (ay - y - 0.5) - (cy - y - 0.5) * (ax - x - 0.5)) / ar;
			w2 = 1 - w0 - w1;
			if (w0 >= 0 && w1 >= 0 && w2 >= 0)
				hd_px(d, x, y, c);
		}
}

/* copie un canevas (avatar, boss) avec un facteur d'échelle */
void	hd_canvas(t_hd *d, const t_canvas *c, double x, double y, double s, double fade)
{
	int	i;
	int	j;
	int	u;
	int	v;

	if (s <= 0)
		return ;
	for (j = 0; j < (int)(c->h * s); j++)
		for (i = 0; i < (int)(c->w * s); i++)
		{
			u = (int)(i / s);
			v = (int)(j / s);
			if (u < c->w && v < c->h && c->a[v * c->w + u])
				hd_blend(d, (int)x + i, (int)y + j, c->px[v * c->w + u], fade);
		}
}

/* sprite en caractères : chaque lettre est une couleur de la palette */
void	hd_art(t_hd *d, const char *const *rows, const t_key *pal, double x, double y, double s)
{
	int		i;
	int		j;
	int		k;
	int		w;
	int		h;
	char	ch;

	for (h = 0; rows[h]; h++)
		;
	w = (int)strlen(rows[0]);
	for (j = 0; j < (int)(h * s); j++)
		for (i = 0; i < (int)(w * s); i++)
		{
			ch = rows[(int)(j / s)][(int)(i / s)];
			for (k = 0; pal[k].k && pal[k].k != ch; k++)
				;
			if (ch != '.' && pal[k].k)
				hd_px(d, (int)x + i, (int)y + j, pal[k].c);
		}
}

void	hd_stars(t_hd *d, double t, int ymax, unsigned seed)
{
	int		i;
	int		n;
	double	tw;

	t = floor(t * 4) / 4;
	n = d->w * ymax / 60;
	for (i = 0; i < n; i++)
	{
		seed = seed * 1103515245u + 12345u;
		tw = 0.5 + 0.5 * sin(t * (1.5 + (seed >> 28)) + i);
		hd_blend(d, (int)((seed >> 8) % (unsigned)d->w), (int)((seed >> 3) % (unsigned)(ymax > 0 ? ymax : 1)),
			rgb(255, 255, 240), 0.25 + 0.6 * tw);
	}
}

/* une case de texte : un caractère UTF-8, sa couleur et son fond */
static void	ovl_put(t_hd *d, int x, int y, const char *ch, int n, t_rgb c, t_rgb bg)
{
	int	i;

	if (x < 0 || y < 0 || x >= d->cw || y >= d->ch || n <= 0 || n > 7)
		return ;
	i = y * d->cw + x;
	memcpy(d->och[i], ch, n);
	d->och[i][n] = '\0';
	d->ofg[i] = c;
	d->obg[i] = bg;
	d->ohas[i] = 1;
}

static int	u8len(unsigned char c)
{
	if (c >= 0xf0)
		return (4);
	if (c >= 0xe0)
		return (3);
	if (c >= 0xc0)
		return (2);
	return (1);
}

/* texte fusionné à l'image (coordonnées en cases) */
void	hd_text(t_hd *d, int cx, int cy, t_rgb c, t_rgb bg, const char *s)
{
	int	n;

	while (*s)
	{
		n = u8len((unsigned char)*s);
		ovl_put(d, cx++, cy, s, n, c, bg);
		while (n-- && *s)
			s++;
	}
}

/* fond de la case : la couleur du pixel du bas */
static t_rgb	under(t_hd *d, int x, int y)
{
	if (x < 0 || y < 0 || x >= d->cw || y >= d->ch)
		return (rgb(0, 0, 0));
	return (d->px[(y * 2 + 1) * d->w + x]);
}

/* bulle de dialogue façon jeu vidéo ; (cx, cy) = case pointée par la queue */
void	hd_bubble(t_hd *d, int cx, int cy, const char *s, t_rgb ink)
{
	char		lines[4][160];
	char		row[200];
	int			n;
	int			w;
	int			i;
	int			x;
	int			y;
	const char	*p;
	const char	*e;
	t_rgb		paper;

	n = 0;
	w = 0;
	for (p = s; *p && n < 4; n++)
	{
		e = strchr(p, '\n');
		snprintf(lines[n], sizeof(lines[n]), "%.*s", e ? (int)(e - p) : (int)strlen(p), p);
		if (ulen(lines[n]) > w)
			w = ulen(lines[n]);
		p = e ? e + 1 : p + strlen(p);
	}
	paper = rgb(250, 248, 240);
	x = cx - w / 2 - 2;
	if (x + w + 4 > d->cw)
		x = d->cw - w - 4;
	if (x < 0)
		x = 0;
	y = cy - n - 2;
	if (y < 0)
		y = 0;
	for (i = -1; i <= n; i++)
	{
		if (i < 0 || i == n)
			snprintf(row, sizeof(row), "%*s", w + 4, "");
		else
			snprintf(row, sizeof(row), "  %s%*s  ", lines[i], w - ulen(lines[i]), "");
		hd_text(d, x, y + 1 + i, ink, paper, row);
	}
	cx = cx < x + 1 ? x + 1 : (cx > x + w + 2 ? x + w + 2 : cx);
	hd_text(d, cx, y + n + 2, paper, under(d, cx, y + n + 2), "▼");
}

/*
** Joue une scène : scene(d, t) dessine l'image au temps t (en secondes).
** Une touche passe la cinématique. Renvoie 1 si elle a été passée.
*/
int	cine_play(int (*scene)(t_hd *, double), double dur)
{
	t_hd	d;
	long	t0;
	double	t;
	double	speed;
	int		k;

	if (anim_off() || g_save.cfg.lore == 2)
		return (1);
	speed = g_save.cfg.anim == 1 ? 2.0 : 1.0;
	hd_begin(&d);
	bgc(rgb(0, 0, 0));
	frame();
	t0 = now_ms();
	k = -1;
	while (1)
	{
		t = (now_ms() - t0) / 1000.0 * speed;
		if (t > dur)
			break ;
		memset(d.ohas, 0, (size_t)d.cw * d.ch);
		if (scene(&d, t))
			break ;
		present_real(&d);
		flush();
		k = key_read(33);
		if (k >= 0)
			break ;
	}
	flush();
	hd_end(&d);
	out("\033[?25l");
	return (k >= 0);
}

/* ---------------------------------------------------------------- mise en scène */

/*
** Toutes les cinématiques sont dessinées pour 140 x 84 pixels (la fenêtre de
** 140 x 42 que le testeur ouvre). Plus grand : on agrandit d'un facteur
** entier, jamais déformé. Plus petit : on coupe les bords, centré.
*/
void	stage_fit(t_hd *d, t_stage *s, int shake)
{
	s->k = d->w / 140 < d->h / 84 ? d->w / 140 : d->h / 84;
	if (s->k < 1)
		s->k = 1;
	s->ox = (d->w - 140 * s->k) / 2 + shake * s->k;
	s->oy = (d->h - 84 * s->k) / 2;
}

static int	clampi(int v, int lo, int hi)
{
	return (v < lo ? lo : (v > hi ? hi : v));
}

/* décor plein écran ; sy = première ligne de l'image (décors hauts qui défilent) */
void	stage_bg(t_hd *d, const t_stage *s, const char *name, int sy, t_rgb top, t_rgb bot)
{
	const t_pic		*p;
	int				x;
	int				y;
	int				u;
	int				v;
	int				out_x;
	int				out_y;
	unsigned char	*c;

	p = art_pic(name);
	if (!p)
	{
		hd_vgrad(d, 0, d->h, top, bot);
		return ;
	}
	for (y = 0; y < d->h; y++)
	{
		v = y - s->oy;
		out_y = v < 0 || v >= 84 * s->k;
		v = clampi((v < 0 ? v - s->k + 1 : v) / s->k + sy, 0, p->h - 1);
		for (x = 0; x < d->w; x++)
		{
			u = x - s->ox;
			out_x = u < 0 || u >= 140 * s->k;
			u = clampi((u < 0 ? u - s->k + 1 : u) / s->k, 0, p->w - 1);
			c = p->rgb + ((size_t)v * p->w + u) * 3;
			if (out_x || out_y)
				hd_px(d, x, y, mix(rgb(c[0], c[1], c[2]), rgb(0, 0, 0), 0.6));
			else
				hd_px(d, x, y, rgb(c[0], c[1], c[2]));
		}
	}
}

int	stage_col(const t_stage *s, double x)
{
	return (s->ox + (int)floor(x * s->k));
}

int	stage_row(const t_stage *s, double y)
{
	return ((s->oy + (int)floor(y * s->k)) / 2);
}

/* image (ou case fw de large d'une planche) en coordonnées de scène ; zoom entier */
void	stage_pic(t_hd *d, const t_stage *s, const char *name, int fw, int frame, double x, double y, int zoom)
{
	hd_pic(d, art_pic(name), fw, frame, s->ox + (int)floor(x * s->k), s->oy + (int)floor(y * s->k),
		s->k * (zoom < 1 ? 1 : zoom), 0);
}

/* ombre douce au sol, sous un personnage */
void	stage_shadow(t_hd *d, const t_stage *s, double cx, double cy, double rx, double k)
{
	int		x;
	int		y;
	double	ry;
	double	e;
	double	px;
	double	py;

	ry = rx * 0.32 + 0.6;
	for (y = (int)floor((cy - ry) * s->k); y <= (int)ceil((cy + ry) * s->k); y++)
		for (x = (int)floor((cx - rx) * s->k); x <= (int)ceil((cx + rx) * s->k); x++)
		{
			px = (x / (double)s->k - cx) / rx;
			py = (y / (double)s->k - cy) / ry;
			e = px * px + py * py;
			if (e < 1)
				hd_blend(d, s->ox + x, s->oy + y, rgb(8, 6, 16), k * (e < 0.45 ? 1 : (1 - e) / 0.55));
		}
}

/* le perso du joueur, sans son fond de profil, avec une ombre ; (x, y) = coin haut gauche */
void	stage_marvin(t_hd *d, const t_stage *s, double x, double y, double t)
{
	t_canvas	cv;
	t_look		lk;

	lk = g_save.look;
	lk.eq[SL_FOND] = -1;
	stage_shadow(d, s, x + 14, y + 31.5, 10, 0.55);
	cv_init(&cv, 28, 32);
	avatar_draw(&cv, 0, 0, &lk, (long)(t * 1000));
	hd_canvas(d, &cv, s->ox + (int)floor(x * s->k), s->oy + (int)floor(y * s->k), s->k, 1);
	cv_free(&cv);
}

/* un gardien (41 x 37) à taille entière */
void	stage_boss(t_hd *d, const t_stage *s, int p, double x, double y, double fade)
{
	t_canvas	cv;

	cv_init(&cv, 41, 37);
	cv_spr(&cv, &g_boss_spr[p], 0, 0, g_bosses[p].glow, rgb(255, 255, 255), 0);
	hd_canvas(d, &cv, s->ox + (int)floor(x * s->k), s->oy + (int)floor(y * s->k), s->k, fade);
	cv_free(&cv);
}

void	stage_glow(t_hd *d, const t_stage *s, double x, double y, double r, t_rgb c, double k)
{
	hd_glow(d, s->ox + x * s->k, s->oy + y * s->k, r * s->k, c, k);
}

/* tout l'écran vers une couleur (fondu au noir, flash blanc) */
void	stage_fade(t_hd *d, t_rgb c, double k)
{
	int	i;

	if (k <= 0)
		return ;
	k = k > 1 ? 1 : k;
	for (i = 0; i < d->w * d->h; i++)
		d->px[i] = mix(d->px[i], c, k);
}

/* gerbe d'étincelles qui retombent ; age en secondes */
void	stage_sparks(t_hd *d, const t_stage *s, double x, double y, double age, t_rgb c, int seed)
{
	int			i;
	unsigned	h;
	double		a;
	double		v;
	double		px;
	double		py;

	if (age < 0 || age > 0.9)
		return ;
	for (i = 0; i < 14; i++)
	{
		h = (unsigned)(i + seed * 31) * 2654435761u;
		a = (h % 628) / 100.0;
		v = 18 + (h >> 12) % 22;
		px = x + cos(a) * v * age;
		py = y + sin(a) * v * age * 0.7 + 30 * age * age;
		hd_blend(d, s->ox + (int)(px * s->k), s->oy + (int)(py * s->k), i % 3 ? c : rgb(255, 255, 255), 1 - age / 0.9);
		if (s->k > 1)
			hd_blend(d, s->ox + (int)(px * s->k) + 1, s->oy + (int)(py * s->k), c, 1 - age / 0.9);
	}
}
