#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

void	foot_line(int x, const char *s)
{
	text_clip(x, term_rows() - 1, theme()->dim, s, term_cols() - x - 1);
}

void	avatar_panel(int x, int y, const t_look *lk, int title, long t)
{
	t_canvas	cv;
	char		tmp[120];
	char		tn[80];
	int			lv;

	cv_init(&cv, 28, 32);
	avatar_draw(&cv, 0, 0, lk, t);
	cv_print(&cv, x, y, theme()->bg);
	cv_free(&cv);
	text_at(x, y + 16, pseudo_color(), g_save.login);
	title_name(title, tn, sizeof(tn));
	snprintf(tmp, sizeof(tmp), "« %.24s »", tn);
	text_at(x, y + 17, theme()->dim, tmp);
	lv = level_of(g_save.xp);
	snprintf(tmp, sizeof(tmp), L("Niveau %d", "Level %d"), lv);
	text_at(x, y + 18, theme()->text, tmp);
	bar(x + 10, y + 18, 18, (g_save.xp - xp_for(lv)) / (double)(xp_for(lv + 1) - xp_for(lv)),
		theme()->accent, theme()->accent2);
	snprintf(tmp, sizeof(tmp), L("%ld ◈ Octets", "%ld ◈ Octets"), g_save.octets);
	text_at(x, y + 19, rgb(255, 215, 0), tmp);
}

#ifndef TDF_PRINTF
int	screen_menu(void)
{
	static int	sel;
	const char	*items[14];
	int			n;
	int			k;
	int			i;
	int			x;
	int			y;
	long		t;
	char		tmp[200];

	n = 0;
	items[n++] = L("Tester                  (r)", "Test                    (r)");
	items[n++] = L("Chevalier", "Knight");
	items[n++] = L("Lore", "Lore");
	items[n++] = L("Stats", "Stats");
	items[n++] = L("Paramètres", "Settings");
	items[n++] = L("Quitter", "Quit");
	cls();
	while (1)
	{
		t = now_ms();
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		y = (term_rows() - 22) / 2;
		if (y < 0)
			y = 0;
		bgc(theme()->bg);
		frame();
		avatar_panel(x, y + 1, &g_save.look, g_save.title, t);
		at(x + 34, y + 1);
		for (i = 0; "LE TESTEUR DES FOUS"[i]; i++)
		{
			fg(hue(i / 30.0));
			outf("%c", "LE TESTEUR DES FOUS"[i]);
		}
		text_at(x + 34 + 20, y + 1, theme()->dim, "v" TDF_VERSION);
		text_at(x + 34, y + 2, theme()->dim, L("Libft · créé et vibe-codé par Splatch",
				"Libft · created and vibe-coded by Splatch"));
		for (i = 0; i < n; i++)
		{
			at(x + 34, y + 4 + i * 2);
			if (i == sel)
			{
				bgc(mix(theme()->bg, theme()->accent, 0.22));
				fg(theme()->accent2);
				outf(" ▶ %-34s ", items[i]);
				bgc(theme()->bg);
			}
			else
			{
				fg(i == 0 ? theme()->accent : theme()->text);
				outf("   %-34s ", items[i]);
			}
		}
		for (k = 0, i = 0; i < NFN; i++)
			k += st_pass(g_run.fn[i].st);
		snprintf(tmp, sizeof(tmp), L("test : %d/43 · boss %d/3 · entr. %d/21",
				"test: %d/43 · boss %d/3 · train %d/21"), k,
			g_save.boss[0] + g_save.boss[1] + g_save.boss[2],
			exo_done_count());
		text_at(x + 34, y + 5 + n * 2, theme()->dim, tmp);
		tdf_menu_aide(x + 34, y + 7 + n * 2, sel);
		flush();
		k = key_dir(key_read(40));
		if (k == 1000)
			sel = (sel + n - 1) % n;
		else if (k == 1001)
			sel = (sel + 1) % n;
		else if (k == '\r' || k == '\n' || k == ' ')
		{
			cls();
			return (sel);
		}
		else if (k == 'r' || k == 'R')
		{
			cls();
			return (100);
		}
		else if (key_quit(k))
			sel = n - 1;
	}
}

extern const int			g_fou_petit_w;
extern const int			g_fou_petit_h;
extern const char *const	g_fou_petit_txt[];
extern const unsigned char	g_fou_petit_rgb[][30][3];

static double	g_phase = -1000.0;

static t_rgb	lumiere(int x, int y, int r, int g, int b)
{
	double	p;
	double	k;

	p = (x + y * 0.6) - g_phase;
	k = 0.85 * exp(-(p * p) / 40.0) * 0.75;
	if (k < 0.02)
		return (rgb(r, g, b));
	return (rgb((int)(r + (255 - r) * k), (int)(g + (255 - g) * k), (int)(b + (255 - b) * k)));
}

static void	fou_petit(int x, int y)
{
	const unsigned char	*h;
	const unsigned char	*b;
	int					r;
	int					c;

	for (r = 0; r < g_fou_bloc_h; r++)
	{
		at(x, y + r);
		for (c = 0; c < g_fou_bloc_w; c++)
		{
			h = g_fou_bloc[r][c][0];
			b = g_fou_bloc[r][c][1];
			bgc(b[3] && h[3] ? lumiere(x + c, y + r, b[0], b[1], b[2]) : theme()->bg);
			if (h[3])
			{
				fg(lumiere(x + c, y + r, h[0], h[1], h[2]));
				out("▀");
			}
			else if (b[3])
			{
				fg(lumiere(x + c, y + r, b[0], b[1], b[2]));
				out("▄");
			}
			else
				out(" ");
		}
		bgc(theme()->bg);
	}
}

static void	titre_bloc(int x, int y, const char *mot, long t)
{
	const char	*l;
	const char	*s;
	int			r;
	int			i;
	int			dx;

	(void)t;
	for (r = 0; r < 5; r++)
	{
		at(x, y + r);
		dx = 0;
		for (s = mot; *s; s++)
		{
			l = entete_police(*s, r);
			for (i = 0; l[i]; i++, dx++)
			{
				if (l[i] == '#')
				{
					fg(lumiere(x + dx, y + r, 100 + 60 * dx / 44, 200, 238 - 40 * dx / 44));
					out("█");
				}
				else
					out(" ");
			}
			out(" ");
			dx++;
		}
	}
}

static void	label_of_item(const char *item, char *b, size_t n)
{
	size_t	l;

	l = strcspn(item, "|");
	if (l >= n)
		l = n - 1;
	memcpy(b, item, l);
	b[l] = '\0';
}

typedef struct s_mu
{
	const char	*items[24];
	char		lab[24][200];
	const char	*act[24];
	char		chemin[24][600];
	int			n;
}	t_mu;

static void	mu_ajouter(t_mu *m, const char *act, const char *lab, const char *chemin)
{
	if (m->n >= 24)
		return ;
	snprintf(m->lab[m->n], sizeof(m->lab[0]), "%s", lab);
	m->items[m->n] = m->lab[m->n];
	m->act[m->n] = act;
	snprintf(m->chemin[m->n], sizeof(m->chemin[0]), "%s", chemin ? chemin : "");
	m->n++;
}

static void	mu_charger(t_mu *m)
{
	m->n = 0;
	mu_ajouter(m, "projet", L("Tester mon projet|Choisis le dossier de ta Libft, de ton ft_printf ou de ton get_next_line. « . » : le dossier où tu es.", "Test my project|Pick the folder of your Libft, ft_printf or get_next_line. '.': the folder you are in."), NULL);
	mu_ajouter(m, "exam", L("Exam en C|Examen blanc de 3 h, série d'exercices ou entraînement : tout se règle aux flèches", "C exam|3-hour mock exam, exercise series or training: all set with the arrows"), NULL);
	mu_ajouter(m, "banque", L("Banque d'exercices|Les exercices du testeur au choix : exam, Libft, printf", "Exercise bank|The tester's exercises, your pick: exam, Libft, printf"), NULL);
	if (tdf_derniere_existe())
		mu_ajouter(m, "revoir", L("Revoir la dernière tentative|Tous les tests de ton dernier passage, avec pourquoi ça passe ou non", "Review the last attempt|Every test of your last run, with why it passes or not"), NULL);
	mu_ajouter(m, "chevalier", L("Chevalier|Ton avatar, la boutique, ton profil et tes succès", "Knight|Your avatar, the shop, your profile and achievements"), NULL);
	mu_ajouter(m, "lore", L("Lore|Les boss, les cinématiques et la carte de la Cité", "Lore|The bosses, the cutscenes and the map of the City"), NULL);
	mu_ajouter(m, "stats", L("Stats|Ta progression, tes badges et le holygraph 42 Next", "Stats|Your progress, your badges and the 42 Next holygraph"), NULL);
	mu_ajouter(m, "params", L("Paramètres|Réglages, langue, interface, commande tdf", "Settings|Settings, language, interface, tdf command"), NULL);
	mu_ajouter(m, "quitter", L("Quitter|Fermer le testeur. Ta progression est déjà enregistrée.", "Quit|Close the tester. Your progress is already saved."), NULL);
}

static const char	*astuce(int i)
{
	switch (i)
	{
		case 0:
			return (L("↑ ↓ pour choisir, Entrée pour valider, Échap pour revenir : tout le testeur marche comme ça.", "↑ ↓ to pick, Enter to confirm, Esc to go back: the whole tester works like this."));
		case 1:
			return (L("« Tester ce dossier » teste le dossier d'où tu as lancé le testeur.", "'Test this folder' tests the folder you started the tester from."));
		case 2:
			return (L("Tes 10 derniers tests sont dans « Tester mon projet » : Entrée les relance.", "Your last 10 tests are in 'Test my project': Enter runs them again."));
		case 3:
			return (L("Tout en haut de « Tester mon projet », ↑ pour écrire un chemin : Tab complète comme zsh.", "At the top of 'Test my project', ↑ to type a path: Tab completes like zsh."));
		case 4:
			return (L("Range le testeur à côté de tes projets, pas dedans : un git add . l'enverrait à la moulinette.", "Keep the tester next to your projects, not inside: a git add . would send it to the moulinette."));
		case 5:
			return (L("Installe la commande tdf dans Paramètres : ensuite « tdf » suffit, dans n'importe quel dossier.", "Install the tdf command in Settings: then 'tdf' is enough, in any folder."));
		case 6:
			return (L("Mets le testeur sur un côté de l'écran et ton éditeur en grand à côté.", "Put the tester on one side of the screen and your editor large next to it."));
		case 7:
			return (L("Dans les résultats Libft, f ouvre la fiche d'une fonction : chaque test et ce qui cloche.", "In the Libft results, f opens a function's card: every test and what is wrong."));
		case 8:
			return (L("Dans les résultats Libft, m ouvre l'audit moulinette : Makefile, relink, fichiers en trop.", "In the Libft results, m opens the moulinette audit: Makefile, relink, extra files."));
		case 9:
			return (L("Tu as corrigé ? r dans les résultats relance les tests tout de suite.", "Fixed it? r in the results runs the tests again right away."));
		case 10:
			return (L("« Revoir la dernière tentative » rouvre tes derniers résultats sans rien relancer.", "'Review the last attempt' reopens your last results without running anything."));
		case 11:
			return (L("Un .gitignore ou un README en trop dans ton rendu : la moulinette compte un fichier parasite.", "An extra .gitignore or README in your repo: the moulinette counts a garbage file."));
		case 12:
			return (L("La Norme compte : installe norminette (python3 -m pip install --user norminette).", "The Norm counts: install norminette (python3 -m pip install --user norminette)."));
		case 13:
			return (L("Le testeur compile avec -Wall -Wextra -Werror, comme la moulinette : zéro warning.", "The tester compiles with -Wall -Wextra -Werror, like the moulinette: zero warnings."));
		case 14:
			return (L("Tes fonctions d'aide doivent être static : le testeur te le signale.", "Your helper functions must be static: the tester tells you."));
		case 15:
			return (L("Une fuite mémoire fait échouer la fonction, même si le résultat est juste.", "A memory leak fails the function, even if the result is right."));
		case 16:
			return (L("Le testeur fait échouer malloc exprès : ta fonction doit renvoyer NULL proprement.", "The tester makes malloc fail on purpose: your function must return NULL cleanly."));
		case 17:
			return (L("ft_printf : la trace au format moulinette est dans ~/.testeur_des_fous/traces/.", "ft_printf: the moulinette-style trace is in ~/.testeur_des_fous/traces/."));
		case 18:
			return (L("ft_printf : chaque conversion est comparée au vrai printf, valeur de retour comprise.", "ft_printf: every conversion is compared to the real printf, return value included."));
		case 19:
			return (L("get_next_line : le testeur essaie 7 BUFFER_SIZE, de 1 à 10 000 000.", "get_next_line: the tester tries 7 BUFFER_SIZE values, from 1 to 10,000,000."));
		case 20:
			return (L("get_next_line doit compiler avec et sans -D BUFFER_SIZE.", "get_next_line must compile with and without -D BUFFER_SIZE."));
		case 21:
			return (L("Examshell des fous : écris ton code dans un autre terminal, puis tape grademe.", "Examshell des fous: write your code in another terminal, then type grademe."));
		case 22:
			return (L("Une commande mal tapée dans l'examshell ? Il te propose la bonne : Entrée pour accepter.", "Mistyped a command in the examshell? It suggests the right one: Enter to accept."));
		case 23:
			return (L("« sujet es », « sujet en »… : relis le sujet dans une autre langue.", "'sujet es', 'sujet en'…: reread the subject in another language."));
		case 24:
			return (L("En examen, l'attente après un échec grandit : relis ton code avant grademe.", "In the exam, the wait after a failure grows: reread your code before grademe."));
		case 25:
			return (L("« Conditions réelles » : 3 h, attente après échec, aucun cheat, comme le vrai exam.", "'Real conditions': 3 h, wait after failure, no cheats, like the real exam."));
		case 26:
			return (L("La banque : 94 exercices, à faire dans l'ordre que tu veux.", "The bank: 94 exercises, in any order you like."));
		case 27:
			return (L("Dans la banque, ◀ ▶ change de catégorie : Exam, Libft, printf.", "In the bank, ◀ ▶ switches category: Exam, Libft, printf."));
		case 28:
			return (L("Chaque exercice réussi de la banque rapporte des Octets et de l'XP.", "Every bank exercise you pass earns Octets and XP."));
		case 29:
			return (L("Les Octets se dépensent dans la Boutique : objets, thèmes, titres.", "Octets are spent in the Shop: items, themes, titles."));
		case 30:
			return (L("Chevalier : équipe tes objets dans la garde-robe et choisis leurs couleurs.", "Knight: equip your items in the wardrobe and pick their colours."));
		case 31:
			return (L("Les succès se débloquent en avançant : regarde-les dans ton Profil.", "Achievements unlock as you progress: see them in your Profile."));
		case 32:
			return (L("Le holygraph montre tes projets validés : une fois validé, c'est pour toujours.", "The holygraph shows your validated projects: once validated, it is for good."));
		case 33:
			return (L("Lore : revois les cinématiques que tu as débloquées.", "Lore: replay the cutscenes you unlocked."));
		case 34:
			return (L("La carte de la Cité s'allume au fil de ta progression.", "The map of the City lights up as you progress."));
		case 35:
			return (L("Stats : ton historique et ce qui échoue le plus souvent.", "Stats: your history and what fails most often."));
		case 36:
			return (L("Le testeur ne contient aucune solution : il casse ton code et te dit où regarder.", "The tester holds no solution: it breaks your code and tells you where to look."));
		case 37:
			return (L("Un bug ? Paramètres, Nous aider : crée une trace et envoie-la sur le Discord.", "A bug? Settings, Help us: create a trace and send it on the Discord."));
		case 38:
			return (L("bash run.sh --debug affiche tous les rapports détaillés.", "bash run.sh --debug shows every detailed report."));
		case 39:
			return (L("Ta progression est dans ~/.testeur_des_fous : tu peux recloner le testeur sans rien perdre.", "Your progress is in ~/.testeur_des_fous: you can clone the tester again without losing anything."));
		case 40:
			return (L("git pull dans le dossier du testeur pour le mettre à jour.", "git pull in the tester folder to update it."));
		case 41:
			return (L("Le testeur parle 8 langues : change-la dans Paramètres, Réglages.", "The tester speaks 8 languages: change it in Settings, Options."));
		case 42:
			return (L("Pendant les tests, Échap arrête l'analyse.", "During the tests, Esc stops the analysis."));
		case 43:
			return (L("Un test bloqué est coupé au bout de 30 s : boucle infinie ?", "A stuck test is cut after 30 s: infinite loop?"));
		case 44:
			return (L("AddressSanitizer donne la ligne exacte d'une erreur mémoire : lis-la en entier.", "AddressSanitizer gives the exact line of a memory error: read it all."));
		case 45:
			return (L("Valgrind peut repasser sur tes tests : active-le dans Réglages.", "Valgrind can rerun your tests: turn it on in Options."));
		case 46:
			return (L("Le niveau d'aide, dans Réglages, choisit combien de détails tu vois.", "The help level, in Options, sets how much detail you see."));
		case 47:
			return (L("Le testeur compare tes fonctions à la vraie libc sur des centaines de cas.", "The tester compares your functions to the real libc on hundreds of cases."));
		case 48:
			return (L("Prédis le résultat avant de lancer le test : tu apprends deux fois plus.", "Predict the result before running the test: you learn twice as much."));
		case 49:
			return (L("Le Discord 42 Next Global : entraide, bugs, idées.", "The 42 Next Global Discord: help, bugs, ideas."));
		case 50:
			return (L("Le nom « testeur des fous » vient d'un printf(\"Test de fou\"); qu'un des créateurs glissait partout pour déboguer.", "The name 'testeur des fous' comes from a printf(\"Test de fou\"); one of the creators put everywhere to debug."));
		case 51:
			return (L("Si tu lis ces mots, tu as réussi la piscine. Cette place, tu l'as méritée. Bravo.", "If you are reading these words, you passed the piscine. You earned this place. Well done."));
		case 52:
			return (L("Ne pars pas trop vite : à vouloir voler trop haut, on se brûle les ailes.", "Do not rush: fly too high too soon and you burn your wings."));
		case 53:
			return (L("Un segfault n'est pas un échec : c'est ton code qui te montre où regarder.", "A segfault is not a failure: it is your code showing you where to look."));
		case 54:
			return (L("Chaque bug corrigé est une leçon que personne ne pourra te reprendre.", "Every bug you fix is a lesson no one can take from you."));
		case 55:
			return (L("Les tours se construisent pierre par pierre. Ton code aussi.", "Towers are built stone by stone. So is your code."));
		case 56:
			return (L("Tout le monde a bloqué sur ft_split un jour. Tout le monde.", "Everyone got stuck on ft_split one day. Everyone."));
		case 57:
			return (L("Comprendre prend plus de temps que copier, mais ça reste toute une vie.", "Understanding takes longer than copying, but it stays for a lifetime."));
		case 58:
			return (L("Explique ton code à un canard en plastique : souvent, il trouve le bug avant toi.", "Explain your code to a rubber duck: it often finds the bug before you."));
		case 59:
			return (L("Une pause n'est pas du temps perdu : ton cerveau continue de chercher sans toi.", "A break is not wasted time: your brain keeps searching without you."));
		case 60:
			return (L("Lis l'erreur en entier avant de la corriger. Elle dit presque toujours la vérité.", "Read the whole error before fixing it. It almost always tells the truth."));
		case 61:
			return (L("Expliquer à quelqu'un, c'est comprendre deux fois.", "Explaining to someone is understanding twice."));
		case 62:
			return (L("Ton code d'aujourd'hui te semblera maladroit dans un mois. C'est la preuve que tu avances.", "Today's code will look clumsy to you in a month. That is proof you are moving forward."));
		case 63:
			return (L("Pas besoin d'aller vite. Il suffit de ne pas s'arrêter.", "No need to go fast. Just do not stop."));
		case 64:
			return (L("La moulinette juge le code, jamais la personne. Respire, et recommence.", "The moulinette judges the code, never the person. Breathe, and try again."));
		case 65:
			return (L("Patrick est en secret un dinosaure.", "Patrick is secretly a dinosaur."));
		case 66:
			return (L("Paul est en secret Pacman.", "Paul is secretly Pacman."));
		case 67:
			return (L("Roman est en secret FuzeIII.", "Roman is secretly FuzeIII."));
		case 68:
			return (L("Moulinette est la meilleure chatte du monde.", "Moulinette is the best cat in the world."));
		case 69:
			return (L("f6, c'est pour les sportifs olympiques.", "f6 is for Olympic athletes."));
	}
	return ("");
}

typedef struct s_zone
{
	int	x;
	int	y0;
	int	y1;
	int	w;
}	t_zone;

typedef struct s_bulle
{
	int		cur;
	int		prec;
	long	t0;
	long	suite;
	int		x;
	int		y;
	int		w;
	int		n;
	int		cote;
	char	l[6][200];
}	t_bulle;

static t_bulle	g_bulle = {-1, -1, 0, 1200, 0, 0, 0, 0, -1, {{0}}};

static const char	*octets_de(const char *s, int car)
{
	while (*s && car > 0)
	{
		s++;
		while (*s && ((unsigned char)*s & 0xC0) == 0x80)
			s++;
		car--;
	}
	return (s);
}

static int	bulle_couper(const char *s, int w, char l[][200], int max)
{
	int	n;
	int	len;
	int	i;
	int	c;
	int	coupe;

	n = 0;
	while (*s && n < max)
	{
		while (*s == ' ')
			s++;
		len = 0;
		coupe = -1;
		for (i = 0, c = 0; s[i] && c < w; i++)
		{
			if (s[i] == ' ')
				coupe = i;
			if (((unsigned char)s[i] & 0xC0) != 0x80)
				c++;
			len = i + 1;
		}
		while (s[len] && ((unsigned char)s[len] & 0xC0) == 0x80)
			len++;
		if (s[len] && coupe > 0)
			len = coupe;
		snprintf(l[n++], 200, "%.*s", len, s);
		s += len;
	}
	return (n);
}

static void	bulle_choisir(t_zone *z, int nz, long anim)
{
	t_zone	*q;
	int		cand[4];
	int		nc;
	int		i;

	if (nz <= 0)
	{
		g_bulle.suite = anim + 3000;
		return ;
	}
	for (nc = 0, i = 0; i < nz; i++)
		if ((z[i].x < term_cols() / 2) != g_bulle.cote)
			cand[nc++] = i;
	q = nc ? &z[cand[rand() % nc]] : &z[rand() % nz];
	do
		i = rand() % 100 < 10 ? 65 + rand() % 5 : rand() % 65;
	while (i == g_bulle.prec);
	g_bulle.w = q->w < 34 ? q->w : 34;
	g_bulle.n = 0;
	if (i < 65)
		snprintf(g_bulle.l[g_bulle.n++], 200, "%s", L("Le savais-tu ?", "Did you know?"));
	g_bulle.n += bulle_couper(astuce(i), g_bulle.w, g_bulle.l + g_bulle.n, 6 - g_bulle.n);
	if (q->y1 - q->y0 < g_bulle.n)
	{
		g_bulle.suite = anim + 2000;
		return ;
	}
	g_bulle.cur = i;
	g_bulle.prec = i;
	g_bulle.cote = q->x < term_cols() / 2;
	g_bulle.t0 = anim;
	g_bulle.x = q->x;
	g_bulle.y = q->y0 + rand() % (q->y1 - q->y0 - g_bulle.n + 1);
}

static void	ligne_ecartee(int cx, int y, int wmax, t_rgb c, const char *s)
{
	char	b[420];
	int		j;
	int		k;
	int		n;

	for (j = 0, k = 0; s[j] && k < (int)sizeof(b) - 8; )
	{
		n = 1;
		while (s[j + n] && ((unsigned char)s[j + n] & 0xC0) == 0x80)
			n++;
		if (k)
			b[k++] = ' ';
		memcpy(b + k, s + j, n);
		k += n;
		j += n;
	}
	b[k] = '\0';
	n = ulen(b);
	if (n > wmax)
		n = wmax;
	text_clip(cx - n / 2, y, c, octets_de(b, (ulen(b) - n) / 2), n);
}

static void	bulle(t_zone *z, int nz, long anim)
{
	t_rgb	base;
	t_rgb	c;
	long	dt;
	double	k;
	int		cx;
	int		i;

	if (g_bulle.cur < 0)
	{
		if (anim >= g_bulle.suite)
			bulle_choisir(z, nz, anim);
		if (g_bulle.cur < 0)
			return ;
	}
	dt = anim - g_bulle.t0;
	if (dt > 7500)
	{
		g_bulle.cur = -1;
		g_bulle.suite = anim + 1800;
		return ;
	}
	cx = g_bulle.x + g_bulle.w / 2;
	k = 0.10 + 0.10 * sin(anim / 300.0);
	for (i = 0; i < g_bulle.n; i++)
	{
		base = g_bulle.cur >= 65 ? rgb(255, 140, 220) : (i == 0 ? rgb(255, 190, 70) : rgb(255, 255, 85));
		if (dt < 130)
			c = rgb(255, 255, 255);
		else if (dt < 260)
			c = mix(rgb(255, 255, 255), base, (dt - 130) / 130.0);
		else if (dt > 6500)
			c = mix(base, theme()->bg, (dt - 6500) / 1000.0);
		else
			c = mix(base, rgb(255, 255, 255), k);
		if (dt < 130)
			ligne_ecartee(cx, g_bulle.y + i, g_bulle.w + 8, c, g_bulle.l[i]);
		else
			text_clip(cx - ulen(g_bulle.l[i]) / 2, g_bulle.y + i, c, g_bulle.l[i], g_bulle.w);
	}
}

static int	mu_largeur(t_mu *m)
{
	char	lab[200];
	int		w;
	int		i;

	w = 0;
	for (i = 0; i < m->n; i++)
	{
		label_of_item(m->items[i], lab, sizeof(lab));
		if (ulen(lab) > w)
			w = ulen(lab);
	}
	return (w + 6);
}

static int	mu_ecran(t_mu *m, int *sel)
{
	char	lab[200];
	char	tmp[200];
	int		cols;
	int		rows;
	int		haut;
	int		tw;
	int		x;
	int		y;
	int		iw;
	int		ix;
	int		bx;
	int		i;
	int		k;
	int		av;
	long	anim;
	long	calme;
	long	prec;
	t_zone	zones[4];
	int		nz;

	anim = 0;
	calme = 0;
	prec = now_ms();
	cls();
	while (1)
	{
		cols = term_cols();
		rows = term_rows();
		bgc(theme()->bg);
		frame();
		tw = entete_police_largeur("TESTEUR");
		if (entete_police_largeur("DES FOUS") > tw)
			tw = entete_police_largeur("DES FOUS");
		haut = 0;
		if (cols >= tw + 4 + g_fou_petit_w + 4 && rows >= 17 + m->n + 6)
		{
			x = (cols - (tw + 4 + g_fou_petit_w)) / 2;
			titre_bloc(x, 3, "TESTEUR", 0);
			titre_bloc(x, 9, "DES FOUS", 0);
			fou_petit(x + tw + 4, 1);
			haut = 16;
		}
		else if (rows >= 13 + m->n)
		{
			x = (cols - 19) / 2;
			at(x > 0 ? x : 0, 1);
			for (i = 0; "LE TESTEUR DES FOUS"[i]; i++)
			{
				fg(hue(i / 30.0));
				outf("%c", "LE TESTEUR DES FOUS"[i]);
			}
			haut = 2;
		}
		snprintf(tmp, sizeof(tmp), "v%s · %s", TDF_VERSION, L("créé et vibe-codé par Splatch", "created and vibe-coded by Splatch"));
		if (haut)
			center(haut, theme()->dim, tmp);
		y = haut + 2;
		iw = mu_largeur(m);
		av = cols >= 30 + 4 + iw + 4 && rows >= y + 21;
		bx = (cols - ((av ? 32 : 0) + iw)) / 2;
		if (bx < 1)
			bx = 1;
		if (av)
			avatar_panel(bx, y, &g_save.look, g_save.title, 0);
		ix = bx + (av ? 32 : 0);
		for (i = 0; i < m->n; i++)
		{
			label_of_item(m->items[i], lab, sizeof(lab));
			at(ix, y + 1 + i * (rows >= y + 2 * m->n + 6 ? 2 : 1));
			if (i == *sel)
			{
				bgc(mix(theme()->bg, theme()->accent, 0.22));
				fg(theme()->accent2);
			}
			else
				fg(i == 0 ? theme()->accent : theme()->text);
			outf(" %s %-*s", i == *sel ? "▶" : " ", iw - 4, lab);
			bgc(theme()->bg);
		}
		nz = 0;
		if (cols - (ix + iw + 3) - 3 >= 22)
			zones[nz++] = (t_zone){ix + iw + 3, y + 1, y + 1 + m->n * (rows >= y + 2 * m->n + 6 ? 2 : 1), cols - (ix + iw + 3) - 3};
		if (bx - 4 >= 22)
			zones[nz++] = (t_zone){2, y + 1, y + 21 < rows - 3 ? y + 21 : rows - 3, bx - 4};
		if (haut == 16 && x - 6 >= 22)
			zones[nz++] = (t_zone){3, 2, 14, x - 6};
		if (haut == 16 && cols - (x + tw + 4 + g_fou_bloc_w) - 6 >= 22)
			zones[nz++] = (t_zone){x + tw + 4 + g_fou_bloc_w + 3, 2, 14, cols - (x + tw + 4 + g_fou_bloc_w) - 6};
		bulle(zones, nz, anim);
		i = y + 2 + m->n * (rows >= y + 2 * m->n + 6 ? 2 : 1);
		if (strchr(m->items[*sel], '|') && i < rows - 2)
		{
			fill(1, i, cols - 2, 2, theme()->bg);
			text_wrap(ix + 1, i, i + 2, cols - ix - 3 > 60 ? 60 : cols - ix - 3, theme()->accent, strchr(m->items[*sel], '|') + 1);
		}
		foot_line(bx, L("▲ ▼ choisir   Entrée : ouvrir   Échap : quitter", "▲ ▼ pick   Enter: open   Esc: quit"));
		flush();
		k = key_dir(key_read(60));
		if (k >= 0)
			calme = now_ms();
		else if (now_ms() - calme > 400)
		{
			anim += now_ms() - prec;
			g_phase = fmod(anim / 22.0, cols + 90.0) - 40.0;
		}
		prec = now_ms();
		if (k == 1000)
			*sel = ((*sel - tdf_touche_fleche(k)) % m->n + m->n) % m->n;
		else if (k == 1001)
			*sel = (*sel + tdf_touche_fleche(k)) % m->n;
		else if (k == '\r' || k == '\n' || k == ' ')
			return (cls(), *sel);
		else if (key_quit(k))
			return (cls(), m->n - 1);
	}
}

int	tdf_menu_principal(void (*profile)(void), void (*boss)(void), void (*credits)(void))
{
	static t_mu	m;
	static int	sel;
	char		dossier[1100];
	const char	*a;
	int			i;

	g_choix[0] = '\0';
	while (1)
	{
		mu_charger(&m);
		if (sel >= m.n)
			sel = 0;
		i = mu_ecran(&m, &sel);
		a = m.act[i];
		if (!strcmp(a, "exam") || !strcmp(a, "revoir"))
		{
			tdf_sortie(a, NULL);
			return (46);
		}
		if (!strcmp(a, "banque"))
			tdf_banque();
		else if (!strcmp(a, "chevalier"))
			tdf_menu_chevalier(profile);
		else if (!strcmp(a, "lore"))
		{
			tdf_menu_lore(boss);
			if (g_choix[0])
				return (46);
		}
		else if (!strcmp(a, "stats"))
			tdf_menu_stats();
		else if (!strcmp(a, "params"))
		{
			tdf_menu_params(credits);
			if (g_choix[0])
				return (46);
		}
		else if (!strcmp(a, "projet"))
		{
			if (tdf_choisir_dossier(dossier, sizeof(dossier)))
			{
				tdf_sortie("autre", dossier);
				return (46);
			}
		}
		else
			return (0);
	}
}

#endif

typedef struct s_shopst
{
	int	tab;
	int	sel;
	int	top;
	int	wardrobe;
	int	jump;
}	t_shopst;

#define NTAB (NSLOT + 2)

static int	tab_slot(int tab)
{
	static const int	order[NSLOT] = {SL_CASQUE, SL_DECO, SL_YEUX, SL_ARMURE,
		SL_JAMBES, SL_CAPE, SL_ARME, SL_MAIN, SL_FAMILIER, SL_AURA, SL_FOND};

	return (tab < NSLOT ? order[tab] : -1);
}

static int	list_items(const t_shopst *s, int *ids, int max)
{
	int	n;
	int	i;
	int	slot;

	n = 0;
	slot = tab_slot(s->tab);
	if (slot >= 0)
	{
		for (i = 0; i < g_nitems && n < max; i++)
			if (g_items[i].slot == slot && (!s->wardrobe || item_owned(i)))
				ids[n++] = i;
	}
	else if (s->tab == NSLOT)
	{
		for (i = 0; i < g_ntheme && n < max; i++)
			if (!s->wardrobe || g_save.themes[i] == '1')
				ids[n++] = i;
	}
	else
		for (i = 0; i < NTITLE && n < max; i++)
			if (!s->wardrobe || g_save.titles[i] == '1')
				ids[n++] = i;
	return (n);
}

static const char	*tab_name(int tab)
{
	if (tab < NSLOT)
		return (L(g_slot_fr[tab_slot(tab)], g_slot_en[tab_slot(tab)]));
	if (tab == NSLOT)
		return (L("Thèmes de couleur", "Color themes"));
	return (L("Titres (1000)", "Titles (1000)"));
}

static void	coins(int x, int y, int amount)
{
	long	t0;
	long	t;
	int		i;
	char	tmp[40];

	t0 = now_ms();
	while ((t = now_ms() - t0) < anim_ms(650))
	{
		for (i = 0; i < 10; i++)
		{
			at(x + (int)(cos(i * 0.63) * t / 40.0), y + (int)(sin(i * 0.63) * t / 120.0));
			fg(mix(rgb(255, 215, 0), theme()->bg, t / 650.0));
			out("◈");
		}
		snprintf(tmp, sizeof(tmp), "-%d ◈", amount);
		text_at(x - 2, y - 1 - (int)(t / 230), mix(rgb(255, 215, 0), theme()->bg, t / 650.0), tmp);
		flush();
		sleep_ms(25);
	}
}

static int	price_of(const t_shopst *s, int id)
{
	if (tab_slot(s->tab) >= 0)
		return (g_items[id].price);
	if (s->tab == NSLOT)
		return (g_themes[id].price);
	return (title_price(id));
}

static int	owns(const t_shopst *s, int id)
{
	if (tab_slot(s->tab) >= 0)
		return (item_owned(id));
	if (s->tab == NSLOT)
		return (g_save.themes[id] == '1');
	return (g_save.titles[id] == '1');
}

static int	equipped(const t_shopst *s, int id)
{
	if (tab_slot(s->tab) >= 0)
		return (g_save.look.eq[tab_slot(s->tab)] == id);
	if (s->tab == NSLOT)
		return (!strcmp(g_save.theme, g_themes[id].id));
	return (g_save.title == id);
}

static void	count_achs(void)
{
	int	i;
	int	n;

	for (n = 0, i = 0; i < g_nitems; i++)
		n += item_owned(i) && g_items[i].price > 0;
	if (n >= 10)
		ach_give(13);
	for (n = 0, i = 0; i < NTITLE; i++)
		n += g_save.titles[i] == '1';
	if (n >= 25)
		ach_give(14);
	for (n = 0, i = 0; i < NSLOT; i++)
		n += g_save.look.eq[i] >= 0 && g_items[g_save.look.eq[i]].price >= 0
			&& !strstr(g_items[g_save.look.eq[i]].id, "_aucun");
	if (n >= NSLOT - 1)
		ach_give(17);
	if (g_save.octets >= 1000)
		ach_give(15);
}

static void	equip(const t_shopst *s, int id)
{
	if (tab_slot(s->tab) >= 0)
		g_save.look.eq[tab_slot(s->tab)] = id;
	else if (s->tab == NSLOT)
		snprintf(g_save.theme, sizeof(g_save.theme), "%s", g_themes[id].id);
	else
		g_save.title = id;
}

void	screen_shop(int wardrobe)
{
	t_shopst	s;
	int			ids[NTITLE];
	int			n;
	int			k;
	int			i;
	int			x;
	int			id;
	int			rows;
	long		t;
	long		shake;
	t_look		lk;
	int			title;
	char		tmp[200];
	char		tn[80];

	memset(&s, 0, sizeof(s));
	s.wardrobe = wardrobe;
	s.jump = 1;
	shake = 0;
	cls();
	while (1)
	{
		t = now_ms();
		n = list_items(&s, ids, NTITLE);
		if (s.jump)
		{
			s.jump = 0;
			for (i = 0; i < n; i++)
				if (equipped(&s, ids[i]))
					s.sel = i;
		}
		if (s.sel >= n)
			s.sel = n ? n - 1 : 0;
		rows = term_rows() - 9;
		if (rows > 14)
			rows = 14;
		if (s.sel < s.top)
			s.top = s.sel;
		if (s.sel >= s.top + rows)
			s.top = s.sel - rows + 1;
		id = n ? ids[s.sel] : -1;
		lk = g_save.look;
		title = g_save.title;
		if (id >= 0 && tab_slot(s.tab) >= 0)
			lk.eq[tab_slot(s.tab)] = id;
		else if (id >= 0 && s.tab == NSLOT + 1)
			title = id;
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		frame();
		center(0, mix(theme()->accent, theme()->accent2, 0.5 + 0.5 * sin(t / 300.0)),
			wardrobe ? L("◈  G A R D E - R O B E  ◈", "◈  W A R D R O B E  ◈")
			: L("◈  B O U T I Q U E   D E S   F O U S  ◈", "◈  T H E   C R A Z Y   S H O P  ◈"));
		snprintf(tmp, sizeof(tmp), "%ld ◈", g_save.octets);
		text_at(x + 78 - ulen(tmp) + (shake > t ? rand() % 3 - 1 : 0), 0,
			shake > t ? rgb(255, 80, 90) : rgb(255, 215, 0), tmp);
		snprintf(tmp, sizeof(tmp), "◀  %s  ▶   (%d/%d)", tab_name(s.tab), s.tab + 1, NTAB);
		text_at(x, 2, theme()->accent, tmp);
		for (i = s.top; i < n && i < s.top + rows; i++)
		{
			at(x, 4 + i - s.top);
			if (i == s.sel)
				bgc(mix(theme()->bg, theme()->accent, 0.2));
			fg(equipped(&s, ids[i]) ? rgb(80, 230, 140) : theme()->text);
			if (tab_slot(s.tab) >= 0)
				snprintf(tn, sizeof(tn), "%s", L(g_items[ids[i]].fr, g_items[ids[i]].en));
			else if (s.tab == NSLOT)
				snprintf(tn, sizeof(tn), "%s", g_themes[ids[i]].name);
			else
				title_name(ids[i], tn, sizeof(tn));
			outf(" %s ", i == s.sel ? "▶" : " ");
			out(tn);
			outf("%*s", 30 - ulen(tn) > 0 ? 30 - ulen(tn) : 1, "");
			if (equipped(&s, ids[i]))
				out(L("ÉQUIPÉ  ", "EQUIPPED"));
			else if (owns(&s, ids[i]))
				out(L("possédé ", "owned   "));
			else if (price_of(&s, ids[i]) < 0)
			{
				fg(rgb(255, 200, 80));
				out(L("exclusif", "exclusive"));
			}
			else
			{
				fg(g_save.octets >= price_of(&s, ids[i]) ? rgb(255, 215, 0) : theme()->dim);
				outf("%4d ◈  ", price_of(&s, ids[i]));
			}
			bgc(theme()->bg);
		}
		if (!n)
			text_at(x, 4, theme()->dim, L("Rien ici pour l'instant. Passe à la boutique !",
					"Nothing here yet. Visit the shop!"));
		avatar_panel(x + 50, 3, &lk, title, t);
		if (id >= 0 && tab_slot(s.tab) >= 0 && !equipped(&s, id))
			text_at(x + 50, 24 > term_rows() - 3 ? term_rows() - 3 : 24, rgb(255, 200, 80),
				L("Aperçu : pas encore équipé (Entrée)", "Preview: not equipped yet (Enter)"));
		else if (id >= 0 && tab_slot(s.tab) >= 0)
			text_at(x + 50, 24 > term_rows() - 3 ? term_rows() - 3 : 24, rgb(80, 230, 140),
				L("Ton chevalier, tel qu'il est équipé", "Your knight, as equipped"));
		if (s.tab == NSLOT && id >= 0)
		{
			for (i = 0; i < 6; i++)
				fill(x + 50 + i * 4, 23 > term_rows() - 2 ? term_rows() - 2 : 23, 3, 1,
					((t_rgb[]){g_themes[id].accent, g_themes[id].accent2, g_themes[id].wall,
						g_themes[id].floor, g_themes[id].text, g_themes[id].bg})[i]);
		}
		text_at(x, term_rows() - 2, theme()->dim,
			L("◀ ▶ rayon  ▲ ▼ article  Entrée : acheter/équiper  Échap : retour",
				"◀ ▶ section  ▲ ▼ item  Enter: buy/equip  Esc: back"));
		if (s.tab == NSLOT + 1)
			foot_line(x, L("+ / - : sauter de 20   r : titre au hasard", "+ / -: jump 20   r: random title"));
		flush();
		k = key_dir(key_read(40));
		if (key_quit(k) || k == 127 || k == 8)
			break ;
		if (k == 1002)
		{
			s.tab = (s.tab + 1) % NTAB;
			s.sel = 0;
			s.top = 0;
			s.jump = 1;
		}
		else if (k == 1003)
		{
			s.tab = (s.tab + NTAB - 1) % NTAB;
			s.sel = 0;
			s.top = 0;
			s.jump = 1;
		}
		else if (k == 1000 && s.sel > 0)
			s.sel--;
		else if (k == 1001 && s.sel < n - 1)
			s.sel++;
		else if (k == '+' && n)
			s.sel = s.sel + 20 < n ? s.sel + 20 : n - 1;
		else if (k == '-')
			s.sel = s.sel - 20 > 0 ? s.sel - 20 : 0;
		else if (k == 'r' && n)
			s.sel = rand() % n;
		else if ((k == '\r' || k == '\n' || k == ' ') && id >= 0)
		{
			if (!owns(&s, id))
			{
				if (price_of(&s, id) < 0)
				{
					toast(L("EXCLUSIF", "EXCLUSIVE"), L("réservé aux Vétérans de la Libft (testeur printf)",
							"for Libft Veterans only (printf tester)"), rgb(255, 200, 80));
					cls();
					continue ;
				}
				if (g_save.octets < price_of(&s, id))
				{
					shake = now_ms() + 500;
					continue ;
				}
				g_save.octets -= price_of(&s, id);
				if (tab_slot(s.tab) >= 0)
					item_own(id);
				else if (s.tab == NSLOT)
					g_save.themes[id] = '1';
				else
					g_save.titles[id] = '1';
				coins(x + 36, 5 + s.sel - s.top, price_of(&s, id));
			}
			equip(&s, id);
			count_achs();
			save_write();
		}
	}
	cls();
}

static const char	*opt_val(int i, char *buf)
{
	static const char	*anim[3][2] = {{"normales", "normal"}, {"rapides", "fast"},
		{"aucune", "none"}};
	static const char	*keys[3] = {"flèches + ZQSD", "flèches + WASD", "flèches + HJKL"};
	static const char	*aud[3][2] = {{"auto", "auto"}, {"toujours", "always"},
		{"jamais", "never"}};

	if (i == 0)
		return (L(anim[g_save.cfg.anim][0], anim[g_save.cfg.anim][1]));
	if (i == 1)
		return (g_save.cfg.col256 ? "256" : L("24 bits (millions)", "24-bit (millions)"));
	if (i == 2)
		return (langue_nom(g_save.cfg.lang));
	if (i == 3)
		return (keys[g_save.cfg.keys]);
	if (i == 4)
		return (g_save.cfg.sound ? L("activé", "on") : L("coupé", "off"));
	if (i == 5)
		return (theme()->name);
	if (i == 6)
		return (((const char *[3]){L("aucune (le cas seul)", "none (the case only)"),
			L("+ la ligne de ton code", "+ your code line"),
			L("+ la ligne et un conseil", "+ line and advice")})[g_save.cfg.help]);
	if (i == 7)
		return (sprintf(buf, "%d s", g_save.cfg.tmo), buf);
	if (i >= 8 && i <= 11)
		return (((int []){g_save.cfg.asan, g_save.cfg.ubsan, g_save.cfg.valg,
				g_save.cfg.norm})[i - 8] ? L("oui", "yes") : L("non", "no"));
	if (i == 12)
		return (L(aud[g_save.cfg.audit][0], aud[g_save.cfg.audit][1]));
	if (i == 13)
		return (g_save.login);
	if (i == 15)
		return (g_save.cfg.lore == 2 ? L("non", "no") : L("oui", "yes"));
	if (i == 16 && !g_save.cfg.rendu)
		return (sprintf(buf, "auto (%s)", rendu_nom(rendu_palier())), buf);
	if (i == 16)
		return (rendu_nom(rendu_palier()));
	if (i == 17)
		return (sprintf(buf, "%d %%", rendu_deploiement(NULL)), buf);
	return ("");
}

static void	opt_change(int i, int d)
{
	static const int	tmo[4] = {10, 30, 60, 120};
	int					k;

	if (i == 0)
		g_save.cfg.anim = (g_save.cfg.anim + 3 + d) % 3;
	else if (i == 1)
		g_save.cfg.col256 = !g_save.cfg.col256;
	else if (i == 2)
		g_save.cfg.lang = (g_save.cfg.lang + NLANG + d) % NLANG;
	else if (i == 3)
		g_save.cfg.keys = (g_save.cfg.keys + 3 + d) % 3;
	else if (i == 4)
		g_save.cfg.sound = !g_save.cfg.sound;
	else if (i == 5)
	{
		for (k = 0; k < g_ntheme && strcmp(g_themes[k].id, g_save.theme); k++)
			;
		do
			k = (k + g_ntheme + d) % g_ntheme;
		while (g_save.themes[k] != '1');
		snprintf(g_save.theme, sizeof(g_save.theme), "%s", g_themes[k].id);
	}
	else if (i == 6)
		g_save.cfg.help = (g_save.cfg.help + 3 + d) % 3;
	else if (i == 7)
	{
		for (k = 0; k < 4 && tmo[k] != g_save.cfg.tmo; k++)
			;
		g_save.cfg.tmo = tmo[(k + 4 + d) % 4];
	}
	else if (i == 8)
		g_save.cfg.asan = !g_save.cfg.asan;
	else if (i == 9)
		g_save.cfg.ubsan = !g_save.cfg.ubsan;
	else if (i == 10)
		g_save.cfg.valg = !g_save.cfg.valg;
	else if (i == 11)
		g_save.cfg.norm = !g_save.cfg.norm;
	else if (i == 12)
		g_save.cfg.audit = (g_save.cfg.audit + 3 + d) % 3;
	else if (i == 15)
		g_save.cfg.lore = g_save.cfg.lore == 2 ? 1 : 2;
	else if (i == 16)
	{
		k = g_save.cfg.rendu;
		do
			k = (k + 5 + d) % 5;
		while (k && !rendu_dispo(k - 1));
		g_save.cfg.rendu = k;
	}
}

int	confirm(const char *q)
{
	int	k;

	fill(0, term_rows() / 2 - 2, term_cols(), 5, mix(theme()->bg, rgb(255, 60, 60), 0.2));
	center(term_rows() / 2 - 1, rgb(255, 120, 120), q);
	center(term_rows() / 2 + 1, theme()->text, L("o : oui   n : non", "y: yes   n: no"));
	flush();
	while ((k = key_read(100)) < 0)
		;
	return (k == 'o' || k == 'y' || k == 'O' || k == 'Y');
}

void	screen_settings(void)
{
	static const int	ids[18] = {0, 1, 16, 17, 2, 3, 4, 5, 15, 6, 7, 8, 9, 10, 11, 12, 13, 14};
	static int			row;
	const char			*lab[18];
	char				buf[64];
	int					k;
	int					r;
	int					i;
	int					sel;
	int					x;

	cls();
	while (1)
	{
		lab[0] = L("Animations", "Animations");
		lab[1] = L("Couleurs", "Colors");
		lab[2] = L("Langue", "Language");
		lab[3] = L("Touches", "Keys");
		lab[4] = L("Son (bip)", "Sound (beep)");
		lab[5] = L("Thème (ceux que tu possèdes)", "Theme (owned ones)");
		lab[6] = L("Niveau d'aide", "Help level");
		lab[7] = L("Temps max par test", "Max time per test");
		lab[8] = "AddressSanitizer";
		lab[9] = "UndefinedBehaviorSanitizer";
		lab[10] = L("Repasser sous valgrind", "Rerun under valgrind");
		lab[11] = L("Norminette", "Norminette");
		lab[12] = L("Audit moulinette", "Moulinette audit");
		lab[13] = L("Pseudo", "Nickname");
		lab[14] = L("Remettre la sauvegarde à zéro", "Reset the save");
		lab[15] = L("Histoire et cinématiques", "Story and cutscenes");
		lab[16] = L("Rendu des images", "Image rendering");
		lab[17] = L("Déploiement graphique", "Graphics deployment");
		x = (term_cols() - 72) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		frame();
		center(1, theme()->accent2, L("P A R A M È T R E S", "S E T T I N G S"));
		sel = ids[row];
		for (r = 0; r < 18; r++)
		{
			i = ids[r];
			at(x, 3 + r);
			if (i == sel)
				bgc(mix(theme()->bg, theme()->accent, 0.2));
			fg(i == 14 ? rgb(255, 120, 120) : theme()->text);
			outf(" %s %s%*s", i == sel ? "▶" : " ", lab[i], 33 - ulen(lab[i]), "");
			fg(theme()->accent);
			if (i < 13 || i == 15 || i == 16)
				outf("◀ %s ▶", opt_val(i, buf));
			else if (i == 17)
				outf("%s  (Entrée : détail)", opt_val(i, buf));
			else if (i == 13)
				outf("%s  (Entrée)", opt_val(i, buf));
			outf("   ");
			bgc(theme()->bg);
		}
		text_at(x, 21, theme()->dim, L("Les réglages des tests (temps, sanitizers, valgrind, audit)",
				"Test settings (time, sanitizers, valgrind, audit)"));
		if (sel == 11)
			text_at(x, 22, theme()->dim, L("La Norme reste obligatoire à l'entraînement.",
					"The Norm stays required in training."));
		else
			text_at(x, 22, theme()->dim, L("s'appliquent au prochain « Relancer les tests ».",
					"apply to the next \"Run the tests again\"."));
		foot_line(x, L("▲ ▼ choisir   ◀ ▶ changer   Échap : retour",
				"▲ ▼ pick   ◀ ▶ change   Esc: back"));
		flush();
		k = key_dir(key_read(60));
		if (key_quit(k) || k == 127 || k == 8)
			break ;
		if (k == 1000)
			row = (row + 17) % 18;
		else if (k == 1001)
			row = (row + 1) % 18;
		else if ((k == 1002 || k == 1003) && (sel < 13 || sel == 15 || sel == 16))
		{
			opt_change(sel, k == 1002 ? 1 : -1);
			save_write();
			cls();
		}
		else if ((k == '\r' || k == '\n' || k == ' ') && sel == 17)
		{
			screen_rendu();
			cls();
		}
		else if ((k == '\r' || k == '\n' || k == ' ') && sel == 13)
		{
			screen_login();
			cls();
		}
		else if ((k == '\r' || k == '\n' || k == ' ') && sel == 14)
		{
			if (confirm(L("Tout effacer ? XP, Octets, objets, boss, entraînement...",
						"Erase everything? XP, Octets, items, bosses, training..."))
				&& confirm(L("Vraiment ? C'est définitif.", "Really? This is final.")))
			{
				t_cfg	keep;
				char	login[32];

				keep = g_save.cfg;
				snprintf(login, sizeof(login), "%s", g_save.login);
				memset(g_save.ach, '0', 32);
				g_save.xp = 0;
				g_save.octets = 0;
				g_save.runs = 0;
				g_save.boss[0] = 0;
				g_save.boss[1] = 0;
				g_save.boss[2] = 0;
				memset(g_save.exo, '0', NEXO);
				memset(g_save.cleared, '0', NFN);
				memset(g_save.titles, '0', NTITLE);
				g_save.titles[1] = '1';
				g_save.title = 1;
				memset(g_save.themes, '0', 15);
				g_save.themes[0] = '1';
				strcpy(g_save.theme, "nuit");
				for (i = 0; i < g_nitems; i++)
					g_save.items[i] = g_items[i].price == 0 ? '1' : '0';
				look_default(&g_save.look);
				g_save.cfg = keep;
				snprintf(g_save.login, sizeof(g_save.login), "%s", login);
				save_write();
			}
			cls();
		}
	}
	cls();
}

int	ask_text(const char *q, const char *hint, char *buf, int size)
{
	const char	*shown;
	int			n;
	int			k;
	int			w;
	long		t;

	n = strlen(buf);
	w = term_cols() - 8;
	if (w > 76)
		w = 76;
	cls();
	while (1)
	{
		t = now_ms();
		center(5, theme()->accent2, q);
		center(6, theme()->dim, hint);
		at((term_cols() - w) / 2, 8);
		bgc(mix(theme()->bg, theme()->accent, 0.15));
		fg(theme()->text);
		shown = n > w - 3 ? buf + n - (w - 3) : buf;
		outf(" %-*s%s ", w - 3, shown, (t / 400) % 2 ? "▏" : " ");
		bgc(theme()->bg);
		center(11, theme()->dim, L("Entrée : valider   Échap : annuler", "Enter: confirm   Esc: cancel"));
		flush();
		k = key_read(60);
		if (k == 27)
			return (0);
		if ((k == '\r' || k == '\n') && n > 0)
			return (1);
		if ((k == 127 || k == 8) && n > 0)
			buf[--n] = '\0';
		else if (k >= 32 && k < 127 && n < size - 1)
		{
			buf[n++] = k;
			buf[n] = '\0';
		}
	}
}

static void	path_expand(const char *in, char *out, size_t sz)
{
	const char	*h;

	h = getenv("HOME");
	if (in[0] == '~' && (in[1] == '/' || !in[1]) && h)
		snprintf(out, sz, "%s%s", h, in + 1);
	else
		snprintf(out, sz, "%s", in);
}

static int	connect_file(const char *path, int ask)
{
	static t_save	p;
	char			q[160];
	int				r;

	r = save_probe(path, &p);
	if (r)
		return (r);
	if ((g_save.runs > 0 || g_save.xp > 0) && strcmp(p.login, g_save.login))
		return (-3);
	if (ask && (g_save.runs > 0 || g_save.xp > 0))
	{
		snprintf(q, sizeof(q), L("Remplacer ta progression (niveau %d) par celle du fichier (niveau %d) ?",
				"Replace your progress (level %d) with the file's (level %d)?"),
			level_of(g_save.xp), level_of(p.xp));
		if (!confirm(q))
			return (1);
	}
	save_adopt(&p);
	return (0);
}

static void	connect_report(int r, const char *login)
{
	char	tmp[200];

	if (r == 0)
	{
		snprintf(tmp, sizeof(tmp), L("Bienvenue, %s · niveau %d", "Welcome, %s · level %d"),
			g_save.login, level_of(g_save.xp));
		toast(L("CONNECTÉ", "CONNECTED"), tmp, rgb(80, 230, 140));
	}
	else if (r == -1)
		toast(L("FICHIER ILLISIBLE", "UNREADABLE FILE"),
			L("introuvable ou non lisible", "not found or not readable"), rgb(255, 90, 100));
	else if (r == -2)
		toast(L("SAUVEGARDE MODIFIÉE", "SAVE EDITED"),
			L("la somme de contrôle ne correspond pas : refusée", "checksum mismatch: refused"),
			rgb(255, 90, 100));
	else if (r == -3)
	{
		snprintf(tmp, sizeof(tmp), L("ce fichier est celui de %s, tu es %s",
				"this file belongs to %s, you are %s"), login, g_save.login);
		toast(L("AUTRE LOGIN : REFUSÉ", "OTHER LOGIN: REFUSED"), tmp, rgb(255, 90, 100));
	}
}

int	save_connect_cli(const char *path)
{
	static t_save	p;
	char			full[600];
	int				r;

	path_expand(path, full, sizeof(full));
	r = connect_file(full, 1);
	if (r == -3)
	{
		save_probe(full, &p);
		connect_report(r, p.login);
	}
	else if (r != 1)
		connect_report(r, "");
	return (r);
}

static void	save_screen_export(void)
{
	char	raw[560];
	char	full[600];
	int		r;

	snprintf(raw, sizeof(raw), "~/testeur_des_fous_save.txt");
	if (!ask_text(L("Où copier ta sauvegarde ?", "Where to copy your save?"),
			L("Un chemin de fichier, par exemple ~/ma_sauvegarde.txt",
				"A file path, for example ~/my_save.txt"), raw, sizeof(raw)))
		return ;
	path_expand(raw, full, sizeof(full));
	r = save_export(full, 0);
	if (r == -2 && confirm(L("Ce fichier existe déjà. L'écraser ?", "That file already exists. Overwrite it?")))
		r = save_export(full, 1);
	if (r == 0)
		toast(L("SAUVEGARDE EXPORTÉE", "SAVE EXPORTED"), full, rgb(80, 230, 140));
	else if (r == -2)
		toast(L("ANNULÉ", "CANCELLED"), L("rien n'a été copié", "nothing was copied"), theme()->accent);
	else
		toast(L("ÉCHEC DE LA COPIE", "COPY FAILED"),
			L("vérifie le chemin et les droits", "check the path and permissions"), rgb(255, 90, 100));
}

static void	save_screen_load(void)
{
	static t_save	p;
	char			raw[560];
	char			full[600];
	int				r;

	raw[0] = '\0';
	if (!ask_text(L("Quel fichier de sauvegarde charger ?", "Which save file to load?"),
			L("Le chemin de ton save.txt copié (par exemple ~/ma_sauvegarde.txt)",
				"The path of your copied save.txt (for example ~/my_save.txt)"), raw, sizeof(raw)))
		return ;
	path_expand(raw, full, sizeof(full));
	r = connect_file(full, 1);
	if (r == -3)
	{
		save_probe(full, &p);
		connect_report(r, p.login);
	}
	else if (r != 1)
		connect_report(r, "");
}

void	screen_save(void)
{
	static int	sel;
	const char	*it[3];
	char		tmp[300];
	int			k;
	int			i;
	int			x;

	it[0] = L("Exporter ma sauvegarde", "Export my save");
	it[1] = L("Charger un fichier de sauvegarde (se connecter)", "Load a save file (log in)");
	it[2] = L("Retour", "Back");
	cls();
	while (1)
	{
		x = (term_cols() - 72) / 2;
		if (x < 0)
			x = 0;
		center(1, theme()->accent2, L("S A U V E G A R D E   E T   C O N N E X I O N",
				"S A V E   A N D   L O G I N"));
		snprintf(tmp, sizeof(tmp), L("Connecté : %s · niveau %d · %ld ◈", "Logged in: %s · level %d · %ld ◈"),
			g_save.login, level_of(g_save.xp), g_save.octets);
		center(3, theme()->text, tmp);
		for (i = 0; i < 3; i++)
		{
			at(x + 6, 6 + i * 2);
			if (i == sel)
			{
				bgc(mix(theme()->bg, theme()->accent, 0.2));
				fg(theme()->accent2);
			}
			else
				fg(theme()->text);
			outf(" %s %-52s ", i == sel ? "▶" : " ", it[i]);
			bgc(theme()->bg);
		}
		snprintf(tmp, sizeof(tmp), L("Ta sauvegarde : %s/save.txt (dans ton dossier personnel, pas dans le dossier du testeur).",
				"Your save: %s/save.txt (in your home folder, not in the tester folder)."), g_savedir);
		text_clip(x, 13, theme()->dim, tmp, term_cols() - x - 1);
		text_clip(x, 14, theme()->dim, L("Tu peux supprimer le dossier du testeur : ta progression reste ici.",
				"You can delete the tester folder: your progress stays here."), term_cols() - x - 1);
		text_clip(x, 15, theme()->dim, L("Pour la garder ailleurs ou sur un autre poste : exporte-la, puis charge-la.",
				"To keep it elsewhere or on another machine: export it, then load it."), term_cols() - x - 1);
		text_clip(x, 16, theme()->dim, L("Un fichier modifié à la main ou d'un autre login est refusé.",
				"A hand-edited file, or one from another login, is refused."), term_cols() - x - 1);
		foot_line(x, L("▲ ▼ choisir   Entrée : valider   Échap : retour",
				"▲ ▼ pick   Enter: confirm   Esc: back"));
		flush();
		k = key_read(60);
		if (key_quit(k))
			break ;
		if (k == 1000 || k == 'k')
			sel = (sel + 2) % 3;
		else if (k == 1001 || k == 'j')
			sel = (sel + 1) % 3;
		else if (k == '\r' || k == '\n' || k == ' ')
		{
			if (sel == 2)
				break ;
			if (sel == 0)
				save_screen_export();
			else
				save_screen_load();
			cls();
		}
	}
	cls();
}

#ifndef TDF_PRINTF
void	screen_boss_menu(void)
{
	static int	sel;
	t_canvas	cv;
	t_scene_out	g;
	int			k;
	int			i;
	int			x;
	long		t;
	char		tmp[200];

	cls();
	while (1)
	{
		t = now_ms();
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		frame();
		center(0, theme()->accent2, L("L E S   B O S S", "T H E   B O S S E S"));
		cv_init(&cv, 40, 36);
		if (!gardien_sprite(&cv, sel, 0, (int)((t / 450) % 2), g_boss_spr[sel].w, g_boss_spr[sel].h, 0))
			cv_spr(&cv, &g_boss_spr[sel], 0, (int)((t / 450) % 2), g_bosses[sel].glow,
			mix(g_bosses[sel].glow, rgb(255, 255, 255), 0.5 + 0.5 * sin(t / 160.0)),
			g_save.boss[sel] ? 0 : 0);
		cv_print(&cv, x + 38, 2, theme()->bg);
		cv_free(&cv);
		for (i = 0; i < 3; i++)
		{
			at(x, 3 + i * 3);
			if (i == sel)
				bgc(mix(theme()->bg, g_bosses[i].glow, 0.2));
			fg(g_bosses[i].glow);
			outf(" %s %-28s", i == sel ? "▶" : " ", L(g_bosses[i].name, g_bosses[i].en));
			bgc(theme()->bg);
			snprintf(tmp, sizeof(tmp), L("partie %d · %s", "part %d · %s"), i + 1,
				g_save.boss[i] ? L("VAINCU", "BEATEN") : L("invaincu", "unbeaten"));
			text_at(x + 3, 4 + i * 3, g_save.boss[i] ? rgb(80, 230, 140) : theme()->dim, tmp);
		}
		text_at(x, 13, theme()->dim, L("Un boss tombe quand toutes", "A boss falls when every"));
		text_at(x, 14, theme()->dim, L("les fonctions de sa partie", "function of its part"));
		text_at(x, 15, theme()->dim, L("passent le testeur.", "passes the tester."));
		text_at(x, 17, theme()->dim, L("Le battre débloque 7 niveaux", "Beating it unlocks 7"));
		text_at(x, 18, theme()->dim, L("d'entraînement.", "training levels."));
		foot_line(x, g_run.loaded
			? L("▲ ▼ choisir   Entrée : rejouer le combat   Échap : retour", "▲ ▼ pick   Enter: replay the fight   Esc: back")
			: L("▲ ▼ choisir   (lance « Relancer les tests » (touche r) pour combattre)   Échap : retour",
				"▲ ▼ pick   (run \"Run the tests again\" (key r) to fight)   Esc: back"));
		flush();
		k = key_dir(key_read(60));
		if (key_quit(k) || k == 127 || k == 8)
			break ;
		if (k == 1000)
			sel = (sel + 2) % 3;
		else if (k == 1001)
			sel = (sel + 1) % 3;
		else if ((k == '\r' || k == '\n' || k == ' ') && g_run.loaded)
		{
			scene_play(&g, sel, 1);
			if ((g.crashed || g.frozen) && screen_crash_choice(&g) && g.fn >= 0)
				g_quar[g.fn] = 1;
			save_write();
		}
	}
	cls();
}
#endif
