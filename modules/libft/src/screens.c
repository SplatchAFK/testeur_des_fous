#include <dirent.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include "fous.h"

t_run		g_run;

static long	g_reveal[NFN];
static int	g_shown[NFN];

void	results_tout_montrer(void)
{
	int	i;

	for (i = 0; i < NFN; i++)
		g_shown[i] = 1;
}

int	st_bad(int st)
{
	return (st != ST_OK && st != ST_WAIT && st != ST_WARN);
}

int	st_pass(int st)
{
	return (st == ST_OK || st == ST_WARN);
}

const char	*st_label(int st)
{
	static const char	*l[] = {"...", "OK", "FAUX", "CRASH", "FUITE", "UB",
		"BOUCLE", "COMPIL", "ABSENT", "NOTE"};
	static const char	*e[] = {"...", "OK", "WRONG", "CRASH", "LEAK", "UB",
		"LOOP", "BUILD", "MISSING", "NOTE"};

	return (L(l[st], e[st]));
}

t_rgb	st_color(int st)
{
	if (st == ST_OK)
		return (rgb(80, 230, 140));
	if (st == ST_KO)
		return (rgb(255, 90, 100));
	if (st == ST_CRASH)
		return (rgb(255, 50, 160));
	if (st == ST_LEAK || st == ST_UB)
		return (rgb(255, 200, 70));
	if (st == ST_TIME)
		return (rgb(180, 140, 255));
	if (st == ST_BUILD)
		return (rgb(110, 190, 255));
	if (st == ST_SKIP)
		return (rgb(110, 110, 130));
	if (st == ST_WARN)
		return (rgb(190, 230, 90));
	return (theme()->dim);
}

int	part_clean(int p)
{
	int	i;

	for (i = g_bosses[p].first; i <= g_bosses[p].last; i++)
		if (!st_pass(g_run.fn[i].st))
			return (0);
	return (1);
}

static int	st_parse(const char *s)
{
	static const char	*n[] = {"wait", "ok", "ko", "crash", "leak", "ub",
		"time", "build", "skip", "warn"};
	int					i;

	for (i = 0; i < 10; i++)
		if (!strcmp(n[i], s))
			return (i);
	return (ST_KO);
}

static int	split_tab(char *s, char **f, int max);

static void	comparer_avant(char *out, size_t n)
{
	char	p[800];
	char	l[2048];
	char	*f[8];
	char	rep[300];
	char	cas[300];
	FILE	*fp;
	int		k;
	int		av;
	int		nr;
	int		nc;

	out[0] = '\0';
	snprintf(p, sizeof(p), getenv("TDF_REVOIR") ? "%s/derniere/libft.avant.res" : "%s/derniere/libft.res", g_savedir);
	fp = fopen(p, "r");
	if (!fp)
		return ;
	rep[0] = '\0';
	cas[0] = '\0';
	nr = 0;
	nc = 0;
	while (fgets(l, sizeof(l), fp))
	{
		l[strcspn(l, "\r\n")] = '\0';
		if (split_tab(l, f, 8) < 3 || strcmp(f[0], "T") || (k = fn_index(f[1])) < 0)
			continue ;
		av = st_parse(f[2]);
		if (av == ST_WAIT || g_run.fn[k].st == ST_WAIT)
			continue ;
		if (!st_pass(av) && st_pass(g_run.fn[k].st) && ++nr <= 4)
			snprintf(rep + strlen(rep), sizeof(rep) - strlen(rep), "%s%s", nr > 1 ? ", " : "", f[1]);
		else if (st_pass(av) && st_bad(g_run.fn[k].st) && ++nc <= 4)
			snprintf(cas + strlen(cas), sizeof(cas) - strlen(cas), "%s%s", nc > 1 ? ", " : "", f[1]);
	}
	fclose(fp);
	if (nr + nc == 0)
		snprintf(out, n, "%s", L("depuis la dernière fois : rien n'a changé", "since last time: nothing changed"));
	else
		snprintf(out, n, L("depuis la dernière fois : %d réparée(s)%s%s%s · %d cassée(s)%s%s%s",
				"since last time: %d fixed%s%s%s · %d broken%s%s%s"), nr, nr ? " (" : "", rep, nr ? ")" : "",
			nc, nc ? " (" : "", cas, nc ? ")" : "");
}

static int	split_tab(char *s, char **f, int max)
{
	int	n;

	n = 0;
	while (n < max)
	{
		f[n++] = s;
		s = strchr(s, '\t');
		if (!s)
			break ;
		*s++ = '\0';
	}
	return (n);
}

static void	utf8_fix(char *s)
{
	size_t			n;
	size_t			k;
	unsigned char	c;
	size_t			need;

	n = strlen(s);
	k = n;
	while (k > 0 && ((unsigned char)s[k - 1] & 0xC0) == 0x80)
		k--;
	if (k > 0 && (unsigned char)s[k - 1] >= 0xC0)
	{
		c = (unsigned char)s[k - 1];
		need = c >= 0xF0 ? 4 : (c >= 0xE0 ? 3 : 2);
		if (n - (k - 1) < need)
			s[k - 1] = '\0';
	}
}

static void	parse_line(char *line)
{
	char	*f[16];
	char	*cut;
	int		n;
	int		i;
	int		k;
	t_res	*r;

	n = split_tab(line, f, 16);
	if (!strcmp(f[0], "END"))
		g_run.done = 1;
	else if (!strcmp(f[0], "AD"))
		g_run.audit_done = 1;
	else if (!strcmp(f[0], "PD"))
		g_run.protos_done = 1;
	else if (!strcmp(f[0], "S") && n > 1)
		snprintf(g_run.stage, sizeof(g_run.stage), "%s", f[1]);
	else if (!strcmp(f[0], "G") && n > 1 && g_run.nwarn < 6)
		snprintf(g_run.warn[g_run.nwarn++], 200, "%s", f[1]);
	else if (!strcmp(f[0], "P") && n > 2 && (k = fn_index(f[1])) >= 0)
	{
		g_run.fn[k].proto = !strcmp(f[2], "ok") || !strcmp(f[2], "warn") ? 1 : (!strcmp(f[2], "missing") ? 3 : 2);
		if (!strcmp(f[2], "warn") && n > 3 && g_run.fn[k].nd < 5)
			snprintf(g_run.fn[k].detail[g_run.fn[k].nd++], 200, "%s %s", L("remarque :", "note:"), f[3]);
		if (n > 3 && g_run.fn[k].proto != 1 && g_run.fn[k].nd < 5)
		{
			cut = strstr(f[3], "   attendu");
			if (cut)
				*cut = '\0';
			snprintf(g_run.fn[k].detail[g_run.fn[k].nd++], 200, "prototype %s", f[3]);
			if (cut)
				snprintf(g_run.fn[k].detail[g_run.fn[k].nd++], 200, "          %s", cut + 3);
		}
	}
	else if (!strcmp(f[0], "T") && n > 4 && (k = fn_index(f[1])) >= 0)
	{
		r = &g_run.fn[k];
		r->st = st_parse(f[2]);
		r->checks = atoi(f[3]);
		r->ms = atoi(f[4]);
		for (i = 5; i < n && r->nd < 8; i++)
			if (f[i][0])
			{
				snprintf(r->detail[r->nd], 200, "%s", f[i]);
				utf8_fix(r->detail[r->nd++]);
			}
		g_run.tests_done++;
	}
	else if (!strcmp(f[0], "A") && n > 2 && g_run.naudit < 32)
	{
		g_run.audit[g_run.naudit].st = !strcmp(f[1], "ok") ? ST_OK
			: (!strcmp(f[1], "warn") ? ST_LEAK : ST_KO);
		snprintf(g_run.audit[g_run.naudit].label, 96, "%s", f[2]);
		snprintf(g_run.audit[g_run.naudit].detail, 160, "%s", n > 3 ? f[3] : "");
		g_run.naudit++;
	}
}

void	results_load(const char *path, long *off)
{
	FILE	*f;
	char	line[2048];
	size_t	l;

	f = fopen(path, "r");
	if (!f)
		return ;
	g_run.loaded = 1;
	fseek(f, *off, SEEK_SET);
	while (fgets(line, sizeof(line), f))
	{
		l = strlen(line);
		if (l == 0 || line[l - 1] != '\n')
			break ;
		*off += l;
		line[l - 1] = '\0';
		parse_line(line);
	}
	fclose(f);
}

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
			text_at(x + 3, y + 1, mix(c, rgb(255, 255, 255), 0.5 + 0.5 * sin(t / 80.0)), title);
			text_at(x + 3, y + 2, theme()->text, msg);
			bgc(theme()->bg);
		}
		flush();
		if (key_read(30) >= 0)
			break ;
	}
	fill(0, 0, term_cols(), 5, theme()->bg);
}

void	screen_intro(void)
{
	long		t0;
	double		t;
	int			k;
	int			cx;
	int			cy;
	char		sub[64];
	const char	*sub_full = "·  L E   T E S T E U R   D E S   F O U S  ·";
	const char	*cred;

	cred = L("Créé et vibe-codé par Splatch  (login : romasant)",
			"Created and vibe-coded by Splatch  (login: romasant)");
	t0 = now_ms();
	cls();
	while (!anim_off())
	{
		t = (now_ms() - t0) / 1000.0 * (g_save.cfg.anim == 1 ? 2.5 : 1.0);
		if (t > 7.0)
			break ;
		cx = (term_cols() - big_width("LE TESTEUR")) / 2;
		cy = (term_rows() - 17) / 2;
		bgc(theme()->bg);
		frame();
		bgc(theme()->bg);
		big_text(cx, cy, "LE TESTEUR", t, (t > 1.3 && t < 1.65) ? 1 : 2);
		big_text((term_cols() - big_width("DES FOUS")) / 2, cy + 6, "DES FOUS",
			t - 0.5, (t > 1.45 && t < 1.7) ? 1 : 2);
		if (t > 1.8)
		{
			k = (int)((t - 1.8) * 40);
			snprintf(sub, sizeof(sub), "%.*s", k, sub_full);
			center(cy + 12, theme()->accent2, sub);
		}
		if (t > 2.2)
			center(cy + 13, theme()->dim, "v" TDF_VERSION "  ·  " TDF_DATE);
		if (t > 2.6)
			center(cy + 14, mix(theme()->bg, theme()->text, (t - 2.6) / 0.9), cred);
		if (t > 3.0)
			center(cy + 15, mix(theme()->bg, theme()->accent, (t - 3.0 > 0.8 ? 0.8 : t - 3.0)),
				L("Pour les étudiants du cursus 42 Next · Paris et autres campus",
				"For 42 Next students · Paris and other campuses"));
		if (t > 3.4 && ((int)(t * 2.5)) % 2)
			center(cy + 17, theme()->dim, L("[ appuie sur une touche ]", "[ press any key ]"));
		flush();
		if (key_read(33) >= 0 && t > 0.4)
			break ;
	}
	cls();
}

static void	tile(int x, int y, int i, long now)
{
	t_res	*r;
	t_rgb	c;
	t_rgb	b;
	long	dt;
	int		sx;
	int		st;
	static char	names[NFN][16];

	r = &g_run.fn[i];
	st = g_shown[i] ? r->st : ST_WAIT;
	c = st_color(st);
	dt = g_reveal[i] ? now - g_reveal[i] : 99999;
	sx = 0;
	if (st_bad(st) && dt < 500 && !anim_off())
		sx = (int)(2 * sin(dt / 25.0));
	b = mix(theme()->bg, c, 0.22);
	if (dt < 350 && !anim_off())
		b = mix(mix(c, rgb(255, 255, 255), 0.4), b, dt / 350.0);
	if (st == ST_WAIT)
	{
		b = mix(theme()->bg, theme()->dim, 0.12);
		c = theme()->dim;
		if (g_run.current == i)
		{
			b = mix(theme()->bg, theme()->accent, 0.2 + 0.15 * sin(now / 90.0));
			c = theme()->accent;
		}
	}
	at(x + sx, y);
	bgc(b);
	fg(r->proto == 2 || r->proto == 3 ? rgb(255, 90, 100)
		: (r->proto == 1 ? rgb(80, 230, 140) : theme()->dim));
	out(r->proto ? "▌" : " ");
	fg(c);
	if (!names[i][0])
		snprintf(names[i], sizeof(names[i]), "%-13.13s", g_fn_name[i]);
	out(names[i]);
	if (st == ST_WAIT && g_run.current == i)
		outf("%s", (const char *[]){"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"}[(now / 80) % 10]);
	else if (st_pass(st))
	{
		if (dt < 400)
			fg(rgb(255, 230, 120));
		out(dt < 400 ? "★" : (st == ST_WARN ? "≈" : "✔"));
	}
	else if (st == ST_WAIT)
		out("·");
	else
		out("✘");
	bgc(theme()->bg);
}

static void	layout(int *ox, int *oy)
{
	*ox = (term_cols() - 4 * 16) / 2;
	if (*ox < 0)
		*ox = 0;
	*oy = 3;
}

static void	tile_pos(int i, int ox, int oy, int *x, int *y)
{
	int	p;
	int	k;

	p = i < E_substr ? 0 : (i < E_lstnew ? 1 : 2);
	k = i - (p == 0 ? 0 : (p == 1 ? E_substr : E_lstnew));
	*x = ox + (k % 4) * 16;
	*y = oy + (p == 0 ? 1 : (p == 1 ? 9 : 14)) + (k / 4);
}

static void	draw_tour(void)
{
	int	etat[NFN];
	int	ox;
	int	oy;
	int	i;

	for (i = 0; i < NFN; i++)
	{
		etat[i] = 0;
		if (g_shown[i] && st_pass(g_run.fn[i].st))
			etat[i] = 1;
		else if (g_shown[i] && st_bad(g_run.fn[i].st))
			etat[i] = 2;
	}
	layout(&ox, &oy);
	tour_dessiner(etat, NFN, ox + 4 * 16 + 3, oy, 17);
}

static void	draw_grid(long now, int sel)
{
	int	ox;
	int	oy;
	int	i;
	int	x;
	int	y;

	layout(&ox, &oy);
	text_at(ox, oy, g_save.boss[0] ? g_bosses[0].glow : theme()->accent,
		L("PARTIE 1 · LIBC", "PART 1 · LIBC"));
	text_at(ox, oy + 8, g_save.boss[1] ? g_bosses[1].glow : theme()->accent,
		L("PARTIE 2 · ADDITIONNELLES", "PART 2 · ADDITIONAL"));
	text_at(ox, oy + 13, g_save.boss[2] ? g_bosses[2].glow : theme()->accent,
		L("PARTIE 3 · LISTE CHAÎNÉE", "PART 3 · LINKED LIST"));
	for (i = 0; i < NFN; i++)
	{
		tile_pos(i, ox, oy, &x, &y);
		tile(x, y, i, now);
		at(x - 1, y);
		if (i == sel)
		{
			fg(mix(theme()->accent2, rgb(255, 255, 255), 0.5 + 0.5 * sin(now / 120.0)));
			out("▶");
		}
		else
			out(" ");
	}
}

static void	counts(int *ok, int *bad, int *done)
{
	int	i;

	*ok = 0;
	*bad = 0;
	*done = 0;
	for (i = 0; i < NFN; i++)
	{
		if (!g_shown[i])
			continue ;
		*ok += st_pass(g_run.fn[i].st);
		*bad += st_bad(g_run.fn[i].st);
		*done += g_run.fn[i].st != ST_WAIT;
	}
}

static int	reveal_next(long now)
{
	static long	last;
	int			i;

	for (i = 0; i < NFN; i++)
	{
		if (g_shown[i])
			continue ;
		if (g_run.fn[i].st == ST_WAIT)
			return (i);
		if (now - last < (g_save.runs == 0 ? anim_ms(70) : 12))
			return (i);
		g_shown[i] = 1;
		g_reveal[i] = now;
		last = now;
		return (i + 1 < NFN ? i + 1 : -1);
	}
	return (-1);
}

static void	fmt_num(long v, char *buf, size_t sz)
{
	char	t[32];
	int		i;
	int		n;
	int		len;

	snprintf(t, sizeof(t), "%ld", v);
	len = (int)strlen(t);
	n = 0;
	for (i = 0; i < len && n < (int)sz - 2; i++)
	{
		if (i && (len - i) % 3 == 0)
			buf[n++] = ' ';
		buf[n++] = t[i];
	}
	buf[n] = '\0';
}

void	screen_scan(const char *path, int pid)
{
	long	off;
	long	now;
	long	t0;
	long	doneat;
	long	tfirst;
	long	chk;
	int		ok;
	int		bad;
	int		done;
	int		i;
	int		j;
	int		k;
	char	tmp[200];
	char	num[40];
	char	eta[40];

	off = 0;
	t0 = now_ms();
	doneat = 0;
	tfirst = 0;
	memset(g_shown, 0, sizeof(g_shown));
	term_child(pid > 0 ? pid : 0);
	cls();
	while (1)
	{
		results_load(path, &off);
		now = now_ms();
		g_run.current = reveal_next(now);
		for (k = 0; k < 3 && g_save.runs > 0 && g_run.current >= 0
			&& g_run.fn[g_run.current].st != ST_WAIT; k++)
			g_run.current = reveal_next(now + (k + 1) * 13);
		if (!g_run.done && pid > 0 && kill(pid, 0) != 0)
			g_run.done = 1;
		if (g_run.done && !doneat && g_run.current < 0)
			doneat = now;
		if (g_run.done && g_run.current >= 0 && g_run.fn[g_run.current].st == ST_WAIT)
			g_shown[g_run.current] = 1;
		if (doneat && now - doneat > (g_save.runs == 0 ? anim_ms(1300) : 120))
			break ;
		bgc(theme()->bg);
		frame();
		center(1, mix(theme()->accent, theme()->accent2, 0.5 + 0.5 * sin(now / 400.0)),
			L("▚▚  A N A L Y S E   D E   T A   L I B F T  ▞▞", "▚▚  S C A N N I N G   Y O U R   L I B F T  ▞▞"));
		snprintf(tmp, sizeof(tmp), "%-70.70s", g_run.stage[0] ? g_run.stage : L("préparation...", "warming up..."));
		k = (term_cols() - 70) / 2;
		text_at(k < 0 ? 0 : k, 2, theme()->dim, tmp);
		draw_grid(now, -1);
		if (!tdf_menu_unique())
			draw_tour();
		counts(&ok, &bad, &done);
		layout(&k, &i);
		if (done > 0 && !tfirst)
			tfirst = now;
		chk = 0;
		for (j = 0; j < NFN; j++)
			if (g_shown[j] && g_run.fn[j].st != ST_WAIT)
				chk += g_run.fn[j].checks;
		fmt_num(chk, num, sizeof(num));
		if (done >= NFN)
			snprintf(eta, sizeof(eta), "%s", L("terminé", "done"));
		else if (done >= 3)
			snprintf(eta, sizeof(eta), L("reste ~%lds", "~%lds left"),
				(now - tfirst) / 1000 * (NFN - done) / done + 1);
		else
			snprintf(eta, sizeof(eta), "%s", "...");
		if (g_run.current >= 0)
			snprintf(tmp, sizeof(tmp), "%3d %%  ·  ft_%s  ·  %s %s  ·  %s", done * 100 / NFN,
				g_fn_name[g_run.current], num, L("contrôles", "checks"), eta);
		else
			snprintf(tmp, sizeof(tmp), "%3d %%  ·  %s  ·  %s %s  ·  %s", done * 100 / NFN,
				L("compilation et préparation", "compiling and preparing"), num,
				L("contrôles", "checks"), eta);
		center(i + 18, theme()->text, tmp);
		bar(k, i + 19, 62, done / (double)NFN, theme()->accent, theme()->accent2);
		at(k, i + 20);
		fg(rgb(80, 230, 140));
		outf("✔ %-3d", ok);
		fg(rgb(255, 90, 100));
		outf(" ✘ %-3d", bad);
		fg(theme()->dim);
		outf(L(" %2d/%d testées   %lds   %s         ", " %2d/%d tested   %lds   %s         "),
			done, NFN, (now - t0) / 1000,
			g_run.protos_done ? L("prototypes vérifiés", "prototypes checked") : L("prototypes...", "prototypes..."));
		center(term_rows() - 1, theme()->dim, L("[ Q / Échap / Ctrl+C : arrêter l'analyse ]",
				"[ Q / Esc / Ctrl+C: stop the scan ]"));
		flush();
		k = key_read(33);
		if (key_stop(k))
		{
			if (pid > 0)
				kill(pid, SIGTERM);
			g_run.done = 1;
			break ;
		}
	}
	term_child(0);
	for (i = 0; i < NFN; i++)
		g_shown[i] = 1;
	cls();
}

static int	moulinette_ok(char *why, size_t n)
{
	int	i;

	for (i = 0; i < NFN; i++)
		if (st_bad(g_run.fn[i].st) || g_run.fn[i].st == ST_WAIT)
		{
			snprintf(why, n, "ft_%s : %s", g_fn_name[i], st_label(g_run.fn[i].st));
			return (0);
		}
	for (i = 0; i < NFN; i++)
		if (g_run.fn[i].proto != 1)
		{
			snprintf(why, n, L("prototype de ft_%s différent du sujet", "ft_%s prototype differs from the subject"),
				g_fn_name[i]);
			return (0);
		}
	for (i = 0; i < g_run.naudit; i++)
		if (g_run.audit[i].st == ST_KO)
		{
			snprintf(why, n, "audit : %s", g_run.audit[i].label);
			return (0);
		}
	for (i = 0; i < g_run.naudit; i++)
		if (!strcmp(g_run.audit[i].label, L("norminette introuvable : la Norme n'est PAS vérifiée", "norminette not found: the Norm is NOT checked"))
			|| !strcmp(g_run.audit[i].label, L("norminette désactivée dans les paramètres", "norminette turned off in the settings")))
		{
			snprintf(why, n, "%s", L("norminette pas vérifiée", "norminette not checked"));
			return (0);
		}
	if (!g_run.audit_done)
	{
		snprintf(why, n, "%s", L("audit non lancé", "audit not run"));
		return (0);
	}
	snprintf(why, n, "%s", L("tout est propre", "everything is clean"));
	return (1);
}

static const char	*norme_libft(void)
{
	static char	p[700];

	snprintf(p, sizeof(p), "%s/derniere/norme_libft.txt", g_savedir);
	return (p);
}

static void	detail_panel(int sel, int y)
{
	t_res	*r;
	int		x;
	int		i;
	int		n;
	char	tmp[200];
	char	nl[3][200];

	layout(&x, &i);
	r = &g_run.fn[sel];
	at(x, y);
	fg(st_color(r->st));
	outf("ft_%s  %s", g_fn_name[sel], st_label(r->st));
	fg(theme()->dim);
	if (r->checks)
		outf(L("   %d contrôles  %d.%02ds", "   %d checks  %d.%02ds"), r->checks, r->ms / 1000, r->ms % 1000 / 10);
	outf("   prototype %s", r->proto == 1 ? "OK" : (r->proto == 3 ? L("ABSENT", "MISSING")
			: (r->proto == 2 ? L("FAUX", "WRONG") : "?")));
	for (i = 0; i < r->nd && i < 5 && y + 1 + i < term_rows() - 1; i++)
	{
		snprintf(tmp, sizeof(tmp), "  %.*s", term_cols() - x - 3 > 0 ? term_cols() - x - 3 : 10, r->detail[i]);
		text_at(x, y + 1 + i, theme()->text, tmp);
	}
	y += 1 + i;
	snprintf(tmp, sizeof(tmp), "ft_%s.c", g_fn_name[sel]);
	n = tdf_norme_de(norme_libft(), tmp, nl, 3);
	for (i = 0; i < n && y + i < term_rows() - 3; i++)
		text_clip(x + 2, y + i, rgb(255, 120, 120), nl[i], term_cols() - x - 4);
}

static int	audit_lines(char lines[][200], int *kind, int max)
{
	int	n;
	int	i;

	n = 0;
	if (!g_run.audit_done && n < max)
	{
		kind[n] = -1;
		snprintf(lines[n++], 200, "%s", L("L'audit complet se lance quand les 43 fonctions sont propres.",
				"The full audit runs once the 43 functions are clean."));
	}
	for (i = 0; i < g_run.naudit && n < max; i++)
	{
		kind[n] = g_run.audit[i].st;
		snprintf(lines[n++], 200, "%s", g_run.audit[i].label);
		if (g_run.audit[i].detail[0] && n < max)
		{
			kind[n] = -2;
			snprintf(lines[n++], 200, "%s", g_run.audit[i].detail);
		}
	}
	for (i = 0; i < g_run.nwarn && n < max; i++)
	{
		kind[n] = -3;
		snprintf(lines[n++], 200, "%s", g_run.warn[i]);
	}
	return (n);
}

static void	audit_view(void)
{
	static char	lines[96][200];
	int			kind[96];
	int			n;
	int			top;
	int			h;
	int			i;
	int			k;
	int			x;
	int			w;

	n = audit_lines(lines, kind, 96);
	top = 0;
	cls();
	while (1)
	{
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		w = term_cols() - x - 1;
		h = term_rows() - 5;
		bgc(theme()->bg);
		frame();
		text_at(x, 1, theme()->accent, L("AUDIT · CE QUE LA MOULINETTE REGARDE", "AUDIT · WHAT THE MOULINETTE CHECKS"));
		for (i = 0; i < h && top + i < n; i++)
		{
			k = kind[top + i];
			if (k >= 0)
			{
				at(x, 3 + i);
				bgc(st_color(k));
				fg(rgb(10, 10, 10));
				outf(" %-5s ", k == ST_OK ? "OK" : (k == ST_KO ? L("FAUX", "FAIL") : "WARN"));
				bgc(theme()->bg);
				text_clip(x + 9, 3 + i, theme()->text, lines[top + i], w - 9);
			}
			else if (k == -2)
				text_clip(x + 9, 3 + i, theme()->dim, lines[top + i], w - 9);
			else if (k == -3)
			{
				text_at(x, 3 + i, rgb(255, 200, 70), "!");
				text_clip(x + 2, 3 + i, rgb(255, 200, 70), lines[top + i], w - 2);
			}
			else
				text_clip(x, 3 + i, theme()->dim, lines[top + i], w);
		}
		if (n > h)
		{
			at(x, term_rows() - 2);
			fg(theme()->accent2);
			outf(L("lignes %d-%d sur %d   ▲ ▼ défiler   ", "lines %d-%d of %d   ▲ ▼ scroll   "),
				top + 1, top + i, n);
		}
		else
			at(x, term_rows() - 2);
		fg(theme()->dim);
		out(L("[ Échap / Entrée : revenir ]", "[ Esc / Enter: back ]"));
		flush();
		k = key_dir(key_read(100));
		if (k < 0)
			continue ;
		if (k == 1001 && top + h < n)
			top++;
		else if (k == 1000 && top > 0)
			top--;
		else if (k != 1001 && k != 1000 && k != 1002 && k != 1003)
			break ;
	}
	cls();
}

#define FICHE_MAX 90

static void	fiche_add(char lines[][220], int *kind, int *n, int w, int k, const char *s)
{
	const char	*e;
	const char	*sp;
	const char	*ind;
	int			cols;
	int			len;
	int			first;
	int			wmax;

	if (w < 10)
		w = 10;
	ind = (!strncmp(s, "• ", strlen("• ")) || !strncmp(s, "  ↳", strlen("  ↳"))) ? "    " : "";
	first = 1;
	while (*s && *n < FICHE_MAX)
	{
		wmax = first ? w : w - (int)strlen(ind);
		cols = 0;
		sp = NULL;
		e = s;
		while (*e && (cols < wmax || (((unsigned char)*e & 0xC0) == 0x80)))
		{
			if (*e == ' ')
				sp = e;
			if (((unsigned char)*e & 0xC0) != 0x80)
				cols++;
			e++;
		}
		if (*e && sp && sp > s)
			e = sp;
		len = (int)(e - s);
		if (len > 210)
			len = 210;
		snprintf(lines[*n], 220, "%s%.*s", first ? "" : ind, len, s);
		first = 0;
		utf8_fix(lines[*n]);
		kind[(*n)++] = k;
		s = e;
		while (*s == ' ')
			s++;
	}
}

static void	hide_lines(char *s)
{
	char	*p;
	char	*q;

	p = s;
	while ((p = strstr(p, ".c:")))
	{
		q = p + 3;
		while (*q >= '0' && *q <= '9')
			q++;
		if (*q == ':')
		{
			q++;
			while (*q >= '0' && *q <= '9')
				q++;
		}
		if (q > p + 3)
			memmove(p + 2, q, strlen(q) + 1);
		p += 2;
	}
}

static const char	*fiche_hint(int st, const char *all, int which)
{
	static const char	*fr[] = {
		"Ton malloc demande plus de place que le résultat n'en utilise. Calcule d'abord la taille exacte du résultat (+ 1 pour le '\\0'), puis fais le malloc avec cette taille. Tripouille le refuse aussi (MKO).",
		"Ton malloc ne laisse pas assez de place (le '\\0' final ?). Recompte la taille avant de remplir le bloc.",
		"Un free vise un bloc déjà libéré, jamais alloué, ou une chaîne littérale. Cherche le double free et les free d'arguments.",
		"Ta fonction libère une mémoire qu'elle n'a pas allouée (un de ses arguments). Elle ne doit libérer que ce qu'elle a elle-même alloué.",
		"Des blocs alloués ne sont jamais libérés. Regarde les chemins d'erreur et les résultats intermédiaires.",
		"Teste chaque malloc. S'il échoue : libère tout ce qui est déjà alloué (par exemple les mots précédents d'un split) et renvoie NULL.",
		"Tu lis ou écris hors d'un bloc. Vérifie la taille allouée, la condition de boucle et le '\\0' final.",
		"Comportement indéfini : variable non initialisée, dépassement d'entier, pointeur NULL ou décalage invalide. La ligne est indiquée plus haut.",
		"Boucle infinie : vérifie la condition d'arrêt (le '\\0', un compteur qui avance).",
		"Le fichier ne compile pas avec -Wall -Wextra -Werror : corrige l'erreur indiquée plus haut.",
		"Compare la ligne « attendu » avec ce que ta fonction renvoie sur ce cas précis, puis rejoue-le seul avec la commande ci-dessous."};
	static const char	*en[] = {
		"Your malloc asks for more room than the result uses. Compute the exact result size first (+1 for the '\\0'), then malloc that. Tripouille rejects it too (MKO).",
		"Your malloc leaves too little room (the final '\\0'?). Recount the size before filling the block.",
		"A free targets a block already freed, never allocated, or a string literal. Look for double frees and frees of arguments.",
		"Your function frees memory it did not allocate (one of its arguments). It must only free what it allocated itself.",
		"Allocated blocks are never freed. Check the error paths and the intermediate results.",
		"Check every malloc. If one fails: free everything already allocated (e.g. the previous words of a split) and return NULL.",
		"You read or write outside a block. Check the allocated size, the loop condition and the final '\\0'.",
		"Undefined behavior: uninitialized variable, integer overflow, NULL pointer or invalid shift. The line is shown above.",
		"Infinite loop: check the stop condition (the '\\0', a counter that advances).",
		"The file does not compile with -Wall -Wextra -Werror: fix the error shown above.",
		"Compare the \"expected\" line with what your function returns on this exact case, then replay it alone with the command below."};
	int					id;

	id = -1;
	if (which == 0 && strstr(all, "trop grand"))
		id = 0;
	else if (which == 1 && strstr(all, "trop petit"))
		id = 1;
	else if (which == 2 && (strstr(all, "n'est pas alloué") || strstr(all, "n'est plus un bloc")))
		id = 2;
	else if (which == 3 && strstr(all, "ne lui appartient"))
		id = 3;
	else if (which == 4 && (st == ST_LEAK || strstr(all, "jamais libérés") || strstr(all, "jamais passés à del")))
		id = 4;
	else if (which == 5 && strstr(all, "quand malloc"))
		id = 5;
	else if (which == 6 && (strstr(all, "buffer-overflow") || strstr(all, "use-after")
			|| strstr(all, "SEGV") || strstr(all, "plantage")))
		id = 6;
	else if (which == 7 && (st == ST_UB || strstr(all, "runtime error")))
		id = 7;
	else if (which == 8 && st == ST_TIME)
		id = 8;
	else if (which == 9 && st == ST_BUILD)
		id = 9;
	else if (which == 10 && st == ST_KO && !strstr(all, "malloc"))
		id = 10;
	if (id < 0)
		return (NULL);
	return (L(fr[id], en[id]));
}

static const char	*fiche_questions(const char *f)
{
	if (!strcmp(f, "isalpha"))
		return (L("Pourquoi la fonction prend un int et pas un char ?§Que renvoie-t-elle pour une lettre : 1 ou une autre valeur non nulle, et est-ce grave ?§Que se passe-t-il avec un caractère accentué comme é ?",
				"Why does it take an int and not a char?§What does it return for a letter: 1 or another non-zero value, and does it matter?§What happens with an accented character like é?"));
	if (!strcmp(f, "isdigit"))
		return (L("Quelle différence entre le caractère '5' et le nombre 5 ?§Comment tester qu'un caractère est un chiffre sans écrire 48 et 57 ?§Que renvoie ta fonction pour -1 ?",
				"What is the difference between the character '5' and the number 5?§How do you check that a character is a digit without writing 48 and 57?§What does your function return for -1?"));
	if (!strcmp(f, "isalnum"))
		return (L("Comment réutiliser tes autres fonctions pour l'écrire ?§Le caractère '_' est-il alphanumérique ?§Pourquoi éviter de recopier le code de ft_isalpha dedans ?",
				"How do you reuse your other functions to write it?§Is the '_' character alphanumeric?§Why avoid copying the code of ft_isalpha into it?"));
	if (!strcmp(f, "isascii"))
		return (L("Quelle est la plage de la table ASCII et pourquoi 127 en fait partie ?§Que renvoie ta fonction pour 128 et pour -1 ?§Quelle différence avec ft_isprint ?",
				"What is the range of the ASCII table and why is 127 part of it?§What does your function return for 128 and for -1?§What is the difference with ft_isprint?"));
	if (!strcmp(f, "isprint"))
		return (L("Pourquoi l'espace est-il imprimable mais pas la tabulation ?§Quelles sont les bornes exactes et d'où viennent-elles ?§Le caractère 127 est-il imprimable ?",
				"Why is the space printable but not the tab?§What are the exact bounds and where do they come from?§Is character 127 printable?"));
	if (!strcmp(f, "strlen"))
		return (L("Pourquoi le type de retour est size_t et pas int ?§Que se passe-t-il si on lui passe NULL, et pourquoi la vraie strlen ne s'en protège pas ?§Combien de tours fait ta boucle pour une chaîne de 5 caractères ?",
				"Why is the return type size_t and not int?§What happens if you pass it NULL, and why does the real strlen not protect against it?§How many times does your loop run for a 5-character string?"));
	if (!strcmp(f, "memset"))
		return (L("Pourquoi le paramètre c est un int alors qu'on écrit des octets ?§Que renvoie la fonction et à quoi ça sert ?§Peut-on l'utiliser pour remplir un tableau d'int avec la valeur 1 ? Pourquoi ?",
				"Why is the parameter c an int when bytes are written?§What does the function return and what is it for?§Can you use it to fill an int array with the value 1? Why?"));
	if (!strcmp(f, "bzero"))
		return (L("Quelle différence avec ft_memset ?§Pourquoi la fonction ne renvoie rien ?§Que se passe-t-il quand n vaut 0 ?",
				"What is the difference with ft_memset?§Why does the function return nothing?§What happens when n is 0?"));
	if (!strcmp(f, "memcpy"))
		return (L("Pourquoi convertir les pointeurs en unsigned char * ?§Que se passe-t-il si les deux zones se chevauchent ?§Pourquoi les paramètres sont des void * ?",
				"Why convert the pointers to unsigned char *?§What happens if the two areas overlap?§Why are the parameters void *?"));
	if (!strcmp(f, "memmove"))
		return (L("Dans quel cas faut-il copier de la fin vers le début ?§Comment ta fonction sait-elle dans quel sens copier ?§Pourquoi ne pas simplement passer par un tableau temporaire ?",
				"In which case must you copy from the end to the start?§How does your function know which direction to copy in?§Why not simply go through a temporary array?"));
	if (!strcmp(f, "strlcpy"))
		return (L("Que renvoie la fonction et pourquoi cette valeur est utile ?§Combien de caractères sont copiés au maximum quand size vaut 10 ?§Que se passe-t-il quand size vaut 0 ?",
				"What does the function return and why is that value useful?§How many characters are copied at most when size is 10?§What happens when size is 0?"));
	if (!strcmp(f, "strlcat"))
		return (L("Que renvoie la fonction quand size est plus petit que la longueur de dst ?§Comment savoir qu'il y a eu une troncature ?§Pourquoi le résultat est-il toujours terminé par '\\0' quand il reste de la place ?",
				"What does the function return when size is smaller than the length of dst?§How do you know that the result was truncated?§Why is the result always ended by '\\0' when there is room left?"));
	if (!strcmp(f, "toupper"))
		return (L("Que renvoie ta fonction pour un caractère qui n'est pas une minuscule ?§Comment passer de 'a' à 'A' sans écrire 32 ?§Pourquoi le paramètre et le retour sont des int ?",
				"What does your function return for a character that is not lowercase?§How do you go from 'a' to 'A' without writing 32?§Why are the parameter and the return value ints?"));
	if (!strcmp(f, "tolower"))
		return (L("Que se passe-t-il avec '@' et '[' qui entourent les majuscules dans la table ASCII ?§Comment vérifies-tu les bornes ?§Quel lien avec ft_toupper ?",
				"What happens with '@' and '[' which surround the capitals in the ASCII table?§How do you check the bounds?§What is the link with ft_toupper?"));
	if (!strcmp(f, "strchr"))
		return (L("Que renvoie la fonction si on cherche '\\0' ?§Pourquoi le retour est char * alors que le paramètre est const char * ?§Que se passe-t-il si c vaut 'a' + 256 ?",
				"What does the function return when you look for '\\0'?§Why is the return char * when the parameter is const char *?§What happens if c is 'a' + 256?"));
	if (!strcmp(f, "strrchr"))
		return (L("Comment trouves-tu la dernière occurrence : en partant de la fin ou du début ?§Que renvoie la fonction quand le caractère est absent ?§Et quand on cherche '\\0' ?",
				"How do you find the last occurrence: starting from the end or from the start?§What does the function return when the character is missing?§And when you look for '\\0'?"));
	if (!strcmp(f, "strncmp"))
		return (L("Pourquoi comparer en unsigned char ?§Que renvoie la fonction quand n vaut 0 ?§Pourquoi s'arrêter au premier '\\0' ?",
				"Why compare as unsigned char?§What does the function return when n is 0?§Why stop at the first '\\0'?"));
	if (!strcmp(f, "memchr"))
		return (L("Quelle différence avec ft_strchr ?§Ta fonction s'arrête-t-elle sur un '\\0' ? Pourquoi ?§Pourquoi convertir c en unsigned char ?",
				"What is the difference with ft_strchr?§Does your function stop at a '\\0'? Why?§Why convert c to unsigned char?"));
	if (!strcmp(f, "memcmp"))
		return (L("Quelle différence avec ft_strncmp face à un '\\0' au milieu ?§Que signifie le signe du résultat ?§Pourquoi comparer en unsigned char ?",
				"What is the difference with ft_strncmp when there is a '\\0' in the middle?§What does the sign of the result mean?§Why compare as unsigned char?"));
	if (!strcmp(f, "strnstr"))
		return (L("Que renvoie la fonction quand la petite chaîne est vide ?§Comment len limite-t-il la recherche ?§Combien de comparaisons fait ta recherche au pire ?",
				"What does the function return when the small string is empty?§How does len limit the search?§How many comparisons does your search make in the worst case?"));
	if (!strcmp(f, "atoi"))
		return (L("Quels caractères sont ignorés au début ?§Que se passe-t-il avec \"--5\" ou \"+-5\" ?§Comment gères-tu INT_MIN sans dépasser la capacité d'un int ?",
				"Which characters are skipped at the start?§What happens with \"--5\" or \"+-5\"?§How do you handle INT_MIN without overflowing an int?"));
	if (!strcmp(f, "calloc"))
		return (L("Pourquoi vérifier que nmemb * size ne déborde pas ?§Que renvoie ta fonction quand nmemb ou size vaut 0 ?§Quelle différence avec malloc ?",
				"Why check that nmemb * size does not overflow?§What does your function return when nmemb or size is 0?§What is the difference with malloc?"));
	if (!strcmp(f, "strdup"))
		return (L("Pourquoi faut-il libérer le résultat ?§Combien d'octets réserves-tu et pourquoi + 1 ?§Que renvoie la fonction si malloc échoue ?",
				"Why must the result be freed?§How many bytes do you allocate and why + 1?§What does the function return if malloc fails?"));
	if (!strcmp(f, "substr"))
		return (L("Que renvoie ta fonction si start dépasse la longueur de s ?§Combien d'octets réserves-tu quand len est plus grand que ce qui reste ?§Pourquoi renvoyer une chaîne vide plutôt que NULL ?",
				"What does your function return if start is past the length of s?§How many bytes do you allocate when len is bigger than what is left?§Why return an empty string rather than NULL?"));
	if (!strcmp(f, "strjoin"))
		return (L("Comment calcules-tu la taille à réserver ?§Que se passe-t-il si malloc échoue ?§Pourquoi ne pas libérer s1 ni s2 ?",
				"How do you compute the size to allocate?§What happens if malloc fails?§Why not free s1 or s2?"));
	if (!strcmp(f, "strtrim"))
		return (L("Comment trouves-tu le début et la fin de la partie à garder ?§Que renvoie la fonction si toute la chaîne est à retirer ?§Pourquoi set est une chaîne et pas un seul caractère ?",
				"How do you find the start and the end of the part to keep?§What does the function return if the whole string must be removed?§Why is set a string and not a single character?"));
	if (!strcmp(f, "split"))
		return (L("Comment comptes-tu les mots avant de réserver le tableau ?§Que fais-tu si un malloc échoue au milieu ?§Que se passe-t-il avec plusieurs séparateurs collés ou au début de la chaîne ?",
				"How do you count the words before allocating the array?§What do you do if a malloc fails halfway?§What happens with several separators in a row or at the start of the string?"));
	if (!strcmp(f, "itoa"))
		return (L("Comment calcules-tu le nombre de caractères à réserver ?§Comment gères-tu INT_MIN ?§Pourquoi remplir la chaîne en partant de la fin ?",
				"How do you compute the number of characters to allocate?§How do you handle INT_MIN?§Why fill the string starting from the end?"));
	if (!strcmp(f, "strmapi"))
		return (L("À quoi sert l'index passé à f ?§Pourquoi la fonction crée une nouvelle chaîne au lieu de modifier s ?§Que se passe-t-il si malloc échoue ?",
				"What is the index passed to f for?§Why does the function create a new string instead of changing s?§What happens if malloc fails?"));
	if (!strcmp(f, "striteri"))
		return (L("Quelle différence avec ft_strmapi ?§Pourquoi f reçoit un char * et pas un char ?§Que renvoie la fonction ?",
				"What is the difference with ft_strmapi?§Why does f receive a char * and not a char?§What does the function return?"));
	if (!strcmp(f, "putchar_fd"))
		return (L("Qu'est-ce qu'un descripteur de fichier ?§À quoi correspondent 0, 1 et 2 ?§Pourquoi utiliser write et pas printf ?",
				"What is a file descriptor?§What do 0, 1 and 2 stand for?§Why use write and not printf?"));
	if (!strcmp(f, "putstr_fd"))
		return (L("Pourquoi un seul write est-il plus efficace qu'un write par caractère ?§Que fais-tu si s vaut NULL ?§Comment vérifier que le texte part bien sur la sortie d'erreur ?",
				"Why is a single write more efficient than one write per character?§What do you do if s is NULL?§How do you check that the text really goes to the error output?"));
	if (!strcmp(f, "putendl_fd"))
		return (L("Quelle différence avec ft_putstr_fd ?§Comment réutilises-tu tes autres fonctions ?§Que se passe-t-il si s est vide ?",
				"What is the difference with ft_putstr_fd?§How do you reuse your other functions?§What happens if s is empty?"));
	if (!strcmp(f, "putnbr_fd"))
		return (L("Comment affiches-tu un nombre chiffre par chiffre ?§Comment gères-tu INT_MIN ?§Pourquoi la récursivité est-elle pratique ici ?",
				"How do you print a number digit by digit?§How do you handle INT_MIN?§Why is recursion handy here?"));
	if (!strcmp(f, "lstnew"))
		return (L("Que vaut next dans le nouveau maillon ?§Pourquoi le contenu est un void * ?§Que renvoie la fonction si malloc échoue ?",
				"What is next in the new node?§Why is the content a void *?§What does the function return if malloc fails?"));
	if (!strcmp(f, "lstadd_front"))
		return (L("Pourquoi lst est un t_list ** et pas un t_list * ?§Que se passe-t-il si la liste est vide ?§Dans quel ordre faut-il changer les pointeurs ?",
				"Why is lst a t_list ** and not a t_list *?§What happens if the list is empty?§In which order must the pointers be changed?"));
	if (!strcmp(f, "lstsize"))
		return (L("Que renvoie la fonction pour une liste vide ?§Comment parcours-tu la liste ?§Quelle condition arrête ta boucle ?",
				"What does the function return for an empty list?§How do you walk through the list?§Which condition stops your loop?"));
	if (!strcmp(f, "lstlast"))
		return (L("Que renvoie la fonction pour une liste vide ?§Pourquoi tester next et pas le maillon lui-même ?§Comment l'utilises-tu dans ft_lstadd_back ?",
				"What does the function return for an empty list?§Why test next and not the node itself?§How do you use it in ft_lstadd_back?"));
	if (!strcmp(f, "lstadd_back"))
		return (L("Que se passe-t-il quand la liste est vide ?§Pourquoi lst est un t_list ** ?§Comment trouves-tu le dernier maillon ?",
				"What happens when the list is empty?§Why is lst a t_list **?§How do you find the last node?"));
	if (!strcmp(f, "lstdelone"))
		return (L("Pourquoi ne pas libérer next ?§À quoi sert la fonction del ?§Que se passe-t-il si on utilise le maillon après l'appel ?",
				"Why not free next?§What is the del function for?§What happens if the node is used after the call?"));
	if (!strcmp(f, "lstclear"))
		return (L("Pourquoi garder next avant de libérer le maillon ?§Que vaut *lst à la fin et pourquoi ?§Comment réutilises-tu ft_lstdelone ?",
				"Why keep next before freeing the node?§What is *lst at the end and why?§How do you reuse ft_lstdelone?"));
	if (!strcmp(f, "lstiter"))
		return (L("Quelle différence avec ft_lstmap ?§La fonction f peut-elle changer le contenu ?§Que se passe-t-il pour une liste vide ?",
				"What is the difference with ft_lstmap?§Can the function f change the content?§What happens for an empty list?"));
	if (!strcmp(f, "lstmap"))
		return (L("Que fais-tu si un malloc échoue au milieu de la nouvelle liste ?§Pourquoi a-t-on besoin de del ?§Que devient le contenu créé par f si le nouveau maillon ne peut pas être créé ?",
				"What do you do if a malloc fails in the middle of the new list?§Why is del needed?§What happens to the content made by f if the new node cannot be created?"));
	return (NULL);
}

static int	fiche_build(int fn, char lines[][220], int *kind, int w)
{
	t_res		*r;
	char		d[220];
	char		all[1700];
	char		tmp[260];
	char		*cas;
	char		*att;
	const char	*h;
	const char	*q;
	const char	*e;
	char		qb[420];
	int			n;
	int			i;

	r = &g_run.fn[fn];
	n = 0;
	all[0] = '\0';
	for (i = 0; i < r->nd; i++)
	{
		strncat(all, r->detail[i], sizeof(all) - strlen(all) - 2);
		strcat(all, " ");
	}
	fiche_add(lines, kind, &n, w, 5, L("CE QUI A ÉTÉ VU", "WHAT WAS SEEN"));
	if (!r->nd)
		fiche_add(lines, kind, &n, w, 0, st_pass(r->st)
			? L("Rien à signaler : toutes les vérifications passent.", "Nothing to report: every check passes.")
			: L("Pas de détail pour cette fonction.", "No details for this function."));
	for (i = 0; i < r->nd; i++)
	{
		snprintf(d, sizeof(d), "%s", r->detail[i]);
		if (g_save.cfg.help < 1)
			hide_lines(d);
		cas = strstr(d, "  · cas : ");
		if (cas)
			*cas = '\0';
		att = strstr(d, ", attendu ");
		if (!att)
			att = strstr(d, ", la libc ");
		if (!att)
			att = strstr(d, ", libc says ");
		if (att)
			*att = '\0';
		snprintf(tmp, sizeof(tmp), "• %s", d);
		fiche_add(lines, kind, &n, w, 0, tmp);
		if (att)
		{
			snprintf(tmp, sizeof(tmp), "  ↳ %s", strstr(r->detail[i], ", attendu ")
				? strstr(r->detail[i], ", attendu ") + 2
				: (strstr(r->detail[i], ", la libc ") ? strstr(r->detail[i], ", la libc ") + 2
					: strstr(r->detail[i], ", libc says ") + 2));
			if ((cas = strstr(tmp, "  · cas : ")))
				*cas = '\0';
			fiche_add(lines, kind, &n, w, 2, tmp);
		}
		cas = strstr(r->detail[i], "  · cas : ");
		if (cas)
		{
			snprintf(tmp, sizeof(tmp), "  ↳ %s : %s", L("cas", "case"), cas + strlen("  · cas : "));
			fiche_add(lines, kind, &n, w, 3, tmp);
		}
	}
	h = NULL;
	for (i = 0; i <= 10 && !h && g_save.cfg.help >= 2; i++)
		h = fiche_hint(r->st, all, i);
	if (h)
	{
		fiche_add(lines, kind, &n, w, 5, L("PISTE", "HINT"));
		fiche_add(lines, kind, &n, w, 4, h);
	}
	q = fiche_questions(g_fn_name[fn]);
	if (q)
	{
		snprintf(qb, sizeof(qb), "%s%s", L("QUESTIONS POUR TE PRÉPARER À LA SOUTENANCE", "QUESTIONS TO PREPARE FOR THE DEFENCE"),
			tdf_nouveau("3.4") ? "   NEW" : "");
		fiche_add(lines, kind, &n, w, 5, qb);
		while (*q)
		{
			e = strstr(q, "§");
			snprintf(qb, sizeof(qb), "• %.*s", e ? (int)(e - q) : (int)strlen(q), q);
			fiche_add(lines, kind, &n, w, 0, qb);
			q = e ? e + strlen("§") : q + strlen(q);
		}
	}
	fiche_add(lines, kind, &n, w, 5, L("TU AS CORRIGÉ ?", "FIXED IT?"));
	fiche_add(lines, kind, &n, w, 6, L("r : relancer les tests sans quitter le testeur", "r: run the tests again without leaving the tester"));
	return (n);
}

static int	screen_fiche(int fn)
{
	static char	lines[FICHE_MAX][220];
	int			kind[FICHE_MAX];
	int			n;
	int			top;
	int			h;
	int			i;
	int			k;
	int			x;
	int			w;
	int			j;
	t_res		*r;
	char		tmp[200];

	top = 0;
	cls();
	while (1)
	{
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		w = term_cols() - x - 3;
		if (w > 74)
			w = 74;
		n = fiche_build(fn, lines, kind, w);
		h = term_rows() - 6;
		if (top > n - h)
			top = n - h > 0 ? n - h : 0;
		r = &g_run.fn[fn];
		bgc(theme()->bg);
		frame();
		snprintf(tmp, sizeof(tmp), L("FICHE · ft_%s", "DETAILS · ft_%s"), g_fn_name[fn]);
		text_at(x, 1, theme()->accent2, tmp);
		at(x, 2);
		bgc(st_color(r->st));
		fg(rgb(10, 10, 10));
		outf(" %-6s ", st_label(r->st));
		bgc(theme()->bg);
		fg(theme()->dim);
		if (r->checks)
			outf(L("   %d contrôles  %d.%02ds", "   %d checks  %d.%02ds"), r->checks,
				r->ms / 1000, r->ms % 1000 / 10);
		for (i = 0; i < h && top + i < n; i++)
		{
			k = kind[top + i];
			text_clip(x, 4 + i, k == 5 ? theme()->accent : (k == 2 ? rgb(80, 230, 140)
					: (k == 3 ? theme()->dim : (k == 4 ? theme()->accent2
							: (k == 6 ? rgb(255, 230, 140) : theme()->text)))),
				lines[top + i], term_cols() - x - 1);
		}
		at(x, term_rows() - 1);
		fg(theme()->accent2);
		if (n > h)
			outf(L("lignes %d-%d sur %d   ▲ ▼ défiler   ", "lines %d-%d of %d   ▲ ▼ scroll   "),
				top + 1, top + i, n);
		fg(theme()->dim);
		out(L("◀ ▶ autre fonction en échec   Échap : retour", "◀ ▶ other failing function   Esc: back"));
		flush();
		k = key_dir(key_read(100));
		if (k < 0)
			continue ;
		if (k == 1001 && top + h < n)
			top++;
		else if (k == 1000 && top > 0)
			top--;
		else if (k == 1002 || k == 1003)
		{
			for (j = fn + (k == 1002 ? 1 : -1); j >= 0 && j < NFN; j += (k == 1002 ? 1 : -1))
				if (st_bad(g_run.fn[j].st) || g_run.fn[j].proto > 1)
					break ;
			if (j >= 0 && j < NFN)
			{
				fn = j;
				top = 0;
			}
		}
		else if (k == 'r' || k == 'R')
			return (cls(), -1);
		else if (k != 1001 && k != 1000)
			break ;
	}
	cls();
	return (fn);
}

static long	auto_date(const char *d)
{
	DIR				*dp;
	struct dirent	*e;
	struct stat		st;
	char			p[1200];
	long			m;
	size_t			l;

	m = 0;
	dp = d ? opendir(d) : NULL;
	if (!dp)
		return (0);
	while ((e = readdir(dp)))
	{
		l = strlen(e->d_name);
		if (strcmp(e->d_name, "Makefile") && (l < 3 || e->d_name[l - 2] != '.'
				|| (e->d_name[l - 1] != 'c' && e->d_name[l - 1] != 'h')))
			continue ;
		snprintf(p, sizeof(p), "%s/%s", d, e->d_name);
		if (!stat(p, &st) && (long)st.st_mtime > m)
			m = (long)st.st_mtime;
	}
	closedir(dp);
	return (m);
}

int	screen_results(int after)
{
	const char	*auto_dir;
	long		auto_ref;
	long		auto_t;
	char	cmp[300];
	int		cmp_fait;
	long	ouvert;
	int		sel;
	int		k;
	int		ok;
	int		bad;
	int		done;
	int		x;
	int		y;
	int		m;
	char	why[200];
	char	tmp[260];

	sel = 0;
	for (k = 0; k < NFN; k++)
		if (st_bad(g_run.fn[k].st) || g_run.fn[k].proto > 1)
		{
			sel = k;
			break ;
		}
	cmp[0] = '\0';
	cmp_fait = 0;
	cls();
	tdf_key_vider();
	ouvert = now_ms();
	auto_dir = getenv("TDF_AUTO_DIR");
	auto_ref = auto_date(auto_dir);
	auto_t = ouvert;
	while (1)
	{
		if (auto_dir && auto_ref && g_run.done && now_ms() - auto_t > 1000)
		{
			auto_t = now_ms();
			if (auto_date(auto_dir) > auto_ref)
				return (cls(), 42);
		}
		counts(&ok, &bad, &done);
		m = moulinette_ok(why, sizeof(why));
		layout(&x, &y);
		bgc(theme()->bg);
		frame();
		snprintf(tmp, sizeof(tmp), L("RÉSULTATS  %d/%d  ·  note moulinette estimée : %s",
				"RESULTS  %d/%d  ·  estimated moulinette grade: %s"), ok, NFN, m ? "100" : "0");
		discord_line(0);
		center(1, m ? rgb(80, 230, 140) : rgb(255, 90, 100), tmp);
		if (g_run.done && !cmp_fait)
		{
			comparer_avant(cmp, sizeof(cmp));
			cmp_fait = 1;
		}
		if (cmp[0])
		{
			snprintf(tmp, sizeof(tmp), "%s  ·  %s", why, cmp);
			center(2, theme()->dim, tmp);
		}
		else
			center(2, theme()->dim, why);
		draw_grid(now_ms(), sel);
		if (!tdf_menu_unique())
			draw_tour();
		detail_panel(sel, y + 17);
		bad = tdf_norme_compte(norme_libft(), &k);
		if (bad > 0)
		{
			snprintf(tmp, sizeof(tmp), L("NORMINETTE : %d erreur(s) dans %d fichier(s) · n pour les voir",
					"NORMINETTE: %d error(s) in %d file(s) · n to see them"), bad, k);
			center(term_rows() - 2, rgb(255, 110, 110), tmp);
		}
		if (auto_dir && auto_ref)
			foot_line(x, L("flèches : choisir   f : fiche   n : norminette   m : audit   relance auto à chaque sauvegarde   Entrée : continuer",
					"arrows: pick   f: details   n: norminette   m: audit   auto rerun on save   Enter: continue"));
		else
			foot_line(x, L("flèches : choisir   f : fiche   n : norminette   m : audit   r : corrigé ? relancer   Entrée : continuer",
					"arrows: pick   f: details   n: norminette   m: audit   r: fixed? run again   Enter: continue"));
		flush();
		k = key_read(60);
		if (k >= 0 && now_ms() - ouvert < 500)
			continue ;
		if (k == '\r' || k == '\n' || k == ' ' || key_quit(k))
			break ;
		if (k == 'r' || k == 'R')
		{
			cls();
			return (42);
		}
		if (k == 'f')
		{
			k = screen_fiche(sel);
			if (k < 0)
				return (cls(), 42);
			sel = k;
			continue ;
		}
		if (k == 'n' || k == 'N')
		{
			tdf_norme_voir(norme_libft(), "Libft");
			continue ;
		}
		if (k == 'm' || (k == 'a' && g_save.cfg.keys != 1))
		{
			audit_view();
			continue ;
		}
		k = key_dir(k);
		if (k == 1002 && sel < NFN - 1)
			sel++;
		if (k == 1003 && sel > 0)
			sel--;
		if (k == 1001)
			sel = sel + 4 < NFN ? sel + 4 : NFN - 1;
		if (k == 1000)
			sel = sel - 4 >= 0 ? sel - 4 : 0;
	}
	(void)after;
	cls();
	return (0);
}

static void	roll(int x, int y, const char *label, long from, long to, t_rgb c)
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
		outf("%-40s %+8ld", label, from + (long)((to - from) * k));
		flush();
		sleep_ms(16);
	} while (k < 1);
}

static void	fireworks(int ms)
{
	long	t0;
	long	t;
	int		i;
	int		cx[5];
	int		cy[5];
	double	r;
	double	a;

	for (i = 0; i < 5; i++)
	{
		cx[i] = rand() % term_cols();
		cy[i] = 2 + rand() % (term_rows() - 6);
	}
	t0 = now_ms();
	while ((t = now_ms() - t0) < ms)
	{
		bgc(theme()->bg);
		frame();
		for (i = 0; i < 5; i++)
		{
			r = (t % 700) / 60.0;
			for (a = 0; a < 6.28; a += 0.5)
			{
				if (cx[i] + (int)(cos(a) * r * 2) < 0
					|| cx[i] + (int)(cos(a) * r * 2) >= term_cols()
					|| cy[i] + (int)(sin(a) * r) < 0
					|| cy[i] + (int)(sin(a) * r) >= term_rows())
					continue ;
				at(cx[i] + (int)(cos(a) * r * 2), cy[i] + (int)(sin(a) * r));
				fg(mix(hue(i / 5.0 + a / 6.28), theme()->bg, (t % 700) / 700.0));
				out(r > 8 ? " " : "✦");
			}
		}
		flush();
		sleep_ms(30);
	}
}

static void	stats_push(void)
{
	int	i;
	int	ok;

	ok = 0;
	tdf_stats_begin("libft");
	for (i = 0; i < NFN; i++)
	{
		if (st_pass(g_run.fn[i].st))
			ok++;
		else if (st_bad(g_run.fn[i].st))
			tdf_stats_fail(g_fn_name[i], st_label(g_run.fn[i].st));
	}
	tdf_stats_end(ok, NFN);
}

int	record_progress(void)
{
	int		i;
	int		news;
	int		clean;
	time_t	now;

	news = 0;
	clean = 0;
	for (i = 0; i < NFN; i++)
	{
		if (!st_pass(g_run.fn[i].st))
			continue ;
		clean++;
		if (g_save.cleared[i] != '1')
		{
			news++;
			g_save.cleared[i] = '1';
		}
	}
	g_save.runs++;
	stats_push();
	g_save.xp += news * 40;
	g_save.octets += news * 20;
	now = time(NULL);
	if (now - g_save.lastreward >= 1800)
	{
		g_save.xp += clean * 2;
		g_save.octets += clean;
		g_save.lastreward = now;
	}
	badges_update(1);
	save_write();
	return (clean);
}

static void	rewards_achs(int clean, int all_tested, int noleak, int allproto)
{
	int	i;

	ach_give(0);
	if (all_tested && noleak)
		ach_give(1);
	if (clean == NFN)
		ach_give(2);
	if (allproto)
		ach_give(3);
	if (g_run.audit_done)
	{
		for (i = 0; i < g_run.naudit && g_run.audit[i].st != ST_KO; i++)
			;
		if (i == g_run.naudit)
			ach_give(4);
	}
	if (st_pass(g_run.fn[E_memmove].st))
		ach_give(6);
	if (g_save.runs >= 10)
		ach_give(16);
	if (g_save.octets >= 1000)
		ach_give(15);
	badges_update(0);
	save_write();
}

int	rewards_news(void)
{
	int	i;

	for (i = 0; i < NFN; i++)
		if (st_pass(g_run.fn[i].st) && g_save.cleared[i] != '1')
			return (1);
	return (0);
}

void	rewards_quiet(void)
{
	int	i;
	int	clean;
	int	all_tested;
	int	noleak;
	int	allproto;

	all_tested = 1;
	noleak = 1;
	allproto = 1;
	for (i = 0; i < NFN; i++)
	{
		if (g_run.fn[i].st == ST_SKIP || g_run.fn[i].st == ST_BUILD || g_run.fn[i].st == ST_WAIT)
			all_tested = 0;
		if (g_run.fn[i].st == ST_LEAK)
			noleak = 0;
		if (g_run.fn[i].proto != 1)
			allproto = 0;
	}
	clean = record_progress();
	rewards_achs(clean, all_tested, noleak, allproto);
}

void	screen_rewards(void)
{
	long	xp0;
	long	oct0;
	long	cur;
	int		news;
	int		clean;
	int		i;
	int		x;
	int		y;
	int		l0;
	int		lv;
	int		all_tested;
	int		noleak;
	int		allproto;
	long	t0;
	double	k;
	time_t	now;
	char	tmp[200];

	cls();
	xp0 = g_save.xp;
	oct0 = g_save.octets;
	l0 = level_of(xp0);
	news = 0;
	clean = 0;
	all_tested = 1;
	noleak = 1;
	allproto = 1;
	for (i = 0; i < NFN; i++)
	{
		if (st_pass(g_run.fn[i].st))
		{
			clean++;
			if (g_save.cleared[i] != '1')
			{
				news++;
				g_save.cleared[i] = '1';
			}
		}
		if (g_run.fn[i].st == ST_SKIP || g_run.fn[i].st == ST_BUILD || g_run.fn[i].st == ST_WAIT)
			all_tested = 0;
		if (g_run.fn[i].st == ST_LEAK)
			noleak = 0;
		if (g_run.fn[i].proto != 1)
			allproto = 0;
	}
	x = (term_cols() - 50) / 2;
	y = 3;
	center(1, theme()->accent2, L("R É C O M P E N S E S", "R E W A R D S"));
	g_save.runs++;
	stats_push();
	roll(x, y++, L("XP des fonctions réussies pour la 1re fois", "XP for first-time clean functions"),
		0, news * 40, rgb(255, 230, 120));
	roll(x, y++, L("Octets des premières victoires", "Octets for first wins"), 0, news * 20, theme()->accent);
	g_save.xp += news * 40;
	g_save.octets += news * 20;
	now = time(NULL);
	if (now - g_save.lastreward >= 1800)
	{
		roll(x, y++, L("XP du run (2 par fonction propre)", "Run XP (2 per clean function)"), 0, clean * 2,
			rgb(255, 230, 120));
		roll(x, y++, L("Octets du run (1 par fonction propre)", "Run Octets (1 per clean function)"), 0, clean,
			theme()->accent);
		g_save.xp += clean * 2;
		g_save.octets += clean;
		g_save.lastreward = now;
	}
	else
	{
		snprintf(tmp, sizeof(tmp), L("run déjà récompensé il y a %ld min (attente 30 min)",
				"run already rewarded %ld min ago (30 min cooldown)"), (long)((now - g_save.lastreward) / 60));
		text_at(x, y++, theme()->dim, tmp);
	}
	y++;
	snprintf(tmp, sizeof(tmp), "Wallet : %ld ◈ Octets   (+%ld)", g_save.octets, g_save.octets - oct0);
	text_at(x, y++, rgb(255, 215, 0), tmp);
	t0 = now_ms();
	do
	{
		k = anim_off() ? 1 : (now_ms() - t0) / (double)anim_ms(1400);
		if (k > 1)
			k = 1;
		cur = xp0 + (long)((g_save.xp - xp0) * (1 - pow(1 - k, 3)));
		lv = level_of(cur);
		at(x, y);
		fg(theme()->text);
		outf(L("NIVEAU %-4d %32ld XP", "LEVEL %-4d %33ld XP"), lv, cur);
		bar(x, y + 1, 50, (cur - xp_for(lv)) / (double)(xp_for(lv + 1) - xp_for(lv)),
			theme()->accent, theme()->accent2);
		flush();
		sleep_ms(16);
	} while (k < 1);
	if (level_of(g_save.xp) > l0)
	{
		bell();
		fireworks(anim_ms(1600));
		cls();
		snprintf(tmp, sizeof(tmp), L("NIVEAU %d", "LEVEL %d"), level_of(g_save.xp));
		big_text((term_cols() - big_width(tmp)) / 2, 6, tmp, 1, 3);
		if ((level_of(g_save.xp) - 1) / 10 != (l0 - 1) / 10)
		{
			center(14, rgb(255, 215, 0), L("N O U V E A U   R A N G", "N E W   R A N K"));
			center(16, theme()->accent2, tdf_rang(level_of(g_save.xp), tmp, sizeof(tmp)));
		}
		flush();
		sleep_ms(anim_ms(1400));
		cls();
	}
	rewards_achs(clean, all_tested, noleak, allproto);
	center(term_rows() - 2, theme()->dim, L("[ une touche pour continuer ]", "[ any key to continue ]"));
	flush();
	while (key_read(100) < 0)
		;
	cls();
}

static int	boss_ready(int p)
{
	int	f;

	for (f = g_bosses[p].first; f <= g_bosses[p].last; f++)
		if (!st_pass(g_run.fn[f].st))
			return (0);
	return (part_clean(p));
}

void	boss_battles(int only)
{
	t_scene_out	g;
	int			p;
	int			before;
	char		tmp[200];
	int			all0;

	all0 = g_save.boss[0] && g_save.boss[1] && g_save.boss[2];
	for (p = 0; p < 3; p++)
	{
		if (only >= 0 && p != only)
			continue ;
		if (only == -2 && (g_save.boss[p] || !boss_ready(p)))
			continue ;
		before = g_save.boss[p];
		if (only < 0)
			story_room(p);
		scene_play(&g, p, 0);
		if (g.crashed || g.frozen)
		{
			g_save.crashes++;
			save_write();
			if (screen_crash_choice(&g) && g.fn >= 0)
			{
				g_quar[g.fn] = 1;
				ach_give(5);
				p--;
				continue ;
			}
			break ;
		}
		if (!before && g_save.boss[p])
		{
			g_save.xp += 300;
			g_save.octets += 150;
			bell();
			snprintf(tmp, sizeof(tmp), L("%s · +300 XP  +150 ◈  ·  7 niveaux débloqués",
					"%s · +300 XP  +150 ◈  ·  7 levels unlocked"), L(g_bosses[p].name, g_bosses[p].en));
			toast(L("BOSS VAINCU", "BOSS DEFEATED"), tmp, g_bosses[p].glow);
			ach_give(7 + p);
			save_write();
		}
		if (g.quit)
			break ;
	}
	if (!all0 && g_save.boss[0] && g_save.boss[1] && g_save.boss[2])
		story_libft_finale();
}

void	screen_login(void)
{
	char	buf[32];
	int		premier;

	snprintf(buf, sizeof(buf), "%s", g_save.login);
	premier = !g_save.login[0] || g_save.runs == 0;
	if (!tdf_saisir(premier ? L("Bienvenue, nouveau fou. Ton login 42 :", "Welcome, new lunatic. Your 42 login:")
			: L("Ton pseudo", "Your nickname"),
			premier ? L("Il s'affiche sous ton chevalier et sur ton profil.", "It shows under your knight and on your profile.")
			: L("Il s'affiche sous ton chevalier et sur ton profil.", "It shows under your knight and on your profile."),
			buf, sizeof(buf), 20, 1, g_save.login[0] != '\0'))
		return ;
	snprintf(g_save.login, sizeof(g_save.login), "%s", buf);
	save_write();
	cls();
}

void	screen_profile(void)
{
	int		i;
	int		x;
	int		y;
	int		lv;
	int		n;
	int		m;
	int		o;
	char	rg[80];
	char	tmp[200];
	int		sel;
	int		top;
	int		h;
	int		k;
	int		ok;

	sel = 0;
	top = 0;
	cls();
	while (1)
	{
		bgc(theme()->bg);
		frame();
		x = (term_cols() - 80) / 2;
		if (x < 0)
			x = 0;
		lv = level_of(g_save.xp);
		o = profile_header(x, 1);
		center(1 + o, theme()->accent2, L("P R O F I L", "P R O F I L E"));
		snprintf(tmp, sizeof(tmp), L("%s · niveau %d · %s · %ld XP · %ld ◈", "%s · level %d · %s · %ld XP · %ld ◈"),
			g_save.login, lv, tdf_rang(lv, rg, sizeof(rg)), g_save.xp, g_save.octets);
		center(3 + o, pseudo_color(), tmp);
		if (g_save.bio[0])
			center(2 + o, theme()->dim, g_save.bio);
		for (m = 0, n = 0; n < 3; n++)
			if (g_save.vitrine[n] >= 0)
				m += snprintf(tmp + m, sizeof(tmp) - m, "%s★ %s", m ? "   " : "",
					L(g_badges[g_save.vitrine[n]].fr, g_badges[g_save.vitrine[n]].en));
		if (m)
			center(5 + o, rgb(255, 215, 0), tmp);
		bar(x + 16, 4 + o, 48, (g_save.xp - xp_for(lv)) / (double)(xp_for(lv + 1) - xp_for(lv)),
			theme()->accent, theme()->accent2);
		for (n = 0, i = 0; i < g_nitems; i++)
			n += item_owned(i);
		for (m = 0, i = 0; i < NTITLE; i++)
			m += g_save.titles[i] == '1';
		snprintf(tmp, sizeof(tmp), L("runs %d · boss %d/3 · entraînement %d/21 · objets %d/%d · titres %d/1000 · crashs %d",
				"runs %d · bosses %d/3 · training %d/21 · items %d/%d · titles %d/1000 · crashes %d"),
			g_save.runs, g_save.boss[0] + g_save.boss[1] + g_save.boss[2],
			exo_done_count(), n, g_nitems, m, g_save.crashes);
		center(6 + o, theme()->dim, tmp);
		for (i = 0; i < NFN; i++)
		{
			at(x + 18 + (i % 22) * 2, 8 + o + i / 22);
			fg(g_save.cleared[i] == '1' ? rgb(80, 230, 140) : theme()->dim);
			out(g_save.cleared[i] == '1' ? "■" : "□");
		}
		y = 11 + o;
		for (ok = 0, i = 0; i < NACH; i++)
			ok += ach_has(i);
		snprintf(tmp, sizeof(tmp), L("SUCCÈS · %d/%d débloqués", "ACHIEVEMENTS · %d/%d unlocked"), ok, NACH);
		text_at(x, y++, theme()->accent, tmp);
		h = term_rows() - y - 3;
		if (h < 3)
			h = 3;
		if (sel < top)
			top = sel;
		if (sel >= top + h)
			top = sel - h + 1;
		for (i = top; i < NACH && i < top + h; i++)
		{
			at(x, y + i - top);
			if (i == sel)
				bgc(mix(theme()->bg, theme()->accent, 0.22));
			fg(ach_has(i) ? rgb(255, 215, 0) : (i == sel ? theme()->text : theme()->dim));
			outf(" %s %s %-30.30s ", i == sel ? "▶" : " ", ach_has(i) ? "★" : "☆", L(g_ach_fr[i][0], g_ach_en[i][0]));
			bgc(theme()->bg);
		}
		text_at(x + 40, y, ach_has(sel) ? rgb(255, 215, 0) : theme()->text, L(g_ach_fr[sel][0], g_ach_en[sel][0]));
		text_at(x + 40, y + 1, ach_has(sel) ? rgb(80, 230, 140) : theme()->dim,
			ach_has(sel) ? L("Débloqué", "Unlocked") : L("Pas encore débloqué", "Not unlocked yet"));
		text_at(x + 40, y + 3, theme()->accent, L("Pour le débloquer :", "How to unlock it:"));
		text_wrap(x + 40, y + 4, y + 9, term_cols() - x - 42 < 38 ? term_cols() - x - 42 : 38, theme()->text,
			L(g_ach_fr[sel][1], g_ach_en[sel][1]));
		foot_line(x, L("▲ ▼ parcourir les succès   e : modifier mon profil   Échap : retour",
				"▲ ▼ browse achievements   e: edit my profile   Esc: back"));
		flush();
		k = key_dir(key_read(200));
		if (k == 'e' || k == 'E')
		{
			cls();
			screen_profile_edit();
			cls();
		}
		else if (k == 1000)
			sel = (sel + NACH - 1) % NACH;
		else if (k == 1001)
			sel = (sel + 1) % NACH;
		else if (key_quit(k) || k == 127 || k == 8 || k == '\r' || k == '\n')
			break ;
	}
	cls();
}

void	screen_credits(void)
{
	const char	*l[48];
	int			h[48];
	char		ver[64];
	int			n;
	int			i;
	int			y;
	long		t0;
	double		s;

	n = 0;
	memset(h, 0, sizeof(h));
	h[n] = 2;
	l[n++] = "LE TESTEUR DES FOUS";
	snprintf(ver, sizeof(ver), "v%s", TDF_VERSION);
	l[n++] = ver;
	l[n++] = "";
	h[n] = 1;
	l[n++] = L("Créé et vibe-codé par Splatch", "Created and vibe-coded by Splatch");
	l[n++] = L("login 42 : romasant · lore et idées : Splatch", "42 login: romasant · lore and ideas: Splatch");
	l[n++] = L("Un testeur, seulement là pour tester ton code : ce n'est pas un jeu vidéo.", "A tester, only here to test your code: it is not a video game.");
	l[n++] = "";
	h[n] = 1;
	l[n++] = L("Libft · la Tour des Fondations", "Libft · the Tower of Foundations");
	l[n++] = L("43 fonctions, prototypes, audit de la moulinette, norminette", "43 functions, prototypes, moulinette audit, norminette");
	l[n++] = L("3 gardiens à battre · 21 niveaux d'entraînement", "3 guardians to beat · 21 training levels");
	l[n++] = "";
	h[n] = 1;
	l[n++] = L("ft_printf · la Fonderie des Formats", "ft_printf · the Foundry of Formats");
	l[n++] = L("les 9 conversions, les flags du bonus, fuites et échecs de malloc", "the 9 conversions, the bonus flags, leaks and malloc failures");
	l[n++] = L("9 chaînes à briser · 16 niveaux d'entraînement", "9 chains to break · 16 training levels");
	l[n++] = "";
	h[n] = 1;
	l[n++] = L("get_next_line · le Puits", "get_next_line · the Well");
	l[n++] = L("7 tailles de BUFFER_SIZE et sans -D, stdin, tubes, read et malloc qui échouent", "7 BUFFER_SIZE values and without -D, stdin, pipes, failing read and malloc");
	l[n++] = L("22 anneaux à allumer", "22 rings to light up");
	l[n++] = "";
	h[n] = 1;
	l[n++] = L("Examshell des fous · l'Épreuve du Silence", "Examshell des fous · the Trial of Silence");
	l[n++] = L("un exam en C sur 4 niveaux, grademe et traces", "a C exam over 4 levels, grademe and traces");
	l[n++] = L("une banque d'exercices exam, Libft et printf, sujets en 8 langues", "an exercise bank for the exam, Libft and printf, subjects in 8 languages");
	l[n++] = "";
	h[n] = 1;
	l[n++] = L("Moteur de tests", "Test engine");
	l[n++] = L("ASan · UBSan · LeakSanitizer · échecs de malloc · norminette", "ASan · UBSan · LeakSanitizer · malloc failures · norminette");
	l[n++] = "";
	h[n] = 1;
	l[n++] = L("La Cité qui compile", "The City that compiles");
	l[n++] = L("Pixel art dessiné avec Python et Pillow", "Pixel art drawn with Python and Pillow");
	l[n++] = L("8 langues : français, anglais, espagnol, allemand, italien, portugais, turc, russe",
			"8 languages: French, English, Spanish, German, Italian, Portuguese, Turkish, Russian");
	l[n++] = L("Code écrit en vibe-coding, avec une IA", "Code written by vibe-coding, with an AI");
	l[n++] = "";
	l[n++] = L("Aucune fonction de tes projets n'a été écrite par ce testeur.",
			"None of your projects' functions were written by this tester.");
	l[n++] = L("Il ne fait que les casser.", "It only breaks them.");
	l[n++] = "";
	h[n] = 1;
	l[n++] = L("Discord · 42 Next Global - Serveur non Officiel", "Discord · 42 Next Global - Unofficial Server");
	l[n++] = TDF_DISCORD;
	l[n++] = L("Lien mort ? Contacte @splatch_afk sur Discord", "Dead link? Contact @splatch_afk on Discord");
	l[n++] = "";
	h[n] = 1;
	l[n++] = L("Merci d'avoir testé !", "Thanks for testing!");
	t0 = now_ms();
	while (1)
	{
		s = (now_ms() - t0) / 220.0;
		bgc(theme()->bg);
		frame();
		for (i = 0; i < n; i++)
		{
			y = term_rows() - 5 - (int)s + i * 2;
			if (y < 0 || y >= term_rows() - 5)
				continue ;
			center(y, h[i] == 2 ? hue(now_ms() / 2000.0) : (h[i] ? theme()->accent2 : theme()->text), l[i]);
		}
		discord_card((term_cols() - DISCORD_W) / 2, term_rows() - 6);
		flush();
		tdf_lien((term_cols() - DISCORD_W) / 2 + 5, term_rows() - 4, TDF_DISCORD);
		if (key_read(40) >= 0 || term_rows() - 5 - (int)s + n * 2 < 0)
			break ;
	}
	cls();
}

int	screen_crash_choice(t_scene_out *g)
{
	long	t0;
	long	t;
	int		k;
	int		cy;
	char	tmp[260];
	t_res	*r;

	t0 = now_ms();
	r = (g->fn >= 0 && g->fn < NFN) ? &g_run.fn[g->fn] : NULL;
	cls();
	while (1)
	{
		t = now_ms() - t0;
		cy = (term_rows() - 18) / 2;
		bgc(t < 600 && (t / 60) % 2 ? rgb(90, 0, 10) : theme()->bg);
		if (t < 700)
			frame();
		big_text((term_cols() - big_width(g->frozen ? "FREEZE" : "CRASH")) / 2
			+ (t < 700 ? rand() % 3 - 1 : 0), cy, g->frozen ? "FREEZE" : "CRASH", t / 1000.0, 1);
		bgc(theme()->bg);
		if (g->frozen)
			snprintf(tmp, sizeof(tmp), L("La scène ne répond plus depuis 3 s : boucle infinie dans ft_%s ?",
					"The scene froze for 3 s: infinite loop in ft_%s?"), g->fn >= 0 ? g_fn_name[g->fn] : "?");
		else
			snprintf(tmp, sizeof(tmp), L("La scène a planté dans ft_%s  (signal %d)", "The scene crashed in ft_%s  (signal %d)"),
				g->fn >= 0 ? g_fn_name[g->fn] : "?", g->sig);
		center(cy + 7, rgb(255, 120, 130), tmp);
		if (g->fn >= 0)
		{
			snprintf(tmp, sizeof(tmp), L("Dans la scène, ft_%s gère %s.", "In the scene, ft_%s handles %s."),
				g_fn_name[g->fn], g_fn_zone[g->fn]);
			center(cy + 9, theme()->text, tmp);
		}
		if (r && r->nd)
		{
			snprintf(tmp, sizeof(tmp), L("Le testeur avait vu : %.90s", "The tester saw: %.90s"), r->detail[0]);
			center(cy + 10, theme()->dim, tmp);
		}
		center(cy + 11, theme()->dim, L("(le coupable peut aussi être une de tes fonctions qu'elle appelle)",
				"(the culprit may also be one of your functions it calls)"));
		center(cy + 13, theme()->accent2, L("[r] relancer avec cette fonction en quarantaine (version de secours)",
				"[r] restart with this function quarantined (fallback version)"));
		center(cy + 14, theme()->dim, L("[Échap] retour", "[Esc] back"));
		flush();
		k = key_read(40);
		if (k == 'r' || k == 'R')
			return (1);
		if (key_quit(k) || k == 'm')
			return (0);
	}
}
