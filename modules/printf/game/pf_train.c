#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "fous.h"

const char	*g_tester_dir = ".";

const char	*tester_name(void)
{
	const char	*s;

	s = strrchr(g_tester_dir, '/');
	return (s && s[1] ? s + 1 : g_tester_dir);
}

typedef struct s_xres
{
	char	st[16];
	int		checks;
	char	det[10][200];
	int		nd;
}	t_xres;

int	pf_exo_done(void)
{
	int	i;
	int	n;

	for (n = 0, i = 0; i < NPEXO; i++)
		n += g_pf.exo[i] == '1';
	return (n);
}

static int	unlocked(int i)
{
	return (i == 0 || g_pf.exo[i - 1] == '1');
}

static void	exo_dir(int i, char *buf, size_t sz)
{
	snprintf(buf, sz, "%s/entrainement/niveau_%02d", g_tester_dir, i + 1);
}

static void	exo_prepare(int i)
{
	char	d[700];
	char	p[800];
	FILE	*f;

	snprintf(d, sizeof(d), "%s/entrainement", g_tester_dir);
	mkdir(d, 0755);
	exo_dir(i, d, sizeof(d));
	mkdir(d, 0755);
	snprintf(p, sizeof(p), "%s/sujet.txt", d);
	f = fopen(p, "w");
	if (!f)
		return ;
	fprintf(f, "Le testeur des fous printf · Entraînement · niveau %02d\n\n", i + 1);
	fprintf(f, "Fonction : %s\nFichier  : %s.c (dans ce dossier)\nPrototype :\n%s\n\n",
		g_pexos[i].fn, g_pexos[i].fn, g_pexos[i].proto);
	fprintf(f, "%s\n\nNotion : %s\n", g_pexos[i].fr, g_pexos[i].notion);
	fprintf(f, "Fonctions autorisées : write, malloc, free, va_start, va_arg, va_copy, va_end.\n");
	fprintf(f, "Inclus ce dont tu as besoin (<unistd.h>, <stdlib.h>, <stdarg.h>). Pas de libft ici.\n");
	fprintf(f, "Norme obligatoire. Les fonctions d'aide doivent être static.\n");
	fprintf(f, "\nEnglish: %s\n", g_pexos[i].en);
	fclose(f);
}

static void	parse_xres(const char *path, t_xres *r)
{
	FILE	*f;
	char	line[600];
	char	*tab;

	memset(r, 0, sizeof(*r));
	strcpy(r->st, "build");
	f = fopen(path, "r");
	if (!f)
		return ;
	while (fgets(line, sizeof(line), f))
	{
		line[strcspn(line, "\n")] = '\0';
		if (!strncmp(line, "X\t", 2))
		{
			tab = strchr(line + 2, '\t');
			if (tab)
				*tab = '\0';
			snprintf(r->st, sizeof(r->st), "%.15s", line + 2);
			r->checks = tab ? atoi(tab + 1) : 0;
		}
		else if (!strncmp(line, "D\t", 2) && r->nd < 10)
			snprintf(r->det[r->nd++], 200, "%.199s", line + 2);
	}
	fclose(f);
}

static void	run_exo(int i, t_xres *r)
{
	char	out_path[700];
	char	script[700];
	char	num[16];
	pid_t	pid;
	int		status;
	long	t0;
	long	t;
	int		stop;

	snprintf(out_path, sizeof(out_path), "%s/exo_resultat_printf.txt", g_savedir);
	snprintf(script, sizeof(script), "%s/run.sh", g_tester_dir);
	snprintf(num, sizeof(num), "%02d", i + 1);
	unlink(out_path);
	flush();
	pid = fork();
	if (pid == 0)
	{
		setpgid(0, 0);
		if (!freopen("/dev/null", "w", stdout) || !freopen("/dev/null", "w", stderr)
			|| !freopen("/dev/null", "r", stdin))
			_exit(126);
		execl("/bin/bash", "bash", script, "--exo", num, "--out", out_path, (char *)NULL);
		_exit(127);
	}
	if (pid > 0)
	{
		setpgid(pid, pid);
		term_child(-pid);
	}
	t0 = now_ms();
	stop = 0;
	while (pid > 0 && waitpid(pid, &status, WNOHANG) == 0)
	{
		t = now_ms() - t0;
		at((term_cols() - 50) / 2, term_rows() - 4);
		fg(mix(theme()->accent, theme()->accent2, 0.5 + 0.5 * sin(t / 200.0)));
		outf("%s %s", (const char *[]){"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"}[(t / 80) % 10],
			L("Le testeur des fous examine ton code...", "The crazy tester is reading your code..."));
		bar((term_cols() - 50) / 2, term_rows() - 3, 50, fmod(t / 2500.0, 1.0), theme()->accent, theme()->accent2);
		flush();
		if (key_stop(key_read(40)))
			stop = 1;
		if (stop && t > 500)
		{
			kill(-pid, SIGTERM);
			kill(pid, SIGTERM);
			waitpid(pid, &status, 0);
			break ;
		}
	}
	term_child(0);
	parse_xres(out_path, r);
}

static void	reward(int i)
{
	char	tmp[200];
	int		n;

	if (g_pf.exo[i] == '1')
		return ;
	g_pf.exo[i] = '1';
	g_save.xp += 100 + i * 10;
	g_save.octets += 60 + i * 5;
	bell();
	snprintf(tmp, sizeof(tmp), "+%d XP   +%d ◈", 100 + i * 10, 60 + i * 5);
	toast(L("NIVEAU RÉUSSI", "LEVEL CLEARED"), tmp, rgb(80, 230, 140));
	n = pf_exo_done();
	if (n >= 4)
		pf_ach(12);
	if (n >= 8)
		pf_ach(13);
	if (n >= NPEXO)
		pf_ach(14);
	pf_badges(0);
	save_write();
}

static void	level_screen(int i)
{
	t_xres	r;
	char	tmp[400];
	int		k;
	int		x;
	int		y;
	int		w;
	int		j;
	int		has;

	exo_prepare(i);
	has = 0;
	cls();
	while (1)
	{
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		w = term_cols() - x - 1 < 78 ? term_cols() - x - 1 : 78;
		bgc(theme()->bg);
		frame();
		snprintf(tmp, sizeof(tmp), L("NIVEAU %02d · %s", "LEVEL %02d · %s"), i + 1, g_pexos[i].fn);
		text_at(x, 1, theme()->accent2, tmp);
		snprintf(tmp, sizeof(tmp), L("notion : %s", "topic: %s"), g_pexos[i].notion);
		text_clip(x, 2, theme()->dim, tmp, w);
		y = text_wrap(x, 4, 7, w, rgb(255, 230, 140), g_pexos[i].proto);
		y = text_wrap(x, y + 1, term_rows() - 9, w, theme()->text, L(g_pexos[i].fr, g_pexos[i].en));
		snprintf(tmp, sizeof(tmp), L("Ton fichier : %s/entrainement/niveau_%02d/%s.c",
				"Your file: %s/entrainement/niveau_%02d/%s.c"), tester_name(), i + 1, g_pexos[i].fn);
		text_clip(x, y + 1, theme()->accent, tmp, w);
		text_clip(x, y + 2, theme()->dim, L("Autorisé : write, malloc, free, va_*. Norme OBLIGATOIRE. Aucune solution n'est donnée.",
				"Allowed: write, malloc, free, va_*. Norm REQUIRED. No solution is given."), w);
		if (has)
		{
			y += 4;
			snprintf(tmp, sizeof(tmp), "%s · %d %s", !strcmp(r.st, "ok") ? L("RÉUSSI", "PASSED") : L("PAS ENCORE", "NOT YET"),
				r.checks, L("vérifications", "checks"));
			text_at(x, y++, !strcmp(r.st, "ok") ? rgb(80, 230, 140) : rgb(255, 90, 100), tmp);
			for (j = 0; j < r.nd && y < term_rows() - 2; j++)
				text_clip(x + 2, y++, theme()->text, r.det[j], w - 2);
		}
		foot_line(x, L("t : tester mon fichier   Échap : retour", "t: test my file   Esc: back"));
		flush();
		k = key_read(80);
		if (key_quit(k))
			break ;
		if (k == 't' || k == 'T')
		{
			run_exo(i, &r);
			has = 1;
			if (!strcmp(r.st, "ok"))
				reward(i);
			cls();
		}
	}
	cls();
}

void	pf_train(void)
{
	static int	sel;
	char		tmp[200];
	int			k;
	int			i;
	int			x;
	int			top;

	cls();
	while (1)
	{
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		top = sel > term_rows() - 7 ? sel - (term_rows() - 7) : 0;
		bgc(theme()->bg);
		frame();
		center(1, theme()->accent2, L("D L C  ·  E N T R A Î N E M E N T", "D L C  ·  T R A I N I N G"));
		snprintf(tmp, sizeof(tmp), L("%d/%d réussis · les niveaux s'ouvrent dans l'ordre", "%d/%d passed · levels open in order"),
			pf_exo_done(), NPEXO);
		center(2, theme()->dim, tmp);
		for (i = top; i < NPEXO && 4 + i - top < term_rows() - 2; i++)
		{
			snprintf(tmp, sizeof(tmp), " %s %02d  %-22s %s", i == sel ? "▶" : " ", i + 1, g_pexos[i].fn, g_pexos[i].notion);
			text_clip(x, 4 + i - top, g_pf.exo[i] == '1' ? rgb(80, 230, 140) : (unlocked(i) ? theme()->text : theme()->dim),
				tmp, 78);
		}
		foot_line(x, L("▲ ▼ choisir   Entrée : ouvrir   Échap : retour", "▲ ▼ pick   Enter: open   Esc: back"));
		flush();
		k = key_dir(key_read(80));
		if (key_quit(k) || k == 127 || k == 8)
			break ;
		if (k == 1000 && sel > 0)
			sel--;
		else if (k == 1001 && sel < NPEXO - 1)
			sel++;
		else if (k == '\r' || k == '\n' || k == ' ')
		{
			if (unlocked(sel))
				level_screen(sel);
			else
				toast(L("VERROUILLÉ", "LOCKED"), L("réussis d'abord le niveau précédent", "pass the previous level first"),
					rgb(255, 200, 70));
			cls();
		}
	}
	cls();
}
