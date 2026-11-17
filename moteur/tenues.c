#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

#define NTENUE 5

typedef struct s_tenue
{
	int		ok;
	char	nom[24];
	t_look	lk;
}	t_tenue;

static t_tenue	g_tn[NTENUE];

static void	tn_chemin(char *p, size_t n)
{
	snprintf(p, n, "%s/tenues.txt", g_savedir);
}

static void	tn_lire(void)
{
	FILE	*f;
	char	p[700];
	char	l[600];
	char	*c;
	char	*d;
	int		i;
	int		k;

	memset(g_tn, 0, sizeof(g_tn));
	tn_chemin(p, sizeof(p));
	f = fopen(p, "r");
	while (f && fgets(l, sizeof(l), f))
	{
		if (sscanf(l, "%d", &i) != 1 || i < 0 || i >= NTENUE || !(c = strchr(l, '\t')))
			continue ;
		d = strchr(++c, '\t');
		if (!d)
			continue ;
		*d++ = '\0';
		snprintf(g_tn[i].nom, sizeof(g_tn[i].nom), "%s", c);
		for (k = 0; k < NSLOT; k++)
		{
			g_tn[i].lk.eq[k] = (int)strtol(d, &d, 10);
			if (*d == ',')
				d++;
		}
		for (k = 0; k < NSLOT; k++)
		{
			g_tn[i].lk.col[k] = (int)strtol(d, &d, 10);
			if (*d == ',')
				d++;
		}
		g_tn[i].ok = 1;
	}
	if (f)
		fclose(f);
}

static void	tn_ecrire(void)
{
	FILE	*f;
	char	p[700];
	int		i;
	int		k;

	tn_chemin(p, sizeof(p));
	f = fopen(p, "w");
	if (!f)
		return ;
	for (i = 0; i < NTENUE; i++)
	{
		if (!g_tn[i].ok)
			continue ;
		fprintf(f, "%d\t%s\t", i, g_tn[i].nom);
		for (k = 0; k < NSLOT; k++)
			fprintf(f, "%d%s", g_tn[i].lk.eq[k], k + 1 < NSLOT ? "," : " ");
		for (k = 0; k < NSLOT; k++)
			fprintf(f, "%d%s", g_tn[i].lk.col[k], k + 1 < NSLOT ? "," : "\n");
	}
	fclose(f);
}

static t_look	tn_porte(const t_look *src)
{
	t_look	lk;
	int		k;

	lk = g_save.look;
	for (k = 0; k < NSLOT; k++)
		if (src->eq[k] < 0 || (src->eq[k] < g_nitems && item_owned(src->eq[k]) && g_items[src->eq[k]].slot == k))
		{
			lk.eq[k] = src->eq[k];
			lk.col[k] = src->col[k];
		}
	return (lk);
}

void	tdf_grand_apercu(const t_look *lk, int title)
{
	t_canvas	cv;
	t_canvas	gr;
	char		tn[80];
	int			z;
	int			x;
	int			y;
	int			k;
	long		t;

	cls();
	while (1)
	{
		t = now_ms();
		z = (term_rows() - 6) * 2 / 32;
		if ((term_cols() - 4) / 28 < z)
			z = (term_cols() - 4) / 28;
		if (z < 1)
			z = 1;
		bgc(theme()->bg);
		frame();
		cv_init(&cv, 28, 32);
		avatar_draw(&cv, 0, 0, lk, t);
		cv_init(&gr, 28 * z, 32 * z);
		for (y = 0; y < 32 * z; y++)
			for (x = 0; x < 28 * z; x++)
			{
				gr.px[y * gr.w + x] = cv.px[(y / z) * cv.w + x / z];
				gr.a[y * gr.w + x] = cv.a[(y / z) * cv.w + x / z];
			}
		cv_print(&gr, (term_cols() - 28 * z) / 2, 2, theme()->bg);
		cv_free(&gr);
		cv_free(&cv);
		title_name(title, tn, sizeof(tn));
		center(0, pseudo_color(), g_save.login);
		center(1, theme()->dim, tn);
		foot_line((term_cols() - 30) / 2, L("Échap ou Entrée : retour", "Esc or Enter: back"));
		flush();
		k = key_read(60);
		if (k >= 0)
			break ;
	}
	cls();
}

static int	tn_choix(int x, int y, const char *titre, const char **it, int n)
{
	int	sel;
	int	k;
	int	i;
	int	w;

	sel = 0;
	w = ulen(titre) + 4;
	for (i = 0; i < n; i++)
		if (ulen(it[i]) + 6 > w)
			w = ulen(it[i]) + 6;
	if (y + n + 3 > term_rows() - 1)
		y = term_rows() - n - 4;
	if (y < 1)
		y = 1;
	while (1)
	{
		fill(x, y, w, n + 3, mix(theme()->bg, theme()->accent, 0.12));
		box(x, y, w, n + 3, theme()->accent);
		bgc(mix(theme()->bg, theme()->accent, 0.12));
		text_at(x + 2, y + 1, theme()->accent2, titre);
		for (i = 0; i < n; i++)
		{
			at(x + 2, y + 2 + i);
			if (i == sel)
			{
				bgc(mix(theme()->bg, theme()->accent, 0.35));
				fg(theme()->accent2);
			}
			else
			{
				bgc(mix(theme()->bg, theme()->accent, 0.12));
				fg(theme()->text);
			}
			outf(" %s %-*s", i == sel ? "▶" : " ", w - 7, it[i]);
		}
		bgc(theme()->bg);
		flush();
		k = key_read(60);
		if (k == 1000)
			sel = (sel + n - 1) % n;
		else if (k == 1001)
			sel = (sel + 1) % n;
		else if (k == '\r' || k == '\n' || k == ' ')
			return (sel);
		else if (k == 27 || k == 127 || k == 8 || k == 1003)
			return (-1);
	}
}

static void	tn_enregistrer(int i)
{
	char	nom[24];

	if (g_tn[i].ok)
		snprintf(nom, sizeof(nom), "%s", g_tn[i].nom);
	else
		snprintf(nom, sizeof(nom), L("Tenue %d", "Outfit %d"), i + 1);
	tdf_saisie_selection = 1;
	if (!tdf_saisir(L("Nom de la tenue", "Outfit name"), L("Ce que tu portes maintenant sera gardé sous ce nom.",
				"What you wear now will be kept under this name."), nom, sizeof(nom), 20, 0, 1))
		return ;
	g_tn[i].ok = 1;
	snprintf(g_tn[i].nom, sizeof(g_tn[i].nom), "%s", nom);
	g_tn[i].lk = g_save.look;
	tn_ecrire();
	toast(L("TENUE ENREGISTRÉE", "OUTFIT SAVED"), g_tn[i].nom, rgb(80, 230, 140));
}

void	tdf_tenues_lister(int x, int y, int sel)
{
	char	tmp[120];
	int		i;

	tn_lire();
	at(x, y);
	if (sel == 0)
		bgc(mix(theme()->bg, theme()->accent, 0.2));
	fg(sel == 0 ? theme()->accent2 : rgb(80, 230, 140));
	outf(" %s %-40s", sel == 0 ? "▶" : " ", L("＋ Enregistrer ce que je porte", "＋ Save what I am wearing"));
	bgc(theme()->bg);
	for (i = 0; i < NTENUE; i++)
	{
		at(x, y + 2 + i);
		if (sel == i + 1)
			bgc(mix(theme()->bg, theme()->accent, 0.2));
		fg(g_tn[i].ok ? (sel == i + 1 ? theme()->accent2 : theme()->text) : theme()->dim);
		if (g_tn[i].ok)
			snprintf(tmp, sizeof(tmp), "%d. %s", i + 1, g_tn[i].nom);
		else
			snprintf(tmp, sizeof(tmp), L("%d. (emplacement libre)", "%d. (free slot)"), i + 1);
		outf(" %s %-40s", sel == i + 1 ? "▶" : " ", tmp);
		bgc(theme()->bg);
	}
}

int	tdf_tenues_apercu(int sel, t_look *lk)
{
	if (sel < 1 || sel > NTENUE || !g_tn[sel - 1].ok)
		return (0);
	*lk = tn_porte(&g_tn[sel - 1].lk);
	return (1);
}

void	tdf_tenues_entree(int sel, int x, int y)
{
	const char	*it[4];
	char		nom[24];
	int			i;
	int			r;

	tn_lire();
	if (sel == 0)
	{
		for (i = 0; i < NTENUE && g_tn[i].ok; i++)
			;
		if (i == NTENUE)
		{
			r = tn_choix(x, y, L("Les 5 sont prises : laquelle remplacer ?", "All 5 are taken: which one to replace?"),
					(const char *[]){g_tn[0].nom, g_tn[1].nom, g_tn[2].nom, g_tn[3].nom, g_tn[4].nom}, NTENUE);
			if (r < 0)
				return ;
			i = r;
		}
		tn_enregistrer(i);
		return ;
	}
	i = sel - 1;
	if (i < 0 || i >= NTENUE)
		return ;
	if (!g_tn[i].ok)
	{
		tn_enregistrer(i);
		return ;
	}
	it[0] = L("Porter cette tenue", "Wear this outfit");
	it[1] = L("La remplacer par ce que je porte", "Replace it with what I wear");
	it[2] = L("Renommer", "Rename");
	it[3] = L("Supprimer", "Delete");
	r = tn_choix(x, y, g_tn[i].nom, it, 4);
	if (r == 0)
	{
		g_save.look = tn_porte(&g_tn[i].lk);
		save_write();
		toast(L("TENUE ÉQUIPÉE", "OUTFIT EQUIPPED"), g_tn[i].nom, rgb(80, 230, 140));
	}
	else if (r == 1)
	{
		g_tn[i].lk = g_save.look;
		tn_ecrire();
		toast(L("TENUE MISE À JOUR", "OUTFIT UPDATED"), g_tn[i].nom, rgb(80, 230, 140));
	}
	else if (r == 2)
	{
		snprintf(nom, sizeof(nom), "%s", g_tn[i].nom);
		if (tdf_saisir(L("Nouveau nom de la tenue", "New outfit name"), "", nom, sizeof(nom), 20, 0, 1))
		{
			snprintf(g_tn[i].nom, sizeof(g_tn[i].nom), "%s", nom);
			tn_ecrire();
		}
	}
	else if (r == 3 && tn_choix(x, y, L("Supprimer cette tenue ?", "Delete this outfit?"),
			(const char *[]){L("Non, la garder", "No, keep it"), L("Oui, supprimer", "Yes, delete")}, 2) == 1)
	{
		g_tn[i].ok = 0;
		tn_ecrire();
	}
}
