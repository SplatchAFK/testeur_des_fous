#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include "fous.h"

t_run		g_run;

static long	g_reveal[NFN];
static int	g_shown[NFN];

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

/* coupe proprement un texte UTF-8 tronqué par snprintf */
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
		g_run.fn[k].proto = !strcmp(f[2], "ok") ? 1 : (!strcmp(f[2], "missing") ? 3 : 2);
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
	const char	*sub_full = "·  V E R S I O N   L I B F T  ·";
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
		stars_draw();
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
	/* libellé de la case : formaté une fois, pas à chaque image */
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
		stars_draw();
		center(1, mix(theme()->accent, theme()->accent2, 0.5 + 0.5 * sin(now / 400.0)),
			L("▚▚  A N A L Y S E   D E   T A   L I B F T  ▞▞", "▚▚  S C A N N I N G   Y O U R   L I B F T  ▞▞"));
		snprintf(tmp, sizeof(tmp), "%-70.70s", g_run.stage[0] ? g_run.stage : L("préparation...", "warming up..."));
		k = (term_cols() - 70) / 2;
		text_at(k < 0 ? 0 : k, 2, theme()->dim, tmp);
		draw_grid(now, -1);
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
		if (!strncmp(g_run.audit[i].label, "norminette introuvable", 22)
			|| !strncmp(g_run.audit[i].label, "norminette désactivée", 22))
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

static void	detail_panel(int sel, int y)
{
	t_res	*r;
	int		x;
	int		i;
	char	tmp[200];

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

/* niveau d'aide 0 : « ft_x.c:42 » devient « ft_x.c » (on garde le fichier, pas la ligne) */
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

/* un conseil par type de problème (jamais de code à ta place) */
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

static int	fiche_build(int fn, char lines[][220], int *kind, int w)
{
	t_res		*r;
	char		d[220];
	char		all[1700];
	char		tmp[260];
	char		*cas;
	char		*att;
	const char	*h;
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
	fiche_add(lines, kind, &n, w, 5, L("POUR LE REJOUER SEUL", "TO REPLAY IT ALONE"));
	snprintf(tmp, sizeof(tmp), "bash %s/run.sh --ci -v %s", tester_name(), g_fn_name[fn]);
	fiche_add(lines, kind, &n, w, 6, tmp);
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
		else if (k != 1001 && k != 1000)
			break ;
	}
	cls();
	return (fn);
}

/* after : juste après un run ; 42 = relancer les tests (touche r) */
int	screen_results(int after)
{
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
	cls();
	while (1)
	{
		counts(&ok, &bad, &done);
		m = moulinette_ok(why, sizeof(why));
		layout(&x, &y);
		bgc(theme()->bg);
		frame();
		snprintf(tmp, sizeof(tmp), L("RÉSULTATS  %d/%d  ·  note moulinette estimée : %s",
				"RESULTS  %d/%d  ·  estimated moulinette grade: %s"), ok, NFN, m ? "100" : "0");
		discord_line(0);
		center(1, m ? rgb(80, 230, 140) : rgb(255, 90, 100), tmp);
		center(2, theme()->dim, why);
		draw_grid(now_ms(), sel);
		detail_panel(sel, y + 17);
		foot_line(x, L("flèches : choisir   f : fiche   m : audit   r : corrigé ? relancer   Entrée : continuer",
				"arrows: pick   f: details   m: audit   r: fixed? run again   Enter: continue"));
		flush();
		k = key_read(60);
		if (k == '\r' || k == '\n' || k == ' ' || key_quit(k))
			break ;
		if (k == 'r' || k == 'R')
		{
			cls();
			return (42);
		}
		if (k == 'f')
		{
			sel = screen_fiche(sel);
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

/* --no-lore : mêmes gains que l'écran des récompenses, sans affichage */
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

/* succès liés à un run (écran des récompenses ou run sans nouveauté) */
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

/* une fonction est propre pour la première fois : l'écran des récompenses vaut le coup */
int	rewards_news(void)
{
	int	i;

	for (i = 0; i < NFN; i++)
		if (st_pass(g_run.fn[i].st) && g_save.cleared[i] != '1')
			return (1);
	return (0);
}

/* mêmes gains que l'écran des récompenses, sans l'afficher ni attendre */
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

/* toutes les fonctions de la partie p passent : le combat sera gagné */
static int	boss_ready(int p)
{
	int	f;

	for (f = g_bosses[p].first; f <= g_bosses[p].last; f++)
		if (!st_pass(g_run.fn[f].st))
			return (0);
	return (part_clean(p));
}

/*
** only : un seul boss (0..2), tous (-1, premier run ou menu) ou seulement
** ceux qu'on bat pour la 1re fois (-2) : relancer après une correction reste rapide.
*/
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

void	screen_profile(void)
{
	int		i;
	int		x;
	int		y;
	int		lv;
	int		n;
	int		m;
	int		o;
	char	tmp[200];

	cls();
	x = (term_cols() - 80) / 2;
	if (x < 0)
		x = 0;
	lv = level_of(g_save.xp);
	o = profile_header(x, 1);
	center(1 + o, theme()->accent2, L("P R O F I L", "P R O F I L E"));
	snprintf(tmp, sizeof(tmp), L("%s · niveau %d · %ld XP · %ld ◈", "%s · level %d · %ld XP · %ld ◈"),
		g_save.login, lv, g_save.xp, g_save.octets);
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
		exo_done_count(),
		n, g_nitems, m, g_save.crashes);
	center(6 + o, theme()->dim, tmp);
	for (i = 0; i < NFN; i++)
	{
		at(x + 18 + (i % 22) * 2, 8 + o + i / 22);
		fg(g_save.cleared[i] == '1' ? rgb(80, 230, 140) : theme()->dim);
		out(g_save.cleared[i] == '1' ? "■" : "□");
	}
	y = 11 + o;
	text_at(x, y++, theme()->accent, L("SUCCÈS", "ACHIEVEMENTS"));
	for (i = 0; i < NACH && y + i / 2 < term_rows() - 2; i++)
	{
		snprintf(tmp, sizeof(tmp), "%s %s", ach_has(i) ? "★" : "☆", ach_has(i)
			? L(g_ach_fr[i][0], g_ach_en[i][0]) : L(g_ach_fr[i][1], g_ach_en[i][1]));
		text_clip(x + (i % 2) * 41, y + i / 2, ach_has(i) ? rgb(255, 215, 0)
			: theme()->dim, tmp, (term_cols() - x - (i % 2) * 41 - 1) < 39
			? term_cols() - x - (i % 2) * 41 - 1 : 39);
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

void	screen_credits(void)
{
	const char	*l[28];
	char		ver[64];
	int			n;
	int			i;
	int			y;
	long		t0;
	double		s;

	n = 0;
	l[n++] = "LE TESTEUR DES FOUS";
	snprintf(ver, sizeof(ver), "%s · v%s", L("Version Libft", "Libft edition"), TDF_VERSION);
	l[n++] = ver;
	l[n++] = "";
	l[n++] = L("Créé et vibe-codé par Splatch", "Created and vibe-coded by Splatch");
	l[n++] = L("login 42 : romasant", "42 login: romasant");
	l[n++] = "";
	l[n++] = L("Lore et idées : Splatch (romasant)", "Lore and ideas: Splatch (romasant)");
	l[n++] = L("Pour rendre Libft plus marrant.", "To make Libft more fun.");
	l[n++] = "";
	l[n++] = L("Exercices d'entraînement", "Training exercises");
	l[n++] = L("21 niveaux conçus pour le testeur des fous", "21 levels made for the crazy tester");
	l[n++] = "";
	l[n++] = L("Moteur de tests", "Test engine");
	l[n++] = L("ASan · UBSan · LeakSanitizer · échecs de malloc", "ASan · UBSan · LeakSanitizer · malloc failures");
	l[n++] = "";
	l[n++] = L("Pixel art dessiné avec Python et Pillow", "Pixel art drawn with Python and Pillow");
	l[n++] = L("Code écrit en vibe-coding, avec une IA", "Code written by vibe-coding, with an AI");
	l[n++] = "";
	l[n++] = L("Aucune fonction de ta libft n'a été écrite par ce testeur.",
			"None of your libft functions were written by this tester.");
	l[n++] = L("Il ne fait que les casser.", "It only breaks them.");
	l[n++] = "";
	l[n++] = L("Discord · le serveur des fous", "Discord · the fous server");
	l[n++] = TDF_DISCORD;
	l[n++] = "";
	l[n++] = L("Merci d'avoir joué !", "Thanks for playing!");
	t0 = now_ms();
	while (1)
	{
		s = (now_ms() - t0) / 220.0;
		bgc(theme()->bg);
		frame();
		stars_draw();
		for (i = 0; i < n; i++)
		{
			y = term_rows() - 5 - (int)s + i * 2;
			if (y < 0 || y >= term_rows() - 5)
				continue ;
			center(y, i == 0 ? hue(now_ms() / 2000.0) : ((i == 3 || i == 6 || i == 9 || i == 12
						|| i == 15 || i == 21) ? theme()->accent2 : theme()->text), l[i]);
		}
		discord_card((term_cols() - DISCORD_W) / 2, term_rows() - 5);
		flush();
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
