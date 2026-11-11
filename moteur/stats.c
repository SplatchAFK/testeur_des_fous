#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
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
	int		runs[4];
	long	first;
	long	last;
	int		days;
	int		streak;
	int		recent;
	int		before;
	int		nratio;
	int		best;
	float	ratio[SMAXRUN];
	long	day[SMAXRUN];
	long	rt[SMAXRUN];
	char	rmod[SMAXRUN];
	short	rok[SMAXRUN];
	short	rtot[SMAXRUN];
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

#ifndef TDF_STATS_SEUL

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
	g_sd.best = 0;
	prev = -1;
	today = 0;
	for (i = 0; i < g_sd.nratio; i++)
	{
		if (g_sd.day[i] != prev)
		{
			g_sd.days++;
			today = (prev >= 0 && g_sd.day[i] - prev == 1) ? today + 1 : 1;
			if (today > g_sd.best)
				g_sd.best = (int)today;
		}
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
			g_sd.runs[!strcmp(mod, "printf") ? 1 : !strcmp(mod, "exam") ? 2 : 0]++;
			if (!g_sd.first || t < g_sd.first)
				g_sd.first = t;
			if (t > g_sd.last)
				g_sd.last = t;
			if (g_sd.nratio == SMAXRUN)
			{
				memmove(g_sd.ratio, g_sd.ratio + 1, sizeof(float) * (SMAXRUN - 1));
				memmove(g_sd.day, g_sd.day + 1, sizeof(long) * (SMAXRUN - 1));
				memmove(g_sd.rt, g_sd.rt + 1, sizeof(long) * (SMAXRUN - 1));
				memmove(g_sd.rmod, g_sd.rmod + 1, SMAXRUN - 1);
				memmove(g_sd.rok, g_sd.rok + 1, sizeof(short) * (SMAXRUN - 1));
				memmove(g_sd.rtot, g_sd.rtot + 1, sizeof(short) * (SMAXRUN - 1));
				g_sd.nratio--;
			}
			g_sd.ratio[g_sd.nratio] = (float)ok / tot;
			g_sd.rt[g_sd.nratio] = t;
			g_sd.rmod[g_sd.nratio] = !strcmp(mod, "printf") ? 1 : !strcmp(mod, "exam") ? 2 : !strcmp(mod, "gnl") ? 3 : 0;
			g_sd.rok[g_sd.nratio] = (short)ok;
			g_sd.rtot[g_sd.nratio] = (short)tot;
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

static const char	*g_lf[NFN] = {
	"isalpha", "isdigit", "isalnum", "isascii", "isprint", "strlen",
	"memset", "bzero", "memcpy", "memmove", "strlcpy", "strlcat",
	"toupper", "tolower", "strchr", "strrchr", "strncmp", "memchr",
	"memcmp", "strnstr", "atoi", "calloc", "strdup",
	"substr", "strjoin", "strtrim", "split", "itoa", "strmapi", "striteri",
	"putchar_fd", "putstr_fd", "putendl_fd", "putnbr_fd",
	"lstnew", "lstadd_front", "lstsize", "lstlast", "lstadd_back",
	"lstdelone", "lstclear", "lstiter", "lstmap"
};

static t_rgb	vert(void)
{
	return (rgb(120, 230, 150));
}

static t_rgb	orange(void)
{
	return (rgb(255, 150, 90));
}

static void	title(int x, int y, const char *s)
{
	text_at(x, y, theme()->accent2, s);
}

static void	spark(int x, int y, int w, int mod, const char *label)
{
	static const char	*b[8] = {"▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"};
	int					idx[200];
	int					n;
	int					i;
	int					k;
	float				r;

	n = 0;
	for (i = g_sd.nratio - 1; i >= 0 && n < w && n < 200; i--)
		if (mod < 0 || g_sd.rmod[i] == mod)
			idx[n++] = i;
	text_at(x, y, theme()->text, label);
	if (!n)
	{
		text_at(x, y + 1, theme()->dim, L("pas encore de lancement", "no run yet"));
		return ;
	}
	at(x, y + 1);
	for (k = n - 1; k >= 0; k--)
	{
		r = g_sd.ratio[idx[k]];
		fg(r >= 0.999f ? vert() : r >= 0.75f ? rgb(255, 215, 0) : orange());
		outf("%s", b[(int)(r * 7.0f + 0.5f)]);
	}
	text_at(x, y + 2, theme()->dim, L("ancien  →  récent  (hauteur : part de réussite)", "older  →  newer  (height: success share)"));
}

static int	weak(int x, int y, int maxy, int mod, int max)
{
	static const char	*mods[4] = {"libft", "printf", "exam", "gnl"};
	char				tmp[220];
	char				when[40];
	t_sitem				*it;
	int					i;
	int					k;
	int					best;
	int					shown;

	title(x, y++, L("CE QUI ÉCHOUE LE PLUS", "WHAT FAILS MOST"));
	shown = 0;
	for (i = 0; i < g_sd.nit && y < maxy && shown < max; i++)
	{
		it = &g_sd.it[i];
		if (mod >= 0 && strcmp(it->mod, mods[mod]))
			continue ;
		if (!strcmp(it->name, "niveau"))
			continue ;
		best = 0;
		for (k = 1; k < it->nk; k++)
			if (it->kn[k] > it->kn[best])
				best = k;
		ago(it->last, when, sizeof(when));
		snprintf(tmp, sizeof(tmp), L("%-18s %3d échec(s) · surtout %-7s · %s", "%-18s %3d failure(s) · mostly %-7s · %s"),
			it->name, it->fails, it->kind[best], when);
		if (mod < 0)
			snprintf(tmp, sizeof(tmp), L("%-7s %-18s %3d échec(s) · %s", "%-7s %-18s %3d failure(s) · %s"),
				it->mod, it->name, it->fails, when);
		text_at(x, y++, shown < 3 ? orange() : theme()->text, tmp);
		shown++;
	}
	if (!shown)
		text_at(x, y++, theme()->dim, L("Aucun échec enregistré.", "No failure recorded."));
	return (y);
}

static void	page_resume(int x, int y, int maxy)
{
	char	tmp[200];
	int		yy;

	yy = draw_progress(x, y);
	draw_activity(x + 42, y);
	snprintf(tmp, sizeof(tmp), L("Plus longue série : %d j · lancements suivis : %d", "Longest streak: %d d · tracked runs: %d"),
		g_sd.best, g_sd.nratio);
	text_at(x + 42, y + 9, theme()->text, tmp);
	spark(x, yy + 1, 70, -1, L("TES 70 DERNIERS LANCEMENTS (tous modules)", "YOUR LAST 70 RUNS (all modules)"));
	weak(x, yy + 5, maxy, -1, 6);
}

static void	page_libft(int x, int y, int maxy)
{
	static const int	part[4] = {0, 23, 34, 43};
	static const char	*pn[3] = {"Partie 1 · libc", "Partie 2 · ajouts", "Partie 3 · listes"};
	char				tmp[400];
	size_t				n;
	int					p;
	int					i;
	int					c;
	int					yy;

	title(x, y++, L("FONCTIONS VALIDÉES (au moins une fois)", "FUNCTIONS CLEARED (at least once)"));
	for (p = 0; p < 3; p++)
	{
		for (c = 0, i = part[p]; i < part[p + 1]; i++)
			c += g_save.cleared[i] == '1';
		line_bar(x, y++, pn[p], c, part[p + 1] - part[p]);
	}
	line_bar(x, y++, L("Boss battus", "Bosses beaten"), g_save.boss[0] + g_save.boss[1] + g_save.boss[2], 3);
	line_bar(x, y++, L("Entraînement", "Training"), ones(g_save.exo), NEXO);
	y++;
	title(x, y++, L("PAS ENCORE VALIDÉES", "NOT CLEARED YET"));
	n = 0;
	tmp[0] = '\0';
	for (i = 0; i < NFN; i++)
		if (g_save.cleared[i] != '1' && n < sizeof(tmp) - 20)
			n += snprintf(tmp + n, sizeof(tmp) - n, "%sft_%s", n ? ", " : "", g_lf[i]);
	yy = text_wrap(x, y, y + 4, 78, n ? orange() : vert(), n ? tmp : L("Toutes ! Bravo.", "All of them! Well done."));
	y = (yy > y ? yy : y + 1) + 1;
	spark(x + 42, 3, 34, 0, L("TES LANCEMENTS LIBFT", "YOUR LIBFT RUNS"));
	weak(x, y, maxy, 0, 8);
}

static void	page_printf(int x, int y, int maxy)
{
	static const char	*conv[9] = {"%c", "%s", "%p", "%d", "%i", "%u", "%x", "%X", "%%"};
	static const char	*flags[6] = {"-", "0", "largeur", ".", "#", "+ et espace"};
	char				v[300];
	char				tmp[200];
	int					i;

	title(x, y++, L("CONVERSIONS (chaînes brisées)", "CONVERSIONS (chains broken)"));
	save_key("printf.chains", v, sizeof(v));
	for (i = 0; i < 9; i++)
	{
		if (i == 0 || i == 5)
			at(x, y++);
		fg(v[i] == '1' ? vert() : theme()->dim);
		outf("%s %s   ", v[i] == '1' ? "✓" : "·", conv[i]);
	}
	save_key("printf.big", v, sizeof(v));
	text_at(x, y++, atoi(v) ? vert() : theme()->dim, atoi(v) ? L("✓ ft_printf vaincu", "✓ ft_printf beaten") : L("· ft_printf pas encore vaincu", "· ft_printf not beaten yet"));
	y++;
	title(x, y++, L("DLC · FLAGS", "DLC · FLAGS"));
	save_key("printf.dlc", v, sizeof(v));
	for (i = 0; i < 6; i++)
	{
		if (i == 0 || i == 4)
			at(x, y++);
		fg(v[i] == '1' ? vert() : theme()->dim);
		outf("%s %s   ", v[i] == '1' ? "✓" : "·", flags[i]);
	}
	save_key("printf.exo", v, sizeof(v));
	line_bar(x, y++, L("Entraînement", "Training"), ones(v), 16);
	save_key("printf.runs", v, sizeof(v));
	snprintf(tmp, sizeof(tmp), L("Lancements printf : %s", "printf runs: %s"), v[0] ? v : "0");
	text_at(x, y++, theme()->text, tmp);
	spark(x + 42, 3, 34, 1, L("TES LANCEMENTS PRINTF", "YOUR PRINTF RUNS"));
	weak(x, y + 1, maxy, 1, 8);
}

static int	bank_level(const char *name)
{
	FILE		*f;
	char		path[900];
	const char	*r;
	int			lv;

	r = getenv("TDF_RACINE");
	if (!r)
		return (0);
	snprintf(path, sizeof(path), "%s/banque/exam/%s/meta", r, name);
	f = fopen(path, "r");
	if (!f)
		return (0);
	lv = 0;
	while (fscanf(f, " niveau=%d", &lv) != 1 && fgetc(f) != EOF)
		;
	fclose(f);
	return (lv);
}

static void	bank_counts(int *tot)
{
	DIR				*d;
	struct dirent	*e;
	char			path[900];
	const char		*r;
	int				lv;

	memset(tot, 0, sizeof(int) * 5);
	r = getenv("TDF_RACINE");
	if (!r)
		return ;
	snprintf(path, sizeof(path), "%s/banque/exam", r);
	d = opendir(path);
	while (d && (e = readdir(d)))
	{
		if (e->d_name[0] == '.')
			continue ;
		lv = bank_level(e->d_name);
		if (lv >= 1 && lv <= 4)
			tot[lv]++;
	}
	if (d)
		closedir(d);
}

static void	page_exam(int x, int y, int maxy)
{
	char	v[2600];
	char	tmp[200];
	char	*tok;
	int		tot[5];
	int		got[5];
	int		lv;
	long	bt;

	title(x, y++, L("EXAM EN C", "C EXAM"));
	save_key("exam.runs", v, sizeof(v));
	snprintf(tmp, sizeof(tmp), L("Exams passés : %d", "Exams taken: %d"), atoi(v));
	text_at(x, y++, theme()->text, tmp);
	save_key("exam.passed", v, sizeof(v));
	snprintf(tmp, sizeof(tmp), L("Exams validés (100/100) : %d", "Exams passed (100/100): %d"), atoi(v));
	text_at(x, y++, atoi(v) ? vert() : theme()->text, tmp);
	save_key("exam.best", v, sizeof(v));
	snprintf(tmp, sizeof(tmp), L("Meilleure note : %d/100", "Best score: %d/100"), atoi(v));
	text_at(x, y++, theme()->text, tmp);
	save_key("exam.besttime", v, sizeof(v));
	bt = atol(v);
	if (bt > 0)
		snprintf(tmp, sizeof(tmp), L("Meilleur temps validé : %ld h %02ld min", "Best passing time: %ld h %02ld min"), bt / 3600, bt / 60 % 60);
	else
		snprintf(tmp, sizeof(tmp), "%s", L("Meilleur temps validé : -", "Best passing time: -"));
	text_at(x, y++, theme()->text, tmp);
	save_key("exam.cheats", v, sizeof(v));
	snprintf(tmp, sizeof(tmp), L("Cheats utilisés : %d", "Cheats used: %d"), atoi(v));
	text_at(x, y++, theme()->dim, tmp);
	y++;
	title(x, y++, L("EXERCICES RÉUSSIS AU MOINS UNE FOIS", "EXERCISES SOLVED AT LEAST ONCE"));
	bank_counts(tot);
	memset(got, 0, sizeof(got));
	save_key("exam.done", v, sizeof(v));
	for (tok = strtok(v, ","); tok; tok = strtok(NULL, ","))
	{
		lv = bank_level(tok);
		if (lv >= 1 && lv <= 4)
			got[lv]++;
	}
	for (lv = 1; lv <= 4; lv++)
	{
		snprintf(tmp, sizeof(tmp), L("Niveau %d", "Level %d"), lv);
		line_bar(x, y++, tmp, got[lv], tot[lv]);
	}
	spark(x + 42, 3, 34, 2, L("TES PASSAGES (corrections et exams)", "YOUR ATTEMPTS (gradings and exams)"));
	weak(x, y + 1, maxy, 2, 8);
}

static void	page_histo(int x, int y, int maxy)
{
	static const char	*mods[4] = {"Libft ", "printf", "Exam  ", "GNL   "};
	char				tmp[200];
	char				d[40];
	time_t				tt;
	int					i;

	title(x, y++, L("HISTORIQUE · les derniers lancements", "HISTORY · latest runs"));
	if (!g_sd.nratio)
		text_at(x, y, theme()->dim, L("Rien pour l'instant : lance des tests.", "Nothing yet: run some tests."));
	for (i = g_sd.nratio - 1; i >= 0 && y < maxy; i--)
	{
		tt = (time_t)g_sd.rt[i];
		strftime(d, sizeof(d), "%d/%m %H:%M", localtime(&tt));
		snprintf(tmp, sizeof(tmp), "%s  %s  %3d/%-3d", d, mods[(int)g_sd.rmod[i] % 4], g_sd.rok[i], g_sd.rtot[i]);
		text_at(x, y, theme()->text, tmp);
		bar(x + 32, y, 20, g_sd.ratio[i], g_sd.ratio[i] >= 0.999f ? vert() : orange(), g_sd.ratio[i] >= 0.999f ? vert() : rgb(255, 215, 0));
		snprintf(tmp, sizeof(tmp), "%3d %%", (int)(g_sd.ratio[i] * 100 + 0.5f));
		text_at(x + 54, y++, theme()->dim, tmp);
	}
}

void	tdf_screen_stats(void)
{
	static int	page;
	const char	*noms[5];
	int			x;
	int			i;
	int			k;

	noms[0] = L("Résumé", "Summary");
	noms[1] = "Libft";
	noms[2] = "ft_printf";
	noms[3] = "Exam";
	noms[4] = L("Historique", "History");
	save_write();
	stats_load();
	cls();
	while (1)
	{
		x = (term_cols() - 80) / 2;
		if (x < 0)
			x = 0;
		bgc(theme()->bg);
		frame();
		at(x, 1);
		for (i = 0; i < 5; i++)
		{
			if (i == page)
			{
				bgc(mix(theme()->bg, theme()->accent, 0.3));
				fg(theme()->accent2);
			}
			else
				fg(theme()->dim);
			outf(" %s ", noms[i]);
			bgc(theme()->bg);
			outf("  ");
		}
		if (page == 0)
			page_resume(x, 3, term_rows() - 2);
		else if (page == 1)
			page_libft(x, 3, term_rows() - 2);
		else if (page == 2)
			page_printf(x, 3, term_rows() - 2);
		else if (page == 3)
			page_exam(x, 3, term_rows() - 2);
		else
			page_histo(x, 3, term_rows() - 2);
		foot_line(x, L("◀ ▶ changer de page   Échap : retour", "◀ ▶ change page   Esc: back"));
		flush();
		k = key_dir(key_read(200));
		if (k == 1003)
			page = (page + 4) % 5;
		else if (k == 1002)
			page = (page + 1) % 5;
		else if (k >= '1' && k <= '5')
			page = k - '1';
		else if (key_quit(k) || k == 127 || k == 8 || k == '\r' || k == '\n')
			break ;
		if (k > 0)
			cls();
	}
	cls();
}

#endif
