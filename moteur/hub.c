#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/utsname.h>
#include "fous.h"

const t_badge	g_badges[NBADGE] = {
	{"Libft validée", "Libft validated", "43/43 en local et les 3 boss battus",
		"43/43 locally and the 3 bosses beaten", {80, 230, 140}},
	{"Printf validé", "Printf validated", "ft_printf battu dans le testeur printf",
		"ft_printf beaten in the printf tester", {255, 120, 220}},
	{"Zéro fuite", "Zero leak", "un run complet sans aucune fuite",
		"a full run without a single leak", {120, 200, 255}},
	{"Premier essai", "First try", "tout propre dès le tout premier run",
		"everything clean on the very first run", {255, 215, 0}},
	{"Norme impeccable", "Spotless Norm", "norminette sans aucune erreur",
		"norminette without a single error", {170, 255, 120}},
	{"Marathonien", "Marathoner", "50 runs", "50 runs", {255, 160, 80}},
	{"Noctambule", "Night owl", "un run entre minuit et 5 h", "a run between midnight and 5 am",
		{150, 140, 255}},
	{"Vétéran de la Libft", "Libft veteran", "libft validée, puis printf lancé",
		"libft validated, then printf launched", {255, 200, 120}},
	{"Collectionneur", "Collector", "20 objets possédés", "20 items owned", {200, 160, 255}},
	{"Maître du DLC", "DLC master", "les 21 niveaux du DLC", "all 21 DLC levels", {255, 90, 160}},
	{"Connecté", "Connected", "une sauvegarde rechargée", "a save reloaded", {90, 220, 220}},
	{"Aidant", "Helper", "une trace créée pour « Nous aider »", "a trace made for \"Help us\"",
		{255, 240, 160}},
};

int	badge_has(int i)
{
	return (i >= 0 && i < NBADGE && g_save.badges[i] == '1');
}

int	badge_give(int i)
{
	char	tmp[160];

	if (i < 0 || i >= NBADGE || badge_has(i))
		return (0);
	g_save.badges[i] = '1';
	snprintf(tmp, sizeof(tmp), "%s · %s", L(g_badges[i].fr, g_badges[i].en),
		L(g_badges[i].dfr, g_badges[i].den));
	toast(L("NOUVEAU BADGE", "NEW BADGE"), tmp, g_badges[i].c);
	return (1);
}

#ifndef TDF_PRINTF
static int	norm_ok(void)
{
	int	i;

	for (i = 0; i < g_run.naudit; i++)
		if (strstr(g_run.audit[i].label, "norminette"))
			return (g_run.audit[i].st == ST_OK);
	return (0);
}

void	badges_update(int quiet)
{
	int		i;
	int		ok;
	int		leak;
	int		n;
	time_t	t;
	struct tm	*lt;
	char	keep[NBADGE + 1];

	memcpy(keep, g_save.badges, NBADGE);
	keep[NBADGE] = '\0';
	ok = 0;
	leak = 0;
	for (i = 0; i < NFN; i++)
	{
		ok += st_pass(g_run.fn[i].st);
		leak += g_run.fn[i].st == ST_LEAK;
	}
	if (g_run.loaded && ok == NFN && g_save.boss[0] && g_save.boss[1] && g_save.boss[2])
		g_save.badges[0] = '1';
	if (g_run.loaded && ok == NFN && !leak)
		g_save.badges[2] = '1';
	if (g_run.loaded && ok == NFN && g_save.runs == 1)
		g_save.badges[3] = '1';
	if (g_run.loaded && norm_ok())
		g_save.badges[4] = '1';
	if (g_save.runs >= 50)
		g_save.badges[5] = '1';
	t = time(NULL);
	lt = localtime(&t);
	if (g_run.loaded && lt && lt->tm_hour < 5)
		g_save.badges[6] = '1';
	for (n = 0, i = 0; i < g_nitems; i++)
		n += item_owned(i);
	if (n >= 20)
		g_save.badges[8] = '1';
	if (exo_done_count() >= NEXO)
		g_save.badges[9] = '1';
	for (i = 0; i < NBADGE && !quiet; i++)
		if (keep[i] != '1' && g_save.badges[i] == '1')
		{
			g_save.badges[i] = '0';
			badge_give(i);
		}
}
#endif

void	discord_card(int x, int y)
{
	t_rgb	b;
	t_rgb	in;

	b = rgb(88, 101, 242);
	in = mix(theme()->bg, b, 0.2);
	fill(x, y, DISCORD_W, 5, in);
	bgc(in);
	box(x, y, DISCORD_W, 5, b);
	text_at(x + 2, y, rgb(190, 198, 255), L(" DISCORD · LE SERVEUR DES FOUS ", " DISCORD · THE FOUS SERVER "));
	text_at(x + 3, y + 1, theme()->text, L("Entraide, bugs, idées : rejoins-nous", "Help, bugs, ideas: join us"));
	text_at(x + 3, y + 2, rgb(255, 255, 255), "◆ " TDF_DISCORD);
	text_at(x + 3, y + 3, rgb(190, 198, 255), L("Lien mort ? Contacte splatch_afk sur Discord", "Dead link? Contact splatch_afk on Discord"));
	bgc(theme()->bg);
}

void	discord_line(int y)
{
	int	x;

	x = (term_cols() - 13 - ulen(TDF_DISCORD) - ulen(L("Lien mort ? Contacte splatch_afk sur Discord", "Dead link? Contact splatch_afk on Discord"))) / 2;
	at(x < 0 ? 0 : x, y);
	bgc(rgb(88, 101, 242));
	fg(rgb(255, 255, 255));
	out(" DISCORD ");
	bgc(theme()->bg);
	fg(rgb(150, 162, 255));
	out(" " TDF_DISCORD);
	fg(theme()->dim);
	out(" · ");
	out(L("Lien mort ? Contacte splatch_afk sur Discord", "Dead link? Contact splatch_afk on Discord"));
}

static void	choisir_langue(void)
{
	int	k;
	int	sel;
	int	i;
	int	x;

	sel = langue_auto();
	cls();
	while (1)
	{
		x = (term_cols() - 30) / 2;
		if (x < 0)
			x = 0;
		center(3, theme()->accent2, "L E   T E S T E U R   D E S   F O U S");
		center(5, theme()->text, "Langue · Language · Idioma · Sprache · Lingua · Dil · Язык");
		for (i = 0; i < NLANG; i++)
		{
			at(x, 8 + i * 2);
			bgc(i == sel ? mix(theme()->bg, theme()->accent, 0.25) : theme()->bg);
			fg(i == sel ? theme()->accent2 : theme()->text);
			outf(" %s %-24s ", i == sel ? "▶" : " ", langue_nom(i));
		}
		bgc(theme()->bg);
		foot_line(x, "▲ ▼   Enter");
		flush();
		k = key_dir(key_read(80));
		if (k == 1000 || k == 1003)
			sel = (sel + NLANG - 1) % NLANG;
		else if (k == 1001 || k == 1002)
			sel = (sel + 1) % NLANG;
		else if (k == '\r' || k == '\n' || k == ' ')
			break ;
	}
	g_save.cfg.lang = sel;
	cls();
}

void	lore_setup(void)
{
	int	k;
	int	sel;
	int	x;

	if (g_save.cfg.lore == 1 || g_save.cfg.lore == 2)
		return ;
	choisir_langue();
	if (getenv("TDF_INTERFACE") && (!strcmp(getenv("TDF_INTERFACE"), "graphique")
			|| !strcmp(getenv("TDF_INTERFACE"), "texte")))
	{
		g_save.cfg.lore = strcmp(getenv("TDF_INTERFACE"), "texte") ? 1 : 2;
		save_write();
		return ;
	}
	sel = 0;
	cls();
	while (1)
	{
		x = (term_cols() - 70) / 2;
		if (x < 0)
			x = 0;
		center(3, theme()->accent2, "L E   T E S T E U R   D E S   F O U S");
		center(5, theme()->text, L("Première fois ici. Une seule question avant de commencer :",
				"First time here. Just one question before we start:"));
		center(7, rgb(255, 215, 120), L("Veux-tu l'histoire (cinématiques et lore) ?",
				"Do you want the story (cutscenes and lore)?"));
		at(x + 6, 10);
		bgc(sel == 0 ? mix(theme()->bg, theme()->accent, 0.25) : theme()->bg);
		fg(sel == 0 ? theme()->accent2 : theme()->text);
		outf(" %s %-56s ", sel == 0 ? "▶" : " ", L("Oui : l'histoire, les cinématiques et les combats",
				"Yes: the story, the cutscenes and the fights"));
		at(x + 6, 12);
		bgc(sel == 1 ? mix(theme()->bg, theme()->accent, 0.25) : theme()->bg);
		fg(sel == 1 ? theme()->accent2 : theme()->text);
		outf(" %s %-56s ", sel == 1 ? "▶" : " ", L("Non : droit au but, sans cinématiques",
				"No: straight to the point, no cutscenes"));
		bgc(theme()->bg);
		text_wrap(x, 15, 19, 70, theme()->dim, L("Ce choix vaut pour toutes les versions du testeur des fous. "
				"Tu peux le changer quand tu veux dans Paramètres. Pour les tests seuls, sans histoire : "
				"bash run.sh --no-lore.", "This choice applies to every version of the tester. You can change "
				"it anytime in Settings. For tests only, no story: bash run.sh --no-lore."));
		foot_line(x, L("▲ ▼ choisir   Entrée : valider", "▲ ▼ pick   Enter: confirm"));
		flush();
		k = key_dir(key_read(80));
		if (k == 1000 || k == 1001)
			sel = !sel;
		else if (k == 'o' || k == 'O' || k == 'y' || k == 'Y')
			sel = 0;
		else if (k == 'n' || k == 'N')
			sel = 1;
		else if (k == '\r' || k == '\n' || k == ' ')
			break ;
	}
	g_save.cfg.lore = sel ? 2 : 1;
	save_write();
	cls();
}

typedef struct s_holy
{
	const char	*name;
	const char	*proj[14];
}	t_holy;

static const t_holy	g_holy[7] = {
	{"Programming Fundamentals", {"Libft", "Printf", "Get_next_line", "Push_swap",
		"Exam · Programming Fundamentals", NULL}},
	{"Systems and Networks Administration", {"Born2beroot", "Net_practice", "Inception",
		"Exam · Basic Python Algorithms", NULL}},
	{"Python", {"Growing code", "Codecultivation", "Garden guardian", "Data quest",
		"Data archivist", "Code nexus", "The codex", "Datadeck", "The matrix", "Cosmic data",
		"Funcmage", "A_maze_ing", "Pacman", "Exam · Medium Python Algorithms"}},
	{"Algorithmics", {"Push_swap", "Fly in", "Exam · Challenging Python Algorithms", NULL}},
	{"Artificial Intelligence", {"Call_me_maybe", "Rag against the machine", "Agent Smith", NULL}},
	{"System and Network Programming", {"Codexion", "The answer protocol",
		"Exam · In-depth Python Algorithms", NULL}},
	{"Web Programming", {"ft_transcendance", NULL}},
};

static int	holy_count(int m)
{
	int	n;

	n = 0;
	while (n < 14 && g_holy[m].proj[n])
		n++;
	return (n);
}

#ifndef TDF_PRINTF
int	game_holy(const char *p, char *why, size_t n)
{
	int	ok;
	int	b;
	int	i;
	int	k;

	if (!strcmp(p, "Libft"))
	{
		for (ok = 0, i = 0; i < NFN; i++)
			ok += st_pass(g_run.fn[i].st);
		b = g_save.boss[0] + g_save.boss[1] + g_save.boss[2];
		if (g_run.loaded && ok == NFN && b == 3)
			return (snprintf(why, n, "%s", L("Validé : 43/43 en local et les 3 boss battus.",
						"Validated: 43/43 locally and the 3 bosses beaten.")), 2);
		k = snprintf(why, n, "%s", L("Ce qui te manque :", "What you still need:"));
		if (!g_run.loaded)
			k += snprintf(why + k, n - k, L(" lance « Relancer les tests » (touche r) (la preuve se fait en local) ;",
					" run \"Run the tests again\" (key r) (proof is made locally);"));
		else if (ok < NFN)
			k += snprintf(why + k, n - k, L(" 43 fonctions propres en local (%d/43 à ce run) ;",
					" 43 clean functions locally (%d/43 this run);"), ok);
		if (b < 3)
			snprintf(why + k, n - k, L(" battre les 3 boss (%d/3).", " beat the 3 bosses (%d/3)."), b);
		return (ok > 0 || b > 0 ? 1 : 0);
	}
	if (!strcmp(p, "Printf"))
	{
		if (badge_has(1))
			return (snprintf(why, n, "%s", L("Validé dans le testeur des fous printf.",
						"Validated in the printf tester.")), 2);
		snprintf(why, n, "%s", L("Ce qui te manque : installe le testeur des fous printf et bats ft_printf.",
				"What you still need: install the printf tester and beat ft_printf."));
		return (0);
	}
	return (-1);
}
#endif

static int	holy_state(const char *p, char *why, size_t n)
{
	int	k;

	k = game_holy(p, why, n);
	if (k >= 0)
		return (k);
	snprintf(why, n, "%s", L("Pas encore de testeur des fous pour ce projet.",
			"No testeur des fous for this project yet."));
	return (0);
}

static t_rgb	holy_col(int st)
{
	if (st == 2)
		return (rgb(80, 230, 140));
	if (st == 1)
		return (rgb(255, 200, 70));
	return (rgb(110, 116, 140));
}

static void	badge_wall(void)
{
	char	tmp[200];
	int		i;
	int		x;

	cls();
	while (1)
	{
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		center(1, theme()->accent2, L("B A D G E S", "B A D G E S"));
		for (i = 0; i < NBADGE; i++)
		{
			snprintf(tmp, sizeof(tmp), "%s %s", badge_has(i) ? "★" : "☆", L(g_badges[i].fr, g_badges[i].en));
			text_clip(x + (i % 2) * 40, 3 + (i / 2) * 3, badge_has(i) ? g_badges[i].c : theme()->dim, tmp, 38);
			text_clip(x + 2 + (i % 2) * 40, 4 + (i / 2) * 3, badge_has(i) ? theme()->text : theme()->dim,
				L(g_badges[i].dfr, g_badges[i].den), 36);
		}
		foot_line(x, L("Échap : retour   (vitrine : Profil, touche e)",
				"Esc: back   (showcase: Profile, key e)"));
		flush();
		if (key_read(80) >= 0)
			break ;
	}
	cls();
}

void	screen_holy(void)
{
	static int	m;
	static int	p;
	char		why[400];
	char		tmp[200];
	int			i;
	int			k;
	int			x;
	int			st;
	int			done;
	int			tot;

	cls();
	while (1)
	{
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		frame();
		center(1, theme()->accent2, L("H O L Y G R A P H   ·   4 2   N E X T", "H O L Y G R A P H   ·   4 2   N E X T"));
		for (done = 0, tot = 0, k = 0; k < 7; k++)
			for (i = 0; i < holy_count(k); i++, tot++)
				done += holy_state(g_holy[k].proj[i], why, sizeof(why)) == 2;
		snprintf(tmp, sizeof(tmp), L("Common Core · 7 modules · %d/%d validés avec les testeurs des fous",
				"Common Core · 7 modules · %d/%d validated with the testeurs des fous"), done, tot);
		center(2, theme()->dim, tmp);
		for (k = 0; k < 7; k++)
		{
			at(x, 4 + k * 2);
			if (k == m)
			{
				bgc(mix(theme()->bg, theme()->accent, 0.2));
				fg(theme()->accent2);
			}
			else
				fg(theme()->text);
			outf(" %s %-30.30s ", k == m ? "▶" : " ", g_holy[k].name);
			bgc(theme()->bg);
		}
		for (i = 0; i < holy_count(m); i++)
		{
			st = holy_state(g_holy[m].proj[i], why, sizeof(why));
			snprintf(tmp, sizeof(tmp), "%s %s", st == 2 ? "◆" : "◇", g_holy[m].proj[i]);
			at(x + 36, 4 + i);
			if (i == p)
				bgc(mix(theme()->bg, holy_col(st), 0.25));
			fg(holy_col(st));
			outf(" %-38.38s", tmp);
			bgc(theme()->bg);
		}
		if (p >= holy_count(m))
			p = 0;
		st = holy_state(g_holy[m].proj[p], why, sizeof(why));
		text_clip(x, 19, holy_col(st), g_holy[m].proj[p], 78);
		text_wrap(x, 20, term_rows() - 2, 78, theme()->text, why);
		foot_line(x, L("▲ ▼ module   ◀ ▶ projet   b : badges   Échap : retour",
				"▲ ▼ module   ◀ ▶ project   b: badges   Esc: back"));
		flush();
		k = key_dir(key_read(60));
		if (key_quit(k) || k == 127 || k == 8)
			break ;
		if (k == 1000)
			m = (m + 6) % 7;
		else if (k == 1001)
			m = (m + 1) % 7;
		else if (k == 1003)
			p = (p + holy_count(m) - 1) % holy_count(m);
		else if (k == 1002)
			p = (p + 1) % holy_count(m);
		else if (k == 'b')
			badge_wall();
		if (k == 1000 || k == 1001)
			p = 0;
	}
	cls();
}

#ifndef TDF_PRINTF
void	game_trace(void *fp)
{
	FILE	*f;
	int		i;
	int		j;

	f = fp;
	if (!g_run.loaded)
		fprintf(f, "aucun test lancé pendant cette session\n");
	for (i = 0; g_run.loaded && i < NFN; i++)
	{
		fprintf(f, "ft_%-14s %s\n", g_fn_name[i], st_label(g_run.fn[i].st));
		for (j = 0; j < g_run.fn[i].nd; j++)
			fprintf(f, "    %s\n", g_run.fn[i].detail[j]);
	}
	for (i = 0; i < g_run.naudit; i++)
		fprintf(f, "audit : %s · %s · %s\n", st_label(g_run.audit[i].st), g_run.audit[i].label,
			g_run.audit[i].detail);
}
#endif

static int	trace_write(char *path, size_t n)
{
	FILE			*f;
	struct utsname	u;
	time_t			t;

	snprintf(path, n, "%s/testeur_des_fous_trace_%s.txt", getenv("HOME") ? getenv("HOME") : "/tmp",
		TDF_GAME);
	f = fopen(path, "w");
	if (!f)
		return (-1);
	t = time(NULL);
	uname(&u);
	fprintf(f, "Le testeur des fous · trace · version %s v%s\n", TDF_GAME, TDF_VERSION);
	fprintf(f, "date : %s", ctime(&t));
	fprintf(f, "système : %s %s %s\n", u.sysname, u.release, u.machine);
	fprintf(f, "aide : %d/2 · anim : %d · langue : %s\n", g_save.cfg.help, g_save.cfg.anim,
		langue_code(g_save.cfg.lang));
	fprintf(f, "(aucun code, aucun login : seulement les résultats des tests)\n\n");
	game_trace(f);
	fclose(f);
	return (0);
}

void	screen_help_us(void)
{
	char	path[600];
	int		x;
	int		k;
	int		y;

	cls();
	while (1)
	{
		x = (term_cols() - 76) / 2;
		if (x < 0)
			x = 0;
		center(1, theme()->accent2, L("N O U S   A I D E R", "H E L P   U S"));
		y = 3;
		y = text_wrap(x, y, y + 3, 76, theme()->text, L("Le testeur des fous s'améliore grâce à vous. Que tes tests passent ou non, "
				"ta trace aide à trouver les faux positifs, les cas oubliés et les bugs du testeur.",
				"The testeur des fous gets better thanks to you. Whether your tests pass or not, your trace "
				"helps find false positives, missing cases and tester bugs.")) + 1;
		text_at(x, y++, theme()->accent, L("Comment faire", "How"));
		text_at(x, y++, theme()->text, L("1. Appuie sur t : le testeur crée ta trace dans ton dossier personnel.",
				"1. Press t: the tester writes your trace in your home folder."));
		text_at(x, y++, theme()->text, L("2. Envoie-la en message privé à @splatch_afk (serveur 42 Next).",
				"2. Send it as a direct message to @splatch_afk (42 Next server)."));
		text_at(x, y++, theme()->text, L("   Ou dépose-la sur le Discord des fous : " TDF_DISCORD,
				"   Or drop it on the fous Discord: " TDF_DISCORD));
		text_at(x, y++, theme()->dim, L("Lien mort ? Contacte splatch_afk sur Discord", "Dead link? Contact splatch_afk on Discord"));
		text_at(x, y++, theme()->text, L("3. Ajoute ce que tu veux : conseils, suggestions, idées, tout est pris.",
				"3. Add anything: advice, suggestions, ideas, everything is welcome."));
		y++;
		y = text_wrap(x, y, y + 2, 76, theme()->dim, L("La trace ne contient ni ton code, ni ton login : seulement les "
				"résultats des tests, la version du testeur et ton système. Tout reste hors ligne : rien n'est envoyé "
				"sans toi.", "The trace holds neither your code nor your login: only test results, the tester "
				"version and your system. Everything stays offline: nothing is sent without you."));
		foot_line(x, L("t : créer ma trace   Échap : retour",
				"t: create my trace   Esc: back"));
		flush();
		k = key_read(80);
		if (key_quit(k))
			break ;
		if (k == 't' || k == 'T')
		{
			if (trace_write(path, sizeof(path)) == 0)
			{
				toast(L("TRACE CRÉÉE", "TRACE CREATED"), path, rgb(80, 230, 140));
				badge_give(11);
				save_write();
			}
			else
				toast(L("ÉCHEC", "FAILED"), L("impossible d'écrire la trace", "could not write the trace"),
					rgb(255, 90, 100));
			cls();
		}
	}
	cls();
}

t_rgb	pseudo_color(void)
{
	if (g_save.pcol > 0 && g_save.pcol < 10)
		return (g_swatch[g_save.pcol]);
	return (theme()->accent2);
}

static void	vitrine_next(int slot, int d)
{
	int	v;
	int	n;

	v = g_save.vitrine[slot];
	for (n = 0; n <= NBADGE + 1; n++)
	{
		v += d;
		if (v < -1)
			v = NBADGE - 1;
		if (v >= NBADGE)
			v = -1;
		if (v == -1 || badge_has(v))
			break ;
	}
	g_save.vitrine[slot] = v;
}

void	screen_profile_edit(void)
{
	static int	row;
	const char	*lab[6];
	char		buf[120];
	int			k;
	int			i;
	int			x;

	cls();
	while (1)
	{
		lab[0] = L("Bio", "Bio");
		lab[1] = L("Couleur du pseudo", "Name color");
		lab[2] = L("Vitrine 1", "Showcase 1");
		lab[3] = L("Vitrine 2", "Showcase 2");
		lab[4] = L("Vitrine 3", "Showcase 3");
		lab[5] = L("Bannière et photo", "Banner and picture");
		x = (term_cols() - 76) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		frame();
		center(1, theme()->accent2, L("M O N   P R O F I L", "M Y   P R O F I L E"));
		text_at(x, 3, pseudo_color(), g_save.login);
		text_clip(x, 4, theme()->dim, g_save.bio[0] ? g_save.bio : L("(pas de bio)", "(no bio)"), 76);
		for (i = 0; i < 6; i++)
		{
			at(x, 6 + i * 2);
			if (i == row)
				bgc(mix(theme()->bg, theme()->accent, 0.2));
			fg(theme()->text);
			outf(" %s %-22s", i == row ? "▶" : " ", lab[i]);
			fg(theme()->accent);
			if (i == 0)
				outf("%.40s%s  (Entrée)", g_save.bio, strlen(g_save.bio) > 40 ? "…" : "");
			else if (i == 1)
			{
				fg(pseudo_color());
				outf("◀ %s ▶", g_save.pcol ? "■■■" : L("thème", "theme"));
			}
			else if (i <= 4)
				outf("◀ %s ▶", g_save.vitrine[i - 2] < 0 ? L("(vide)", "(empty)")
					: L(g_badges[g_save.vitrine[i - 2]].fr, g_badges[g_save.vitrine[i - 2]].en));
			else
				outf("%s", L("(Entrée : choisir une image)", "(Enter: pick an image)"));
			outf("   ");
			bgc(theme()->bg);
		}
		foot_line(x, L("▲ ▼ choisir   ◀ ▶ changer   Entrée : modifier   Échap : retour",
				"▲ ▼ pick   ◀ ▶ change   Enter: edit   Esc: back"));
		flush();
		k = key_dir(key_read(60));
		if (key_quit(k) || k == 127 || k == 8)
			break ;
		if (k == 1000)
			row = (row + 5) % 6;
		else if (k == 1001)
			row = (row + 1) % 6;
		else if ((k == 1002 || k == 1003) && row == 1)
			g_save.pcol = (g_save.pcol + 10 + (k == 1002 ? 1 : -1)) % 10;
		else if ((k == 1002 || k == 1003) && row >= 2 && row <= 4)
			vitrine_next(row - 2, k == 1002 ? 1 : -1);
		else if ((k == '\r' || k == '\n' || k == ' ') && row == 0)
		{
			snprintf(buf, sizeof(buf), "%s", g_save.bio);
			if (ask_text(L("Ta bio (80 caractères)", "Your bio (80 characters)"),
					L("Une phrase qui te ressemble.", "A sentence that sounds like you."), buf, 81))
				save_set_bio(buf);
		}
		else if ((k == '\r' || k == '\n' || k == ' ') && row == 5)
			screen_images();
		else
			continue ;
		save_write();
		cls();
	}
	cls();
}
