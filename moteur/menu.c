#define _GNU_SOURCE
#include <dirent.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include "fous.h"

static int			g_corr_choix;
static const char	*g_corr_msg;
static int			g_bulles = 1;

int	tdf_nouveau(const char *depuis)
{
	char	cle[24];

	snprintf(cle, sizeof(cle), " %s ", depuis);
	return (strstr(TDF_NEW_ACTIFS, cle) != NULL);
}

void	tdf_badge_new(void)
{
	bgc(rgb(255, 55, 110));
	fg(rgb(255, 255, 255));
	out(" NEW ");
	bgc(theme()->bg);
}

static void	label_of(const char *item, char *b, size_t n)
{
	const char	*s;
	size_t		l;

	s = item[0] == '~' || item[0] == '!' ? item + 1 : item;
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

	cls();
	while (1)
	{
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
				bgc(mix(theme()->bg, theme()->accent, 0.22));
				fg(theme()->accent2);
			}
			else
				fg(items[i][0] == '~' ? theme()->dim : theme()->text);
			label_of(items[i], lab, sizeof(lab));
			outf(" %s %s", i == *sel ? "▶" : " ", lab);
			outf("%*s", 55 - ulen(lab) > 0 ? 55 - ulen(lab) : 1, "");
			bgc(theme()->bg);
			if (items[i][0] == '!')
			{
				at(x + ulen(lab) + 5, 5 + i * 2);
				tdf_badge_new();
			}
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
			*sel = ((*sel - tdf_touche_fleche(k)) % n + n) % n;
		else if (k == 1001)
			*sel = (*sel + tdf_touche_fleche(k)) % n;
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

static void	ecran_rangs(void)
{
	char	tmp[220];
	char	nom[80];
	char	suiv[80];
	long	a;
	long	b;
	int		lv;
	int		cur;
	int		x;
	int		y;
	int		i;
	int		k;
	int		bas;

	cls();
	while (1)
	{
		lv = level_of(g_save.xp);
		cur = (lv - 1) / 10;
		x = (term_cols() - 72) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		frame();
		center(1, theme()->accent2, L("R A N G S", "R A N K S"));
		center(2, theme()->dim, L("Un nouveau rang tous les 10 niveaux. L'XP vient des tests, de la banque et de l'exam.",
				"A new rank every 10 levels. XP comes from tests, the bank and the exam."));
		y = 4;
		for (i = 0; i <= 10 && y < term_rows() - 9; i++, y++)
		{
			tdf_rang(i * 10 + 1, nom, sizeof(nom));
			if (i == 10)
				snprintf(tmp, sizeof(tmp), "%s", L("niv. 101 et plus", "lvl 101 and up"));
			else
				snprintf(tmp, sizeof(tmp), L("niv. %d à %d", "lvl %d to %d"), i * 10 + 1, i * 10 + 10);
			at(x, y);
			if (i == (cur > 10 ? 10 : cur))
				bgc(mix(theme()->bg, theme()->accent, 0.22));
			fg(i < cur ? rgb(80, 230, 140) : i == (cur > 10 ? 10 : cur) ? theme()->accent2 : theme()->dim);
			outf(" %s %-20s %-34s ", i < cur ? "✓" : i == (cur > 10 ? 10 : cur) ? "▶" : "·", tmp, nom);
			bgc(theme()->bg);
		}
		bas = term_rows() - 7;
		if (bas < y + 1)
			bas = y + 1;
		tdf_rang(lv, nom, sizeof(nom));
		snprintf(tmp, sizeof(tmp), L("TON RANG : %s · niveau %d", "YOUR RANK: %s · level %d"), nom, lv);
		text_at(x, bas, rgb(255, 215, 0), tmp);
		a = xp_for(cur * 10 + 1);
		b = xp_for(cur * 10 + 11);
		bar(x, bas + 1, 50, b > a ? (g_save.xp - a) / (double)(b - a) : 1, theme()->accent, theme()->accent2);
		snprintf(tmp, sizeof(tmp), "%d %%", b > a ? (int)(100.0 * (g_save.xp - a) / (b - a)) : 100);
		text_at(x + 52, bas + 1, theme()->text, tmp);
		tdf_rang(cur * 10 + 11, suiv, sizeof(suiv));
		snprintf(tmp, sizeof(tmp), L("Encore %ld XP pour devenir « %s » (niveau %d).", "%ld more XP to become %s (level %d)."),
			b - g_save.xp > 0 ? b - g_save.xp : 0, suiv, cur * 10 + 11);
		text_at(x, bas + 2, theme()->dim, tmp);
		foot_line(x, L("Échap : retour", "Esc: back"));
		flush();
		k = key_read(200);
		if (key_quit(k) || k == 127 || k == 8 || k == '\r' || k == '\n')
			break ;
		if (k > 0)
			cls();
	}
	cls();
}

void	tdf_menu_chevalier(void (*profile)(void))
{
	static int	sel;
	const char	*items[5];
	int			i;

	items[0] = L("Avatar (garde-robe)|Équipe les objets que tu possèdes et choisis leurs couleurs.", "Avatar (wardrobe)|Equip the items you own and pick their colours.");
	items[1] = L("Boutique|Dépense tes Octets : objets, thèmes, titres. Les Octets se gagnent en validant.", "Shop|Spend your Octets: items, themes, titles. You earn Octets by passing tests.");
	items[2] = L("Profil et succès|Bannière, photo, bio et tous tes succès.", "Profile and achievements|Banner, picture, bio and all your achievements.");
	items[3] = L("Rangs|Ton rang, où tu en es avant le prochain, et les rangs à venir.", "Ranks|Your rank, how close the next one is, and the ranks to come.");
	items[4] = tdf_nouveau("3.2") ? L("!Tenues|Garde jusqu'à 5 tenues avec un nom et change-les en un appui. v : ton chevalier en grand.",
			"!Outfits|Keep up to 5 named outfits and switch in one press. v: your knight, large.")
		: L("Tenues|Garde jusqu'à 5 tenues avec un nom et change-les en un appui. v : ton chevalier en grand.",
			"Outfits|Keep up to 5 named outfits and switch in one press. v: your knight, large.");
	while (1)
	{
		i = tdf_list_menu(L("CHEVALIER", "KNIGHT"), items, 5, &sel, NULL);
		if (i < 0)
			return ;
		if (i == 0)
			screen_shop(1);
		else if (i == 1)
			screen_shop(0);
		else if (i == 2)
			profile();
		else if (i == 3)
			ecran_rangs();
		else
			screen_shop(2);
	}
}

static void	lore_cite(const char *a)
{
	char	path[700];
	char	l[64];
	FILE	*f;

	tdf_cine_force = 1;
	cls();
	if (!strcmp(a, "lore_gnl"))
	{
		l[0] = '\0';
		snprintf(path, sizeof(path), "%s/puits.txt", g_savedir);
		f = fopen(path, "r");
		if (f && fgets(l, sizeof(l), f))
			l[strcspn(l, "\r\n")] = '\0';
		if (f)
			fclose(f);
		tdf_histoire("puits", l);
	}
	else if (!strcmp(a, "lore_exam"))
	{
		tdf_histoire("avant", NULL);
		tdf_histoire("apres", "4");
	}
	else
		tdf_histoire("fin", NULL);
	tdf_cine_force = 0;
	cls();
}

void	tdf_menu_lore(void (*boss)(void))
{
	static int	sel;
	const char	*items[7];
	const char	*act[7];
	int			n;
	int			i;

	n = 0;
	act[n] = "libft";
	items[n++] = L("Libft · la Tour des Fondations|Les trois gardiens de la Libft et leurs cinématiques.", "Libft · the Tower of Foundations|The three Libft wardens and their cutscenes.");
	if (tdf_menu_unique())
	{
		act[n] = "lore_printf";
		items[n++] = L("ft_printf · la Fonderie des Formats|Les 9 chaînes, ft_printf le big boss, la victoire et la fin secrète.", "ft_printf · the Foundry of Formats|The 9 chains, ft_printf the big boss, the victory and the secret ending.");
		act[n] = "lore_gnl";
		items[n++] = L("get_next_line · le Puits|La descente du Puits, un anneau par test de ton dernier passage.", "get_next_line · the Well|The descent into the Well, one ring per test of your last run.");
		act[n] = "lore_exam";
		items[n++] = L("Exam · l'Épreuve du Silence|La salle de l'épreuve et ses portes.", "Exam · the Trial of Silence|The trial room and its doors.");
		act[n] = "lore_fin";
		items[n++] = L("La fin|Le dernier mécanisme de la Cité.", "The ending|The last mechanism of the City.");
	}
	act[n] = "carte";
	items[n++] = L("Carte de la Cité|Tous les lieux du lore, allumés selon ta progression. Entrée pour t'approcher.",
			"Map of the City|Every place of the lore, lit by your progress. Enter to go closer.");
	while (1)
	{
		if (sel >= n)
			sel = 0;
		i = tdf_list_menu(L("LORE", "LORE"), items, n, &sel, NULL);
		if (i < 0)
			return ;
		if (!strcmp(act[i], "libft"))
			boss();
		else if (!strcmp(act[i], "carte"))
			tdf_carte();
		else if (!strcmp(act[i], "lore_gnl") || !strcmp(act[i], "lore_exam") || !strcmp(act[i], "lore_fin"))
			lore_cite(act[i]);
		else
		{
			tdf_sortie(act[i], NULL);
			return ;
		}
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

char	g_choix[1100];

void	tdf_sortie(const char *action, const char *chemin)
{
	FILE	*f;
	char	path[700];

	snprintf(g_choix, sizeof(g_choix), "%s|%s", action, chemin ? chemin : "");
	snprintf(path, sizeof(path), "%s/menu_choix", g_savedir);
	f = fopen(path, "w");
	if (!f)
		return ;
	fprintf(f, "%s\n", g_choix);
	fclose(f);
}

int	tdf_menu_unique(void)
{
	return (getenv("TDF_MENU_UNIQUE") != NULL);
}

int	tdf_touche_fleche(int k)
{
	int	n;
	int	c;

	n = 1;
	while ((k == 1000 || k == 1001) && (c = key_read(0)) >= 0)
	{
		c = key_dir(c);
		if (c != k)
			break ;
		n++;
	}
	return (n);
}

static const char	*g_lien;

void	tdf_lien(int x, int y, const char *url)
{
	char	b[700];
	int		n;

	n = snprintf(b, sizeof(b), "\0337\033[%d;%dH\033]8;;%s\033\\%s\033]8;;\033\\\0338", y + 1, x + 1, url, url);
	if (n > 0 && n < (int)sizeof(b))
		(void)!write(1, b, n);
}

static int	page(const char *titre, const char *texte, const char **items, int n, int *sel)
{
	char	lab[160];
	int		k;
	int		i;
	int		x;
	int		y;
	int		w;
	int		ly;

	ly = 0;
	cls();
	while (1)
	{
		w = term_cols() - 8 < 72 ? term_cols() - 8 : 72;
		x = (term_cols() - w) / 2;
		if (x < 1)
			x = 1;
		bgc(theme()->bg);
		frame();
		center(2, theme()->accent2, titre);
		y = text_wrap(x, 5, term_rows() - 6 - n, w, theme()->text, texte) + 1;
		ly = y - 2;
		for (i = 0; i < n; i++)
		{
			at(x, y + i);
			if (i == *sel)
			{
				bgc(mix(theme()->bg, theme()->accent, 0.22));
				fg(theme()->accent2);
			}
			else
				fg(theme()->text);
			label_of(items[i], lab, sizeof(lab));
			outf(" %s %s ", i == *sel ? "▶" : " ", lab);
			bgc(theme()->bg);
			outf("%*s", 30, "");
		}
		foot_line(x, L("▲ ▼ choisir   Entrée : valider   Échap : passer",
				"▲ ▼ pick   Enter: confirm   Esc: skip"));
		flush();
		if (g_lien)
			tdf_lien(x, ly, g_lien);
		k = key_dir(key_read(40));
		if (key_quit(k))
			return (g_lien = NULL, cls(), -1);
		if (k == 1000)
			*sel = (*sel + n - 1) % n;
		else if (k == 1001)
			*sel = (*sel + 1) % n;
		else if (k == '\r' || k == '\n' || k == ' ')
			return (g_lien = NULL, cls(), *sel);
	}
}

void	tdf_tuto(void)
{
	const char	*titres[7];
	const char	*textes[7];
	const char	*items[3];
	char		tt[200];
	int			i;
	int			n;
	int			r;
	int			sel;

	titres[0] = L("Bienvenue", "Welcome");
	titres[1] = L("Tester un projet", "Test a project");
	titres[2] = L("Exam en C", "C exam");
	titres[3] = L("Banque d'exercices", "Exercise bank");
	titres[4] = L("Chevalier, Lore et Stats", "Knight, Lore and Stats");
	titres[5] = L("Mode correction", "Grading mode");
	titres[6] = L("Paramètres", "Settings");
	textes[0] = L("Le testeur des fous teste ta Libft, ton ft_printf et ton get_next_line comme la moulinette, et t'entraîne à l'exam. Tout se fait aux flèches : ↑ ↓ pour choisir, Entrée pour valider, Échap ou q pour revenir.",
			"The testeur des fous tests your Libft, ft_printf and get_next_line like the moulinette, and trains you for the exam. Everything works with the arrows: ↑ ↓ to pick, Enter to confirm, Esc or q to go back.");
	textes[1] = L("Le menu liste les projets trouvés dans ce dossier ou à côté du testeur. Choisis-en un : tests des fonctions, fuites mémoire, Norme, fichiers parasites et audit de la moulinette. Pour un projet rangé ailleurs, « Tester un autre dossier » te laisse parcourir ton PC avec Tab, comme dans zsh.",
			"The menu lists the projects found in this folder or next to the tester. Pick one: function tests, memory leaks, the Norm, garbage files and the moulinette audit. For a project stored elsewhere, 'Test another folder' lets you browse your PC with Tab, like in zsh.");
	textes[2] = L("Examen blanc de 3 h, série d'exercices ou entraînement libre. Tu écris ton code dans un autre terminal, puis tu tapes grademe dans l'examshell. Une commande mal tapée ? Le testeur te propose la bonne.",
			"3-hour mock exam, exercise series or free training. You write your code in another terminal, then type grademe in the examshell. Mistyped a command? The tester suggests the right one.");
	textes[3] = L("Tous les exercices du testeur, dans l'ordre que tu veux. ◀ ▶ change d'onglet (Exam, Libft, printf), Entrée ouvre le sujet, puis « Corriger mon code ». Chaque sujet existe en 8 langues.",
			"Every exercise of the tester, in any order. ◀ ▶ switches tabs (Exam, Libft, printf), Enter opens the subject, then 'Grade my code'. Every subject exists in 8 languages.");
	textes[4] = L("Chaque fonction validée rapporte de l'expérience et des Octets. Habille ton chevalier, débloque l'histoire de la Cité qui compile et suis ta progression dans Stats.",
			"Every function you pass earns experience and Octets. Dress your knight, unlock the story of the City that compiles and follow your progress in Stats.");
	textes[5] = L("Tu corriges quelqu'un ? Le bouton bleu « Mode correction » du menu (ou « tdf correction » dans le terminal) ne garde que le dossier du corrigé : tous ses tests sans cinématiques, les vérifs de rendu en un écran et un test précis avec le détail. Ses résultats ne comptent pas dans ta progression.",
			"Grading someone? The blue 'Grading mode' button of the menu (or 'tdf correction' in the terminal) keeps only the graded folder: all its tests without cutscenes, submission checks on one screen and a single test with every detail. Their results do not count in your progress.");
	textes[6] = L("Langue, interface graphique ou texte, et la commande tdf pour lancer le testeur depuis n'importe quel dossier. Ce tuto et les nouveautés se relancent depuis Paramètres.",
			"Language, graphical or text interface, and the tdf command to start the tester from any folder. This tutorial and the news can be replayed from Settings.");
	i = 0;
	while (i < 7)
	{
		n = 0;
		items[n++] = i < 6 ? L("Suivant|", "Next|") : L("C'est parti|", "Let's go|");
		if (i > 0)
			items[n++] = L("Précédent|", "Previous|");
		if (i < 6)
			items[n++] = L("Passer le tuto|", "Skip the tutorial|");
		snprintf(tt, sizeof(tt), "%s · %d/7 · %s", L("TUTO", "TUTORIAL"), i + 1, titres[i]);
		sel = 0;
		r = page(tt, textes[i], items, n, &sel);
		if (r < 0 || (i < 6 && r == n - 1))
			return ;
		if (i > 0 && r == 1)
			i--;
		else
			i++;
	}
}

void	tdf_nouveautes(void)
{
	char		titre[120];
	char		texte[4000];
	const char	*items[2];
	int			sel;

	snprintf(titre, sizeof(titre), L("CHANGELOG · VERSION %s", "CHANGELOG · VERSION %s"), TDF_VERSION);
	snprintf(texte, sizeof(texte), "%s\n%s\n%s\n%s\n\n%s\n%s\n%s\n\n%s\n" TDF_DISCORD,
		L("NOUVEAU DANS LA 3.4.0", "NEW IN 3.4.0"),
		L("• Prototypes comparés au sujet et aux vraies traces de la moulinette, dans le .h et dans chaque .c (restrict interdit, noms des paramètres).", "• Prototypes compared with the subject and real moulinette traces, in the .h and in every .c (restrict forbidden, parameter names)."),
		L("• Includes vérifiés : include introuvable, .c inclus, fonction sans déclaration, .h qui ne compile pas seul, garde d'inclusion, include inutile.", "• Includes checked: missing include, included .c, undeclared function, .h that does not compile alone, include guard, useless include."),
		L("• Libft, ft_printf et get_next_line.", "• Libft, ft_printf and get_next_line."),
		L("DÉJÀ DANS LA 3.3.0", "ALREADY IN 3.3.0"),
		L("• Mode correction (menu) : le dossier du corrigé seulement, vérifs de rendu en un écran et test précis avec le détail.", "• Grading mode (menu): only the graded folder, submission checks on one screen and single tests with every detail."),
		L("• Mode debug : une barre d'avancement avec le temps restant au lieu d'un écran vide.", "• Debug mode: a progress bar with the time left instead of an empty screen."),
		L("Tu veux en savoir plus ? Viens sur le serveur Discord (Ctrl + clic sur le lien) :", "Want to know more? Join the Discord server (Ctrl + click on the link):"));
	items[0] = L("Continuer|", "Continue|");
	items[1] = L("Voir le tuto|", "See the tutorial|");
	g_lien = TDF_DISCORD;
	sel = 0;
	if (page(titre, texte, items, 2, &sel) == 1)
		tdf_tuto();
}

static int	interface_texte(void)
{
	char	path[700];
	char	l[80];
	FILE	*f;
	int		t;

	snprintf(path, sizeof(path), "%s/lanceur.txt", g_savedir);
	t = 0;
	f = fopen(path, "r");
	while (f && fgets(l, sizeof(l), f))
		if (!strncmp(l, "interface=texte", 15))
			t = 1;
	if (f)
		fclose(f);
	return (t);
}

static void	interface_basculer(void)
{
	char	path[700];
	FILE	*f;
	int		t;

	t = interface_texte();
	snprintf(path, sizeof(path), "%s/lanceur.txt", g_savedir);
	f = fopen(path, "w");
	if (!f)
		return ;
	fprintf(f, "interface=%s\n", t ? "graphique" : "texte");
	fclose(f);
}

static int	tdf_installee(void)
{
	char		path[700];
	char		l[900];
	char		lance[760];
	FILE		*f;
	const char	*racine;
	int			etat;

	snprintf(path, sizeof(path), "%s/.local/bin/tdf", getenv("HOME") ? getenv("HOME") : "/tmp");
	if (access(path, X_OK) != 0)
		return (0);
	racine = getenv("TDF_RACINE");
	if (!racine || !racine[0])
		return (1);
	snprintf(lance, sizeof(lance), "%s/run.sh", racine);
	f = fopen(path, "r");
	etat = 2;
	while (f && fgets(l, sizeof(l), f))
	{
		l[strcspn(l, "\n")] = '\0';
		if ((!strncmp(l, "# racine=", 9) && !strcmp(l + 9, racine))
			|| (!strncmp(l, "exec bash ", 10) && !strncmp(l + 10, lance, strlen(lance))
				&& l[10 + strlen(lance)] == ' '))
			etat = 1;
	}
	if (f)
		fclose(f);
	return (etat);
}

typedef struct s_cand
{
	char	nom[256];
	char	tag[40];
	char	chemin[700];
}	t_cand;

static int	cand_cmp(const void *a, const void *b)
{
	return (strcmp(((const t_cand *)a)->nom, ((const t_cand *)b)->nom));
}

static void	developper(const char *saisie, char *out, size_t n)
{
	const char	*d;

	d = getenv("TDF_DEPART") ? getenv("TDF_DEPART") : ".";
	if (saisie[0] == '~' && getenv("HOME"))
		snprintf(out, n, "%s%s", getenv("HOME"), saisie + 1);
	else if (saisie[0] == '/')
		snprintf(out, n, "%s", saisie);
	else
		snprintf(out, n, "%s/%s", d, saisie);
}

static void	couper(const char *saisie, char *dir, size_t n, const char **pre)
{
	const char	*s;
	char		tmp[1024];

	s = strrchr(saisie, '/');
	*pre = s ? s + 1 : saisie;
	if (!s)
		snprintf(tmp, sizeof(tmp), ".");
	else if (s == saisie)
		snprintf(tmp, sizeof(tmp), "/");
	else
		snprintf(tmp, sizeof(tmp), "%.*s", (int)(s - saisie), saisie);
	developper(tmp, dir, n);
}

static void	etiquette(const char *dir, const char *nom, char *tag, size_t n)
{
	static const char	*f[][2] = {{"ft_printf.h", "ft_printf"}, {"get_next_line.h", "get_next_line"},
		{"libft.h", "libft"}};
	char				p[1400];
	int					i;

	tag[0] = '\0';
	for (i = 0; i < 3; i++)
	{
		snprintf(p, sizeof(p), "%s/%s/%s", dir, nom, f[i][0]);
		if (access(p, R_OK) == 0)
		{
			snprintf(tag, n, "%s", f[i][1]);
			return ;
		}
	}
}

static void	court(const char *p, char *b, size_t n)
{
	const char	*h;

	h = getenv("HOME");
	if (h && *h && !strncmp(p, h, strlen(h)))
		snprintf(b, n, "~%s", p + strlen(h));
	else
		snprintf(b, n, "%s", p);
}

static int	lister(const char *saisie, t_cand *c, int max)
{
	DIR				*d;
	struct dirent	*e;
	struct stat		st;
	char			dir[1024];
	char			p[1400];
	const char		*pre;
	int				n;

	couper(saisie, dir, sizeof(dir), &pre);
	d = opendir(dir);
	n = 0;
	while (d && (e = readdir(d)) && n < max)
	{
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
			continue ;
		if (e->d_name[0] == '.' && pre[0] != '.')
			continue ;
		if (strncasecmp(e->d_name, pre, strlen(pre)))
			continue ;
		snprintf(p, sizeof(p), "%s/%s", dir, e->d_name);
		if (stat(p, &st) != 0 || !S_ISDIR(st.st_mode))
			continue ;
		snprintf(c[n].nom, sizeof(c[n].nom), "%s", e->d_name);
		c[n].chemin[0] = '\0';
		etiquette(dir, e->d_name, c[n].tag, sizeof(c[n].tag));
		n++;
	}
	if (d)
		closedir(d);
	qsort(c, n, sizeof(t_cand), cand_cmp);
	return (n);
}

static void	completer(char *saisie, size_t sz, const char *nom, int slash)
{
	char	*s;

	s = strrchr(saisie, '/');
	if (s)
		s[1] = '\0';
	else
		saisie[0] = '\0';
	snprintf(saisie + strlen(saisie), sz - strlen(saisie), "%s%s", nom, slash ? "/" : "");
}

static void	prefixe_commun(char *saisie, size_t sz, t_cand *c, int n)
{
	char	p[256];
	int		l;
	int		i;

	snprintf(p, sizeof(p), "%s", c[0].nom);
	for (i = 1; i < n; i++)
	{
		for (l = 0; p[l] && tolower((unsigned char)p[l]) == tolower((unsigned char)c[i].nom[l]); l++)
			;
		p[l] = '\0';
	}
	if (strlen(p) < strlen(strrchr(saisie, '/') ? strrchr(saisie, '/') + 1 : saisie))
		return ;
	completer(saisie, sz, p, n == 1);
}

static int	valider(const char *chemin, char *out, size_t n)
{
	char		vrai[PATH_MAX];
	struct stat	st;

	if (stat(chemin, &st) == 0 && S_ISDIR(st.st_mode) && realpath(chemin, vrai))
	{
		snprintf(out, n, "%s", vrai);
		return (1);
	}
	return (0);
}

typedef struct s_ent
{
	char	lab[300];
	char	info[200];
	char	chemin[700];
	char	tag[40];
	int		section;
}	t_ent;

static void	type_projet(const char *p, char *tag, size_t n)
{
	char	f[800];

	tag[0] = '\0';
	snprintf(f, sizeof(f), "%s/ft_printf.h", p);
	if (access(f, R_OK) == 0)
		snprintf(tag, n, "ft_printf");
	snprintf(f, sizeof(f), "%s/get_next_line.h", p);
	if (!tag[0] && access(f, R_OK) == 0)
		snprintf(tag, n, "get_next_line");
	snprintf(f, sizeof(f), "%s/libft.h", p);
	if (!tag[0] && access(f, R_OK) == 0)
		snprintf(tag, n, "libft");
}

static int	deja(t_ent *e, int n, const char *p)
{
	int	i;

	for (i = 0; i < n; i++)
		if (!strcmp(e[i].chemin, p))
			return (1);
	return (0);
}

static int	entrees(t_ent *e, int max)
{
	char		l[1200];
	char		*f[5];
	char		*q;
	const char	*s;
	const char	*fin;
	FILE		*h;
	int			n;
	int			i;
	int			nh;
	t_ent		hist[10];
	time_t		t;
	struct tm	*tm;

	n = 0;
	snprintf(e[n].chemin, sizeof(e[n].chemin), "%s", getenv("TDF_DEPART") ? getenv("TDF_DEPART") : ".");
	court(e[n].chemin, l, sizeof(l));
	snprintf(e[n].lab, sizeof(e[n].lab), "%s  %s", g_corr_choix ? L("Choisir ce dossier :", "Pick this folder:")
		: L("Tester ce dossier :", "Test this folder:"), l);
	type_projet(e[n].chemin, e[n].tag, sizeof(e[n].tag));
	snprintf(e[n].info, sizeof(e[n].info), "%s", e[n].tag[0] ? "" : L("aucun projet reconnu ici", "no project recognised here"));
	e[n++].section = 0;
	snprintf(l, sizeof(l), "%s/historique_projets.txt", g_savedir);
	h = g_corr_choix ? NULL : fopen(l, "r");
	nh = 0;
	while (h && nh < 10 && fgets(l, sizeof(l), h))
	{
		l[strcspn(l, "\n")] = '\0';
		q = l;
		for (i = 0; i < 5 && q; i++)
		{
			f[i] = q;
			q = strchr(q, '\t');
			if (q)
				*q++ = '\0';
		}
		if (i < 5)
			continue ;
		snprintf(hist[nh].chemin, sizeof(hist[0].chemin), "%s", f[2]);
		court(f[2], hist[nh].lab, sizeof(hist[0].lab));
		snprintf(hist[nh].tag, sizeof(hist[0].tag), "%s", !strcmp(f[1], "printf") ? "ft_printf" : !strcmp(f[1], "gnl") ? "get_next_line" : "libft");
		t = (time_t)atol(f[0]);
		tm = localtime(&t);
		snprintf(hist[nh].info, sizeof(hist[0].info), "%s/%s · %02d/%02d %02dh%02d", f[3], f[4],
			tm ? tm->tm_mday : 0, tm ? tm->tm_mon + 1 : 0, tm ? tm->tm_hour : 0, tm ? tm->tm_min : 0);
		hist[nh++].section = 1;
	}
	if (h)
		fclose(h);
	for (i = nh - 1; i >= 0 && n < max; i--)
		if (!deja(e, n, hist[i].chemin))
			e[n++] = hist[i];
	s = getenv("TDF_PROJETS");
	while (s && *s && n < max)
	{
		fin = strchr(s, '\n');
		snprintf(l, sizeof(l), "%.*s", (int)(fin ? fin - s : (long)strlen(s)), s);
		s = fin ? fin + 1 : s + strlen(s);
		q = strchr(l, '\t');
		if (!q || deja(e, n, q + 1))
			continue ;
		*q++ = '\0';
		snprintf(e[n].chemin, sizeof(e[n].chemin), "%s", q);
		court(q, e[n].lab, sizeof(e[n].lab));
		snprintf(e[n].tag, sizeof(e[n].tag), "%s", !strcmp(l, "printf") ? "ft_printf" : !strcmp(l, "gnl") ? "get_next_line" : "libft");
		e[n].info[0] = '\0';
		e[n++].section = 2;
	}
	return (n);
}

static void	dessiner_entrees(t_ent *e, int n, int sel, int x, int y, int h)
{
	static const char	*titres[3];
	int					i;
	int					sec;
	int					top;

	titres[0] = L("Ce dossier", "This folder");
	titres[1] = L("Derniers tests (Entrée pour relancer)", "Last tests (Enter to run again)");
	titres[2] = L("Projets trouvés à côté du testeur", "Projects found next to the tester");
	top = sel > h - 6 ? sel - (h - 6) : 0;
	sec = -1;
	for (i = top; i < n && y < 7 + h; i++)
	{
		if (e[i].section != sec)
		{
			sec = e[i].section;
			y++;
			text_at(x, y++, theme()->dim, titres[sec]);
		}
		at(x, y);
		if (i == sel)
		{
			bgc(mix(theme()->bg, theme()->accent, 0.25));
			fg(theme()->accent2);
		}
		else
			fg(e[i].tag[0] ? theme()->text : theme()->dim);
		outf(" %s %s ", i == sel ? "▶" : " ", e[i].lab);
		bgc(theme()->bg);
		if (e[i].tag[0])
			text_at(x + ulen(e[i].lab) + 5, y, theme()->accent2, e[i].tag);
		if (e[i].info[0])
			text_at(x + ulen(e[i].lab) + 7 + ulen(e[i].tag), y,
				strchr(e[i].info, '/') && atoi(e[i].info) == atoi(strchr(e[i].info, '/') + 1) && atoi(e[i].info) > 0
				? rgb(140, 230, 170) : theme()->dim, e[i].info);
		y++;
	}
}

#define NOPT 7

static int	g_opt[NOPT];

static const char	*opt_cle(int i)
{
	static const char	*c[NOPT] = {"debug", "audit", "valgrind", "sans_histoire", "sans_anim", "pousse", "auto"};

	return (c[i]);
}

static void	opt_lire(void)
{
	FILE	*f;
	char	p[700];
	char	l[80];
	int		i;

	memset(g_opt, 0, sizeof(g_opt));
	snprintf(p, sizeof(p), "%s/options_test.txt", g_savedir);
	f = fopen(p, "r");
	while (f && fgets(l, sizeof(l), f))
		for (i = 0; i < NOPT; i++)
			if (!strncmp(l, opt_cle(i), strlen(opt_cle(i))) && l[strlen(opt_cle(i))] == '=')
				g_opt[i] = atoi(l + strlen(opt_cle(i)) + 1) != 0;
	if (f)
		fclose(f);
}

static void	opt_ecrire(void)
{
	FILE	*f;
	char	p[700];
	int		i;

	snprintf(p, sizeof(p), "%s/options_test.txt", g_savedir);
	f = fopen(p, "w");
	if (!f)
		return ;
	for (i = 0; i < NOPT; i++)
		fprintf(f, "%s=%d\n", opt_cle(i), g_opt[i]);
	fclose(f);
}

static int	dessiner_options(int x, int y, int sel)
{
	const char	*nom[NOPT];
	const char	*desc[NOPT];
	int			i;

	nom[0] = L("Mode debug", "Debug mode");
	desc[0] = L("tous les rapports détaillés (fuites, mémoire, comportements indéfinis), en texte",
			"every detailed report (leaks, memory, undefined behaviour), as text");
	nom[1] = L("Audit complet", "Full audit");
	desc[1] = L("l'audit de la moulinette même si des fonctions échouent (Libft)",
			"the moulinette audit even if some functions fail (Libft)");
	nom[2] = L("Valgrind en plus", "Valgrind too");
	desc[2] = L("repasse les tests sous valgrind, plus lent (Libft, ft_printf)",
			"runs the tests again under valgrind, slower (Libft, ft_printf)");
	nom[3] = L("Sans cinématiques", "No cutscenes");
	desc[3] = L("seulement les tests, sans histoire", "only the tests, no story");
	nom[4] = L("Sans animations", "No animations");
	desc[4] = L("affichage direct, sans effets", "direct display, no effects");
	nom[5] = L("Ce qui est poussé", "What is pushed");
	desc[5] = L("teste ton dernier commit git au lieu du dossier : un fichier oublié dans git se voit",
			"tests your last git commit instead of the folder: a file forgotten in git shows up");
	nom[6] = L("Relance automatique", "Automatic rerun");
	desc[6] = L("Libft : les tests repartent à chaque sauvegarde de tes fichiers, laisse l'écran des résultats à côté de ton éditeur",
			"Libft: the tests run again each time you save your files, keep the results screen next to your editor");
	text_at(x, y, theme()->accent2, L("OPTIONS DU TEST", "TEST OPTIONS"));
	if (tdf_nouveau("3.2"))
	{
		at(x + ulen(L("OPTIONS DU TEST", "TEST OPTIONS")) + 1, y);
		tdf_badge_new();
	}
	y++;
	for (i = 0; i < NOPT; i++, y++)
	{
		at(x, y);
		if (i == sel)
		{
			bgc(mix(theme()->bg, theme()->accent, 0.25));
			fg(theme()->accent2);
		}
		else
			fg(theme()->text);
		outf(" %s %-20s", i == sel ? "▶" : " ", nom[i]);
		fg(g_opt[i] ? rgb(80, 230, 140) : theme()->dim);
		outf(" ◀ %-3s ▶ ", g_opt[i] ? L("oui", "yes") : L("non", "no"));
		bgc(theme()->bg);
		if (i >= 5 && tdf_nouveau("3.4"))
		{
			at(x + 36, y);
			tdf_badge_new();
			text_clip(x + 42, y, theme()->dim, desc[i], term_cols() - x - 44);
		}
		else
			text_clip(x + 36, y, theme()->dim, desc[i], term_cols() - x - 38);
	}
	return (y);
}

static void	aide_chemin(int y0, int y1)
{
	const char	*l[6];
	char		dep[300];
	int			w;
	int			x;
	int			i;
	int			y;

	snprintf(dep, sizeof(dep), L("Tu es dans : %.240s", "You are in: %.240s"), getenv("TDF_DEPART") ? getenv("TDF_DEPART") : ".");
	l[0] = L("OÙ EST TON PROJET ?", "WHERE IS YOUR PROJECT?");
	l[1] = dep;
	l[2] = L("• Le testeur des fous est à côté de ton projet ? Commence par ../ : par exemple ../libft",
			"• The testeur des fous is next to your project? Start with ../ : for example ../libft");
	l[3] = L("• Ton projet est le dossier où tu as lancé le testeur ? Tape . (un point).",
			"• Your project is the folder you started the tester from? Type . (a dot).");
	l[4] = L("• ~/ part de ton dossier perso, / de la racine. Tab complète, Tab répété fait défiler les choix.",
			"• ~/ starts from your home folder, / from the root. Tab completes, pressing Tab again cycles the choices.");
	l[5] = L("• Le dossier doit contenir libft.h, ft_printf.h, ou get_next_line.c et get_next_line.h.",
			"• The folder must contain libft.h, ft_printf.h, or get_next_line.c and get_next_line.h.");
	if (y1 - y0 < 8)
		return ;
	w = term_cols() - 8 < 112 ? term_cols() - 8 : 112;
	x = (term_cols() - w) / 2;
	y = y0 + (y1 - y0 - 8) / 2;
	fill(x, y, w, 8, mix(theme()->bg, theme()->accent, 0.08));
	box(x, y, w, 8, mix(theme()->bg, theme()->accent, 0.45));
	bgc(mix(theme()->bg, theme()->accent, 0.08));
	for (i = 0; i < 6; i++)
		text_clip(x + 3, y + 1 + i, i == 0 ? theme()->accent2 : i == 1 ? theme()->dim : i == 2 ? rgb(255, 215, 120) : theme()->text, l[i], w - 6);
	bgc(theme()->bg);
}

int	tdf_choisir_dossier(char *out, size_t n)
{
	static t_cand	c[400];
	static t_ent	e[30];
	char			saisie[1024];
	char			tmp[1100];
	int				ne;
	int				nc;
	int				mode;
	int				sel;
	int				csel;
	int				ctop;
	int				k;
	int				i;
	int				x;
	int				h;
	const char		*msg;

	ne = entrees(e, 30);
	opt_lire();
	saisie[0] = '\0';
	mode = 0;
	sel = 0;
	csel = -1;
	ctop = 0;
	nc = 0;
	msg = g_corr_choix ? g_corr_msg : NULL;
	cls();
	while (1)
	{
		h = term_rows() - 12;
		if (h > ne + 6 && !mode)
			h = ne + 6;
		if (h > 14)
			h = 14;
		if (h < 4)
			h = 4;
		x = (term_cols() - 80) / 2;
		if (x < 2)
			x = 2;
		if (mode)
			nc = saisie[0] ? lister(saisie, c, 400) : 0;
		if (csel >= nc)
			csel = nc - 1;
		bgc(theme()->bg);
		frame();
		center(1, theme()->accent2, g_corr_choix ? L("DOSSIER DU CORRIGÉ", "FOLDER OF THE STUDENT YOU GRADE")
			: L("TESTER MON PROJET", "TEST MY PROJECT"));
		at(x, 4);
		bgc(mode ? mix(theme()->bg, theme()->accent, 0.18) : theme()->bg);
		if (mode)
		{
			fg(theme()->text);
			outf(" %s▏", saisie);
		}
		else
		{
			fg(theme()->dim);
			outf(" %s", L("↑ pour écrire un chemin (Tab complète comme zsh, majuscules ou non)",
					"↑ to type a path (Tab completes like zsh, any case)"));
		}
		outf("%*s", 2, "");
		bgc(theme()->bg);
		fill(x, 6, term_cols() - x - 2, h + 2, theme()->bg);
		if (!mode)
			dessiner_entrees(e, ne, sel < ne ? sel : -1, x, 5, h);
		else
		{
			if (csel >= 0 && csel < ctop)
				ctop = csel;
			if (csel >= ctop + h)
				ctop = csel - h + 1;
			for (i = ctop; i < nc && i < ctop + h; i++)
			{
				at(x, 6 + i - ctop);
				if (i == csel)
				{
					bgc(mix(theme()->bg, theme()->accent, 0.25));
					fg(theme()->accent2);
				}
				else
					fg(c[i].tag[0] ? theme()->accent : theme()->text);
				outf(" %s %s/ ", i == csel ? "▶" : " ", c[i].nom);
				bgc(theme()->bg);
				if (c[i].tag[0])
					text_at(x + ulen(c[i].nom) + 7, 6 + i - ctop, theme()->accent2, c[i].tag);
			}
			if (saisie[0] && !nc)
				text_at(x, 6, theme()->dim, L("aucun dossier ne correspond", "no folder matches"));
			if (!saisie[0])
				text_at(x, 6, theme()->dim, L("Tape un chemin : ~/ pour ton dossier perso, / pour la racine, . pour ici.",
						"Type a path: ~/ for your home, / for the root, . for here."));
		}
		k = mode || g_corr_choix ? 8 + h : dessiner_options(x, 8 + h, sel - ne) + 1;
		aide_chemin(k, term_rows() - 4);
		if (msg)
			text_at(x, term_rows() - 3, rgb(255, 120, 120), msg);
		foot_line(x, mode ? L("Tab compléter   ▲ ▼ choisir   ▶ ouvrir   ◀ remonter   Entrée valider   Échap : liste",
				"Tab complete   ▲ ▼ pick   ▶ open   ◀ up   Enter confirm   Esc: list")
			: g_corr_choix ? L("▲ ▼ choisir   ↑ tout en haut : écrire un chemin   Entrée valider   Échap retour",
				"▲ ▼ pick   ↑ at the top: type a path   Enter confirm   Esc back")
			: L("▲ ▼ choisir   ↑ tout en haut : écrire un chemin   Entrée tester   Échap retour",
				"▲ ▼ pick   ↑ at the top: type a path   Enter test   Esc back"));
		flush();
		k = key_read(200);
		if (k < 0)
			continue ;
		msg = NULL;
		if (!mode)
		{
			k = key_dir(k);
			if (key_quit(k) || k == 127 || k == 8)
				return (cls(), 0);
			if (k == 1000 && sel == 0)
			{
				mode = 1;
				csel = -1;
			}
			else if (k == 1000)
				sel--;
			else if (k == 1001 && sel < ne + (g_corr_choix ? 0 : NOPT) - 1)
				sel++;
			else if (sel >= ne && (k == '\r' || k == '\n' || k == ' ' || k == 1002 || k == 1003))
			{
				g_opt[sel - ne] = !g_opt[sel - ne];
				opt_ecrire();
			}
			else if (k == '\r' || k == '\n' || k == ' ' || k == 1002)
			{
				if (valider(e[sel].chemin, out, n))
					return (cls(), 1);
				msg = L("ce dossier n'existe plus", "this folder no longer exists");
			}
			continue ;
		}
		if (k == 27 || k == 3)
		{
			mode = 0;
			continue ;
		}
		if (k == 9 && nc && csel >= 0)
			csel = (csel + 1) % nc;
		else if (k == 9 && nc)
		{
			snprintf(tmp, sizeof(tmp), "%s", saisie);
			prefixe_commun(saisie, sizeof(saisie), c, nc);
			if (nc > 1 && !strcmp(tmp, saisie))
				csel = 0;
		}
		else if (k == 1000 && nc)
			csel = csel <= 0 ? nc - 1 : csel - 1;
		else if (k == 1001 && csel < 0 && (!saisie[0] || !nc))
			mode = 0;
		else if (k == 1001 && nc)
			csel = (csel + 1) % nc;
		else if (k == 1002 && csel >= 0)
		{
			completer(saisie, sizeof(saisie), c[csel].nom, 1);
			csel = -1;
			ctop = 0;
		}
		else if (k == 1003 && saisie[0])
		{
			i = (int)strlen(saisie);
			if (i > 1 && saisie[i - 1] == '/')
				saisie[--i] = '\0';
			while (i > 0 && saisie[i - 1] != '/')
				saisie[--i] = '\0';
			csel = -1;
			ctop = 0;
		}
		else if (k == 127 || k == 8)
		{
			if (saisie[0])
				saisie[strlen(saisie) - 1] = '\0';
			csel = -1;
		}
		else if (k == '\r' || k == '\n')
		{
			if (csel >= 0)
			{
				completer(saisie, sizeof(saisie), c[csel].nom, 1);
				csel = -1;
				ctop = 0;
				continue ;
			}
			developper(saisie[0] ? saisie : ".", tmp, sizeof(tmp));
			if (valider(tmp, out, n))
				return (cls(), 1);
			msg = L("ce dossier n'existe pas", "this folder does not exist");
		}
		else if (k >= 32 && k < 127 && strlen(saisie) + 2 < sizeof(saisie))
		{
			saisie[strlen(saisie) + 1] = '\0';
			saisie[strlen(saisie)] = (char)k;
			csel = -1;
			ctop = 0;
		}
	}
}

int	tdf_correction(char *chemin, size_t n)
{
	FILE	*f;
	char	path[700];
	char	l[1100];

	snprintf(path, sizeof(path), "%s/correction.txt", g_savedir);
	f = fopen(path, "r");
	if (!f)
		return (0);
	if (!fgets(l, sizeof(l), f))
		l[0] = '\0';
	fclose(f);
	l[strcspn(l, "\r\n")] = '\0';
	if (chemin && n)
		snprintf(chemin, n, "%s", l);
	return (1);
}

void	tdf_correction_ecrire(const char *chemin)
{
	FILE	*f;
	char	path[700];

	snprintf(path, sizeof(path), "%s/correction.txt", g_savedir);
	if (!chemin)
	{
		remove(path);
		return ;
	}
	f = fopen(path, "w");
	if (!f)
		return ;
	fprintf(f, "%s\n", chemin);
	fclose(f);
}

static int	corr_type(const char *d);

int	tdf_correction_dossier(char *out, size_t n)
{
	int	r;

	g_corr_choix = 1;
	g_corr_msg = NULL;
	r = tdf_choisir_dossier(out, n);
	while (r && !corr_type(out))
	{
		g_corr_msg = L("Ce dossier n'est ni une Libft, ni un ft_printf, ni un get_next_line : choisis le dossier du projet du corrigé.",
				"This folder is neither a Libft, nor a ft_printf, nor a get_next_line: pick the folder of the graded project.");
		r = tdf_choisir_dossier(out, n);
	}
	g_corr_choix = 0;
	g_corr_msg = NULL;
	if (r)
		tdf_correction_ecrire(out);
	return (r);
}

void	tdf_correction_titre(char *out, size_t n)
{
	char	ch[600];
	char	p[700];
	char	l[400];
	char	lg[60];
	FILE	*f;
	char	*b;
	char	*e;

	lg[0] = '\0';
	l[0] = '\0';
	if (tdf_correction(ch, sizeof(ch)) && ch[0])
	{
		snprintf(p, sizeof(p), "%s/README.md", ch);
		f = fopen(p, "r");
		if (f && !fgets(l, sizeof(l), f))
			l[0] = '\0';
		if (f)
			fclose(f);
		b = strstr(l, " by ");
		if (b)
		{
			b += 4;
			e = b + strcspn(b, "*_\r\n");
			while (e > b && (e[-1] == ' ' || e[-1] == '.'))
				e--;
			snprintf(lg, sizeof(lg), "%.*s", (int)(e - b) > 40 ? 40 : (int)(e - b), b);
		}
		b = strrchr(ch, '/');
		if (!lg[0])
			snprintf(lg, sizeof(lg), "%.40s", b && b[1] ? b + 1 : ch);
	}
	snprintf(out, n, "%s%s%s", L("testeur des fous | mode : correction", "testeur des fous | mode: grading"), lg[0] ? " · " : "", lg);
}

void	tdf_bulles_lire(void)
{
	char	p[700];
	FILE	*f;
	int		c;

	snprintf(p, sizeof(p), "%s/bulles.txt", g_savedir);
	f = fopen(p, "r");
	c = f ? fgetc(f) : '1';
	if (f)
		fclose(f);
	g_bulles = c != '0';
}

int	tdf_bulles(void)
{
	return (g_bulles);
}

static void	ech_modifier(t_ech *e, int *n, int k)
{
	const char	*it[3];
	char		buf[100];
	char		iso[12];
	int			sel;
	int			r;

	it[0] = L("Changer la date|AAAA-MM-JJ ou JJ/MM/AAAA.", "Change the date|YYYY-MM-DD or DD/MM/YYYY.");
	it[1] = L("Renommer|", "Rename|");
	it[2] = L("Supprimer|", "Delete|");
	sel = 0;
	r = tdf_list_menu(e[k].nom, it, 3, &sel, NULL);
	if (r == 0)
	{
		snprintf(buf, sizeof(buf), "%s", e[k].date);
		while (tdf_saisir(L("Date de l'échéance", "Deadline date"), L("AAAA-MM-JJ ou JJ/MM/AAAA, par exemple 12/12/2026.",
					"YYYY-MM-DD or DD/MM/YYYY, for example 12/12/2026."), buf, sizeof(buf), 10, 0, 1))
		{
			if (tdf_date_ok(buf, iso, sizeof(iso)))
			{
				snprintf(e[k].date, sizeof(e[k].date), "%s", iso);
				break ;
			}
		}
	}
	else if (r == 1)
	{
		snprintf(buf, sizeof(buf), "%s", e[k].nom);
		if (tdf_saisir(L("Nom de l'échéance", "Deadline name"), "", buf, sizeof(buf), 60, 0, 1) && buf[0])
			snprintf(e[k].nom, sizeof(e[k].nom), "%s", buf);
	}
	else if (r == 2)
	{
		memmove(e + k, e + k + 1, sizeof(t_ech) * (*n - k - 1));
		(*n)--;
	}
	if (r >= 0)
		tdf_echeances_ecrire(e, *n);
}

static void	ecran_echeances(void)
{
	static char	lab[13][200];
	const char	*it[13];
	t_ech		e[12];
	int			n;
	int			i;
	int			sel;
	int			r;

	sel = 0;
	while (1)
	{
		n = tdf_echeances(e, 12);
		for (i = 0; i < n; i++)
		{
			snprintf(lab[i], sizeof(lab[i]), L("%s · %.2s/%.2s/%.4s|%ld jour(s) restant(s). Entrée : changer la date, renommer ou supprimer.",
					"%s · %.2s/%.2s/%.4s|%ld day(s) left. Enter: change the date, rename or delete."),
				e[i].nom, e[i].date + 8, e[i].date + 5, e[i].date, e[i].jours < 0 ? 0 : e[i].jours);
			it[i] = lab[i];
		}
		snprintf(lab[n], sizeof(lab[n]), "%s", L("＋ Ajouter une échéance|Un nom et une date.", "＋ Add a deadline|A name and a date."));
		it[n] = lab[n];
		if (sel > n)
			sel = n;
		r = tdf_list_menu(L("ÉCHÉANCES", "DEADLINES"), it, n + 1, &sel,
				L("Les dates dépendent de ta promo et de ton campus : vérifie-les sur l'intra. Inscriptions aux exams : le vendredi vers 18h42 (42 Paris).",
					"Dates depend on your cohort and campus: check them on the intra. Exam sign-ups: Friday around 6:42 pm (42 Paris)."));
		if (r < 0)
			return ;
		if (r == n && n < 12)
		{
			snprintf(e[n].nom, sizeof(e[n].nom), "%s", L("Nouvelle échéance", "New deadline"));
			snprintf(e[n].date, sizeof(e[n].date), "2027-01-27");
			n++;
			tdf_echeances_ecrire(e, n);
			ech_modifier(e, &n, n - 1);
		}
		else if (r < n)
			ech_modifier(e, &n, r);
	}
}

static void	couleurs_basculer(void)
{
	char	p[700];
	FILE	*f;

	tdf_daltonien_regler(!tdf_daltonien());
	snprintf(p, sizeof(p), "%s/couleurs.txt", g_savedir);
	f = fopen(p, "w");
	if (!f)
		return ;
	fprintf(f, "%d\n", tdf_daltonien());
	fclose(f);
}

static void	bulles_basculer(void)
{
	char	p[700];
	FILE	*f;

	g_bulles = !g_bulles;
	snprintf(p, sizeof(p), "%s/bulles.txt", g_savedir);
	f = fopen(p, "w");
	if (!f)
		return ;
	fprintf(f, "%d\n", g_bulles);
	fclose(f);
}

int	tdf_correction_accueil(void)
{
	char		ch[600];
	char		texte[1000];
	const char	*items[2];
	int			sel;

	if (!tdf_correction(ch, sizeof(ch)))
		return (0);
	snprintf(texte, sizeof(texte), L("Le mode correction est toujours actif%s%s.\n\nTu continues la correction, ou tu reviens au testeur normal ?",
			"Grading mode is still on%s%s.\n\nDo you go on grading, or go back to the normal tester?"), ch[0] ? " : " : "", ch);
	items[0] = L("Continuer la correction|", "Go on grading|");
	items[1] = L("Revenir au testeur normal|", "Back to the normal tester|");
	sel = 0;
	if (page(L("testeur des fous | mode : correction", "testeur des fous | mode: grading"), texte, items, 2, &sel) != 1)
		return (1);
	tdf_correction_ecrire(NULL);
	return (2);
}

static int	corr_existe(const char *d, const char *f)
{
	char	p[1300];

	snprintf(p, sizeof(p), "%s/%s", d, f);
	return (access(p, R_OK) == 0);
}

static int	corr_type(const char *d)
{
	if (corr_existe(d, "ft_printf.h") || corr_existe(d, "include/ft_printf.h")
		|| corr_existe(d, "includes/ft_printf.h") || corr_existe(d, "inc/ft_printf.h"))
		return (2);
	if (corr_existe(d, "get_next_line.h") || corr_existe(d, "get_next_line.c"))
		return (3);
	if (corr_existe(d, "libft.h"))
		return (1);
	return (0);
}

int	tdf_choisir_test(const char *dossier, char *out, size_t n)
{
	static const char	*lf[] = {"isalpha", "isdigit", "isalnum", "isascii", "isprint",
		"strlen", "memset", "bzero", "memcpy", "memmove", "strlcpy", "strlcat", "toupper",
		"tolower", "strchr", "strrchr", "strncmp", "memchr", "memcmp", "strnstr", "atoi",
		"calloc", "strdup", "substr", "strjoin", "strtrim", "split", "itoa", "strmapi",
		"striteri", "putchar_fd", "putstr_fd", "putendl_fd", "putnbr_fd", "lstnew",
		"lstadd_front", "lstsize", "lstlast", "lstadd_back", "lstdelone", "lstclear",
		"lstiter", "lstmap"};
	static const char	*pf[] = {"c", "s", "p", "d", "i", "u", "x", "X", "%", "mix",
		"ret", "limites", "write", "malloc", "stress"};
	static const char	*gn[] = {"1", "2", "7", "42", "1024", "9999", "10000000"};
	const char			*pd[15];
	char				titre_c[200];
	static int			sel;
	const char			**noms;
	const char			*titre;
	char				lab[40];
	int					nb;
	int					t;
	int					cw;
	int					nc;
	int					x;
	int					y;
	int					i;
	int					k;

	t = corr_type(dossier);
	if (t == 1)
	{
		noms = lf;
		nb = 43;
		titre = L("Libft : quelle fonction relancer ?", "Libft: which function to run again?");
	}
	else if (t == 2)
	{
		noms = pf;
		nb = 15;
		titre = L("ft_printf : quelle catégorie relancer ?", "ft_printf: which category to run again?");
	}
	else if (t == 3)
	{
		noms = gn;
		nb = 7;
		titre = L("get_next_line : quel BUFFER_SIZE relancer ?", "get_next_line: which BUFFER_SIZE to run again?");
	}
	else
	{
		out[0] = '\0';
		return (1);
	}
	pd[0] = L("%c : caractères, y compris 0 et les caractères non imprimables.", "%c: characters, including 0 and non printable characters.");
	pd[1] = L("%s : chaînes, NULL compris.", "%s: strings, NULL included.");
	pd[2] = L("%p : pointeurs, NULL compris (glibc « (nil) », macOS « 0x0 »).", "%p: pointers, NULL included (glibc '(nil)', macOS '0x0').");
	pd[3] = L("%d : entiers signés, INT_MIN et INT_MAX compris.", "%d: signed integers, INT_MIN and INT_MAX included.");
	pd[4] = L("%i : entiers signés, INT_MIN et INT_MAX compris.", "%i: signed integers, INT_MIN and INT_MAX included.");
	pd[5] = L("%u : entiers non signés, jusqu'à UINT_MAX.", "%u: unsigned integers, up to UINT_MAX.");
	pd[6] = L("%x : hexadécimal en minuscules.", "%x: lowercase hexadecimal.");
	pd[7] = L("%X : hexadécimal en MAJUSCULES.", "%X: UPPERCASE hexadecimal.");
	pd[8] = L("%% : le signe pourcent.", "%%: the percent sign.");
	pd[9] = L("mix : plusieurs conversions dans le même format, format vide ou très long.", "mix: several conversions in the same format, empty or very long format.");
	pd[10] = L("ret : la valeur de retour (avec \\0 dans les chaînes, gros volumes).", "ret: the return value (with \\0 in strings, big volumes).");
	pd[11] = L("limites : cas limites (NULL, pourcent seul à la fin, conversion inconnue).", "limites: edge cases (NULL, lone percent sign at the end, unknown conversion).");
	pd[12] = L("write : write qui échoue (fd fermé, /dev/full) : ft_printf doit renvoyer -1.", "write: write that fails (closed fd, /dev/full): ft_printf must return -1.");
	pd[13] = L("malloc : chaque malloc échoue à son tour, sans crash ni fuite.", "malloc: each malloc fails in turn, with no crash and no leak.");
	pd[14] = L("stress : 10 000 appels au hasard comparés au vrai printf.", "stress: 10,000 random calls compared with the real printf.");
	tdf_correction_titre(titre_c, sizeof(titre_c));
	if (sel >= nb)
		sel = 0;
	cls();
	while (1)
	{
		cw = 20;
		nc = (term_cols() - 8) / cw;
		if (nc > 5)
			nc = 5;
		if (nc < 1)
			nc = 1;
		x = (term_cols() - nc * cw) / 2;
		if (x < 1)
			x = 1;
		bgc(theme()->bg);
		frame();
		center(2, theme()->accent2, titre_c);
		center(4, theme()->text, titre);
		for (i = 0; i < nb; i++)
		{
			at(x + (i % nc) * cw, 6 + i / nc);
			if (i == sel)
			{
				bgc(mix(theme()->bg, theme()->accent, 0.22));
				fg(theme()->accent2);
			}
			else
				fg(theme()->text);
			if (t == 1)
				snprintf(lab, sizeof(lab), "ft_%s", noms[i]);
			else if (t == 2 && !strcmp(noms[i], "%"))
				snprintf(lab, sizeof(lab), "%%%%");
			else if (t == 2 && strlen(noms[i]) == 1)
				snprintf(lab, sizeof(lab), "%%%s", noms[i]);
			else
				snprintf(lab, sizeof(lab), "%s", noms[i]);
			outf(" %s %-*s", i == sel ? "▶" : " ", cw - 3, lab);
			bgc(theme()->bg);
		}
		y = 7 + (nb - 1) / nc + 1;
		fill(x, y, term_cols() - x - 1, 1, theme()->bg);
		if (t == 2)
			text_at(x + 1, y++, theme()->accent, pd[sel]);
		else if (t == 3)
			text_at(x + 1, y++, theme()->accent, L("Toutes les lectures avec ce BUFFER_SIZE, plus BUFFER_SIZE=0 et -1.", "Every read with this BUFFER_SIZE, plus BUFFER_SIZE=0 and -1."));
		else
			text_at(x + 1, y++, theme()->accent, L("Chaque cas testé s'affiche, avec sa comparaison.", "Every tested case is shown, with its comparison."));
		text_at(x, y + 1, theme()->dim, L("◀ ▲ ▼ ▶ choisir   Entrée : lancer ce test seul, avec tout le détail   Échap : retour",
				"◀ ▲ ▼ ▶ pick   Enter: run this test alone, with every detail   Esc: back"));
		flush();
		k = key_read(200);
		if (k < 0)
			continue ;
		if (k == 27 || k == 127 || k == 8)
			return (cls(), 0);
		if (k == 1000 && sel - nc >= 0)
			sel -= nc;
		else if (k == 1001 && sel + nc < nb)
			sel += nc;
		else if (k == 1002 && sel + 1 < nb)
			sel++;
		else if (k == 1003 && sel > 0)
			sel--;
		else if (k == '\r' || k == '\n' || k == ' ')
		{
			snprintf(out, n, "%s", noms[sel]);
			return (cls(), 1);
		}
	}
}

int	tdf_derniere_existe(void)
{
	char	path[700];

	snprintf(path, sizeof(path), "%s/derniere/module", g_savedir);
	return (access(path, R_OK) == 0);
}

void	tdf_derniere_ecrire(const char *module, const char *fichier)
{
	char	path[800];
	char	buf[4096];
	FILE	*in;
	FILE	*o;
	size_t	r;

	snprintf(path, sizeof(path), "%s/derniere", g_savedir);
	mkdir(path, 0755);
	if (fichier)
	{
		snprintf(path, sizeof(path), "%s/derniere/%s.res", g_savedir, module);
		snprintf(buf, sizeof(buf), "%s/derniere/%s.avant.res", g_savedir, module);
		rename(path, buf);
		in = fopen(fichier, "r");
		o = in ? fopen(path, "w") : NULL;
		while (in && o && (r = fread(buf, 1, sizeof(buf), in)) > 0)
			fwrite(buf, 1, r, o);
		if (in)
			fclose(in);
		if (o)
			fclose(o);
	}
	snprintf(path, sizeof(path), "%s/derniere/module", g_savedir);
	o = fopen(path, "w");
	if (o)
	{
		fprintf(o, "%s\n", module);
		fclose(o);
	}
}

static int	utf8_ok(void)
{
	const char	*v[3];
	int			i;

	v[0] = getenv("LC_ALL");
	v[1] = getenv("LC_CTYPE");
	v[2] = getenv("LANG");
	for (i = 0; i < 3; i++)
		if (v[i] && *v[i])
			return (strcasestr(v[i], "utf-8") || strcasestr(v[i], "utf8"));
	return (0);
}

void	tdf_conseil(int force)
{
	const char	*items[1];
	char		texte[1600];
	char		sig[32];
	char		vu[32];
	char		chemin[700];
	FILE		*f;
	int			n;
	int			petit;
	int			sel;

	petit = term_cols() < 120 || term_rows() < 38;
	if (!force && !petit && utf8_ok() && entete_24bits())
		return ;
	snprintf(sig, sizeof(sig), "%d %d %d", petit, utf8_ok(), entete_24bits());
	snprintf(chemin, sizeof(chemin), "%s/conseil_vu", g_savedir);
	vu[0] = '\0';
	f = fopen(chemin, "r");
	if (f && fgets(vu, sizeof(vu), f))
		vu[strcspn(vu, "\n")] = '\0';
	if (f)
		fclose(f);
	if (!force && !strcmp(vu, sig))
		return ;
	f = fopen(chemin, "w");
	if (f)
	{
		fprintf(f, "%s\n", sig);
		fclose(f);
	}
	n = 0;
	if (petit)
		n += snprintf(texte + n, sizeof(texte) - n, L("• Ton terminal fait %d × %d : mets-le en plein écran (F11 ou double-clic sur la barre de titre). Idéal : au moins 120 × 38.\n",
				"• Your terminal is %d × %d: make it full screen (F11 or double-click the title bar). Best: at least 120 × 38.\n"), term_cols(), term_rows());
	if (!utf8_ok())
		n += snprintf(texte + n, sizeof(texte) - n, "%s", L("• Ton terminal n'est pas en UTF-8 : certains dessins et accents peuvent mal s'afficher. Ajoute « export LANG=C.UTF-8 » à ton ~/.zshrc.\n",
				"• Your terminal is not in UTF-8: some drawings and accents may look wrong. Add 'export LANG=C.UTF-8' to your ~/.zshrc.\n"));
	if (!entete_24bits())
		n += snprintf(texte + n, sizeof(texte) - n, "%s", L("• Ton terminal n'annonce pas les couleurs 24 bits : les couleurs seront approchées. Un terminal récent (GNOME Terminal, Ptyxis, Windows Terminal, kitty) les affiche toutes.\n",
				"• Your terminal does not announce 24-bit colours: colours will be approximated. A recent terminal (GNOME Terminal, Ptyxis, Windows Terminal, kitty) shows them all.\n"));
	n += snprintf(texte + n, sizeof(texte) - n, "%s", L("• Le plus pratique : le testeur sur un côté de l'écran, et VS Code, vim ou ton éditeur en grand à côté. Le testeur s'adapte à la place qu'il a.\n"
			"• Si l'affichage se décale ou se coupe, agrandis la fenêtre puis relance le testeur.",
			"• Most handy: the tester on one side of the screen, and VS Code, vim or your editor large next to it. The tester adapts to the space it has.\n"
			"• If the display shifts or gets cut, enlarge the window and start the tester again."));
	items[0] = L("C'est fait, continuer|", "Done, continue|");
	sel = 0;
	page(L("CONSEILS D'AFFICHAGE", "DISPLAY TIPS"), texte, items, 1, &sel);
}

void	tdf_conseil_plein_ecran(void)
{
	tdf_conseil(0);
}

static int	rapport_lire(const char *path, char lignes[][400], int max)
{
	FILE	*f;
	char	l[1200];
	int		n;
	int		i;
	int		j;

	f = fopen(path, "r");
	n = 0;
	while (f && n < max && fgets(l, sizeof(l), f))
	{
		for (i = 0, j = 0; l[i] && j < 399; i++)
		{
			if (l[i] == 27)
			{
				while (l[i] && !((l[i] >= 'a' && l[i] <= 'z') || (l[i] >= 'A' && l[i] <= 'Z')) )
					i++;
				if (!l[i])
					break ;
				continue ;
			}
			if (l[i] == '\r' || l[i] == '\n')
				continue ;
			lignes[n][j++] = l[i] == '\t' ? ' ' : l[i];
		}
		lignes[n][j] = '\0';
		n++;
	}
	if (f)
		fclose(f);
	return (n);
}

static t_rgb	rapport_couleur(const char *l)
{
	if (strstr(l, " KO ") || strstr(l, "FAUX") || strstr(l, "ECHEC") || strstr(l, "FAIL") || strstr(l, "✘"))
		return (rgb(255, 130, 140));
	if (strstr(l, " OK ") || strstr(l, "✔") || strstr(l, "✓"))
		return (rgb(140, 230, 170));
	if (strstr(l, "WARN") || strstr(l, "!"))
		return (rgb(250, 215, 120));
	return (theme()->text);
}

void	tdf_rapport(const char *path, const char *titre)
{
	static char	lignes[4000][400];
	int			n;
	int			top;
	int			h;
	int			k;
	int			i;
	char		pos[80];

	n = path ? rapport_lire(path, lignes, 4000) : 0;
	top = 0;
	cls();
	while (1)
	{
		h = term_rows() - 6;
		if (h < 3)
			h = 3;
		if (top > n - h)
			top = n - h;
		if (top < 0)
			top = 0;
		bgc(theme()->bg);
		frame();
		center(1, theme()->accent2, titre);
		for (i = 0; i < h && top + i < n; i++)
			text_clip(2, 3 + i, rapport_couleur(lignes[top + i]), lignes[top + i], term_cols() - 4);
		if (!n)
			text_at(2, 3, theme()->dim, L("rapport introuvable", "report not found"));
		snprintf(pos, sizeof(pos), "%d-%d / %d", top + 1, top + h < n ? top + h : n, n);
		text_at(term_cols() - ulen(pos) - 3, 1, theme()->dim, pos);
		foot_line(2, L("▲ ▼ défiler   ◀ ▶ page   Échap ou Entrée : fermer",
				"▲ ▼ scroll   ◀ ▶ page   Esc or Enter: close"));
		flush();
		k = key_dir(key_read(200));
		if (k == 1000)
			top -= tdf_touche_fleche(k);
		else if (k == 1001)
			top += tdf_touche_fleche(k);
		else if (k == 1003)
			top -= h;
		else if (k == 1002 || k == ' ')
			top += h;
		else if (k == '\r' || k == '\n' || key_quit(k))
			return ((void)cls());
	}
}

void	tdf_menu_params(void (*credits)(void))
{
	static int	sel;
	const char	*items[14];
	int			act[14];
	char		bl[200];
	char		cl[300];
	int			n;
	int			i;

	tdf_bulles_lire();
	while (1)
	{
		n = 0;
		act[n] = 0;
		items[n++] = L("Réglages|Langue, animations, histoire, niveau d'aide, outils de test.", "Options|Language, animations, story, help level, test tools.");
		if (tdf_menu_unique())
		{
			if (tdf_installee() == 0)
			{
				act[n] = 5;
				items[n++] = L("Installer la commande tdf|Ensuite tu tapes seulement « tdf » dans n'importe quel dossier (sans sudo).", "Install the tdf command|Then you just type 'tdf' in any folder (no sudo).");
			}
			else if (tdf_installee() == 2)
			{
				act[n] = 5;
				items[n++] = tdf_nouveau("3.5") ? L("!Réinstaller la commande tdf|Elle pointe vers un autre dossier (ancien clone supprimé ou déplacé) : elle lancera ce testeur-ci.", "!Reinstall the tdf command|It points to another folder (old clone deleted or moved): it will run this tester.")
					: L("Réinstaller la commande tdf|Elle pointe vers un autre dossier (ancien clone supprimé ou déplacé) : elle lancera ce testeur-ci.", "Reinstall the tdf command|It points to another folder (old clone deleted or moved): it will run this tester.");
			}
			if (getenv("TDF_NORME_ABSENTE"))
			{
				act[n] = 10;
				items[n++] = tdf_nouveau("3.4") ? L("!Installer la norminette|Introuvable sur ce poste : le testeur l'installe pour toi, sans sudo.", "!Install the norminette|Not found on this computer: the tester installs it for you, without sudo.")
					: L("Installer la norminette|Introuvable sur ce poste : le testeur l'installe pour toi, sans sudo.", "Install the norminette|Not found on this computer: the tester installs it for you, without sudo.");
			}
			act[n] = 6;
			items[n++] = L("Revoir le tuto|Les 6 écrans qui présentent le testeur.", "Replay the tutorial|The 6 screens that present the tester.");
			act[n] = 7;
			items[n++] = L("Changelog|Ce qui a changé dans cette version, et le Discord pour en savoir plus.", "Changelog|What changed in this version, and the Discord to learn more.");
		}
		snprintf(bl, sizeof(bl), "%s%s : %s|%s", tdf_nouveau("3.3") ? "!" : "", L("Bulles « Le savais-tu ? »", "'Did you know?' bubbles"),
			g_bulles ? L("oui", "yes") : L("non", "no"), L("Entrée pour les montrer ou les cacher dans le menu.", "Enter to show or hide them in the menu."));
		act[n] = 8;
		items[n++] = bl;
		snprintf(cl, sizeof(cl), "%s%s : %s|%s", tdf_nouveau("3.4") ? "!" : "", L("Couleurs adaptées", "Adapted colours"),
			tdf_daltonien() ? L("oui", "yes") : L("non", "no"),
			L("Bleu pour OK et orange pour FAUX au lieu de vert et rouge (daltonisme). Entrée pour changer.",
				"Blue for OK and orange for WRONG instead of green and red (colour blindness). Enter to change."));
		act[n] = 12;
		items[n++] = cl;
		if (tdf_derniere_existe())
		{
			act[n] = 11;
			items[n++] = tdf_nouveau("3.4") ? L("!Résumé à partager|Ta dernière tentative en quelques lignes, sans ton code ni tes chemins, à coller sur Discord pour demander de l'aide.",
					"!Summary to share|Your last attempt in a few lines, without your code or paths, to paste on Discord to ask for help.")
				: L("Résumé à partager|Ta dernière tentative en quelques lignes, sans ton code ni tes chemins, à coller sur Discord pour demander de l'aide.",
					"Summary to share|Your last attempt in a few lines, without your code or paths, to paste on Discord to ask for help.");
		}
		act[n] = 9;
		items[n++] = tdf_nouveau("3.4") ? L("!Échéances|Les dates de tes milestones, affichées dans Stats (J-22…).", "!Deadlines|The dates of your milestones, shown in Stats (D-22…).")
			: L("Échéances|Les dates de tes milestones, affichées dans Stats (J-22…).", "Deadlines|The dates of your milestones, shown in Stats (D-22…).");
		act[n] = 1;
		items[n++] = L("Sauvegarde et connexion|Exporter ta progression ou la retrouver sur un autre poste.", "Save and login|Export your progress or get it back on another computer.");
		act[n] = 2;
		items[n++] = L("Nous aider|Crée une trace (ni code, ni login) à envoyer à splatch_afk.", "Help us|Create a trace (no code, no login) to send to splatch_afk.");
		act[n] = 3;
		items[n++] = L("Crédits", "Credits");
		if (sel >= n)
			sel = 0;
		i = tdf_list_menu(L("PARAMÈTRES", "SETTINGS"), items, n, &sel, NULL);
		if (i < 0)
			return ;
		if (act[i] == 0)
			screen_settings();
		else if (act[i] == 1)
			screen_save();
		else if (act[i] == 2)
			screen_help_us();
		else if (act[i] == 3)
			credits();
		else if (act[i] == 4)
			interface_basculer();
		else if (act[i] == 5)
		{
			tdf_sortie("installer", NULL);
			return ;
		}
		else if (act[i] == 6)
			tdf_tuto();
		else if (act[i] == 7)
			tdf_nouveautes();
		else if (act[i] == 8)
			bulles_basculer();
		else if (act[i] == 9)
			ecran_echeances();
		else if (act[i] == 10)
		{
			tdf_sortie("norminette", NULL);
			return ;
		}
		else if (act[i] == 11)
		{
			tdf_sortie("resume", NULL);
			return ;
		}
		else if (act[i] == 12)
			couleurs_basculer();
	}
}

typedef struct s_bx
{
	char	src[8];
	char	nom[48];
	int		niv;
	int		fait;
	char	notion[160];
}	t_bx;

static t_bx	g_bx[300];
static int	g_nbx;

static void	shq(char *d, size_t n, const char *s)
{
	size_t	j;

	j = 0;
	if (j < n - 1)
		d[j++] = '\'';
	for (; *s && j + 5 < n; s++)
	{
		if (*s == '\'')
		{
			memcpy(d + j, "'\\''", 4);
			j += 4;
		}
		else
			d[j++] = *s;
	}
	d[j++] = '\'';
	d[j] = '\0';
}

static void	exam_cmd(char *cmd, size_t n, const char *args)
{
	char	r[800];

	shq(r, sizeof(r), getenv("TDF_RACINE") ? getenv("TDF_RACINE") : ".");
	snprintf(cmd, n, "TDF_LANGUE_N=%d bash %s/modules/exam/run.sh %s 2>&1", g_save.cfg.lang, r, args);
}

static void	banque_charger(void)
{
	char	cmd[1400];
	char	l[600];
	char	*f[6];
	char	*p;
	FILE	*o;
	int		i;

	exam_cmd(cmd, sizeof(cmd), "--banque-liste");
	o = popen(cmd, "r");
	g_nbx = 0;
	while (o && fgets(l, sizeof(l), o) && g_nbx < 300)
	{
		l[strcspn(l, "\n")] = '\0';
		if (strncmp(l, "B\t", 2))
			continue ;
		p = l;
		for (i = 0; i < 6 && p; i++)
		{
			f[i] = p;
			p = strchr(p, '\t');
			if (p)
				*p++ = '\0';
		}
		if (i < 6)
			continue ;
		snprintf(g_bx[g_nbx].src, sizeof(g_bx[0].src), "%s", f[1]);
		snprintf(g_bx[g_nbx].nom, sizeof(g_bx[0].nom), "%s", f[2]);
		g_bx[g_nbx].niv = atoi(f[3]);
		g_bx[g_nbx].fait = atoi(f[4]);
		snprintf(g_bx[g_nbx].notion, sizeof(g_bx[0].notion), "%s", f[5]);
		g_nbx++;
	}
	if (o)
		pclose(o);
}

static const char	*bsrc(const char *s)
{
	if (!strcmp(s, "exam"))
		return ("Exam");
	if (!strcmp(s, "libft"))
		return ("Libft");
	return ("printf");
}

static void	coupe_car(char *d, size_t n, const char *s, int max)
{
	size_t	i;
	int		c;

	for (i = 0, c = 0; s[i] && i < n - 1; i++)
	{
		if (((unsigned char)s[i] & 0xC0) != 0x80 && c++ == max)
			break ;
		d[i] = s[i];
	}
	d[i] = '\0';
}

static int	sujet_lignes(const t_bx *b, int lang, char l[][200], int max)
{
	char	p[1200];
	char	t[1200];
	FILE	*f;
	int		n;

	snprintf(p, sizeof(p), "%s/banque/%s/%s/sujet.%s.txt", getenv("TDF_RACINE") ? getenv("TDF_RACINE") : ".", b->src, b->nom, langue_code(lang));
	if (access(p, R_OK) != 0)
		snprintf(p, sizeof(p), "%s/banque/%s/%s/sujet.%s.txt", getenv("TDF_RACINE") ? getenv("TDF_RACINE") : ".", b->src, b->nom, lang == 0 ? "fr" : "en");
	f = fopen(p, "r");
	n = 0;
	while (f && n < max && fgets(t, sizeof(t), f))
	{
		t[strcspn(t, "\n")] = '\0';
		coupe_car(l[n], 200, t, 150);
		n++;
	}
	if (f)
		fclose(f);
	return (n);
}

static void	badge_at(int x, int y, t_rgb c, const char *s, int actif)
{
	at(x, y);
	bgc(actif ? c : mix(theme()->bg, c, 0.35));
	fg(actif ? rgb(24, 26, 38) : theme()->text);
	outf(" %s ", s);
	bgc(theme()->bg);
}

static void	banque_exo(t_bx *b)
{
	static char	l[400][200];
	char		cmd[1500];
	char		a[300];
	char		sa[200];
	char		sb[100];
	char		res[700];
	char		tmp[300];
	FILE		*o;
	FILE		*w;
	int			lang;
	int			n;
	int			top;
	int			bt;
	int			k;
	int			i;
	int			h;
	int			x;
	int			y;

	shq(sa, sizeof(sa), b->src);
	shq(sb, sizeof(sb), b->nom);
	snprintf(a, sizeof(a), "--banque-ouvrir %s %s", sa, sb);
	exam_cmd(cmd, sizeof(cmd), a);
	o = popen(cmd, "r");
	while (o && fgets(tmp, sizeof(tmp), o))
		;
	if (o)
		pclose(o);
	lang = g_save.cfg.lang;
	n = sujet_lignes(b, lang, l, 400);
	top = 0;
	bt = 0;
	cls();
	while (1)
	{
		h = term_rows() - 10;
		if (h < 3)
			h = 3;
		if (top > n - h)
			top = n - h;
		if (top < 0)
			top = 0;
		x = (term_cols() - 80) / 2;
		if (x < 2)
			x = 2;
		bgc(theme()->bg);
		frame();
		snprintf(tmp, sizeof(tmp), "%s · %s%s", b->nom, bsrc(b->src), b->fait ? "  ✓" : "");
		center(1, b->fait ? rgb(140, 230, 170) : theme()->accent2, tmp);
		snprintf(tmp, sizeof(tmp), b->fait ? L("Déjà réussi : les %d ◈ sont dans ta poche.", "Already passed: the %d ◈ are in your pocket.")
			: L("Première réussite : +%d ◈", "First success: +%d ◈"), 10 + 5 * b->niv);
		center(2, b->fait ? theme()->dim : rgb(255, 215, 0), tmp);
		snprintf(tmp, sizeof(tmp), L("Ton rendu : banque_travail/%s/%s/   (écris ton code dans un autre terminal)",
				"Your work: banque_travail/%s/%s/   (write your code in another terminal)"), b->src, b->nom);
		text_clip(x, 3, theme()->dim, tmp, term_cols() - x - 2);
		for (i = 0; i < h && top + i < n; i++)
			text_clip(x, 5 + i, l[top + i][0] == '$' ? theme()->accent : theme()->text, l[top + i], term_cols() - x - 2);
		y = term_rows() - 4;
		badge_at(x, y, rgb(140, 230, 170), L("Corriger mon code", "Grade my code"), bt == 0);
		snprintf(tmp, sizeof(tmp), "%s ◀ %s ▶", L("Langue du sujet", "Subject language"), langue_nom(lang));
		badge_at(x + 24, y, rgb(150, 195, 255), tmp, bt == 1);
		badge_at(x + 26 + ulen(tmp) + 2, y, rgb(255, 165, 175), L("Retour", "Back"), bt == 2);
		foot_line(x, L("◀ ▶ choisir un bouton   ▲ ▼ faire défiler le sujet   Entrée valider   Échap retour",
				"◀ ▶ pick a button   ▲ ▼ scroll the subject   Enter confirm   Esc back"));
		flush();
		k = key_dir(key_read(200));
		if (key_quit(k))
			return ((void)cls());
		if (k == 1000)
			top -= tdf_touche_fleche(k);
		else if (k == 1001)
			top += tdf_touche_fleche(k);
		else if (k == 1002)
			bt = (bt + 1) % 3;
		else if (k == 1003)
			bt = (bt + 2) % 3;
		else if ((k == '\r' || k == '\n' || k == ' ') && bt == 2)
			return ((void)cls());
		else if ((k == '\r' || k == '\n' || k == ' ') && bt == 1)
		{
			lang = (lang + 1) % NLANG;
			n = sujet_lignes(b, lang, l, 400);
			top = 0;
		}
		else if (k == '\r' || k == '\n' || k == ' ')
		{
			bgc(theme()->bg);
			frame();
			center(term_rows() / 2, theme()->accent2, L("Correction en cours...", "Grading..."));
			flush();
			snprintf(a, sizeof(a), "--banque-corriger %s %s", sa, sb);
			exam_cmd(cmd, sizeof(cmd), a);
			snprintf(res, sizeof(res), "%s/derniere", g_savedir);
			mkdir(res, 0755);
			snprintf(res, sizeof(res), "%s/derniere/banque.txt", g_savedir);
			o = popen(cmd, "r");
			w = fopen(res, "w");
			while (o && fgets(tmp, sizeof(tmp), o))
				if (w)
					fputs(tmp, w);
			b->fait = (o ? pclose(o) : 1) == 0;
			if (w)
				fclose(w);
			tdf_derniere_ecrire("banque", NULL);
			snprintf(tmp, sizeof(tmp), "%s · %s", b->nom, b->fait ? "SUCCESS" : "FAILURE");
			tdf_rapport(res, tmp);
		}
	}
}

void	tdf_banque(void)
{
	static const char	*src[3] = {"exam", "libft", "printf"};
	static int			onglet;
	static int			sel;
	int					vue[300];
	int					nv;
	int					top;
	int					ok;
	int					k;
	int					i;
	int					h;
	int					x;
	char				tmp[300];
	char				nt[200];

	bgc(theme()->bg);
	frame();
	center(term_rows() / 2, theme()->accent2, L("Chargement de la banque...", "Loading the bank..."));
	flush();
	banque_charger();
	top = 0;
	cls();
	while (1)
	{
		for (nv = 0, ok = 0, i = 0; i < g_nbx; i++)
		{
			ok += g_bx[i].fait;
			if (onglet == 0 || !strcmp(g_bx[i].src, src[onglet - 1]))
				vue[nv++] = i;
		}
		if (sel >= nv)
			sel = nv ? nv - 1 : 0;
		h = term_rows() - 9;
		if (h < 3)
			h = 3;
		if (sel < top)
			top = sel;
		if (sel >= top + h)
			top = sel - h + 1;
		x = (term_cols() - 96) / 2;
		if (x < 2)
			x = 2;
		bgc(theme()->bg);
		frame();
		center(1, theme()->accent2, L("BANQUE D'EXERCICES", "EXERCISE BANK"));
		for (ok = 0, i = 0; i < nv; i++)
			ok += g_bx[vue[i]].fait;
		snprintf(tmp, sizeof(tmp), L("%s · %d exercices · %d réussis", "%s · %d exercises · %d passed"),
			onglet ? bsrc(src[onglet - 1]) : L("Tout", "All"), nv, ok);
		center(2, ok == nv && nv ? rgb(140, 230, 170) : theme()->accent, tmp);
		at(x, 4);
		for (i = 0; i < 4; i++)
		{
			badge_at(x + i * 14, 4, rgb(150, 195, 255), i ? bsrc(src[i - 1]) : L("Tout", "All"), i == onglet);
		}
		for (i = top; i < nv && i < top + h; i++)
		{
			t_bx	*b;

			b = &g_bx[vue[i]];
			at(x, 6 + i - top);
			if (i == sel)
				bgc(mix(theme()->bg, theme()->accent, 0.22));
			fg(i == sel ? theme()->accent2 : theme()->text);
			outf(" %s ", i == sel ? "▶" : " ");
			fg(b->fait ? rgb(140, 230, 170) : theme()->dim);
			out(b->fait ? "✓" : "·");
			fg(i == sel ? theme()->accent2 : theme()->text);
			outf(" %-22s ", b->nom);
			fg(theme()->dim);
			outf("%-7s %s %-2d ", bsrc(b->src), L("niv.", "lvl"), b->niv);
			fg(b->fait ? theme()->dim : rgb(255, 215, 0));
			if (b->fait)
				outf("%-8s ", L("obtenu", "earned"));
			else
				outf("+%-3d ◈   ", 10 + 5 * b->niv);
			fg(theme()->dim);
			coupe_car(nt, sizeof(nt), b->notion, 38);
			outf("%s", nt);
			bgc(theme()->bg);
			outf("%*s", 38 - ulen(nt) > 0 ? 38 - ulen(nt) : 0, "");
		}
		if (nv > h)
		{
			snprintf(tmp, sizeof(tmp), "%d-%d / %d", top + 1, top + h < nv ? top + h : nv, nv);
			text_at(x, 7 + h, theme()->dim, tmp);
		}
		foot_line(x, L("▲ ▼ choisir   ◀ ▶ Tout, Exam, Libft, printf   Entrée ouvrir   Échap retour",
				"▲ ▼ pick   ◀ ▶ All, Exam, Libft, printf   Enter open   Esc back"));
		flush();
		k = key_dir(key_read(200));
		if (key_quit(k))
			return ((void)cls());
		if (k == 1000 && nv)
			sel = ((sel - tdf_touche_fleche(k)) % nv + nv) % nv;
		else if (k == 1001 && nv)
			sel = (sel + tdf_touche_fleche(k)) % nv;
		else if (k == 1002 || k == 1003)
		{
			onglet = (onglet + (k == 1002 ? 1 : 3)) % 4;
			sel = 0;
			top = 0;
		}
		else if ((k == '\r' || k == '\n' || k == ' ') && nv)
			banque_exo(&g_bx[vue[sel]]);
	}
}
