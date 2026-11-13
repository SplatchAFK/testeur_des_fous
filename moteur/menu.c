#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

static void	label_of(const char *item, char *b, size_t n)
{
	const char	*s;
	size_t		l;

	s = item[0] == '~' ? item + 1 : item;
	l = strcspn(s, "|");
	if (l >= n)
		l = n - 1;
	memcpy(b, s, l);
	b[l] = '\0';
}

static const char	*desc_of(const char *item)
{
	const char	*p;

	p = strchr(item, '|');
	return (p ? p + 1 : NULL);
}

int	tdf_list_menu(const char *title, const char **items, int n, int *sel, const char *foot)
{
	char	lab[160];
	int		k;
	int		i;
	int		x;
	int		y;
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
			label_of(items[i], lab, sizeof(lab));
			outf(" %s %s", i == *sel ? "▶" : " ", lab);
			outf("%*s", 55 - ulen(lab) > 0 ? 55 - ulen(lab) : 1, "");
			bgc(theme()->bg);
		}
		y = 6 + n * 2;
		if (desc_of(items[*sel]))
		{
			fill(x, y, 60, 2, theme()->bg);
			y = text_wrap(x + 3, y, y + 2, 56, theme()->accent, desc_of(items[*sel])) + 1;
		}
		if (foot)
			text_wrap(x, y, term_rows() - 2, 60, theme()->dim, foot);
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

void	tdf_menu_aide(int x, int y, int sel)
{
	const char	*d[6];

	d[0] = L("Lancer les tests de ton projet, voir les résultats, l'entraînement et l'exam.",
			"Run your project's tests, see results, training and the exam.");
	d[1] = L("Habiller ton chevalier, la boutique, ton profil et tes succès.",
			"Dress your knight, the shop, your profile and achievements.");
	d[2] = L("Revoir les boss et les cinématiques débloqués.", "Replay the bosses and cutscenes you unlocked.");
	d[3] = L("Ta progression, ce qui échoue le plus, ton historique.", "Your progress, what fails most, your history.");
	d[4] = L("Réglages, sauvegarde, nous aider, crédits.", "Settings, save, help us, credits.");
	d[5] = L("Fermer le testeur. Ta progression est déjà enregistrée.", "Close the tester. Your progress is already saved.");
	if (sel < 0 || sel > 5)
		return ;
	fill(x, y, 44, 3, theme()->bg);
	text_wrap(x, y, y + 3, 44, theme()->accent, d[sel]);
	foot_line(x - 34 > 0 ? x - 34 : 0, L("▲ ▼ choisir   Entrée : ouvrir   r : lancer les tests   Échap : quitter",
			"▲ ▼ pick   Enter: open   r: run the tests   Esc: quit"));
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
	char		sw[300];
	int			dispo;
	int			i;

	dispo = module_dispo(other);
	snprintf(sw, sizeof(sw), "%s%s%s%s", dispo ? "" : "~", L("Passer à ", "Switch to "), other_label,
		dispo ? L("|Change de projet sans quitter le testeur.", "|Switch project without leaving the tester.")
		: L("|Range ce projet à côté du testeur pour l'activer.", "|Put that project next to the tester to enable it."));
	items[0] = L("Lancer les tests        (r)|Recompile ton projet et le teste en entier.", "Run the tests           (r)|Rebuilds your project and tests all of it.");
	items[1] = L("Résultats détaillés|Chaque fonction ou catégorie, avec le cas qui échoue et une piste.", "Detailed results|Every function or category, with the failing case and a hint.");
	items[2] = train;
	items[3] = sw;
	items[4] = L("Exam en C|L'exam du rang 02 en ligne de commande, comme le vrai : grademe, 4 niveaux, 3 h.", "C exam|The rank 02 exam on the command line, like the real one: grademe, 4 levels, 3 h.");
	items[5] = L("Banque d'exercices|Les 94 exercices du testeur (exam, Libft, printf) : cherche, ouvre, grademe.",
			"Exercise bank|All 94 exercises of the tester (exam, Libft, printf): search, open, grademe.");
	items[6] = module_dispo("gnl")
		? L("get_next_line|Teste ton get_next_line avec 7 BUFFER_SIZE, stdin, tubes, erreurs de read et de malloc.",
			"get_next_line|Tests your get_next_line with 7 BUFFER_SIZE values, stdin, pipes, read and malloc errors.")
		: L("~get_next_line|Range ton dossier get_next_line à côté du testeur pour l'activer.",
			"~get_next_line|Put your get_next_line folder next to the tester to enable it.");
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
		if (i == 4)
			return (module_switch("exam"));
		if (i == 5)
			return (module_switch("banque"));
		if (i == 6 && module_dispo("gnl"))
			return (module_switch("gnl"));
		if (i == 3 || i == 6)
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

	items[0] = L("Avatar (garde-robe)|Équipe les objets que tu possèdes et choisis leurs couleurs.", "Avatar (wardrobe)|Equip the items you own and pick their colours.");
	items[1] = L("Boutique|Dépense tes Octets : objets, thèmes, titres. Les Octets se gagnent en validant.", "Shop|Spend your Octets: items, themes, titles. You earn Octets by passing tests.");
	items[2] = L("Profil et succès|Bannière, photo, bio et tous tes succès.", "Profile and achievements|Banner, picture, bio and all your achievements.");
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

	items[0] = L("Boss et cinématiques|Revois les combats et les cinématiques déjà débloqués.", "Bosses and cutscenes|Replay the fights and cutscenes you unlocked.");
	items[1] = L("Carte de la Cité|Tous les lieux du lore, allumés selon ta progression. Entrée pour t'approcher.",
			"Map of the City|Every place of the lore, lit by your progress. Enter to go closer.");
	while (1)
	{
		i = tdf_list_menu(L("LORE", "LORE"), items, 2, &sel, NULL);
		if (i < 0)
			return ;
		if (i == 0)
			boss();
		else
			tdf_carte();
	}
}

void	tdf_menu_stats(void)
{
	static int	sel;
	const char	*items[2];
	int			i;

	items[0] = L("Mes stats|Ta progression, ce qui échoue le plus, ton historique. ◀ ▶ pour changer de page.", "My stats|Your progress, what fails most, your history. ◀ ▶ to change page.");
	items[1] = L("Badges et holygraph 42 Next|Tes badges et les projets du Common Core, avec ce qui manque.", "Badges and 42 Next holygraph|Your badges and the Common Core projects, with what is missing.");
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

	items[0] = L("Réglages|Langue, animations, histoire, niveau d'aide, outils de test.", "Settings|Language, animations, story, help level, test tools.");
	items[1] = L("Sauvegarde et connexion|Exporter ta progression ou la retrouver sur un autre poste.", "Save and login|Export your progress or get it back on another computer.");
	items[2] = L("Nous aider|Crée une trace (ni code, ni login) à envoyer à splatch_afk.", "Help us|Create a trace (no code, no login) to send to splatch_afk.");
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
