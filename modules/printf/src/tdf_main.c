#include "tdf.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

static void	usage(void)
{
	dprintf(2, "usage : tdf_runner [--only CAT[,CAT]] [--seed N] [--stress N] "
		"[--timeout S] [--bonus|--no-bonus] [-v]\n"
		"        [--project DIR] [--srclist FICHIER] [--vglogdir DIR] [--exe CHEMIN]\n"
		"        [--aide 0|1|2] [--machine FICHIER]\n");
	exit(2);
}

static void	parse(int ac, char **av)
{
	int	i;

	g_opt.stress_n = 10000;
	g_opt.help = 2;
	g_opt.timeout = TDF_VALGRIND ? 10.0 : 3.0;
	i = 0;
	while (++i < ac)
	{
		if (!strcmp(av[i], "--only") && i + 1 < ac)
			g_opt.only = av[++i];
		else if (!strcmp(av[i], "--seed") && i + 1 < ac)
		{
			g_opt.seed = (unsigned)strtoul(av[++i], NULL, 10);
			g_opt.seed_set = 1;
		}
		else if (!strcmp(av[i], "--stress") && i + 1 < ac)
			g_opt.stress_n = atoi(av[++i]);
		else if (!strcmp(av[i], "--timeout") && i + 1 < ac)
			g_opt.timeout = atof(av[++i]);
		else if (!strcmp(av[i], "--bonus"))
			g_opt.bonus = 1;
		else if (!strcmp(av[i], "--no-bonus"))
			g_opt.bonus = -1;
		else if (!strcmp(av[i], "-v"))
			g_opt.verbose = 1;
		else if (!strcmp(av[i], "--project") && i + 1 < ac)
			g_opt.project = av[++i];
		else if (!strcmp(av[i], "--srclist") && i + 1 < ac)
			g_opt.srclist = av[++i];
		else if (!strcmp(av[i], "--vglogdir") && i + 1 < ac)
			g_opt.vglogdir = av[++i];
		else if (!strcmp(av[i], "--exe") && i + 1 < ac)
			g_opt.exe = av[++i];
		else if (!strcmp(av[i], "--aide") && i + 1 < ac)
			g_opt.help = atoi(av[++i]);
		else if (!strcmp(av[i], "--machine") && i + 1 < ac)
			g_opt.machine = av[++i];
		else
			usage();
	}
	if (!g_opt.seed_set)
		g_opt.seed = (unsigned)time(NULL) ^ ((unsigned)getpid() << 16);
}

int	main(int ac, char **av)
{
	static char	exe[4096];
	ssize_t		n;

	parse(ac, av);
	if (!g_opt.exe)
	{
		n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
		if (n > 0)
		{
			exe[n] = '\0';
			g_opt.exe = exe;
		}
	}
	tdf_init();
	tdf_tests_mandatory();
	tdf_tests_edge();
	tdf_tests_stress();
	tdf_tests_bonus();
	return (tdf_summary());
}
