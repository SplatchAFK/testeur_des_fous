#include <dirent.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
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

static int	page(const char *titre, const char *texte, const char **items, int n, int *sel)
{
	char	lab[160];
	int		k;
	int		i;
	int		x;
	int		y;
	int		w;

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
		k = key_dir(key_read(40));
		if (key_quit(k))
			return (cls(), -1);
		if (k == 1000)
			*sel = (*sel + n - 1) % n;
		else if (k == 1001)
			*sel = (*sel + 1) % n;
		else if (k == '\r' || k == '\n' || k == ' ')
			return (cls(), *sel);
	}
}

void	tdf_tuto(void)
{
	const char	*titres[6];
	const char	*textes[6];
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
	titres[5] = L("Paramètres", "Settings");
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
	textes[5] = L("Langue, interface graphique ou texte, et la commande tdf pour lancer le testeur depuis n'importe quel dossier. Ce tuto et les nouveautés se relancent depuis Paramètres.",
			"Language, graphical or text interface, and the tdf command to start the tester from any folder. This tutorial and the news can be replayed from Settings.");
	i = 0;
	while (i < 6)
	{
		n = 0;
		items[n++] = i < 5 ? L("Suivant|", "Next|") : L("C'est parti|", "Let's go|");
		if (i > 0)
			items[n++] = L("Précédent|", "Previous|");
		if (i < 5)
			items[n++] = L("Passer le tuto|", "Skip the tutorial|");
		snprintf(tt, sizeof(tt), "%s · %d/6 · %s", L("TUTO", "TUTORIAL"), i + 1, titres[i]);
		sel = 0;
		r = page(tt, textes[i], items, n, &sel);
		if (r < 0 || (i < 5 && r == n - 1))
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
	char		texte[1200];
	const char	*items[2];
	int			sel;

	snprintf(titre, sizeof(titre), L("NOUVEAUTÉS DE LA VERSION %s", "WHAT'S NEW IN VERSION %s"), TDF_VERSION);
	snprintf(texte, sizeof(texte), "%s\n%s\n%s\n%s\n%s\n%s\n%s",
		L("• Un seul menu graphique : bash run.sh ou tdf l'ouvre directement, et tout s'y passe.", "• One single graphical menu: bash run.sh or tdf opens it directly, and everything happens there."),
		L("• Tester mon projet : « . » pour le dossier où tu es, ou parcours ton PC avec Tab, comme dans zsh.", "• Test my project: '.' for the folder you are in, or browse your PC with Tab, like in zsh."),
		L("• Après un test, tous les résultats s'affichent directement ; « Revoir la dernière tentative » les rouvre depuis le menu.", "• After a test, every result shows up directly; 'Review the last attempt' reopens them from the menu."),
		L("• Banque d'exercices graphique : onglets, sujet, correction en un clic.", "• Graphical exercise bank: tabs, subject, grading in one click."),
		L("• Sujets de l'exam et de la banque en 8 langues.", "• Exam and bank subjects in 8 languages."),
		L("• Lore : toutes les cinématiques, Libft, ft_printf, get_next_line et l'exam.", "• Lore: every cutscene, Libft, ft_printf, get_next_line and the exam."),
		L("• Examshell des fous plus clair : commande mal tapée ? Le testeur propose la bonne.", "• Clearer Examshell des fous: mistyped a command? The tester suggests the right one."));
	items[0] = L("Continuer|", "Continue|");
	items[1] = L("Voir le tuto|", "See the tutorial|");
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
	char	path[700];

	snprintf(path, sizeof(path), "%s/.local/bin/tdf", getenv("HOME") ? getenv("HOME") : "/tmp");
	return (access(path, X_OK) == 0);
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

static int	raccourcis(t_cand *c, int max)
{
	const char	*e;
	const char	*fin;
	char		ligne[700];
	char		*tab;
	int			n;

	n = 0;
	snprintf(c[n].chemin, sizeof(c[n].chemin), "%s", getenv("TDF_DEPART") ? getenv("TDF_DEPART") : ".");
	court(c[n].chemin, ligne, sizeof(ligne));
	snprintf(c[n].nom, sizeof(c[n].nom), ".  %s", ligne);
	snprintf(c[n].tag, sizeof(c[n].tag), "%s", L("dossier où tu es", "folder you are in"));
	n++;
	e = getenv("TDF_PROJETS");
	while (e && *e && n < max)
	{
		fin = strchr(e, '\n');
		snprintf(ligne, sizeof(ligne), "%.*s", (int)(fin ? fin - e : (long)strlen(e)), e);
		e = fin ? fin + 1 : e + strlen(e);
		tab = strchr(ligne, '\t');
		if (!tab)
			continue ;
		*tab = '\0';
		snprintf(c[n].chemin, sizeof(c[n].chemin), "%s", tab + 1);
		court(tab + 1, c[n].nom, sizeof(c[n].nom));
		snprintf(c[n].tag, sizeof(c[n].tag), "%s", !strcmp(ligne, "printf") ? "ft_printf"
			: !strcmp(ligne, "gnl") ? "get_next_line" : "libft");
		n++;
	}
	return (n);
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

	if (!saisie[0])
		return (raccourcis(c, max));
	couper(saisie, dir, sizeof(dir), &pre);
	d = opendir(dir);
	n = 0;
	while (d && (e = readdir(d)) && n < max)
	{
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
			continue ;
		if (e->d_name[0] == '.' && pre[0] != '.')
			continue ;
		if (strncmp(e->d_name, pre, strlen(pre)))
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
		for (l = 0; p[l] && p[l] == c[i].nom[l]; l++)
			;
		p[l] = '\0';
	}
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

int	tdf_choisir_dossier(char *out, size_t n)
{
	static t_cand	c[400];
	char			saisie[1024];
	char			tmp[1100];
	int				nc;
	int				sel;
	int				top;
	int				k;
	int				i;
	int				x;
	int				h;
	const char		*msg;

	saisie[0] = '\0';
	sel = 0;
	top = 0;
	msg = NULL;
	cls();
	while (1)
	{
		nc = lister(saisie, c, 400);
		if (sel >= nc)
			sel = nc - 1;
		h = term_rows() - 13;
		if (h < 3)
			h = 3;
		if (sel >= 0 && sel < top)
			top = sel;
		if (sel >= top + h)
			top = sel - h + 1;
		x = (term_cols() - 76) / 2;
		if (x < 2)
			x = 2;
		bgc(theme()->bg);
		frame();
		center(2, theme()->accent2, L("TESTER MON PROJET", "TEST MY PROJECT"));
		text_at(x, 4, theme()->dim, L("Dossier du projet (. = là où tu es, ~ ou / pour parcourir, Tab complète) :",
				"Project folder (. = where you are, ~ or / to browse, Tab completes):"));
		at(x, 5);
		bgc(mix(theme()->bg, theme()->accent, 0.15));
		fg(theme()->text);
		outf(" %s▏ ", saisie);
		bgc(theme()->bg);
		outf("%*s", term_cols() - x - ulen(saisie) - 6 > 0 ? term_cols() - x - ulen(saisie) - 6 : 0, "");
		fill(x, 7, term_cols() - x - 2, h + 2, theme()->bg);
		if (!saisie[0])
			text_at(x, 7, theme()->dim, L("Projets trouvés :", "Projects found:"));
		for (i = top; i < nc && i < top + h; i++)
		{
			at(x, 8 + i - top);
			if (i == sel)
			{
				bgc(mix(theme()->bg, theme()->accent, 0.25));
				fg(theme()->accent2);
			}
			else
				fg(c[i].tag[0] ? theme()->accent : theme()->text);
			outf(" %s %s%s ", i == sel ? "▶" : " ", c[i].nom, c[i].chemin[0] ? "" : "/");
			bgc(theme()->bg);
			if (c[i].tag[0])
				text_at(x + ulen(c[i].nom) + 7, 8 + i - top, theme()->accent2, c[i].tag);
		}
		if (!nc)
			text_at(x, 8, theme()->dim, L("aucun dossier ne correspond", "no folder matches"));
		if (msg)
			text_at(x, term_rows() - 3, rgb(255, 120, 120), msg);
		foot_line(x, L("Tab compléter   ▲ ▼ choisir   ▶ ouvrir   ◀ remonter   Entrée valider   Échap retour",
				"Tab complete   ▲ ▼ pick   ▶ open   ◀ up   Enter confirm   Esc back"));
		flush();
		k = key_read(200);
		if (k < 0)
			continue ;
		if (k != 9 && k != 1000 && k != 1001)
			msg = NULL;
		if (k == 27 || k == 3)
			return (cls(), 0);
		if (k == 9 && nc && (sel >= 0 || !saisie[0]))
			sel = (sel + 1) % nc;
		else if (k == 9 && nc)
		{
			snprintf(tmp, sizeof(tmp), "%s", saisie);
			prefixe_commun(saisie, sizeof(saisie), c, nc);
			if (nc > 1 && !strcmp(tmp, saisie))
				sel = 0;
		}
		else if (k == 1000 && nc)
			sel = sel <= 0 ? nc - 1 : sel - 1;
		else if (k == 1001 && nc)
			sel = (sel + 1) % nc;
		else if ((k == 1002 || k == '\r' || k == '\n') && sel >= 0 && c[sel].chemin[0])
		{
			if (valider(c[sel].chemin, out, n))
				return (cls(), 1);
			msg = L("ce dossier n'existe pas", "this folder does not exist");
		}
		else if (k == 1002 && sel >= 0)
		{
			completer(saisie, sizeof(saisie), c[sel].nom, 1);
			sel = -1;
			top = 0;
		}
		else if (k == 1003 && saisie[0])
		{
			i = (int)strlen(saisie);
			if (i > 1 && saisie[i - 1] == '/')
				saisie[--i] = '\0';
			while (i > 0 && saisie[i - 1] != '/')
				saisie[--i] = '\0';
			sel = saisie[0] ? -1 : 0;
			top = 0;
		}
		else if (k == 127 || k == 8)
		{
			if (saisie[0])
				saisie[strlen(saisie) - 1] = '\0';
			sel = saisie[0] ? -1 : 0;
		}
		else if (k == '\r' || k == '\n')
		{
			if (sel >= 0 && saisie[0])
			{
				completer(saisie, sizeof(saisie), c[sel].nom, 1);
				sel = -1;
				top = 0;
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
			sel = -1;
			top = 0;
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

void	tdf_conseil_plein_ecran(void)
{
	const char	*items[1];
	char		texte[600];
	int			sel;

	if (term_cols() >= 120 && term_rows() >= 38)
		return ;
	snprintf(texte, sizeof(texte), L("Ton terminal fait %d × %d. Mets-le en plein écran (F11 ou double-clic sur la barre de titre) avant de continuer : le menu, les tests et les cinématiques s'affichent mieux, sans texte coupé. Idéal : au moins 120 × 38.",
			"Your terminal is %d × %d. Make it full screen (F11 or double-click the title bar) before you go on: the menu, the tests and the cutscenes display better, with no cut text. Best: at least 120 × 38."), term_cols(), term_rows());
	items[0] = L("C'est fait, continuer|", "Done, continue|");
	sel = 0;
	page(L("CONSEIL", "TIP"), texte, items, 1, &sel);
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
	const char	*items[9];
	int			act[9];
	int			n;
	int			i;

	while (1)
	{
		n = 0;
		act[n] = 0;
		items[n++] = L("Réglages|Langue, animations, histoire, niveau d'aide, outils de test.", "Options|Language, animations, story, help level, test tools.");
		if (tdf_menu_unique())
		{
			act[n] = 4;
			items[n++] = interface_texte() ? L("Interface : mode texte|Les tests seuls, sans histoire. Entrée pour repasser à l'interface graphique.", "Interface: text mode|Tests only, no story. Enter to switch back to the graphical interface.")
				: L("Interface : graphique|Le lore, les cinématiques et les combats. Entrée pour passer au mode texte.", "Interface: graphical|The lore, the cutscenes and the fights. Enter to switch to text mode.");
			if (!tdf_installee())
			{
				act[n] = 5;
				items[n++] = L("Installer la commande tdf|Ensuite tu tapes seulement « tdf » dans n'importe quel dossier (sans sudo).", "Install the tdf command|Then you just type 'tdf' in any folder (no sudo).");
			}
			act[n] = 6;
			items[n++] = L("Revoir le tuto|Les 6 écrans qui présentent le testeur.", "Replay the tutorial|The 6 screens that present the tester.");
			act[n] = 7;
			items[n++] = L("Nouveautés de cette version|Ce qui a changé depuis la dernière mise à jour.", "What's new in this version|What changed since the last update.");
		}
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
		snprintf(tmp, sizeof(tmp), L("%d/%d réussis", "%d/%d passed"), ok, g_nbx);
		center(2, theme()->dim, tmp);
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
			coupe_car(nt, sizeof(nt), b->notion, 46);
			outf("%s", nt);
			bgc(theme()->bg);
			outf("%*s", 46 - ulen(nt) > 0 ? 46 - ulen(nt) : 0, "");
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
