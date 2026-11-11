#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fous.h"

t_prun	g_pr;
t_psave	g_pf;

const t_chain	g_chains[NCHAIN] = {
	{"c", "%c", "le Caractère solitaire", "the Lonely Character", {120, 220, 255}},
	{"s", "%s", "la Longue Chaîne", "the Long String", {255, 210, 90}},
	{"p", "%p", "l'Adresse perdue", "the Lost Address", {200, 140, 255}},
	{"d", "%d", "le Signé", "the Signed", {255, 120, 120}},
	{"i", "%i", "le Jumeau", "the Twin", {255, 160, 110}},
	{"u", "%u", "le Sans-Signe", "the Unsigned", {120, 255, 170}},
	{"x", "%x", "le Petit Hexa", "the Little Hex", {110, 200, 255}},
	{"X", "%X", "le Grand Hexa", "the Big Hex", {90, 150, 255}},
	{"%", "%%", "le Pourcent", "the Percent", {255, 120, 220}},
};

const t_phase	g_phases[NPHASE] = {
	{"La Norme", "The Norm", "", "norminette"},
	{"Makefile, en-tête et README", "Makefile, header and README", "", "Makefile|ft_printf.h|README.md"},
	{"Fonctions interdites", "Forbidden functions", "", "fonctions externes"},
	{"Mélanges et valeur de retour", "Mixes and return value", "mix,ret", ""},
	{"Bonus : cas limites et write en échec", "Bonus: edge cases and failing write", "limites,write", ""},
	{"malloc qui échoue", "Failing malloc", "malloc", ""},
	{"Le Dévoreur : 10 000 appels", "The Devourer: 10,000 calls", "stress", ""},
};

const t_chain	g_dlcb[NDLC] = {
	{"minus", "-", "le Gaucher", "the Left-hander", {255, 150, 90}},
	{"zero", "0", "le Remplisseur", "the Filler", {200, 200, 210}},
	{"width", "8", "le Géant", "the Giant", {120, 220, 160}},
	{"prec", ".", "le Précis", "the Precise", {120, 180, 255}},
	{"hash", "#", "l'Alterné", "the Alternate", {230, 120, 255}},
	{"sign", "+ ", "le Signataire", "the Signer", {255, 220, 100}},
};

const char	*g_pach_fr[NPACH][2] = {
	{"Premier maillon", "Briser une première chaîne."},
	{"Neuf chaînes", "Briser les 9 chaînes."},
	{"Chocolat chaud", "Battre ft_printf."},
	{"Du premier coup", "Battre ft_printf dès ton premier run."},
	{"Pourcent parfait", "Briser la chaîne %%."},
	{"L'adresse retrouvée", "Briser la chaîne %p."},
	{"INT_MIN apprivoisé", "Briser les chaînes %d et %i."},
	{"Hexadécimal", "Briser les chaînes %x et %X."},
	{"Fin secrète", "Voir la fin secrète."},
	{"Vétéran", "Te connecter avec une libft validée."},
	{"Premier flag", "Battre un boss du DLC."},
	{"DLC complet", "Battre les 6 boss du DLC."},
	{"Apprenti variadique", "Réussir 4 niveaux d'entraînement."},
	{"Compagnon du format", "Réussir 8 niveaux d'entraînement."},
	{"Maître du format", "Réussir les 16 niveaux d'entraînement."},
	{"Têtu", "Faire 10 runs."},
	{"Marathon printf", "Faire 30 runs."},
	{"Noctambule du format", "Un run entre minuit et 5 h."},
	{"Riche", "Avoir 2000 Octets."},
	{"Styliste", "Équiper un objet printf."},
	{"Fan de Moulinette", "Équiper le familier Moulinette."},
	{"Zéro avertissement", "Un run sans aucun avertissement."},
	{"Norme d'acier", "Norminette sans erreur."},
	{"README modèle", "Un README conforme au sujet."},
	{"Holygraph doré", "Libft et Printf validés."},
};

const char	*g_pach_en[NPACH][2] = {
	{"First link", "Break a first chain."},
	{"Nine chains", "Break all 9 chains."},
	{"Hot chocolate", "Beat ft_printf."},
	{"First try", "Beat ft_printf on your first run."},
	{"Perfect percent", "Break the %% chain."},
	{"Address found", "Break the %p chain."},
	{"INT_MIN tamed", "Break the %d and %i chains."},
	{"Hexadecimal", "Break the %x and %X chains."},
	{"Secret ending", "See the secret ending."},
	{"Veteran", "Log in with a validated libft."},
	{"First flag", "Beat a DLC boss."},
	{"Full DLC", "Beat the 6 DLC bosses."},
	{"Variadic apprentice", "Pass 4 training levels."},
	{"Format companion", "Pass 8 training levels."},
	{"Format master", "Pass all 16 training levels."},
	{"Stubborn", "Do 10 runs."},
	{"printf marathon", "Do 30 runs."},
	{"Format night owl", "A run between midnight and 5 am."},
	{"Rich", "Own 2000 Octets."},
	{"Stylist", "Equip a printf item."},
	{"Moulinette fan", "Equip the Moulinette pet."},
	{"Zero warning", "A run without any warning."},
	{"Steel Norm", "Norminette without errors."},
	{"Model README", "A README that follows the subject."},
	{"Golden holygraph", "Libft and Printf validated."},
};

const t_pexo	g_pexos[NPEXO] = {
	{"ft_putchar_count", "int\tft_putchar_count(char c);",
		"Écris le caractère c sur la sortie standard. Renvoie le nombre d'octets écrits (1), ou -1 si write échoue.",
		"Write c to standard output. Return the number of bytes written (1), or -1 if write fails.", "write et sa valeur de retour"},
	{"ft_putstr_count", "int\tft_putstr_count(char *s);",
		"Écris s. Si s vaut NULL, écris \"(null)\" comme printf sous Linux. Renvoie le nombre d'octets écrits, -1 si write échoue.",
		"Write s. If s is NULL, write \"(null)\" like printf on Linux. Return the number of bytes written, -1 if write fails.", "chaîne NULL"},
	{"ft_putnbr_count", "int\tft_putnbr_count(int n);",
		"Écris n en base 10 (INT_MIN compris, sans comportement indéfini). Renvoie le nombre de caractères écrits.",
		"Write n in base 10 (INT_MIN included, no undefined behavior). Return the number of characters written.", "INT_MIN sans dépassement"},
	{"ft_putbase_count", "int\tft_putbase_count(unsigned long n, char *base);",
		"Écris n dans la base donnée (base = \"0123456789abcdef\" par exemple). Renvoie le nombre de caractères écrits.",
		"Write n in the given base (base = \"0123456789abcdef\" for example). Return the number of characters written.", "conversion de base"},
	{"ft_puthex_count", "int\tft_puthex_count(unsigned int n, int upper);",
		"Écris n en hexadécimal, en majuscules si upper vaut 1. Renvoie le nombre de caractères écrits.",
		"Write n in hexadecimal, uppercase if upper is 1. Return the number of characters written.", "hexadécimal et casse"},
	{"ft_putptr_count", "int\tft_putptr_count(void *p);",
		"Écris l'adresse p comme %p de printf sous Linux (0x... ; NULL donne \"(nil)\"). Renvoie le nombre de caractères écrits.",
		"Write the address p like printf's %p on Linux (0x... ; NULL gives \"(nil)\"). Return the number of characters written.", "pointeurs et unsigned long"},
	{"ft_is_conv", "int\tft_is_conv(char c);",
		"Renvoie 1 si c est une conversion du sujet (cspdiuxX%), 0 sinon.",
		"Return 1 if c is a conversion from the subject (cspdiuxX%), 0 otherwise.", "lecture du format"},
	{"ft_count_conv", "int\tft_count_conv(const char *fmt);",
		"Compte les conversions de fmt (« %% » compte pour une). Un '%' tout seul à la fin ne compte pas. Ne lis jamais après le '\\0'.",
		"Count the conversions in fmt (\"%%\" counts as one). A lone '%' at the end does not count. Never read past the '\\0'.", "le '%' final"},
	{"ft_sum", "int\tft_sum(int n, ...);",
		"Renvoie la somme des n entiers qui suivent n. Utilise va_start, va_arg et va_end.",
		"Return the sum of the n integers after n. Use va_start, va_arg and va_end.", "fonctions variadiques"},
	{"ft_max", "int\tft_max(int n, ...);",
		"Renvoie le plus grand des n entiers qui suivent n (n >= 1).",
		"Return the largest of the n integers after n (n >= 1).", "va_arg en boucle"},
	{"ft_strjoin_n", "char\t*ft_strjoin_n(int n, ...);",
		"Renvoie une nouvelle chaîne (malloc de la taille exacte) qui colle les n chaînes qui suivent n. NULL si malloc échoue.",
		"Return a new string (malloc of the exact size) joining the n strings after n. NULL if malloc fails.", "deux passages sur les arguments"},
	{"ft_sum_twice", "int\tft_sum_twice(int n, ...);",
		"Lis les n entiers deux fois grâce à va_copy : renvoie (somme des n entiers) multipliée par (nombre d'entiers pairs).",
		"Read the n integers twice with va_copy: return (sum of the n integers) times (number of even integers).", "va_copy"},
	{"ft_putstr_fd_count", "int\tft_putstr_fd_count(int fd, char *s);",
		"Écris s sur fd. Renvoie le nombre d'octets écrits, ou -1 si un write échoue (fd fermé par exemple).",
		"Write s to fd. Return the number of bytes written, or -1 if a write fails (closed fd for example).", "descripteurs et erreurs"},
	{"ft_utoa_base", "char\t*ft_utoa_base(unsigned long n, const char *base);",
		"Renvoie n écrit dans base, dans une chaîne allouée à la taille exacte. NULL si malloc échoue.",
		"Return n written in base, in a string allocated to the exact size. NULL if malloc fails.", "malloc de taille exacte"},
	{"ft_vprintf_mini", "int\tft_vprintf_mini(const char *fmt, va_list ap);",
		"Comme printf avec seulement %d, %s et %%, mais reçoit une va_list déjà démarrée (comme vprintf). Renvoie le nombre d'octets écrits.",
		"Like printf with only %d, %s and %%, but takes an already started va_list (like vprintf). Return the number of bytes written.", "passer une va_list"},
	{"ft_dprintf_mini", "int\tft_dprintf_mini(int fd, const char *fmt, ...);",
		"Comme dprintf avec seulement %c, %s et %d : écrit sur fd. Renvoie le nombre d'octets écrits, -1 si write échoue.",
		"Like dprintf with only %c, %s and %d: writes to fd. Return the number of bytes written, -1 if write fails.", "dprintf"},
};

void	game_hold(int restore)
{
	static t_psave	keep;

	if (restore)
		g_pf = keep;
	else
		keep = g_pf;
}

void	game_reset(void)
{
	memset(&g_pf, 0, sizeof(g_pf));
	memset(g_pf.chains, '0', NCHAIN);
	memset(g_pf.dlc, '0', NDLC);
	memset(g_pf.exo, '0', NPEXO);
	memset(g_pf.ach, '0', NPACH);
}

static void	bits(char *dst, const char *v, int n)
{
	int	i;

	for (i = 0; i < n && v[i]; i++)
		dst[i] = v[i] == '1' ? '1' : '0';
}

void	game_kv(const char *k, const char *v)
{
	if (!strcmp(k, "runs"))
		g_pf.runs = atoi(v);
	else if (!strcmp(k, "chains"))
		bits(g_pf.chains, v, NCHAIN);
	else if (!strcmp(k, "big"))
		g_pf.big = atoi(v);
	else if (!strcmp(k, "dlc"))
		bits(g_pf.dlc, v, NDLC);
	else if (!strcmp(k, "exo"))
		bits(g_pf.exo, v, NPEXO);
	else if (!strcmp(k, "ach"))
		bits(g_pf.ach, v, NPACH);
	else if (!strcmp(k, "lastreward"))
		g_pf.lastreward = atol(v);
	else if (!strcmp(k, "crashes"))
		g_pf.crashes = atoi(v);
	else if (!strcmp(k, "vet"))
		g_pf.vet = atoi(v);
	else if (!strcmp(k, "secret"))
		g_pf.secret = atoi(v);
	else if (!strcmp(k, "seen"))
		g_pf.seen = atoi(v);
}

void	game_text(char *b, size_t n)
{
	snprintf(b, n, "printf.runs=%d\nprintf.chains=%s\nprintf.big=%d\nprintf.dlc=%s\nprintf.exo=%s\n"
		"printf.ach=%s\nprintf.lastreward=%ld\nprintf.crashes=%d\nprintf.vet=%d\nprintf.secret=%d\n"
		"printf.seen=%d\n", g_pf.runs, g_pf.chains, g_pf.big, g_pf.dlc, g_pf.exo, g_pf.ach,
		g_pf.lastreward, g_pf.crashes, g_pf.vet, g_pf.secret, g_pf.seen);
}

t_pcat	*pf_cat(const char *key)
{
	int	i;

	for (i = 0; i < g_pr.ncat; i++)
		if (!strcmp(g_pr.cat[i].key, key))
			return (&g_pr.cat[i]);
	return (NULL);
}

int	pf_chain_ko(int i)
{
	t_pcat	*c;

	c = pf_cat(g_chains[i].key);
	return (c ? c->ko : -1);
}

int	pf_chain_ok(int i)
{
	t_pcat	*c;

	c = pf_cat(g_chains[i].key);
	return (c && c->ko == 0 && c->ok + c->warn > 0);
}

static int	stat_ok(const char *kw)
{
	char	w[64];
	int		found;
	int		i;

	found = 0;
	while (*kw)
	{
		snprintf(w, sizeof(w), "%.*s", (int)(strchr(kw, '|') ? strchr(kw, '|') - kw : (long)strlen(kw)), kw);
		for (i = 0; i < g_pr.nst; i++)
			if (strstr(g_pr.st[i].title, w))
			{
				found++;
				if (g_pr.st[i].st == 2)
					return (0);
			}
		kw += strlen(w);
		if (*kw == '|')
			kw++;
	}
	return (found > 0);
}

int	pf_phase_ok(int i)
{
	const char	*p;
	char		k[16];
	t_pcat		*c;
	int			n;

	if (g_phases[i].stats[0] && !stat_ok(g_phases[i].stats))
		return (0);
	p = g_phases[i].cats;
	while (*p)
	{
		n = strchr(p, ',') ? (int)(strchr(p, ',') - p) : (int)strlen(p);
		snprintf(k, sizeof(k), "%.*s", n, p);
		c = pf_cat(k);
		if (!c || c->ko)
			return (0);
		p += n + (p[n] == ',');
	}
	return (1);
}

int	pf_all_ok(void)
{
	int	i;

	if (!g_pr.loaded || !g_pr.done)
		return (0);
	for (i = 0; i < NCHAIN; i++)
		if (!pf_chain_ok(i))
			return (0);
	for (i = 0; i < NPHASE; i++)
		if (!pf_phase_ok(i))
			return (0);
	return (1);
}

int	pf_libft_done(void)
{
	int	i;

	if (badge_has(0))
		return (1);
	if (!(g_save.boss[0] && g_save.boss[1] && g_save.boss[2]))
		return (0);
	for (i = 0; i < NFN; i++)
		if (g_save.cleared[i] != '1')
			return (0);
	return (1);
}

int	pf_connected(void)
{
	return (g_save.runs > 0 || badge_has(0));
}

int	pf_ach(int i)
{
	char	tmp[200];

	if (i < 0 || i >= NPACH || g_pf.ach[i] == '1')
		return (0);
	g_pf.ach[i] = '1';
	g_save.octets += 50;
	g_save.xp += 50;
	bell();
	snprintf(tmp, sizeof(tmp), "%s · %s", L(g_pach_fr[i][0], g_pach_en[i][0]), L(g_pach_fr[i][1], g_pach_en[i][1]));
	toast(L("SUCCÈS DÉBLOQUÉ  +50 ◈", "ACHIEVEMENT UNLOCKED  +50 ◈"), tmp, theme()->accent2);
	return (1);
}

static int	stat_state(const char *kw)
{
	int	i;

	for (i = 0; i < g_pr.nst; i++)
		if (strstr(g_pr.st[i].title, kw))
			return (g_pr.st[i].st);
	return (0);
}

int	game_holy(const char *p, char *why, size_t n)
{
	int	k;
	int	i;
	int	c;

	if (!strcmp(p, "Libft"))
	{
		if (pf_libft_done() && g_pr.loaded && stat_state("libft/ :") == 1)
			return (snprintf(why, n, "%s", L("Validé : badge « Libft validée » (43/43 et 3 boss dans le testeur libft) "
						"et ta libft/ passe le contrôle rapide ici, en local.", "Validated: \"Libft validated\" badge "
						"(43/43 and 3 bosses in the libft tester) and your libft/ passes the quick check here, locally.")), 2);
		if (pf_libft_done() && g_pr.loaded && stat_state("libft/ :") == 2)
		{
			snprintf(why, n, "%s", L("Ce qui te manque : ta libft/ ne passe pas le contrôle rapide (Résultats détaillés).",
					"What you still need: your libft/ fails the quick check (Detailed results)."));
			return (1);
		}
		if (pf_libft_done() && (!g_pr.loaded || stat_state("libft/ :") == 0))
			return (snprintf(why, n, "%s", L("Validé par le badge « Libft validée » (pas de libft/ dans ta ft_printf, "
						"ou pas encore de run ici).", "Validated by the \"Libft validated\" badge (no libft/ in your "
						"ft_printf, or no run here yet).")), 2);
		if (!pf_connected())
			snprintf(why, n, "%s", L("Ce qui te manque : connecte ta sauvegarde du testeur libft (Sauvegarde et "
					"connexion), ou installe le testeur des fous libft.", "What you still need: log in with your libft "
					"tester save (Save and login), or install the libft tester."));
		else if (!pf_libft_done())
			snprintf(why, n, "%s", L("Ce qui te manque : valide libft dans le testeur libft (43/43 en local + 3 boss).",
					"What you still need: validate libft in the libft tester (43/43 locally + 3 bosses)."));
		else
			snprintf(why, n, "%s", L("Ce qui te manque : ta libft/ doit compiler avec le Makefile de ft_printf.",
					"What you still need: your libft/ must build with the ft_printf Makefile."));
		return (pf_connected() ? 1 : 0);
	}
	if (!strcmp(p, "Printf"))
	{
		if (pf_all_ok())
			return (snprintf(why, n, "%s", L("Validé : les 9 chaînes et ft_printf battus, en local, à ce run.",
						"Validated: the 9 chains and ft_printf beaten, locally, this run.")), 2);
		if (!g_pr.loaded)
			return (snprintf(why, n, "%s", L("Ce qui te manque : lance « Relancer les tests » (touche r) (la preuve se fait en local).",
						"What you still need: run \"Run the tests again\" (key r) (proof is made locally).")), badge_has(1));
		for (c = 0, i = 0; i < NCHAIN; i++)
			c += pf_chain_ok(i);
		k = snprintf(why, n, L("Ce qui te manque : chaînes %d/9", "What you still need: chains %d/9"), c);
		for (i = 0; i < NPHASE && k < (int)n - 60; i++)
			if (!pf_phase_ok(i))
				k += snprintf(why + k, n - k, " · %s", L(g_phases[i].fr, g_phases[i].en));
		return (c > 0 ? 1 : 0);
	}
	return (-1);
}

void	game_trace(void *fp)
{
	FILE	*f;
	int		i;

	f = fp;
	if (!g_pr.loaded)
		fprintf(f, "aucun test lancé pendant cette session\n");
	for (i = 0; i < g_pr.nst; i++)
		fprintf(f, "statique : %s · %s · %s\n", g_pr.st[i].st == 1 ? "OK" : (g_pr.st[i].st == 2 ? "KO" : "~~"),
			g_pr.st[i].title, g_pr.st[i].detail);
	for (i = 0; i < g_pr.ncat; i++)
		fprintf(f, "catégorie %-8s %4d OK %4d KO %4d avert. · %s\n", g_pr.cat[i].key, g_pr.cat[i].ok,
			g_pr.cat[i].ko, g_pr.cat[i].warn, g_pr.cat[i].title);
	for (i = 0; i < g_pr.nca; i++)
		fprintf(f, "cause [%s] %s (%d cas)\n", g_pr.ca[i].cat, g_pr.ca[i].title, g_pr.ca[i].count);
	for (i = 0; i < g_pr.ncs && i < 60; i++)
		fprintf(f, "cas [%s] %s %s\n", g_pr.cs[i].cat, g_pr.cs[i].ko ? "KO" : "avert.", g_pr.cs[i].desc);
	fprintf(f, "total : %d OK, %d KO, %d avertissements (%s)\n", g_pr.tok, g_pr.tko, g_pr.twarn, g_pr.mode);
}

void	pf_badges(int quiet)
{
	char		keep[NBADGE + 1];
	time_t		t;
	struct tm	*lt;
	int			i;
	int			n;

	memcpy(keep, g_save.badges, NBADGE);
	if (pf_all_ok())
		g_save.badges[1] = '1';
	if (g_pr.done && g_pr.loaded && g_pr.tko == 0 && pf_all_ok())
		g_save.badges[2] = '1';
	if (pf_all_ok() && g_pf.runs == 1)
		g_save.badges[3] = '1';
	if (stat_state("norminette") == 1)
		g_save.badges[4] = '1';
	if (g_save.runs + g_pf.runs >= 50)
		g_save.badges[5] = '1';
	t = time(NULL);
	lt = localtime(&t);
	if (g_pr.loaded && lt && lt->tm_hour < 5)
		g_save.badges[6] = '1';
	if (pf_libft_done())
		g_save.badges[7] = '1';
	for (n = 0, i = 0; i < g_nitems; i++)
		n += item_owned(i);
	if (n >= 20)
		g_save.badges[8] = '1';
	if (pf_exo_done() >= NPEXO)
		g_save.badges[9] = '1';
	for (i = 0; i < NBADGE && !quiet; i++)
		if (keep[i] != '1' && g_save.badges[i] == '1')
		{
			g_save.badges[i] = '0';
			badge_give(i);
		}
}
