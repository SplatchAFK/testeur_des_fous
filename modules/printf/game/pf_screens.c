#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fous.h"

/* ---------------------------------------------------------------- communs */

void	toast(const char *title, const char *msg, t_rgb c)
{
	long	t0;
	long	t;
	int		w;
	int		x;
	int		y;
	double	k;

	w = ulen(msg) + 8;
	if (w < ulen(title) + 8)
		w = ulen(title) + 8;
	if (w > term_cols())
		w = term_cols();
	t0 = now_ms();
	while ((t = now_ms() - t0) < anim_ms(1700) + 300)
	{
		k = (t < 250 && !anim_off()) ? t / 250.0 : 1.0;
		x = (term_cols() - w) / 2;
		y = (int)(-4 + 5 * k);
		if (y > 0)
			fill(x, 0, w, y, theme()->bg);
		if (y >= 0)
		{
			fill(x, y, w, 4, mix(theme()->bg, c, 0.25));
			box(x, y, w, 4, c);
			bgc(mix(theme()->bg, c, 0.25));
			text_clip(x + 3, y + 1, mix(c, rgb(255, 255, 255), 0.5 + 0.5 * sin(t / 80.0)), title, w - 5);
			text_clip(x + 3, y + 2, theme()->text, msg, w - 5);
			bgc(theme()->bg);
		}
		flush();
		if (key_read(30) >= 0)
			break ;
	}
	fill(0, 0, term_cols(), 5, theme()->bg);
}

void	screen_login(void)
{
	char	buf[32];
	int		n;
	int		k;
	long	t;

	snprintf(buf, sizeof(buf), "%s", g_save.login);
	n = strlen(buf);
	cls();
	while (1)
	{
		t = now_ms();
		center(6, theme()->accent2, L("Bienvenue, nouveau fou.", "Welcome, new lunatic."));
		center(8, theme()->text, L("Ton login 42 (Entrée pour valider) :", "Your 42 login (Enter to confirm):"));
		at((term_cols() - 24) / 2, 10);
		bgc(mix(theme()->bg, theme()->accent, 0.15));
		fg(theme()->text);
		outf(" %-20s%s ", buf, (t / 400) % 2 ? "▏" : " ");
		bgc(theme()->bg);
		flush();
		k = key_read(60);
		if ((k == '\r' || k == '\n') && n > 0)
			break ;
		if ((k == 127 || k == 8) && n > 0)
			buf[--n] = '\0';
		else if (k > 32 && k < 127 && n < 20)
		{
			buf[n++] = k;
			buf[n] = '\0';
		}
	}
	snprintf(g_save.login, sizeof(g_save.login), "%s", buf);
	save_write();
	cls();
}

static t_rgb	col_of(int ko, int warn, int seen)
{
	if (!seen)
		return (rgb(110, 116, 140));
	if (ko)
		return (rgb(255, 90, 100));
	if (warn)
		return (rgb(255, 200, 70));
	return (rgb(80, 230, 140));
}

static const char	*lab_of(int ko, int warn, int seen)
{
	if (!seen)
		return ("..");
	if (ko)
		return ("KO");
	if (warn)
		return ("~~");
	return ("OK");
}

/* ---------------------------------------------------------------- connexion */

void	pf_grant_veteran(void)
{
	static const char	*ids[3] = {"cape_veteran", "casque_veteran", "fam_moulinette"};
	int					i;

	if (!pf_libft_done() || g_pf.vet)
		return ;
	for (i = 0; i < 3; i++)
		if (item_find(ids[i]) >= 0)
			item_own(item_find(ids[i]));
	for (i = 0; i < g_ntheme; i++)
		if (!strcmp(g_themes[i].id, "veteran"))
			g_save.themes[i] = '1';
	g_save.titles[NTITLE - 1] = '1';
	g_save.octets += 300;
	g_pf.vet = 1;
	toast(L("PRIVILÈGES DU VÉTÉRAN", "VETERAN PERKS"), L("cape, casque, familier Moulinette, thème doré, titre et +300 ◈",
			"cape, helmet, Moulinette pet, golden theme, title and +300 ◈"), rgb(255, 200, 80));
	pf_ach(9);
	save_write();
}

void	pf_connect_screen(void)
{
	char	tmp[200];
	char	path[560];
	int		k;
	int		x;
	int		y;

	if (pf_connected())
	{
		snprintf(tmp, sizeof(tmp), L("%s · libft %s", "%s · libft %s"), g_save.login,
			pf_libft_done() ? L("validée ✓ : tout le lore est débloqué", "validated ✓: the full lore is unlocked")
			: L("pas encore validée", "not validated yet"));
		toast(L("CONNECTÉ AU TESTEUR DES FOUS LIBFT", "LOGGED IN TO THE LIBFT TESTER"), tmp,
			pf_libft_done() ? rgb(80, 230, 140) : rgb(255, 200, 70));
		pf_grant_veteran();
		return ;
	}
	cls();
	while (1)
	{
		x = (term_cols() - 76) / 2;
		if (x < 0)
			x = 0;
		center(1, theme()->accent2, L("C O N N E X I O N", "L O G   I N"));
		y = 3;
		y = text_wrap(x, y, y + 3, 76, theme()->text, L("Aucune sauvegarde du testeur des fous libft n'a été trouvée. "
				"Tu peux jouer tout de suite : tous les tests et tous les combats sont là.",
				"No libft tester save was found. You can play right away: every test and every fight is here.")) + 1;
		text_at(x, y++, rgb(255, 200, 80), L("Installe d'abord le testeur des fous libft pour avoir tout le lore :",
				"Install the libft tester first to get the full lore:"));
		text_at(x, y++, theme()->text, L("  ★ « Précédemment, dans la Libft » et la fin secrète",
				"  ★ \"Previously, in the Libft\" and the secret ending"));
		text_at(x, y++, theme()->text, L("  ★ ton chevalier Marvin, tes objets, titres, Octets et XP",
				"  ★ your knight Marvin, items, titles, Octets and XP"));
		text_at(x, y++, theme()->text, L("  ★ les exclusifs du Vétéran : cape, casque, familier Moulinette, thème doré",
				"  ★ Veteran exclusives: cape, helmet, Moulinette pet, golden theme"));
		text_at(x, y++, theme()->text, L("  ★ le DLC débloqué tout de suite et 300 Octets offerts",
				"  ★ the DLC unlocked right away and 300 free Octets"));
		y++;
		text_at(x, y++, theme()->dim, "github.com/SplatchAFK/testeur_des_fous_libft");
		y++;
		text_at(x, y++, theme()->text, L("Tu as gardé ton save.txt ? Charge-le pour te connecter.",
				"Kept your save.txt? Load it to log in."));
		foot_line(x, L("c : charger un save.txt   Entrée : continuer sans",
				"c: load a save.txt   Enter: continue without"));
		flush();
		k = key_read(80);
		if (k == '\r' || k == '\n' || k == ' ' || key_quit(k))
			break ;
		if (k == 'c' || k == 'C')
		{
			path[0] = '\0';
			if (ask_text(L("Chemin de ta sauvegarde", "Path of your save"),
					L("Par exemple ~/testeur_des_fous_save.txt", "For example ~/testeur_des_fous_save.txt"),
					path, sizeof(path)) && save_connect_cli(path) == 0)
			{
				pf_grant_veteran();
				break ;
			}
			cls();
		}
	}
	cls();
}

/* ---------------------------------------------------------------- analyse */

static void	scan_draw(long t, int dead)
{
	char	tmp[200];
	int		x;
	int		y;
	int		i;
	int		n;

	x = (term_cols() - 76) / 2;
	if (x < 0)
		x = 0;
	center(1, theme()->accent2, L("A N A L Y S E   D E   T O N   F T _ P R I N T F",
			"S C A N N I N G   Y O U R   F T _ P R I N T F"));
	snprintf(tmp, sizeof(tmp), L("%s chaque appel est comparé au vrai printf, octet par octet",
			"%s every call is compared to the real printf, byte by byte"),
		g_pr.done || dead ? "✓" : (const char *[4]){"◐", "◓", "◑", "◒"}[(t / 120) % 4]);
	center(2, theme()->dim, tmp);
	y = 4;
	for (i = 0; i < g_pr.nst && y < term_rows() - 3; i++, y++)
	{
		snprintf(tmp, sizeof(tmp), "%-3s %s", g_pr.st[i].st == 1 ? "OK" : (g_pr.st[i].st == 2 ? "KO" : "~~"), g_pr.st[i].title);
		text_clip(x, y, col_of(g_pr.st[i].st == 2, g_pr.st[i].st == 3, 1), tmp, 76);
	}
	y++;
	for (i = 0; i < g_pr.ncat && y < term_rows() - 3; i++, y++)
	{
		snprintf(tmp, sizeof(tmp), "%-3s %-52.52s %5d/%-5d", lab_of(g_pr.cat[i].ko, g_pr.cat[i].warn, 1),
			g_pr.cat[i].title, g_pr.cat[i].ok + g_pr.cat[i].warn, g_pr.cat[i].ok + g_pr.cat[i].ko + g_pr.cat[i].warn);
		text_clip(x, y, col_of(g_pr.cat[i].ko, g_pr.cat[i].warn, 1), tmp, 76);
	}
	n = g_pr.ncat;
	if (g_pr.ptot > 0 && g_pr.pcur < g_pr.ptot && !g_pr.done && !dead)
	{
		/* lot long en cours (stress) : il avance à l'écran au lieu de le figer */
		snprintf(tmp, sizeof(tmp), "%s %-8.8s %6d/%-6d %3d %%", (const char *[4]){"◐", "◓", "◑", "◒"}[(t / 120) % 4],
			g_pr.plabel, g_pr.pcur, g_pr.ptot, (int)((long)g_pr.pcur * 100 / g_pr.ptot));
		text_at(x, term_rows() - 3, theme()->accent2, tmp);
		bar(x + 32, term_rows() - 3, 44, g_pr.pcur / (double)g_pr.ptot, theme()->accent2, theme()->accent);
	}
	bar(x, term_rows() - 2, 76, n / 16.0 > 1 ? 1 : n / 16.0, theme()->accent, theme()->accent2);
	if (!g_pr.done && !dead)
		foot_line(x, L("[ Q / Échap / Ctrl+C : arrêter l'analyse ]",
				"[ Q / Esc / Ctrl+C: stop the scan ]"));
}

/* renvoie 1 si le joueur a arrêté l'analyse (Échap, q ou Ctrl+C), 0 sinon */
int	pf_scan(const char *path, int pid)
{
	long	off;
	long	t0;
	int		dead;
	int		stopped;

	off = 0;
	t0 = now_ms();
	dead = 0;
	stopped = 0;
	term_child(pid > 0 ? pid : 0);
	cls();
	while (1)
	{
		pf_load(path, &off);
		if (!dead && pid > 0 && kill(pid, 0) != 0)
		{
			dead = 1;
			pf_load(path, &off);
		}
		bgc(theme()->bg);
		frame();
		stars_draw();
		scan_draw(now_ms() - t0, dead);
		flush();
		if (g_pr.done || dead)
			break ;
		if (key_stop(key_read(80)))
		{
			if (pid > 0)
				kill(pid, SIGTERM);
			stopped = 1;
			break ;
		}
	}
	term_child(0);
	g_pr.done = 1;
	if (stopped)
	{
		cls();
		return (1);
	}
	text_at((term_cols() - 76) / 2 > 0 ? (term_cols() - 76) / 2 : 0, term_rows() - 1, theme()->dim,
		g_pf.runs == 0 ? L("[ une touche pour continuer · les combats commencent dans 8 s ]",
			"[ any key to continue · fights start in 8 s ]") : "");
	flush();
	t0 = now_ms();
	if (!anim_off() && g_pf.runs == 0)
		while (key_read(100) < 0 && now_ms() - t0 < 8000)
			;
	cls();
	return (0);
}

/* ---------------------------------------------------------------- résultats */

static int	detail_lines(int sel, char lines[][240], t_rgb *col, int max)
{
	const char	*key;
	const char	*p;
	const char	*e;
	char		tmp[400];
	int			n;
	int			i;

	n = 0;
	if (sel < g_pr.nst)
	{
		snprintf(lines[n], 240, "%s", g_pr.st[sel].title);
		col[n++] = col_of(g_pr.st[sel].st == 2, g_pr.st[sel].st == 3, 1);
		for (p = g_pr.st[sel].detail; *p && n < max; p = *e ? e + 1 : e)
		{
			e = strchr(p, '\n') ? strchr(p, '\n') : p + strlen(p);
			snprintf(lines[n], 240, "  %.*s", (int)(e - p), p);
			col[n++] = theme()->text;
		}
		return (n);
	}
	key = g_pr.cat[sel - g_pr.nst].key;
	snprintf(lines[n], 240, "%s", g_pr.cat[sel - g_pr.nst].title);
	col[n++] = theme()->accent2;
	for (i = 0; i < g_pr.nca && n < max - 3; i++)
	{
		if (strcmp(g_pr.ca[i].cat, key))
			continue ;
		if (g_save.cfg.help >= 1 && g_pr.ca[i].where[0])
			snprintf(tmp, sizeof(tmp), "▸ %s · %s (%d cas)", g_pr.ca[i].title, g_pr.ca[i].where, g_pr.ca[i].count);
		else
			snprintf(tmp, sizeof(tmp), "▸ %s (%d cas)", g_pr.ca[i].title, g_pr.ca[i].count);
		snprintf(lines[n], 240, "%s", tmp);
		col[n++] = rgb(255, 90, 100);
		if (g_save.cfg.help >= 2 && g_pr.ca[i].hint[0])
		{
			snprintf(lines[n], 240, "  → %s", g_pr.ca[i].hint);
			col[n++] = theme()->text;
		}
	}
	for (i = 0; i < g_pr.ncs && n < max - 1; i++)
	{
		if (strcmp(g_pr.cs[i].cat, key))
			continue ;
		snprintf(lines[n], 240, "%s %s", g_pr.cs[i].ko ? "✗" : "⚠", g_pr.cs[i].desc);
		col[n++] = g_pr.cs[i].ko ? rgb(255, 120, 120) : rgb(255, 200, 70);
		for (p = g_pr.cs[i].msg; *p && n < max; p = *e ? e + 1 : e)
		{
			e = strchr(p, '\n') ? strchr(p, '\n') : p + strlen(p);
			while (*p == ' ' && p < e)
				p++;
			if (e == p)
				continue ;
			if (g_save.cfg.help < 1 && (strstr(p, "ligne du bug") || strstr(p, "alloué ici"))
				&& (strstr(p, "ligne du bug") < e || strstr(p, "alloué ici") < e))
				continue ;
			snprintf(lines[n], 240, "    %.*s", (int)(e - p), p);
			col[n++] = theme()->dim;
		}
	}
	if (n == 1)
	{
		snprintf(lines[n], 240, "%s", L("Rien à signaler : tout est identique au vrai printf.",
				"Nothing to report: everything matches the real printf."));
		col[n++] = rgb(80, 230, 140);
	}
	return (n);
}

static void	detail_view(int sel)
{
	static char	lines[400][240];
	static t_rgb	col[400];
	int			n;
	int			top;
	int			k;
	int			i;
	int			x;
	int			h;

	n = detail_lines(sel, lines, col, 400);
	top = 0;
	cls();
	while (1)
	{
		x = 2;
		h = term_rows() - 3;
		bgc(theme()->bg);
		frame();
		for (i = 0; i < h && top + i < n; i++)
			text_clip(x, 1 + i, col[top + i], lines[top + i], term_cols() - 4);
		foot_line(x, L("▲ ▼ défiler   Échap : retour   (aide : Paramètres)",
				"▲ ▼ scroll   Esc: back   (help: Settings)"));
		flush();
		k = key_dir(key_read(80));
		if (key_quit(k) || k == 127 || k == 8)
			break ;
		if (k == 1000 && top > 0)
			top--;
		else if (k == 1001 && top + h < n)
			top++;
	}
	cls();
}

/* after : ouvert juste après un run (curseur sur le 1er KO) ; 42 = relancer */
int	pf_results(int after)
{
	static int	sel;
	char		tmp[220];
	int			n;
	int			i;
	int			k;
	int			x;
	int			y;
	int			top;

	if (after)
	{
		for (sel = 0; sel < g_pr.nst && g_pr.st[sel].st != 2; sel++)
			;
		if (sel == g_pr.nst)
			for (i = 0; i < g_pr.ncat; i++)
				if (g_pr.cat[i].ko)
				{
					sel = g_pr.nst + i;
					break ;
				}
	}
	cls();
	while (1)
	{
		n = g_pr.nst + g_pr.ncat;
		if (sel >= n)
			sel = n ? n - 1 : 0;
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		top = sel > term_rows() - 8 ? sel - (term_rows() - 8) : 0;
		bgc(theme()->bg);
		frame();
		discord_line(0);
		center(1, theme()->accent2, L("R É S U L T A T S   D É T A I L L É S", "D E T A I L E D   R E S U L T S"));
		snprintf(tmp, sizeof(tmp), L("%d OK · %d KO · %d avertissements · mode %s · aide %d/2",
				"%d OK · %d KO · %d warnings · mode %s · help %d/2"), g_pr.tok, g_pr.tko, g_pr.twarn, g_pr.mode,
			g_save.cfg.help);
		center(2, theme()->dim, tmp);
		for (y = 4, i = top; i < n && y < term_rows() - 2; i++, y++)
		{
			if (i < g_pr.nst)
				snprintf(tmp, sizeof(tmp), " %s %-3s %s", i == sel ? "▶" : " ",
					g_pr.st[i].st == 1 ? "OK" : (g_pr.st[i].st == 2 ? "KO" : "~~"), g_pr.st[i].title);
			else
				snprintf(tmp, sizeof(tmp), " %s %-3s %-50.50s %5d/%d", i == sel ? "▶" : " ",
					lab_of(g_pr.cat[i - g_pr.nst].ko, g_pr.cat[i - g_pr.nst].warn, 1), g_pr.cat[i - g_pr.nst].title,
					g_pr.cat[i - g_pr.nst].ok + g_pr.cat[i - g_pr.nst].warn,
					g_pr.cat[i - g_pr.nst].ok + g_pr.cat[i - g_pr.nst].ko + g_pr.cat[i - g_pr.nst].warn);
			if (i == sel)
				fill(x, y, 78, 1, mix(theme()->bg, theme()->accent, 0.18));
			bgc(i == sel ? mix(theme()->bg, theme()->accent, 0.18) : theme()->bg);
			if (i < g_pr.nst)
				text_clip(x, y, col_of(g_pr.st[i].st == 2, g_pr.st[i].st == 3, 1), tmp, 78);
			else
				text_clip(x, y, col_of(g_pr.cat[i - g_pr.nst].ko, g_pr.cat[i - g_pr.nst].warn, 1), tmp, 78);
			bgc(theme()->bg);
		}
		if (!n)
			center(5, theme()->dim, L("Aucun résultat : lance « Relancer les tests » (touche r).", "No results: run \"Run the tests again\" (key r)."));
		foot_line(x, L("Entrée : détail   r : corrigé ? relancer les tests   Échap : menu",
				"Enter: details   r: fixed? run the tests again   Esc: menu"));
		flush();
		k = key_read(80);
		if (k == 'r' || k == 'R')
		{
			cls();
			return (42);
		}
		k = key_dir(k);
		if (key_quit(k) || k == 127 || k == 8)
			break ;
		if (k == 1000 && sel > 0)
			sel--;
		else if (k == 1001 && sel < n - 1)
			sel++;
		else if ((k == '\r' || k == '\n' || k == ' ') && n)
			detail_view(sel);
	}
	cls();
	return (0);
}

/* ---------------------------------------------------------------- récompenses */

static int	gains(int *news, int *bigfirst)
{
	time_t	now;
	int		i;
	int		run;

	*news = 0;
	*bigfirst = 0;
	for (i = 0; i < NCHAIN; i++)
		if (pf_chain_ok(i) && g_pf.chains[i] != '1')
		{
			g_pf.chains[i] = '1';
			(*news)++;
		}
	if (pf_all_ok() && !g_pf.big)
	{
		g_pf.big = 1;
		*bigfirst = 1;
	}
	g_pf.runs++;
	g_save.xp += *news * 60 + *bigfirst * 500;
	g_save.octets += *news * 30 + *bigfirst * 250;
	run = 0;
	now = time(NULL);
	if (now - g_pf.lastreward >= 1800)
	{
		for (i = 0; i < NCHAIN; i++)
			run += pf_chain_ok(i);
		run += pf_all_ok() ? 9 : 0;
		g_save.xp += run * 3;
		g_save.octets += run * 2;
		g_pf.lastreward = now;
	}
	return (run);
}

static void	pf_achs(void)
{
	int			i;
	int			n;
	time_t		t;
	struct tm	*lt;

	for (n = 0, i = 0; i < NCHAIN; i++)
		n += g_pf.chains[i] == '1';
	if (n >= 1)
		pf_ach(0);
	if (n == NCHAIN)
		pf_ach(1);
	if (g_pf.big)
		pf_ach(2);
	if (pf_all_ok() && g_pf.runs == 1)
		pf_ach(3);
	if (pf_chain_ok(8))
		pf_ach(4);
	if (pf_chain_ok(2))
		pf_ach(5);
	if (pf_chain_ok(3) && pf_chain_ok(4))
		pf_ach(6);
	if (pf_chain_ok(6) && pf_chain_ok(7))
		pf_ach(7);
	if (g_pf.runs >= 10)
		pf_ach(15);
	if (g_pf.runs >= 30)
		pf_ach(16);
	t = time(NULL);
	lt = localtime(&t);
	if (lt && lt->tm_hour < 5)
		pf_ach(17);
	if (g_save.octets >= 2000)
		pf_ach(18);
	if (g_pr.done && g_pr.twarn == 0 && g_pr.ncat > 0)
		pf_ach(21);
	for (i = 0; i < g_pr.nst; i++)
	{
		if (strstr(g_pr.st[i].title, "norminette") && g_pr.st[i].st == 1)
			pf_ach(22);
		if (strstr(g_pr.st[i].title, "README.md (") && g_pr.st[i].st == 1)
			pf_ach(23);
	}
	if (pf_libft_done() && g_pf.big)
		pf_ach(24);
}

int	pf_record(void)
{
	int	news;
	int	big;

	gains(&news, &big);
	pf_badges(1);
	save_write();
	return (news);
}

static void	roll(int x, int y, const char *label, long to, t_rgb c)
{
	long	t0;
	double	k;

	t0 = now_ms();
	do
	{
		k = anim_off() ? 1 : ((now_ms() - t0) / (double)anim_ms(600));
		if (k > 1)
			k = 1;
		at(x, y);
		fg(c);
		outf("%-46s %+8ld", label, (long)(to * k));
		flush();
		sleep_ms(16);
	} while (k < 1);
}

void	pf_rewards(void)
{
	int		news;
	int		big;
	int		run;
	int		x;
	int		y;
	long	oct0;
	char	tmp[200];

	oct0 = g_save.octets;
	run = gains(&news, &big);
	if (!news && !big)
	{
		pf_achs();
		pf_badges(0);
		save_write();
		return ;
	}
	cls();
	x = (term_cols() - 56) / 2;
	y = 3;
	center(1, theme()->accent2, L("R É C O M P E N S E S", "R E W A R D S"));
	roll(x, y++, L("XP des chaînes brisées pour la 1re fois", "XP for chains broken the first time"), news * 60, rgb(255, 230, 120));
	roll(x, y++, L("Octets des chaînes brisées", "Octets for broken chains"), news * 30, theme()->accent);
	if (big)
	{
		roll(x, y++, L("XP : ft_printf battu pour la 1re fois !", "XP: ft_printf beaten for the first time!"), 500, rgb(255, 230, 120));
		roll(x, y++, L("Octets : ft_printf battu", "Octets: ft_printf beaten"), 250, theme()->accent);
	}
	if (run)
	{
		roll(x, y++, L("XP du run", "Run XP"), run * 3, rgb(255, 230, 120));
		roll(x, y++, L("Octets du run", "Run Octets"), run * 2, theme()->accent);
	}
	else
		text_at(x, y++, theme()->dim, L("run déjà récompensé (attente 30 min)", "run already rewarded (30 min cooldown)"));
	y++;
	snprintf(tmp, sizeof(tmp), "Wallet : %ld ◈ Octets   (+%ld)", g_save.octets, g_save.octets - oct0);
	text_at(x, y++, rgb(255, 215, 0), tmp);
	flush();
	pf_achs();
	pf_badges(0);
	save_write();
	center(term_rows() - 2, theme()->dim, L("[ une touche pour continuer ]", "[ any key to continue ]"));
	flush();
	while (key_read(100) < 0)
		;
	cls();
}

/* ---------------------------------------------------------------- profil */

void	pf_profile(void)
{
	char	tmp[220];
	int		x;
	int		y;
	int		i;
	int		n;
	int		m;
	int		lv;
	int		o;

	cls();
	x = (term_cols() - 80) / 2;
	if (x < 0)
		x = 0;
	lv = level_of(g_save.xp);
	o = profile_header(x, 1);
	center(1 + o, theme()->accent2, L("P R O F I L", "P R O F I L E"));
	if (g_save.bio[0])
		center(2 + o, theme()->dim, g_save.bio);
	snprintf(tmp, sizeof(tmp), L("%s · niveau %d · %ld XP · %ld ◈", "%s · level %d · %ld XP · %ld ◈"),
		g_save.login, lv, g_save.xp, g_save.octets);
	center(3 + o, pseudo_color(), tmp);
	bar(x + 16, 4 + o, 48, (g_save.xp - xp_for(lv)) / (double)(xp_for(lv + 1) - xp_for(lv)),
		theme()->accent, theme()->accent2);
	for (m = 0, n = 0; n < 3; n++)
		if (g_save.vitrine[n] >= 0)
			m += snprintf(tmp + m, sizeof(tmp) - m, "%s★ %s", m ? "   " : "",
				L(g_badges[g_save.vitrine[n]].fr, g_badges[g_save.vitrine[n]].en));
	if (m)
		center(5 + o, rgb(255, 215, 0), tmp);
	at(x + 10, 7 + o);
	for (i = 0; i < NCHAIN; i++)
	{
		fg(g_pf.chains[i] == '1' ? g_chains[i].c : theme()->dim);
		outf(" %s ", g_chains[i].conv);
	}
	fg(g_pf.big ? rgb(255, 120, 220) : theme()->dim);
	out(g_pf.big ? "  ★ ft_printf" : "  ☆ ft_printf");
	for (n = 0, i = 0; i < NDLC; i++)
		n += g_pf.dlc[i] == '1';
	snprintf(tmp, sizeof(tmp), L("runs %d · DLC %d/6 · entraînement %d/16 · libft %s · crashs %d",
			"runs %d · DLC %d/6 · training %d/16 · libft %s · crashes %d"), g_pf.runs, n, pf_exo_done(),
		pf_libft_done() ? "✓" : "✗", g_pf.crashes);
	center(8 + o, theme()->dim, tmp);
	y = 10 + o;
	text_at(x, y++, theme()->accent, L("SUCCÈS", "ACHIEVEMENTS"));
	for (i = 0; i < NPACH && y + i / 2 < term_rows() - 2; i++)
	{
		snprintf(tmp, sizeof(tmp), "%s %s", g_pf.ach[i] == '1' ? "★" : "☆",
			g_pf.ach[i] == '1' ? L(g_pach_fr[i][0], g_pach_en[i][0]) : L(g_pach_fr[i][1], g_pach_en[i][1]));
		text_clip(x + (i % 2) * 41, y + i / 2, g_pf.ach[i] == '1' ? rgb(255, 215, 0) : theme()->dim, tmp, 39);
	}
	center(term_rows() - 1, theme()->dim, L("[ e : modifier mon profil   ·   une autre touche : retour ]",
			"[ e: edit my profile   ·   any other key: back ]"));
	flush();
	while ((n = key_read(100)) < 0)
		;
	cls();
	if (n == 'e' || n == 'E')
		screen_profile_edit();
}

void	pf_credits(void)
{
	const char	*l[24];
	char		ver[64];
	int			n;
	int			i;
	long		t0;
	double		s;

	n = 0;
	l[n++] = "LE TESTEUR DES FOUS";
	snprintf(ver, sizeof(ver), "%s · v%s", L("Version printf", "printf edition"), TDF_VERSION);
	l[n++] = ver;
	l[n++] = "";
	l[n++] = L("Créé et vibe-codé par Splatch", "Created and vibe-coded by Splatch");
	l[n++] = L("login 42 : romasant", "42 login: romasant");
	l[n++] = "";
	l[n++] = L("Moteur de tests : Splatch (beta du testeur printf)", "Test engine: Splatch (printf tester beta)");
	l[n++] = L("Lore et idées : Splatch (romasant)", "Lore and ideas: Splatch (romasant)");
	l[n++] = "";
	l[n++] = L("Pour les étudiants du cursus 42 Next", "For 42 Next students");
	l[n++] = L("Paris et autres campus", "Paris and other campuses");
	l[n++] = "";
	l[n++] = L("Discord · le serveur des fous", "Discord · the fous server");
	l[n++] = TDF_DISCORD;
	l[n++] = "";
	l[n++] = L("Merci à Moulinette, chatte grise et libre.", "Thanks to Moulinette, a free grey cat.");
	l[n++] = L("Licence MIT · rien n'est envoyé, mise à jour sur ton accord",
			"MIT licence · nothing is sent, updates only if you agree");
	t0 = now_ms();
	cls();
	while (1)
	{
		s = anim_off() ? term_rows() - 5 : (now_ms() - t0) / 220.0;
		bgc(theme()->bg);
		frame();
		stars_draw();
		for (i = 0; i < n; i++)
			if (term_rows() - 5 - (int)s + i * 2 >= 0 && term_rows() - 5 - (int)s + i * 2 < term_rows() - 5)
				center(term_rows() - 5 - (int)s + i * 2 > 2 ? term_rows() - 5 - (int)s + i * 2 : 2,
					(i == 0 || i == 12) ? theme()->accent2 : theme()->text, l[i]);
		discord_card((term_cols() - DISCORD_W) / 2, term_rows() - 5);
		flush();
		if (key_read(60) >= 0 || s > term_rows() + n * 2 + 4)
			break ;
	}
	cls();
}
