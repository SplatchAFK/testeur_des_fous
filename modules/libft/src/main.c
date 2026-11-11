#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fous.h"

static void	summary(void)
{
	int	ok;
	int	i;

	ok = 0;
	for (i = 0; i < NFN; i++)
		ok += st_pass(g_run.fn[i].st);
	printf("\n  \033[1;38;2;200;120;255mLe testeur des fous\033[0m · Version Libft v%s · %d/%d"
		" %s · %s %d · boss %d/3 · %ld Octets\n", TDF_VERSION, ok, NFN, L("fonctions OK", "functions OK"),
		L("niveau", "level"), level_of(g_save.xp), g_save.boss[0] + g_save.boss[1] + g_save.boss[2],
		g_save.octets);
	for (i = 0; i < NFN; i++)
		if (st_bad(g_run.fn[i].st))
			printf("  \033[38;2;255;90;100m✘\033[0m ft_%-14s %s%s%s\n", g_fn_name[i],
				st_label(g_run.fn[i].st), g_run.fn[i].nd ? " · " : "",
				g_run.fn[i].nd ? g_run.fn[i].detail[0] : "");
	printf("  %s\n", L("Créé et vibe-codé par Splatch (romasant)",
			"Created and vibe-coded by Splatch (romasant)"));
	printf("  \033[48;2;88;101;242m\033[38;2;255;255;255m DISCORD \033[0m \033[38;2;150;162;255m%s\033[0m\n\n",
		TDF_DISCORD);
}

static void	version_notice(void)
{
	char	path[600];
	char	old[32];
	char	msg[120];
	FILE	*f;

	snprintf(path, sizeof(path), "%s/version.txt", g_savedir);
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
		snprintf(msg, sizeof(msg), L("v%s  →  v%s   (voir CHANGELOG.md)",
				"v%s  →  v%s   (see CHANGELOG.md)"), old, TDF_VERSION);
		toast(L("MISE À JOUR DU TESTEUR", "TESTER UPDATED"), msg, rgb(120, 220, 255));
	}
	f = fopen(path, "w");
	if (f)
	{
		fprintf(f, "%s\n", TDF_VERSION);
		fclose(f);
	}
}

static int	menu_loop(void)
{
	int	i;
	int	r;

	while (1)
	{
		i = screen_menu();
		if (i == 100)
			return (42);
		if (i == 0)
		{
			r = tdf_menu_tester(L("Entraînement (21 niveaux)|Des fonctions nouvelles, hors Libft, pour t'entraîner. Débloquées par les boss.", "Training (21 levels)|New functions, beyond Libft, to train. Unlocked by bosses."), "printf", "ft_printf");
			if (r == 1 || r == 45)
				return (r == 1 ? 42 : 45);
			if (r == 2 && g_run.loaded && screen_results(0) == 42)
				return (42);
			if (r == 3)
				screen_train();
		}
		else if (i == 1)
			tdf_menu_chevalier(screen_profile);
		else if (i == 2)
			tdf_menu_lore(screen_boss_menu);
		else if (i == 3)
			tdf_menu_stats();
		else if (i == 4)
			tdf_menu_params(screen_credits);
		else if (i == 5)
			return (0);
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
	int			i;
	int			code;
	int			bad;
	int			first;

	results = NULL;
	reco = NULL;
	pid = 0;
	relance = 0;
	record = 0;
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
		else if (!strcmp(av[i], "--record"))
			record = 1;
		else if (!strcmp(av[i], "--relance"))
			relance = 1;
		else if (!strcmp(av[i], "--fast") || !strcmp(av[i], "--no-anim"))
			term_fast(1);
	}
	snprintf(dir, sizeof(dir), "%s/.testeur_des_fous",
		getenv("HOME") ? getenv("HOME") : "/tmp");
	g_savedir = dir;
	srand(getpid() ^ now_ms());
	save_load();
	cfg_export();
	lib_init();
	if (record && results)
	{
		long	off;

		off = 0;
		results_load(results, &off);
		i = record_progress();
		printf("  progression enregistrée : %s · niveau %d · %ld Octets · %d/%d fonctions propres\n",
			g_save.login, level_of(g_save.xp), g_save.octets, i, NFN);
		return (0);
	}
	if (!isatty(0) || !isatty(1))
	{
		fprintf(stderr, "le testeur des fous a besoin d'un vrai terminal\n");
		return (2);
	}
	term_init();
	first = g_save.runs == 0;
	if (!relance)
		lore_setup();
	if (!relance && first)
		screen_intro();
	if (!relance)
		version_notice();
	if (g_save.valid == -1)
		toast(L("SAUVEGARDE MODIFIÉE À LA MAIN", "SAVE EDITED BY HAND"),
			L("remise à zéro (ancienne copie : save.txt.triche)", "reset (old copy: save.txt.triche)"),
			rgb(255, 90, 100));
	if (reco)
		save_connect_cli(reco);
	if (g_save.runs == 0)
		screen_login();
	if (results && !relance && first)
		story_libft_intro();
	code = 0;
	if (results)
	{
		screen_scan(results, pid);
		if (screen_results(1) == 42)
		{
			rewards_quiet();
			code = 42;
		}
		else
		{
			boss_battles(first ? -1 : -2);
			if (first || rewards_news())
				screen_rewards();
			else
				rewards_quiet();
		}
	}
	if (code != 42)
		code = menu_loop();
	save_write();
	term_end();
	if (code == 42 || code == 45)
		return (code);
	summary();
	bad = 0;
	for (i = 0; i < NFN; i++)
		bad += st_bad(g_run.fn[i].st) || g_run.fn[i].st == ST_WAIT
			|| g_run.fn[i].proto != 1;
	return (results && bad ? 1 : 0);
}
