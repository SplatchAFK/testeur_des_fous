#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

int	tdf_list_menu(const char *title, const char **items, int n, int *sel, const char *foot)
{
	int		k;
	int		i;
	int		x;
	long	t;

	cls();
	while (1)
	{
		t = now_ms();
		x = (term_cols() - 60) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		frame();
		stars_draw();
		center(2, theme()->accent2, title);
		for (i = 0; i < n; i++)
		{
			at(x, 5 + i * 2);
			if (i == *sel)
			{
				bgc(mix(theme()->bg, theme()->accent, 0.18 + 0.08 * sin(t / 150.0)));
				fg(theme()->accent2);
			}
			else
				fg(items[i][0] == '~' ? theme()->dim : theme()->text);
			outf(" %s %-54s ", i == *sel ? "▶" : " ", items[i][0] == '~' ? items[i] + 1 : items[i]);
			bgc(theme()->bg);
		}
		if (foot)
			text_wrap(x, 6 + n * 2, term_rows() - 2, 60, theme()->dim, foot);
		foot_line(x, L("▲ ▼ choisir   Entrée : valider   Échap : retour",
				"▲ ▼ pick   Enter: confirm   Esc: back"));
		flush();
		k = key_dir(key_read(40));
		if (key_quit(k) || k == 127 || k == 8)
			return (cls(), -1);
		if (k == 1000)
			*sel = (*sel + n - 1) % n;
		else if (k == 1001)
			*sel = (*sel + 1) % n;
		else if (k == '\r' || k == '\n' || k == ' ')
			return (cls(), *sel);
	}
}

static int	module_dispo(const char *name)
{
	const char	*d;
	const char	*p;
	size_t		n;

	d = getenv("TDF_DISPO");
	n = strlen(name);
	for (p = d; p && (p = strstr(p, name)); p += n)
		if ((p == d || p[-1] == ' ') && (p[n] == ' ' || p[n] == '\0'))
			return (1);
	return (0);
}

static int	module_switch(const char *name)
{
	FILE	*f;
	char	path[620];

	snprintf(path, sizeof(path), "%s/suivant", g_savedir);
	f = fopen(path, "w");
	if (!f)
		return (0);
	fprintf(f, "%s\n", name);
	fclose(f);
	return (45);
}

static void	not_yet(const char *msg)
{
	toast(L("PAS ENCORE", "NOT YET"), msg, theme()->dim);
}

int	tdf_menu_tester(const char *train, const char *other, const char *other_label)
{
	static int	sel;
	const char	*items[8];
	char		sw[120];
	int			dispo;
	int			i;

	dispo = module_dispo(other);
	snprintf(sw, sizeof(sw), "%s%s%s", dispo ? "" : "~", L("Passer à ", "Switch to "), other_label);
	items[0] = L("Relancer les tests      (r)", "Run the tests again     (r)");
	items[1] = L("Résultats détaillés", "Detailed results");
	items[2] = train;
	items[3] = sw;
	items[4] = L("~Exam en C               (bientôt)", "~C exam                  (soon)");
	items[5] = L("~Banque d'exercices      (bientôt)", "~Exercise bank           (soon)");
	items[6] = L("~get_next_line           (plus tard)", "~get_next_line           (later)");
	items[7] = L("~push_swap               (plus tard)", "~push_swap               (later)");
	while (1)
	{
		i = tdf_list_menu(L("TESTER", "TEST"), items, 8, &sel, NULL);
		if (i < 0)
			return (0);
		if (i <= 2)
			return (i + 1);
		if (i == 3 && dispo)
			return (module_switch(other));
		if (i == 3)
			not_yet(L("range ce projet à côté du testeur, puis relance", "put that project next to the tester, then relaunch"));
		else
			not_yet(L("ce module arrive dans une prochaine version", "this module comes in a later version"));
	}
}

void	tdf_menu_chevalier(void (*profile)(void))
{
	static int	sel;
	const char	*items[3];
	int			i;

	items[0] = L("Avatar (garde-robe)", "Avatar (wardrobe)");
	items[1] = L("Boutique", "Shop");
	items[2] = L("Profil et succès", "Profile and achievements");
	while (1)
	{
		i = tdf_list_menu(L("CHEVALIER", "KNIGHT"), items, 3, &sel, NULL);
		if (i < 0)
			return ;
		if (i == 0)
			screen_shop(1);
		else if (i == 1)
			screen_shop(0);
		else
			profile();
	}
}

void	tdf_menu_lore(void (*boss)(void))
{
	static int	sel;
	const char	*items[2];
	int			i;

	items[0] = L("Boss et cinématiques", "Bosses and cutscenes");
	items[1] = L("~Carte du monde           (bientôt)", "~World map               (soon)");
	while (1)
	{
		i = tdf_list_menu(L("LORE", "LORE"), items, 2, &sel, NULL);
		if (i < 0)
			return ;
		if (i == 0)
			boss();
		else
			not_yet(L("la carte arrive avec le nouveau lore", "the map comes with the new lore"));
	}
}

void	tdf_menu_stats(void)
{
	static int	sel;
	const char	*items[2];
	int			i;

	items[0] = L("Mes stats", "My stats");
	items[1] = L("Badges et holygraph 42 Next", "Badges and 42 Next holygraph");
	while (1)
	{
		i = tdf_list_menu(L("STATS", "STATS"), items, 2, &sel, NULL);
		if (i < 0)
			return ;
		if (i == 0)
			tdf_screen_stats();
		else
			screen_holy();
	}
}

void	tdf_menu_params(void (*credits)(void))
{
	static int	sel;
	const char	*items[4];
	int			i;

	items[0] = L("Réglages", "Settings");
	items[1] = L("Sauvegarde et connexion", "Save and login");
	items[2] = L("Nous aider", "Help us");
	items[3] = L("Crédits", "Credits");
	while (1)
	{
		i = tdf_list_menu(L("PARAMÈTRES", "SETTINGS"), items, 4, &sel, NULL);
		if (i < 0)
			return ;
		if (i == 0)
			screen_settings();
		else if (i == 1)
			screen_save();
		else if (i == 2)
			screen_help_us();
		else
			credits();
	}
}
