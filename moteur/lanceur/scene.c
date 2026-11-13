#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

t_save		g_save;
const char	*g_tester_dir = ".";

static const t_theme	g_th = {"nuit", "Nuit", 0, {118, 222, 238}, {234, 142, 72}, {30, 30, 40},
	{20, 20, 28}, {220, 222, 228}, {120, 126, 140}, {12, 14, 20}};

static const int	g_ann[22][2] = {
	{233, 47}, {407, 60}, {233, 73}, {407, 86}, {233, 99}, {407, 112}, {233, 125}, {407, 138},
	{233, 151}, {407, 163}, {233, 176}, {407, 189}, {233, 202}, {407, 215}, {233, 228}, {407, 241},
	{233, 254}, {407, 267}, {233, 279}, {407, 292}, {233, 305}, {407, 318}
};

const t_theme	*theme(void)
{
	return (&g_th);
}

void	toast(const char *title, const char *msg, t_rgb c)
{
	(void)title;
	(void)msg;
	(void)c;
}

void	save_write(void)
{
}

void	foot_line(int x, const char *s)
{
	text_clip(x, term_rows() - 1, g_th.dim, s, term_cols() - x - 1);
}

t_rgb	pseudo_color(void)
{
	return (g_th.accent);
}

int	ask_text(const char *q, const char *hint, char *buf, int size)
{
	(void)q;
	(void)hint;
	if (size > 0)
		buf[0] = '\0';
	return (0);
}

void	hd_px(t_hd *d, int x, int y, t_rgb c)
{
	(void)d;
	(void)x;
	(void)y;
	(void)c;
}

void	hd_blend(t_hd *d, int x, int y, t_rgb c, double k)
{
	(void)d;
	(void)x;
	(void)y;
	(void)c;
	(void)k;
}

static void	lueur(unsigned char *img, int w, int h, double cx, double cy, double r, t_rgb c, double k)
{
	int		x;
	int		y;
	double	d;
	double	a;

	for (y = (int)(cy - r * 2); y <= (int)(cy + r * 2); y++)
		for (x = (int)(cx - r * 2); x <= (int)(cx + r * 2); x++)
		{
			if (x < 0 || y < 0 || x >= w || y >= h)
				continue ;
			d = ((x - cx) * (x - cx) + (y - cy) * (y - cy)) / (r * r);
			if (d >= 4)
				continue ;
			a = k * (1 - d / 4) * (1 - d / 4);
			img[(y * w + x) * 3] += (c.r - img[(y * w + x) * 3]) * a;
			img[(y * w + x) * 3 + 1] += (c.g - img[(y * w + x) * 3 + 1]) * a;
			img[(y * w + x) * 3 + 2] += (c.b - img[(y * w + x) * 3 + 2]) * a;
		}
}

static unsigned char	*composer(const t_pic *b, const char *res, long t, int *prof, int *nok)
{
	unsigned char	*img;
	int				i;
	double			pu;

	if (!(img = malloc((size_t)b->w * b->h * 3)))
		return (NULL);
	memcpy(img, b->rgb, (size_t)b->w * b->h * 3);
	*prof = -1;
	*nok = 0;
	for (i = 0; i < 22 && res[i]; i++)
	{
		if (res[i] == 'O')
			(*nok)++;
		if (res[i] == 'O' && *prof == i - 1)
			*prof = i;
	}
	for (i = 0; i < 22 && res[i]; i++)
	{
		if (res[i] == 'O')
			lueur(img, b->w, b->h, g_ann[i][0], g_ann[i][1], 4, rgb(118, 222, 238), 0.75);
		else if (res[i] == 'K')
			lueur(img, b->w, b->h, g_ann[i][0], g_ann[i][1], 4, rgb(220, 70, 60), 0.6);
	}
	pu = 1.0;
	if (*prof >= 0)
		lueur(img, b->w, b->h, 320, g_ann[*prof][1], 14 * pu, rgb(160, 236, 244), 0.8);
	else
		lueur(img, b->w, b->h, 320, 18, 14 * pu, rgb(160, 236, 244), 0.8);
	if (*nok == 22)
		for (i = 0; i < 5; i++)
			lueur(img, b->w, b->h, 428 + i * 13.5, 312 - i, 3 + pu * 2, rgb(150, 240, 250), 0.9);
	return (img);
}

static void	empiler(unsigned char *img, const t_pic *p, int w, int h)
{
	int		i;
	double	a;

	if (!p || p->w != w || p->h != h)
		return ;
	for (i = 0; i < w * h; i++)
	{
		if (p->a[i] < 6)
			continue ;
		a = p->a[i] / 255.0;
		img[i * 3] += (p->rgb[i * 3] - img[i * 3]) * a;
		img[i * 3 + 1] += (p->rgb[i * 3 + 1] - img[i * 3 + 1]) * a;
		img[i * 3 + 2] += (p->rgb[i * 3 + 2] - img[i * 3 + 2]) * a;
	}
}

static unsigned char	*epreuve(const t_pic *b, int n)
{
	unsigned char	*img;
	char			nom[40];
	int				i;

	if (!(img = malloc((size_t)b->w * b->h * 3)))
		return (NULL);
	memcpy(img, b->rgb, (size_t)b->w * b->h * 3);
	for (i = 0; i < n && i < 4; i++)
	{
		snprintf(nom, sizeof(nom), "epreuve_porte_%d", i);
		empiler(img, art_pic(nom), b->w, b->h);
	}
	if (n >= 4)
		empiler(img, art_pic("epreuve_lanterne"), b->w, b->h);
	return (img);
}

static unsigned char	*copie(const t_pic *b)
{
	unsigned char	*img;

	if ((img = malloc((size_t)b->w * b->h * 3)))
		memcpy(img, b->rgb, (size_t)b->w * b->h * 3);
	return (img);
}

static void	montrer(const char *mode, const char *arg)
{
	const t_pic		*b;
	unsigned char	*img;
	char			tmp[200];
	const char		*titre;
	const char		*texte;
	const char		*pied;
	long			t0;
	int				k;
	int				cw;
	int				ch;
	int				prof;
	int				nok;
	int				n;

	prof = -1;
	nok = 0;
	n = arg ? atoi(arg) : 0;
	titre = "";
	texte = "";
	pied = "Entrée : continuer";
	tmp[0] = '\0';
	if (!strcmp(mode, "puits"))
		b = art_pic("puits_fond");
	else if (!strcmp(mode, "avant") || !strcmp(mode, "apres"))
		b = art_pic(!strcmp(mode, "avant") ? "epreuve_avant" : "epreuve_salle");
	else
		b = art_pic(!strcmp(mode, "fin") ? "fin" : "fin_secrete");
	if (!b)
		return ;
	if (!strcmp(mode, "avant"))
	{
		titre = "L ' É P R E U V E   D U   S I L E N C E";
		texte = "Marvin pose sa lanterne et entre seul. Ici, personne ne t'aidera.";
		pied = "Entrée : entrer dans la salle";
	}
	else if (!strcmp(mode, "apres"))
	{
		titre = "L ' É P R E U V E   D U   S I L E N C E";
		snprintf(tmp, sizeof(tmp), "%d porte(s) sur 4 se sont ouvertes", n < 0 ? 0 : (n > 4 ? 4 : n));
		texte = n >= 4 ? "Les quatre portes sont ouvertes. Dehors, la lanterne s'est rallumée toute seule."
			: "Les portes encore fermées attendent la prochaine épreuve.";
	}
	t0 = now_ms();
	while (1)
	{
		frame();
		bgc(g_th.bg);
		if (!strcmp(mode, "puits"))
		{
			img = composer(b, arg ? arg : "", now_ms() - t0, &prof, &nok);
			titre = "L E   P U I T S";
			snprintf(tmp, sizeof(tmp), "%d anneaux sur 22 · la lanterne est descendue de %d ligne(s)", nok, prof + 1);
			texte = nok == 22 ? "Au fond, de l'eau calme. Les empreintes de Moulinette mènent vers une ouverture."
				: "Le Puits ne se descend qu'une ligne à la fois. Les anneaux rouges ne tiennent pas encore.";
		}
		else if (!strcmp(mode, "apres"))
			img = epreuve(b, n);
		else
			img = copie(b);
		center(1, g_th.accent, titre);
		center(2, g_th.dim, tmp);
		cw = term_cols() - 4;
		ch = (int)(cw * (double)b->h / b->w / 2.0);
		if (ch > term_rows() - 8)
		{
			ch = term_rows() - 8;
			cw = (int)(ch * 2.0 * b->w / b->h);
		}
		if (img)
			rendu_image(img, b->w, b->h, (term_cols() - cw) / 2, 4, cw, ch);
		free(img);
		center(5 + ch, g_th.text, texte);
		foot_line((term_cols() - (int)strlen(pied)) / 2, pied);
		flush();
		k = key_read(120);
		if (k == '\r' || k == '\n' || k == ' ' || key_quit(k) || k == 'q')
			break ;
	}
	if (!strcmp(mode, "puits") && nok == 22 && art_pic("fin"))
	{
		cls();
		montrer("fin", NULL);
	}
}

int	main(int ac, char **av)
{
	if (ac < 3)
		return (2);
	g_tester_dir = av[1];
	strcpy(g_save.theme, "nuit");
	term_init();
	montrer(av[2], ac > 3 ? av[3] : NULL);
	term_end();
	return (0);
}
