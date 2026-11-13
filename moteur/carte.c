#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

typedef struct s_lieu
{
	const char	*nom;
	const char	*en;
	const char	*desc;
	const char	*desc_en;
	const char	*module;
	const char	*zoom;
	int			x;
	int			y;
}	t_lieu;

static const t_lieu	g_lieux[6] = {
	{"La Tour des Fondations", "The Tower of Foundations", "Libft · 43 fenêtres, trois gardiens. Moulinette y était en cage.",
		"Libft · 43 windows, three wardens. Moulinette was caged there.", "libft", "tour_eteinte", 154, 126},
	{"La Fonderie des Formats", "The Foundry of Formats", "ft_printf · la machine qui donne forme aux mots. Neuf chaînes.",
		"ft_printf · the machine that shapes words. Nine chains.", "printf", "fonderie_fond", 455, 151},
	{"Le Puits", "The Well", "get_next_line · on ne le descend qu'une ligne à la fois.",
		"get_next_line · you go down one line at a time.", "gnl", "puits_fond", 320, 234},
	{"Les Deux Piles", "The Two Stacks", "push_swap · deux colonnes de pierre à remettre en ordre. Plus tard.",
		"push_swap · two stone columns to sort. Later.", NULL, NULL, 142, 289},
	{"L'Épreuve du Silence", "The Trial of Silence", "Exam · une salle sans aide, trois heures, quatre portes.",
		"Exam · a room without help, three hours, four doors.", "exam", "epreuve_salle", 542, 295},
	{"Le dernier mécanisme", "The last mechanism", "La fin · tout en bas, sous le Puits.",
		"The end · at the very bottom, under the Well.", "gnl", "fin", 320, 335},
};

typedef struct s_prog
{
	int	vu;
	int	ok;
	int	total;
}	t_prog;

static void	lire_progression(t_prog *p)
{
	FILE	*f;
	char	ligne[256];
	char	mod[32];
	char	chemin[700];
	long	ep;
	int		ok;
	int		tot;
	int		i;

	memset(p, 0, sizeof(t_prog) * 6);
	snprintf(chemin, sizeof(chemin), "%s/stats.txt", g_savedir ? g_savedir : ".");
	if (!(f = fopen(chemin, "r")))
		return ;
	while (fgets(ligne, sizeof(ligne), f))
	{
		if (sscanf(ligne, "R %ld %31s %d %d", &ep, mod, &ok, &tot) != 4 || tot <= 0)
			continue ;
		for (i = 0; i < 6; i++)
		{
			if (!g_lieux[i].module || strcmp(g_lieux[i].module, mod))
				continue ;
			if (!strcmp(mod, "exam") && p[i].vu && ok * p[i].total <= p[i].ok * tot)
				continue ;
			p[i].vu = 1;
			p[i].ok = ok;
			p[i].total = tot;
		}
	}
	fclose(f);
	p[5].vu = p[2].vu && p[2].ok == p[2].total;
}

static int	fini(const t_prog *p)
{
	return (p->vu && p->total > 0 && p->ok >= p->total);
}

static void	teinter(unsigned char *img, int w, int h, int cx, int cy, double r, int cr, int cg, int cb, double k)
{
	int		x;
	int		y;
	double	d;
	double	a;

	for (y = (int)(cy - r); y <= (int)(cy + r); y++)
		for (x = (int)(cx - r); x <= (int)(cx + r); x++)
		{
			if (x < 0 || y < 0 || x >= w || y >= h)
				continue ;
			d = sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy)) / r;
			if (d >= 1)
				continue ;
			a = k * (1 - d);
			img[(y * w + x) * 3] += (cr - img[(y * w + x) * 3]) * a;
			img[(y * w + x) * 3 + 1] += (cg - img[(y * w + x) * 3 + 1]) * a;
			img[(y * w + x) * 3 + 2] += (cb - img[(y * w + x) * 3 + 2]) * a;
		}
}

static void	anneau(unsigned char *img, int w, int h, int cx, int cy, double r, double e)
{
	int		a;
	double	t;
	int		x;
	int		y;

	for (a = 0; a < 720; a++)
	{
		t = a * 3.14159265358979 / 360;
		x = cx + (int)(cos(t) * r);
		y = cy + (int)(sin(t) * r * 0.75);
		teinter(img, w, h, x, y, 3 * e, 118, 222, 238, 0.9);
	}
}

static unsigned char	*composer_carte(const t_pic *b, const t_prog *p, int sel)
{
	unsigned char	*img;
	double			e;
	int				i;

	if (!(img = malloc((size_t)b->w * b->h * 3)))
		return (NULL);
	memcpy(img, b->rgb, (size_t)b->w * b->h * 3);
	e = b->w / 640.0;
	for (i = 0; i < 6; i++)
	{
		if (!p[i].vu)
			teinter(img, b->w, b->h, g_lieux[i].x * e, g_lieux[i].y * e, 58 * e, 4, 4, 6, 0.75);
		else if (fini(&p[i]))
			teinter(img, b->w, b->h, g_lieux[i].x * e, g_lieux[i].y * e, 46 * e, 240, 196, 78, 0.18);
	}
	anneau(img, b->w, b->h, g_lieux[sel].x * e, g_lieux[sel].y * e, 40 * e, e);
	return (img);
}

static void	placer(const t_pic *b, int top, int bas, int *x, int *cw, int *ch)
{
	*cw = term_cols() - 4;
	*ch = (int)(*cw * (double)b->h / b->w / 2.0);
	if (*ch > term_rows() - top - bas)
	{
		*ch = term_rows() - top - bas;
		*cw = (int)(*ch * 2.0 * b->w / b->h);
	}
	*x = (term_cols() - *cw) / 2;
}

static void	approcher(int i, const t_prog *p)
{
	const t_pic	*z;
	char		tmp[200];
	int			k;
	int			x;
	int			cw;
	int			ch;

	z = g_lieux[i].zoom && p[i].vu ? art_pic(g_lieux[i].zoom) : NULL;
	cls();
	while (1)
	{
		bgc(theme()->bg);
		frame();
		center(1, theme()->accent2, L(g_lieux[i].nom, g_lieux[i].en));
		if (z)
		{
			placer(z, 3, 6, &x, &cw, &ch);
			rendu_image(z->rgb, z->w, z->h, x, 3, cw, ch);
		}
		else
			ch = 0;
		center(4 + ch, theme()->text, L(g_lieux[i].desc, g_lieux[i].desc_en));
		if (!g_lieux[i].module)
			snprintf(tmp, sizeof(tmp), "%s", L("Ce lieu n'est pas encore ouvert.", "This place is not open yet."));
		else if (!p[i].vu)
			snprintf(tmp, sizeof(tmp), "%s", L("Tu n'es jamais venu ici.", "You have never been here."));
		else
			snprintf(tmp, sizeof(tmp), L("Dernier passage : %d / %d%s", "Last visit: %d / %d%s"), p[i].ok, p[i].total,
				fini(&p[i]) ? L("  · terminé", "  · done") : "");
		center(5 + ch, fini(&p[i]) ? rgb(240, 196, 78) : theme()->dim, tmp);
		foot_line(2, L("Entrée ou Échap : revenir à la carte", "Enter or Esc: back to the map"));
		flush();
		k = key_read(120);
		if (k == '\r' || k == '\n' || k == ' ' || key_quit(k) || k == 127 || k == 8)
			break ;
	}
	cls();
}

void	tdf_carte(void)
{
	static int	sel;
	t_prog		p[6];
	const t_pic	*b;
	unsigned char	*img;
	char		tmp[200];
	const char	*nom;
	int			pad;
	int			k;
	int			i;
	int			x;
	int			cw;
	int			ch;
	int			y;

	if (!(b = art_pic("carte")))
		return ;
	lire_progression(p);
	cls();
	while (1)
	{
		bgc(theme()->bg);
		frame();
		center(1, theme()->accent2, L("L A   C I T É", "T H E   C I T Y"));
		placer(b, 3, 10, &x, &cw, &ch);
		img = composer_carte(b, p, sel);
		if (img)
			rendu_image(img, b->w, b->h, x, 3, cw, ch);
		free(img);
		y = 4 + ch;
		for (i = 0; i < 6 && y < term_rows() - 2; i++, y++)
		{
			at(x, y);
			bgc(i == sel ? mix(theme()->bg, theme()->accent, 0.2) : theme()->bg);
			fg(!p[i].vu ? theme()->dim : (fini(&p[i]) ? rgb(240, 196, 78) : theme()->text));
			nom = L(g_lieux[i].nom, g_lieux[i].en);
			pad = 30 - ulen(nom);
			if (p[i].vu && p[i].total > 0)
				snprintf(tmp, sizeof(tmp), " %s %s%*s%3d %%", i == sel ? "▶" : " ", nom, pad > 0 ? pad : 1, "",
					p[i].ok * 100 / p[i].total);
			else
				snprintf(tmp, sizeof(tmp), " %s %s%*s%s", i == sel ? "▶" : " ", nom, pad > 0 ? pad : 1, "",
					g_lieux[i].module ? "  ·" : L(" fermé", " closed"));
			out(tmp);
			bgc(theme()->bg);
		}
		text_clip(x, y, theme()->dim, L(g_lieux[sel].desc, g_lieux[sel].desc_en), term_cols() - x - 1);
		foot_line(2, L("▲ ▼ choisir un lieu   Entrée : s'approcher   Échap : retour",
				"▲ ▼ pick a place   Enter: go closer   Esc: back"));
		flush();
		k = key_dir(key_read(80));
		if (key_quit(k) || k == 127 || k == 8)
			break ;
		if (k == 1000 || k == 1003)
			sel = (sel + 5) % 6;
		else if (k == 1001 || k == 1002)
			sel = (sel + 1) % 6;
		else if (k == '\r' || k == '\n' || k == ' ')
			approcher(sel, p);
	}
	cls();
}
