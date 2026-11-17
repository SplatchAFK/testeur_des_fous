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

void	tdf_tenues(void)
{
	static int	sel;
	t_look		lk;
	char		tmp[200];
	char		nom[24];
	int			x;
	int			i;
	int			k;
	int			sur;

	tn_lire();
	sur = -1;
	cls();
	while (1)
	{
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		frame();
		center(0, theme()->accent2, L("◈  T E N U E S  ◈", "◈  O U T F I T S  ◈"));
		text_at(x, 2, theme()->dim, L("Garde jusqu'à 5 tenues et change-les en un appui.", "Keep up to 5 outfits and switch in one press."));
		for (i = 0; i < NTENUE; i++)
		{
			at(x, 4 + i * 2);
			if (i == sel)
				bgc(mix(theme()->bg, theme()->accent, 0.2));
			fg(g_tn[i].ok ? theme()->text : theme()->dim);
			if (g_tn[i].ok)
				snprintf(tmp, sizeof(tmp), " %s %d. %-22s ", i == sel ? "▶" : " ", i + 1, g_tn[i].nom);
			else
				snprintf(tmp, sizeof(tmp), " %s %d. %-22s ", i == sel ? "▶" : " ", i + 1, L("(vide)", "(empty)"));
			out(tmp);
			bgc(theme()->bg);
		}
		lk = g_tn[sel].ok ? tn_porte(&g_tn[sel].lk) : g_save.look;
		avatar_panel(x + 50, 3, &lk, g_save.title, now_ms());
		text_at(x + 50, 24 > term_rows() - 3 ? term_rows() - 3 : 24, g_tn[sel].ok ? rgb(255, 200, 80) : rgb(80, 230, 140),
			g_tn[sel].ok ? L("Aperçu de cette tenue", "Preview of this outfit") : L("Ton chevalier, tel qu'il est équipé", "Your knight, as equipped"));
		if (sur == sel)
			text_at(x, 16, rgb(255, 120, 120), L("Appuie encore sur x pour supprimer cette tenue.", "Press x again to delete this outfit."));
		text_at(x, term_rows() - 3, theme()->dim, L("Entrée : porter   e : enregistrer ma tenue ici   n : renommer   x : supprimer",
				"Enter: wear   e: save my outfit here   n: rename   x: delete"));
		foot_line(x, L("▲ ▼ choisir   v : grand aperçu   Échap : retour", "▲ ▼ pick   v: big preview   Esc: back"));
		flush();
		k = key_read(60);
		if (k != 'e' && k != 'E' && k != 'n' && k != 'N' && k != 'x' && k != 'X' && k != 'v' && k != 'V')
			k = key_dir(k);
		if (k != 'x' && k >= 0)
			sur = -1;
		if (key_quit(k) || k == 127 || k == 8)
			break ;
		if (k == 1000)
			sel = (sel + NTENUE - 1) % NTENUE;
		else if (k == 1001)
			sel = (sel + 1) % NTENUE;
		else if ((k == '\r' || k == '\n' || k == ' ') && g_tn[sel].ok)
		{
			g_save.look = tn_porte(&g_tn[sel].lk);
			save_write();
			snprintf(tmp, sizeof(tmp), L("Tu portes « %s ».", "You are wearing %s."), g_tn[sel].nom);
			toast(L("TENUE ÉQUIPÉE", "OUTFIT EQUIPPED"), tmp, rgb(80, 230, 140));
		}
		else if (k == 'e' || k == 'E')
		{
			if (g_tn[sel].ok)
				snprintf(nom, sizeof(nom), "%s", g_tn[sel].nom);
			else
				snprintf(nom, sizeof(nom), L("Tenue %d", "Outfit %d"), sel + 1);
			if (tdf_saisir(L("Nom de la tenue", "Outfit name"), L("Ta tenue actuelle sera gardée sous ce nom.", "Your current outfit will be kept under this name."),
					nom, sizeof(nom), 20, 0, 1))
			{
				g_tn[sel].ok = 1;
				snprintf(g_tn[sel].nom, sizeof(g_tn[sel].nom), "%s", nom);
				g_tn[sel].lk = g_save.look;
				tn_ecrire();
			}
		}
		else if ((k == 'n' || k == 'N') && g_tn[sel].ok)
		{
			snprintf(nom, sizeof(nom), "%s", g_tn[sel].nom);
			if (tdf_saisir(L("Nouveau nom de la tenue", "New outfit name"), "", nom, sizeof(nom), 20, 0, 1))
			{
				snprintf(g_tn[sel].nom, sizeof(g_tn[sel].nom), "%s", nom);
				tn_ecrire();
			}
		}
		else if ((k == 'x' || k == 'X') && g_tn[sel].ok)
		{
			if (sur == sel)
			{
				g_tn[sel].ok = 0;
				tn_ecrire();
				sur = -1;
			}
			else
				sur = sel;
		}
		else if (k == 'v' || k == 'V')
			tdf_grand_apercu(&lk, g_save.title);
		if (k > 0)
			cls();
	}
	cls();
}
