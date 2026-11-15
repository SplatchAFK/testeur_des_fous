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
	long	t;

	cls();
	while (1)
	{
		t = now_ms();
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
				bgc(mix(theme()->bg, theme()->accent, 0.18 + 0.08 * sin(t / 150.0)));
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
	snprintf(texte, sizeof(texte), "%s\n%s\n%s\n%s\n%s\n%s",
		L("• Un seul menu graphique : bash run.sh ou tdf l'ouvre directement, et tout s'y passe.", "• One single graphical menu: bash run.sh or tdf opens it directly, and everything happens there."),
		L("• Tester un autre dossier : parcours ton PC avec Tab, comme dans zsh.", "• Test another folder: browse your PC with Tab, like in zsh."),
		L("• Banque d'exercices aux flèches : onglets, sujet, correction en un clic.", "• Exercise bank with the arrows: tabs, subject, grading in one click."),
		L("• Sujets de l'exam et de la banque en 8 langues.", "• Exam and bank subjects in 8 languages."),
		L("• Examshell des fous plus clair : commande mal tapée ? Le testeur propose la bonne.", "• Clearer Examshell des fous: mistyped a command? The tester suggests the right one."),
		L("• Un tuto, à revoir depuis Paramètres.", "• A tutorial, to replay from Settings."));
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
	char	tag[16];
}	t_cand;

static int	cand_cmp(const void *a, const void *b)
{
	return (strcmp(((const t_cand *)a)->nom, ((const t_cand *)b)->nom));
}

static void	developper(const char *saisie, char *out, size_t n)
{
	if (saisie[0] == '~' && getenv("HOME"))
		snprintf(out, n, "%s%s", getenv("HOME"), saisie + 1);
	else
		snprintf(out, n, "%s", saisie);
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
	static const char	*f[][2] = {{"ft_printf.h", "ft_printf"}, {"get_next_line.h", "gnl"},
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
		if (strncmp(e->d_name, pre, strlen(pre)))
			continue ;
		snprintf(p, sizeof(p), "%s/%s", dir, e->d_name);
		if (stat(p, &st) != 0 || !S_ISDIR(st.st_mode))
			continue ;
		snprintf(c[n].nom, sizeof(c[n].nom), "%s", e->d_name);
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

int	tdf_choisir_dossier(char *out, size_t n)
{
	static t_cand	c[400];
	char			saisie[1024];
	char			vrai[PATH_MAX];
	char			tmp[1100];
	struct stat		st;
	int				nc;
	int				sel;
	int				top;
	int				k;
	int				i;
	int				x;
	int				h;
	const char		*msg;

	if (getenv("TDF_DEPART") && getenv("HOME") && !strncmp(getenv("TDF_DEPART"), getenv("HOME"), strlen(getenv("HOME"))))
		snprintf(saisie, sizeof(saisie), "~%s/", getenv("TDF_DEPART") + strlen(getenv("HOME")));
	else
		snprintf(saisie, sizeof(saisie), "%s/", getenv("TDF_DEPART") ? getenv("TDF_DEPART") : "~");
	if (!strcmp(saisie, "~//"))
		snprintf(saisie, sizeof(saisie), "~/");
	sel = -1;
	top = 0;
	msg = NULL;
	cls();
	while (1)
	{
		nc = lister(saisie, c, 400);
		if (sel >= nc)
			sel = nc - 1;
		h = term_rows() - 12;
		if (h < 3)
			h = 3;
		if (sel >= 0 && sel < top)
			top = sel;
		if (sel >= top + h)
			top = sel - h + 1;
		x = 4;
		bgc(theme()->bg);
		frame();
		center(2, theme()->accent2, L("TESTER UN AUTRE DOSSIER", "TEST ANOTHER FOLDER"));
		text_at(x, 4, theme()->dim, L("Chemin du projet :", "Project path:"));
		at(x, 5);
		bgc(mix(theme()->bg, theme()->accent, 0.15));
		fg(theme()->text);
		outf(" %s%s ", saisie, (now_ms() / 400) % 2 ? "▏" : " ");
		bgc(theme()->bg);
		outf("%*s", term_cols() - x - ulen(saisie) - 6 > 0 ? term_cols() - x - ulen(saisie) - 6 : 0, "");
		fill(x, 7, term_cols() - x - 2, h + 1, theme()->bg);
		for (i = top; i < nc && i < top + h; i++)
		{
			at(x, 7 + i - top);
			if (i == sel)
			{
				bgc(mix(theme()->bg, theme()->accent, 0.25));
				fg(theme()->accent2);
			}
			else
				fg(c[i].tag[0] ? theme()->accent : theme()->text);
			outf(" %s %s/ ", i == sel ? "▶" : " ", c[i].nom);
			bgc(theme()->bg);
			if (c[i].tag[0])
				text_at(x + ulen(c[i].nom) + 7, 7 + i - top, theme()->accent2, c[i].tag);
		}
		if (!nc)
			text_at(x, 7, theme()->dim, L("aucun dossier ne correspond", "no folder matches"));
		if (msg)
			text_at(x, term_rows() - 3, rgb(255, 120, 120), msg);
		foot_line(x, L("Tab compléter   ▲ ▼ choisir   ▶ ouvrir   ◀ remonter   Entrée valider   Échap retour",
				"Tab complete   ▲ ▼ pick   ▶ open   ◀ up   Enter confirm   Esc back"));
		flush();
		k = key_read(40);
		if (k < 0)
			continue ;
		if (k != 9 && k != 1000 && k != 1001)
			msg = NULL;
		if (k == 27 || k == 3)
			return (cls(), 0);
		if (k == 9 && nc && sel >= 0)
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
		else if (k == 1002 && sel >= 0)
		{
			completer(saisie, sizeof(saisie), c[sel].nom, 1);
			sel = -1;
			top = 0;
		}
		else if (k == 1003)
		{
			i = (int)strlen(saisie);
			if (i > 1 && saisie[i - 1] == '/')
				saisie[--i] = '\0';
			while (i > 0 && saisie[i - 1] != '/')
				saisie[--i] = '\0';
			if (!saisie[0])
				snprintf(saisie, sizeof(saisie), "/");
			sel = -1;
			top = 0;
		}
		else if (k == 127 || k == 8)
		{
			if (saisie[0])
				saisie[strlen(saisie) - 1] = '\0';
			sel = -1;
		}
		else if (k == '\r' || k == '\n')
		{
			if (sel >= 0)
			{
				completer(saisie, sizeof(saisie), c[sel].nom, 1);
				sel = -1;
				top = 0;
				continue ;
			}
			developper(saisie, tmp, sizeof(tmp));
			if (stat(tmp, &st) == 0 && S_ISDIR(st.st_mode) && realpath(tmp, vrai))
			{
				snprintf(out, n, "%s", vrai);
				return (cls(), 1);
			}
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
