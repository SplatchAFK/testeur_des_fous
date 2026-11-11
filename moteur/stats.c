#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "fous.h"

#define SMAXITEM 120
#define SMAXRUN 3000
#define SMAXKIND 10
#define SMAXFILE 400000

typedef struct s_sitem
{
	char	mod[12];
	char	name[40];
	int		fails;
	long	last;
	char	kind[4][16];
	int		kn[4];
	int		nk;
}	t_sitem;

typedef struct s_sdata
{
	int		runs[2];
	long	first;
	long	last;
	int		days;
	int		streak;
	int		recent;
	int		before;
	int		nratio;
	float	ratio[SMAXRUN];
	long	day[SMAXRUN];
	t_sitem	it[SMAXITEM];
	int		nit;
	char	kind[SMAXKIND][16];
	int		kn[SMAXKIND];
	int		nk;
}	t_sdata;

static FILE		*g_sf;
static char		g_smod[12];
static long		g_st;
static t_sdata	g_sd;

static void	stats_path(char *b, size_t n)
{
	snprintf(b, n, "%s/stats.txt", g_savedir);
}

static void	stats_trim(const char *path)
{
	FILE	*f;
	char	*buf;
	char	*cut;
	long	size;

	f = fopen(path, "rb");
	if (!f)
		return ;
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	if (size < SMAXFILE)
	{
		fclose(f);
		return ;
	}
	rewind(f);
	buf = malloc(size + 1);
	if (!buf || fread(buf, 1, size, f) != (size_t)size)
	{
		free(buf);
		fclose(f);
		return ;
	}
	fclose(f);
	buf[size] = '\0';
	cut = strchr(buf + size / 2, '\n');
	f = cut ? fopen(path, "wb") : NULL;
	if (f)
	{
		fputs(cut + 1, f);
		fclose(f);
	}
	free(buf);
}

static void	stats_word(char *dst, size_t n, const char *src)
{
	size_t	i;

	for (i = 0; src && src[i] && i + 1 < n; i++)
		dst[i] = (src[i] == ' ' || src[i] == '\t' || src[i] == '\n') ? '_' : src[i];
	dst[i] = '\0';
	if (!i && n > 1)
	{
		dst[0] = '?';
		dst[1] = '\0';
	}
}

void	tdf_stats_begin(const char *module)
{
	char	path[620];

	stats_path(path, sizeof(path));
	stats_trim(path);
	g_sf = fopen(path, "a");
	if (!g_sf)
		return ;
	chmod(path, 0600);
	stats_word(g_smod, sizeof(g_smod), module);
	g_st = (long)time(NULL);
}

void	tdf_stats_fail(const char *name, const char *kind)
{
	char	a[40];
	char	b[16];

	if (!g_sf)
		return ;
	stats_word(a, sizeof(a), name);
	stats_word(b, sizeof(b), kind);
	fprintf(g_sf, "K %ld %s %s %s\n", g_st, g_smod, a, b);
}

void	tdf_stats_end(int ok, int total)
{
	if (!g_sf)
		return ;
	fprintf(g_sf, "R %ld %s %d %d\n", g_st, g_smod, ok, total);
	fclose(g_sf);
	g_sf = NULL;
}

static long	day_of(long t)
{
	time_t		tt;
	struct tm	tmv;

	tt = (time_t)t;
	tmv = *localtime(&tt);
	tmv.tm_hour = 12;
	tmv.tm_min = 0;
	tmv.tm_sec = 0;
	return ((long)mktime(&tmv) / 86400);
}

static void	kind_add(char kind[][16], int *kn, int *nk, int max, const char *k)
{
	int	i;

	for (i = 0; i < *nk; i++)
	{
		if (!strcmp(kind[i], k))
		{
			kn[i]++;
			return ;
		}
	}
	if (*nk >= max)
		return ;
	snprintf(kind[*nk], 16, "%s", k);
	kn[*nk] = 1;
	(*nk)++;
}

static void	item_add(long t, const char *mod, const char *name, const char *kind)
{
	t_sitem	*it;
	int		i;

	for (i = 0; i < g_sd.nit; i++)
		if (!strcmp(g_sd.it[i].name, name) && !strcmp(g_sd.it[i].mod, mod))
			break ;
	if (i == g_sd.nit)
	{
		if (g_sd.nit >= SMAXITEM)
			return ;
		it = &g_sd.it[g_sd.nit++];
		memset(it, 0, sizeof(*it));
		snprintf(it->mod, sizeof(it->mod), "%s", mod);
		snprintf(it->name, sizeof(it->name), "%s", name);
	}
	it = &g_sd.it[i];
	it->fails++;
	if (t > it->last)
		it->last = t;
	kind_add(it->kind, it->kn, &it->nk, 4, kind);
	kind_add(g_sd.kind, g_sd.kn, &g_sd.nk, SMAXKIND, kind);
}

static int	item_cmp(const void *a, const void *b)
{
	const t_sitem	*x = a;
	const t_sitem	*y = b;

	if (x->fails != y->fails)
		return (y->fails - x->fails);
	return (y->last > x->last) - (y->last < x->last);
}

static void	stats_window(void)
{
	double	a;
	double	b;
	int		n;
	int		i;
	int		w;

	n = g_sd.nratio;
	w = n >= 20 ? 10 : n / 2;
	g_sd.recent = -1;
	g_sd.before = -1;
	if (n == 0)
		return ;
	if (w == 0)
		w = 1;
	a = 0;
	for (i = n - w; i < n; i++)
		a += g_sd.ratio[i];
	g_sd.recent = (int)(100.0 * a / w + 0.5);
	if (n - 2 * w < 0)
		return ;
	b = 0;
	for (i = n - 2 * w; i < n - w; i++)
		b += g_sd.ratio[i];
	g_sd.before = (int)(100.0 * b / w + 0.5);
}

static void	stats_days(void)
{
	long	today;
	long	prev;
	int		i;

	g_sd.days = 0;
	g_sd.streak = 0;
	prev = -1;
	for (i = 0; i < g_sd.nratio; i++)
	{
		if (g_sd.day[i] != prev)
			g_sd.days++;
		prev = g_sd.day[i];
	}
	if (!g_sd.nratio)
		return ;
	today = day_of((long)time(NULL));
	prev = g_sd.day[g_sd.nratio - 1];
	if (today - prev > 1)
		return ;
	g_sd.streak = 1;
	for (i = g_sd.nratio - 2; i >= 0; i--)
	{
		if (g_sd.day[i] == prev)
			continue ;
		if (prev - g_sd.day[i] != 1)
			break ;
		g_sd.streak++;
		prev = g_sd.day[i];
	}
}

static void	stats_load(void)
{
	FILE	*f;
	char	path[620];
	char	line[200];
	char	mod[12];
	char	name[40];
	char	kind[16];
	long	t;
	int		ok;
	int		tot;

	memset(&g_sd, 0, sizeof(g_sd));
	stats_path(path, sizeof(path));
	f = fopen(path, "r");
	while (f && fgets(line, sizeof(line), f))
	{
		if (sscanf(line, "R %ld %11s %d %d", &t, mod, &ok, &tot) == 4 && tot > 0)
		{
			g_sd.runs[strcmp(mod, "printf") == 0]++;
			if (!g_sd.first || t < g_sd.first)
				g_sd.first = t;
			if (t > g_sd.last)
				g_sd.last = t;
			if (g_sd.nratio == SMAXRUN)
			{
				memmove(g_sd.ratio, g_sd.ratio + 1, sizeof(float) * (SMAXRUN - 1));
				memmove(g_sd.day, g_sd.day + 1, sizeof(long) * (SMAXRUN - 1));
				g_sd.nratio--;
			}
			g_sd.ratio[g_sd.nratio] = (float)ok / tot;
			g_sd.day[g_sd.nratio++] = day_of(t);
		}
		else if (sscanf(line, "K %ld %11s %39s %15s", &t, mod, name, kind) == 4)
			item_add(t, mod, name, kind);
	}
	if (f)
		fclose(f);
	qsort(g_sd.it, g_sd.nit, sizeof(t_sitem), item_cmp);
	stats_window();
	stats_days();
}

static void	save_key(const char *key, char *out, size_t n)
{
	FILE	*f;
	char	path[620];
	char	line[9000];
	size_t	kl;

	out[0] = '\0';
	snprintf(path, sizeof(path), "%s/save.txt", g_savedir);
	f = fopen(path, "r");
	if (!f)
		return ;
	kl = strlen(key);
	while (fgets(line, sizeof(line), f))
	{
		if (!strncmp(line, key, kl) && line[kl] == '=')
		{
			line[strcspn(line, "\r\n")] = '\0';
			snprintf(out, n, "%s", line + kl + 1);
			break ;
		}
	}
	fclose(f);
}

static int	ones(const char *s)
{
	int	n;

	for (n = 0; *s; s++)
		n += *s == '1';
	return (n);
}

static void	date_of(long t, char *b, size_t n)
{
	time_t	tt;

	tt = (time_t)t;
	if (!t || !strftime(b, n, "%d/%m/%Y", localtime(&tt)))
		snprintf(b, n, "-");
}

static void	ago(long t, char *b, size_t n)
{
	long	d;

	d = day_of((long)time(NULL)) - day_of(t);
	if (d <= 0)
		snprintf(b, n, "%s", L("aujourd'hui", "today"));
	else if (d == 1)
		snprintf(b, n, "%s", L("hier", "yesterday"));
	else
		snprintf(b, n, L("il y a %ld j", "%ld d ago"), d);
}

static void	line_bar(int x, int y, const char *label, int v, int max)
{
	char	tmp[80];

	text_at(x, y, theme()->text, label);
	bar(x + 22, y, 10, max ? (double)v / max : 0, theme()->accent, theme()->accent2);
	snprintf(tmp, sizeof(tmp), "%d/%d", v, max);
	text_at(x + 33, y, v == max ? rgb(120, 230, 150) : theme()->dim, tmp);
}

static int	draw_progress(int x, int y)
{
	char	v[300];
	char	tmp[160];
	int		done;
	int		n;
	int		big;

	done = 0;
	text_at(x, y++, theme()->accent2, L("PROGRESSION", "PROGRESS"));
	snprintf(tmp, sizeof(tmp), L("Niveau %d · %ld XP · %ld Octets", "Level %d · %ld XP · %ld Octets"),
		level_of(g_save.xp), g_save.xp, g_save.octets);
	text_at(x, y++, theme()->text, tmp);
	n = ones(g_save.cleared);
	done += n;
	line_bar(x, y++, L("Libft · fonctions", "Libft · functions"), n, NFN);
	n = g_save.boss[0] + g_save.boss[1] + g_save.boss[2];
	done += n;
	line_bar(x, y++, L("Libft · boss", "Libft · bosses"), n, 3);
	n = ones(g_save.exo);
	done += n;
	line_bar(x, y++, L("Libft · entraînement", "Libft · training"), n, NEXO);
	save_key("printf.chains", v, sizeof(v));
	n = ones(v);
	done += n;
	line_bar(x, y++, L("printf · chaînes", "printf · chains"), n, 9);
	save_key("printf.big", v, sizeof(v));
	big = atoi(v) != 0;
	done += big;
	line_bar(x, y++, L("printf · ft_printf", "printf · ft_printf"), big, 1);
	save_key("printf.dlc", v, sizeof(v));
	n = ones(v);
	done += n;
	line_bar(x, y++, L("printf · flags", "printf · flags"), n, 6);
	save_key("printf.exo", v, sizeof(v));
	n = ones(v);
	done += n;
	line_bar(x, y++, L("printf · entraînement", "printf · training"), n, 16);
	n = NFN + 3 + NEXO + 9 + 1 + 6 + 16;
	text_at(x, y, theme()->accent, L("Avancement global", "Overall progress"));
	bar(x + 22, y, 10, (double)done / n, rgb(120, 230, 150), rgb(255, 215, 0));
	snprintf(tmp, sizeof(tmp), "%d %%", (int)(100.0 * done / n + 0.5));
	text_at(x + 33, y++, theme()->text, tmp);
	return (y);
}

static void	draw_activity(int x, int y)
{
	char	v[300];
	char	tmp[200];
	char	d1[24];
	char	d2[24];
	int		ach;

	text_at(x, y++, theme()->accent2, L("ACTIVITÉ", "ACTIVITY"));
	save_key("printf.runs", v, sizeof(v));
	snprintf(tmp, sizeof(tmp), L("Lancements : Libft %d · printf %d", "Runs: Libft %d · printf %d"),
		g_save.runs, atoi(v));
	text_at(x, y++, theme()->text, tmp);
	snprintf(tmp, sizeof(tmp), L("Jours actifs : %d · série : %d j", "Active days: %d · streak: %d d"),
		g_sd.days, g_sd.streak);
	text_at(x, y++, theme()->text, tmp);
	date_of(g_sd.first, d1, sizeof(d1));
	date_of(g_sd.last, d2, sizeof(d2));
	snprintf(tmp, sizeof(tmp), L("Suivi : du %s au %s", "Tracked: %s to %s"), d1, d2);
	text_at(x, y++, theme()->dim, tmp);
	if (g_sd.recent < 0)
		snprintf(tmp, sizeof(tmp), "%s", L("Réussite récente : -", "Recent success: -"));
	else if (g_sd.before < 0)
		snprintf(tmp, sizeof(tmp), L("Réussite récente : %d %%", "Recent success: %d %%"), g_sd.recent);
	else
		snprintf(tmp, sizeof(tmp), L("Réussite récente : %d %%  %s %+d", "Recent success: %d %%  %s %+d"),
			g_sd.recent, g_sd.recent >= g_sd.before ? "▲" : "▼", g_sd.recent - g_sd.before);
	text_at(x, y++, g_sd.recent >= g_sd.before ? rgb(120, 230, 150) : rgb(255, 150, 90), tmp);
	save_key("printf.ach", v, sizeof(v));
	ach = ones(g_save.ach) + ones(v);
	snprintf(tmp, sizeof(tmp), L("Succès : %d · Badges : %d/%d", "Achievements: %d · Badges: %d/%d"),
		ach, ones(g_save.badges), NBADGE);
	text_at(x, y++, theme()->text, tmp);
	snprintf(tmp, sizeof(tmp), L("Objets : %d · Titres : %d", "Items: %d · Titles: %d"),
		ones(g_save.items), ones(g_save.titles));
	text_at(x, y++, theme()->text, tmp);
	snprintf(tmp, sizeof(tmp), L("Plantages du menu évités : %d", "Menu crashes avoided: %d"), g_save.crashes);
	text_at(x, y++, theme()->dim, tmp);
}

static void	draw_weak(int x, int y, int maxy)
{
	char	tmp[220];
	char	when[40];
	t_sitem	*it;
	int		i;
	int		k;
	int		best;
	size_t	n;

	text_at(x, y++, theme()->accent2, L("FAIBLESSES · ce qui échoue le plus souvent", "WEAK SPOTS · what fails most often"));
	if (!g_sd.nit)
	{
		text_at(x, y, theme()->dim, g_sd.nratio
			? L("Aucun échec enregistré. Rien à signaler.", "No failure recorded. Nothing to report.")
			: L("Lance des tests : cet écran se remplit à chaque lancement.",
				"Run the tests: this screen fills up at every run."));
		return ;
	}
	for (i = 0; i < g_sd.nit && y < maxy - 2; i++)
	{
		it = &g_sd.it[i];
		best = 0;
		for (k = 1; k < it->nk; k++)
			if (it->kn[k] > it->kn[best])
				best = k;
		ago(it->last, when, sizeof(when));
		snprintf(tmp, sizeof(tmp), L("%-7s %-18s %3d échec(s) · surtout %-7s · %s",
				"%-7s %-18s %3d failure(s) · mostly %-7s · %s"), it->mod, it->name, it->fails, it->kind[best], when);
		text_at(x, y++, i < 3 ? rgb(255, 150, 90) : theme()->text, tmp);
	}
	n = snprintf(tmp, sizeof(tmp), "%s", L("Types d'échec : ", "Failure types: "));
	for (k = 0; k < g_sd.nk && n < sizeof(tmp) - 30; k++)
		n += snprintf(tmp + n, sizeof(tmp) - n, "%s%s %d", k ? " · " : "", g_sd.kind[k], g_sd.kn[k]);
	text_at(x, y + (y < maxy - 1), theme()->dim, tmp);
}

void	tdf_screen_stats(void)
{
	int	x;
	int	y;
	int	k;

	save_write();
	stats_load();
	cls();
	while (1)
	{
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		frame();
		center(1, theme()->accent, L("STATS", "STATS"));
		y = draw_progress(x, 3);
		draw_activity(x + 42, 3);
		draw_weak(x, y + 1, term_rows() - 1);
		foot_line(x, L("Échap : retour", "Esc: back"));
		flush();
		k = key_dir(key_read(200));
		if (key_quit(k) || k == 127 || k == 8 || k == '\r' || k == '\n')
			break ;
	}
	cls();
}
