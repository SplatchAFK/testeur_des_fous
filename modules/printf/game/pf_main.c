#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fous.h"

static void	version_notice(void)
{
	char	path[600];
	char	old[32];
	char	msg[120];
	FILE	*f;

	snprintf(path, sizeof(path), "%s/version_printf.txt", g_savedir);
	old[0] = '\0';
	f = fopen(path, "r");
	if (f)
	{
		if (!fgets(old, sizeof(old), f))
			old[0] = '\0';
		fclose(f);
	}
	old[strcspn(old, "\r\n")] = '\0';
	if (!strcmp(old, TDF_VERSION))
		return ;
	if (old[0])
	{
		snprintf(msg, sizeof(msg), L("v%s  →  v%s   (voir CHANGELOG.md)", "v%s  →  v%s   (see CHANGELOG.md)"),
			old, TDF_VERSION);
		toast(L("MISE À JOUR DU TESTEUR", "TESTER UPDATED"), msg, rgb(120, 220, 255));
	}
	f = fopen(path, "w");
	if (f)
	{
		fprintf(f, "%s\n", TDF_VERSION);
		fclose(f);
	}
}

static void	summary(void)
{
	int	i;
	int	c;

	for (c = 0, i = 0; i < NCHAIN; i++)
		c += pf_chain_ok(i);
	printf("\n  \033[1;38;2;255;120;220mLe testeur des fous\033[0m · Version printf v%s · chaînes %d/9 · "
		"ft_printf %s · %s %d · %ld Octets\n", TDF_VERSION, c, pf_all_ok() ? "battu" : "debout",
		L("niveau", "level"), level_of(g_save.xp), g_save.octets);
	for (i = 0; i < g_pr.ncat; i++)
		if (g_pr.cat[i].ko)
			printf("  \033[38;2;255;90;100m✘\033[0m %-10s %d KO · %s\n", g_pr.cat[i].key, g_pr.cat[i].ko,
				g_pr.cat[i].title);
	printf("  %s\n", L("Créé et vibe-codé par Splatch (romasant)", "Created and vibe-coded by Splatch (romasant)"));
	printf("  \033[48;2;88;101;242m\033[38;2;255;255;255m DISCORD \033[0m \033[38;2;150;162;255m%s\033[0m \033[2m· %s\033[0m\n\n",
		TDF_DISCORD, L("Lien mort ? Contacte splatch_afk sur Discord", "Dead link? Contact splatch_afk on Discord"));
}

static void	battles(int dlc, int first, const char *dlc0)
{
	int	i;
	int	ok;
	int	news;

	if (dlc)
	{
		for (news = 0, i = 0; i < NDLC; i++)
			news += g_pf.dlc[i] == '1' && dlc0[i] != '1';
		if (news)
			story_dlc();
		for (ok = 0, i = 0; i < NDLC; i++)
			ok += g_pf.dlc[i] == '1';
		if (ok >= 1)
			pf_ach(10);
		if (ok == NDLC)
			pf_ach(11);
		save_write();
		return ;
	}
	for (ok = 0, news = 0, i = 0; i < NCHAIN; i++)
	{
		ok += pf_chain_ok(i);
		news += pf_chain_ok(i) && g_pf.chains[i] != '1';
	}
	if (first || news)
		story_chains(0);
	if (ok < NCHAIN)
		return ;
	if (first || news)
		story_freed();
	if (!first && (g_pf.big || !pf_all_ok()))
		return ;
	story_preboss();
	if (!story_bigboss())
		return ;
	story_victory(pf_libft_done());
	if (pf_libft_done() && !g_pf.secret)
	{
		g_pf.secret = 1;
		pf_ach(8);
	}
}

static void	dlc_results(void)
{
	char	key[32];
	t_pcat	*c;
	int		i;

	for (i = 0; i < NDLC; i++)
	{
		snprintf(key, sizeof(key), "bonus_%s", g_dlcb[i].key);
		c = pf_cat(key);
		if (c && c->ko == 0 && c->ok > 0)
			g_pf.dlc[i] = '1';
	}
}

static int	menu_loop(void)
{
	int	i;
	int	r;

	while (1)
	{
		i = pf_menu();
		if (i == 100)
			return (42);
		if (i == 0)
		{
			r = tdf_menu_tester(L("DLC · flags et entraînement|Les flags du bonus (- 0 . # + espace) et 16 niveaux d'entraînement.", "DLC · flags and training|The bonus flags (- 0 . # + space) and 16 training levels."), "libft", "Libft");
			if (r == 1 || r == 45)
				return (r == 1 ? 42 : 45);
			if (r == 2 && pf_results(0) == 42)
				return (42);
			if (r == 3 && pf_dlc_menu() == 43)
				return (43);
		}
		else if (i == 1)
			tdf_menu_chevalier(pf_profile);
		else if (i == 2)
			tdf_menu_lore(pf_boss_menu);
		else if (i == 3)
			tdf_menu_stats();
		else if (i == 4)
			tdf_menu_params(pf_credits);
		else if (i == 5)
			return (0);
	}
}

static void	cosmetic_achs(void)
{
	int	i;

	for (i = 0; i < NSLOT; i++)
	{
		if (g_save.look.eq[i] < 0)
			continue ;
		if (!strcmp(g_items[g_save.look.eq[i]].id, "fam_moulinette"))
			pf_ach(20);
		if (!strcmp(g_items[g_save.look.eq[i]].id, "cape_format")
			|| !strcmp(g_items[g_save.look.eq[i]].id, "main_braise")
			|| !strcmp(g_items[g_save.look.eq[i]].id, "casque_pourcent"))
			pf_ach(19);
	}
}

int	main(int ac, char **av)
{
	const char	*results;
	const char	*reco;
	char		dir[512];
	int			pid;
	int			relance;
	int			record;
	int			dlc;
	int			code;
	int			i;
	int			first;
	char		dlc0[NDLC + 1];

	results = NULL;
	reco = NULL;
	pid = 0;
	relance = 0;
	record = 0;
	dlc = 0;
	for (i = 1; i < ac; i++)
	{
		if (!strcmp(av[i], "--results") && i + 1 < ac)
			results = av[++i];
		else if (!strcmp(av[i], "--pid") && i + 1 < ac)
			pid = atoi(av[++i]);
		else if (!strcmp(av[i], "--dir") && i + 1 < ac)
			g_tester_dir = av[++i];
		else if (!strcmp(av[i], "--reco") && i + 1 < ac)
			reco = av[++i];
		else if (!strcmp(av[i], "--relance"))
			relance = 1;
		else if (!strcmp(av[i], "--fast") || !strcmp(av[i], "--no-anim"))
			term_fast(1);
		else if (!strcmp(av[i], "--record"))
			record = 1;
		else if (!strcmp(av[i], "--dlc"))
			dlc = 1;
	}
	snprintf(dir, sizeof(dir), "%s/.testeur_des_fous", getenv("HOME") ? getenv("HOME") : "/tmp");
	g_savedir = dir;
	srand(getpid() ^ now_ms());
	save_load();
	cfg_export();
	if (record && results)
	{
		long off = 0;
		pf_load(results, &off);
		g_pr.done = 1;
		i = pf_record();
		printf("  progression enregistrée : %s · niveau %d · %ld Octets · %d chaîne(s) nouvelle(s)\n",
			g_save.login, level_of(g_save.xp), g_save.octets, i);
		return (0);
	}
	if (!isatty(0) || !isatty(1))
	{
		fprintf(stderr, "le testeur des fous a besoin d'un vrai terminal\n");
		return (2);
	}
	term_init();
	first = g_pf.runs == 0;
	if (!relance)
		lore_setup();
	if (!relance && first)
		story_splash();
	if (!relance)
		version_notice();
	if (g_save.valid == -1)
		toast(L("SAUVEGARDE MODIFIÉE À LA MAIN", "SAVE EDITED BY HAND"),
			L("remise à zéro (ancienne copie : save.txt.triche)", "reset (old copy: save.txt.triche)"), rgb(255, 90, 100));
	if (reco)
		save_connect_cli(reco);
	if (!relance && first)
		pf_connect_screen();
	else if (!relance && pf_connected())
		pf_grant_veteran();
	if (g_save.runs == 0 && g_pf.runs == 0)
		screen_login();
	if (results && !relance && !dlc && first)
	{
		if (pf_libft_done())
			story_previously();
		story_printf_intro();
	}
	if (results && pf_scan(results, pid))
		results = NULL;
	if (results)
	{
		memcpy(dlc0, g_pf.dlc, sizeof(dlc0));
		if (dlc)
			dlc_results();
		battles(dlc, first, dlc0);
		pf_rewards();
	}
	cosmetic_achs();
	code = 0;
	if (results && !dlc && !pf_all_ok() && pf_results(1) == 42)
		code = 42;
	if (code != 42)
		code = menu_loop();
	save_write();
	term_end();
	if (code == 42 || code == 43 || code == 45)
		return (code);
	summary();
	return (results && !pf_all_ok() ? 1 : 0);
}
