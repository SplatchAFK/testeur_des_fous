#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <termios.h>
#include "fous.h"

#define XMAXEXO 160
#define XMAXTEST 64
#define XMAXARG 24
#define XOUT 262144
#define XTMO 5
#define XLEVELS 4
#define XPOINTS 25

typedef struct s_xexo
{
	char	name[48];
	int		level;
	int		prog;
	char	files[6][64];
	int		nfiles;
	char	allowed[32][32];
	int		nallowed;
}	t_xexo;

typedef struct s_xcfg
{
	int	mode;
	int	chrono;
	int	wait;
	int	trace;
	int	lang;
	int	real;
	int	level;
	int	serie;
	int	diff;
}	t_xcfg;

typedef struct s_xstate
{
	int		active;
	int		mode;
	int		level;
	int		exo;
	long	start;
	long	limit;
	int		points;
	int		fails;
	int		tfails;
	int		cheats;
	long	lastfail;
	int		solved;
	int		wait;
	int		trace;
	int		lang;
}	t_xstate;

typedef struct s_xsave
{
	int		runs;
	int		best;
	int		passed;
	long	besttime;
	int		cheats;
	long	lastreward;
	char	done[2400];
	char	code[600];
	char	bdone[2400];
}	t_xsave;

static t_xexo	g_bank[XMAXEXO];
static int		g_nbank;
static t_xcfg	g_cfg = {0, 1, 0, 2, 0, 0, 1, 4, 0};
static t_xstate	g_st;
static t_xsave	g_xs;
static char		g_root[900];
static char		g_bankdir[1000];
static char		g_work[1000];
static char		g_build[1100];
static char		g_home[900];
static int		g_color;

void	bell(void)
{
}

void	toast(const char *title, const char *msg, t_rgb c)
{
	(void)c;
	printf("  %s · %s\n", title, msg);
}

void	game_reset(void)
{
	memset(&g_xs, 0, sizeof(g_xs));
}

void	game_hold(int restore)
{
	static t_xsave	keep;

	if (restore)
		g_xs = keep;
	else
		keep = g_xs;
}

void	game_kv(const char *k, const char *v)
{
	if (!strcmp(k, "runs"))
		g_xs.runs = atoi(v);
	else if (!strcmp(k, "best"))
		g_xs.best = atoi(v);
	else if (!strcmp(k, "passed"))
		g_xs.passed = atoi(v);
	else if (!strcmp(k, "besttime"))
		g_xs.besttime = atol(v);
	else if (!strcmp(k, "cheats"))
		g_xs.cheats = atoi(v);
	else if (!strcmp(k, "lastreward"))
		g_xs.lastreward = atol(v);
	else if (!strcmp(k, "done"))
		snprintf(g_xs.done, sizeof(g_xs.done), "%s", v);
	else if (!strcmp(k, "code"))
		snprintf(g_xs.code, sizeof(g_xs.code), "%s", v);
	else if (!strcmp(k, "bdone"))
		snprintf(g_xs.bdone, sizeof(g_xs.bdone), "%s", v);
}

void	game_text(char *b, size_t n)
{
	snprintf(b, n, "exam.runs=%d\nexam.best=%d\nexam.passed=%d\nexam.besttime=%ld\nexam.cheats=%d\n"
		"exam.lastreward=%ld\nexam.done=%s\nexam.code=%s\nexam.bdone=%s\n", g_xs.runs, g_xs.best, g_xs.passed, g_xs.besttime,
		g_xs.cheats, g_xs.lastreward, g_xs.done, g_xs.code, g_xs.bdone);
}

static const char	*col(const char *code)
{
	return (g_color ? code : "");
}

#define C_R col("\033[0m")
#define C_B col("\033[1m")
#define C_D col("\033[2m")
#define C_OK col("\033[1;32m")
#define C_KO col("\033[1;31m")
#define C_WN col("\033[1;33m")
#define C_AC col("\033[1;36m")

static const char	*level_color(int level)
{
	static const char	*c[5] = {"", "\033[1;32m", "\033[1;33m", "\033[38;5;208m", "\033[1;31m"};

	return (col(c[level < 1 || level > 4 ? 0 : level]));
}

static void	die(const char *msg)
{
	fprintf(stderr, "exam : %s\n", msg);
	exit(1);
}

static int	sh(const char *fmt, ...)
{
	char	cmd[8000];
	va_list	ap;
	int		r;

	va_start(ap, fmt);
	vsnprintf(cmd, sizeof(cmd), fmt, ap);
	va_end(ap);
	r = system(cmd);
	if (r == -1 || !WIFEXITED(r))
		return (-1);
	return (WEXITSTATUS(r));
}

static const char	*q(const char *s)
{
	static char	buf[6][2400];
	static int	k;
	char		*b;
	size_t		n;

	b = buf[k++ % 6];
	n = 0;
	b[n++] = '\'';
	for (; *s && n < sizeof(buf[0]) - 8; s++)
	{
		if (*s == '\'')
		{
			memcpy(b + n, "'\\''", 4);
			n += 4;
		}
		else
			b[n++] = *s;
	}
	b[n++] = '\'';
	b[n] = '\0';
	return (b);
}

static char	*slurp(const char *path, size_t *len)
{
	FILE	*f;
	char	*b;
	long	n;

	f = fopen(path, "rb");
	if (!f)
		return (NULL);
	fseek(f, 0, SEEK_END);
	n = ftell(f);
	rewind(f);
	b = malloc(n + 1);
	if (!b || fread(b, 1, n, f) != (size_t)n)
	{
		free(b);
		fclose(f);
		return (NULL);
	}
	fclose(f);
	b[n] = '\0';
	if (len)
		*len = n;
	return (b);
}

static void	split_words(const char *v, char dst[][64], int *n, int max, size_t width)
{
	char	tmp[600];
	char	*tok;

	snprintf(tmp, sizeof(tmp), "%s", v);
	for (tok = strtok(tmp, " \t,"); tok && *n < max; tok = strtok(NULL, " \t,"))
		snprintf(dst[(*n)++], width, "%s", tok);
}

static int	exo_cmp(const void *a, const void *b)
{
	const t_xexo	*x = a;
	const t_xexo	*y = b;

	if (x->level != y->level)
		return (x->level - y->level);
	return (strcmp(x->name, y->name));
}

static void	bank_load(void)
{
	DIR				*d;
	struct dirent	*e;
	FILE			*f;
	char			path[1200];
	char			line[700];
	char			words[32][64];
	t_xexo			*x;
	int				n;
	int				i;

	d = opendir(g_bankdir);
	if (!d)
		die("banque d'exercices introuvable");
	while ((e = readdir(d)) && g_nbank < XMAXEXO)
	{
		if (e->d_name[0] == '.')
			continue ;
		snprintf(path, sizeof(path), "%s/%s/meta", g_bankdir, e->d_name);
		f = fopen(path, "r");
		if (!f)
			continue ;
		x = &g_bank[g_nbank];
		memset(x, 0, sizeof(*x));
		snprintf(x->name, sizeof(x->name), "%s", e->d_name);
		while (fgets(line, sizeof(line), f))
		{
			line[strcspn(line, "\r\n")] = '\0';
			if (!strncmp(line, "niveau=", 7))
				x->level = atoi(line + 7);
			else if (!strncmp(line, "type=", 5))
				x->prog = !strcmp(line + 5, "programme");
			else if (!strncmp(line, "fichiers=", 9))
				split_words(line + 9, x->files, &x->nfiles, 6, 64);
			else if (!strncmp(line, "autorise=", 9))
			{
				n = 0;
				split_words(line + 9, words, &n, 32, 64);
				for (i = 0; i < n; i++)
					snprintf(x->allowed[x->nallowed++], 32, "%s", words[i]);
			}
		}
		fclose(f);
		if (x->level >= 1 && x->level <= XLEVELS && x->nfiles > 0)
			g_nbank++;
	}
	closedir(d);
	qsort(g_bank, g_nbank, sizeof(t_xexo), exo_cmp);
	if (!g_nbank)
		die("la banque d'exercices est vide");
}

static void	cfg_path(char *b, size_t n)
{
	snprintf(b, n, "%s/exam.cfg", g_home);
}

static void	cfg_load(void)
{
	FILE	*f;
	char	path[1000];
	char	k[40];
	int		v;

	cfg_path(path, sizeof(path));
	f = fopen(path, "r");
	if (!f)
		return ;
	while (fscanf(f, "%39[a-z]=%d\n", k, &v) == 2)
	{
		if (!strcmp(k, "mode") && v >= 0 && v <= 2)
			g_cfg.mode = v;
		else if (!strcmp(k, "serie") && v >= 1 && v <= 5)
			g_cfg.serie = v;
		else if (!strcmp(k, "difficulte") && v >= 0 && v <= XLEVELS)
			g_cfg.diff = v;
		else if (!strcmp(k, "chrono"))
			g_cfg.chrono = v != 0;
		else if (!strcmp(k, "attente"))
			g_cfg.wait = v != 0;
		else if (!strcmp(k, "trace") && v >= 0 && v <= 2)
			g_cfg.trace = v;
		else if (!strcmp(k, "langue"))
			g_cfg.lang = v != 0;
		else if (!strcmp(k, "reel"))
			g_cfg.real = v != 0;
		else if (!strcmp(k, "niveau") && v >= 1 && v <= XLEVELS)
			g_cfg.level = v;
	}
	fclose(f);
}

static void	cfg_save(void)
{
	FILE	*f;
	char	path[1000];

	cfg_path(path, sizeof(path));
	f = fopen(path, "w");
	if (!f)
		return ;
	fprintf(f, "mode=%d\nchrono=%d\nattente=%d\ntrace=%d\nlangue=%d\nreel=%d\nniveau=%d\nserie=%d\ndifficulte=%d\n",
		g_cfg.mode, g_cfg.chrono, g_cfg.wait, g_cfg.trace, g_cfg.lang, g_cfg.real, g_cfg.level, g_cfg.serie,
		g_cfg.diff);
	fclose(f);
}

static void	state_path(char *b, size_t n)
{
	snprintf(b, n, "%s/etat", g_work);
}

static void	state_save(void)
{
	FILE	*f;
	char	path[1200];

	state_path(path, sizeof(path));
	if (!g_st.active)
	{
		unlink(path);
		return ;
	}
	f = fopen(path, "w");
	if (!f)
		return ;
	fprintf(f, "%d %d %d %s %ld %ld %d %d %d %d %ld %d %d %d %d\n", g_st.active, g_st.mode, g_st.level,
		g_bank[g_st.exo].name, g_st.start, g_st.limit, g_st.points, g_st.fails, g_st.tfails, g_st.cheats,
		g_st.lastfail, g_st.solved, g_st.wait, g_st.trace, g_st.lang);
	fclose(f);
}

static int	exo_find(const char *name)
{
	int	i;

	for (i = 0; i < g_nbank; i++)
		if (!strcmp(g_bank[i].name, name))
			return (i);
	return (-1);
}

static int	state_load(void)
{
	FILE	*f;
	char	path[1200];
	char	name[48];
	int		ok;

	state_path(path, sizeof(path));
	f = fopen(path, "r");
	if (!f)
		return (0);
	ok = fscanf(f, "%d %d %d %47s %ld %ld %d %d %d %d %ld %d %d %d %d", &g_st.active, &g_st.mode, &g_st.level,
			name, &g_st.start, &g_st.limit, &g_st.points, &g_st.fails, &g_st.tfails, &g_st.cheats,
			&g_st.lastfail, &g_st.solved, &g_st.wait, &g_st.trace, &g_st.lang) == 15;
	fclose(f);
	g_st.exo = ok ? exo_find(name) : -1;
	if (!ok || g_st.exo < 0 || !g_st.active)
	{
		memset(&g_st, 0, sizeof(g_st));
		return (0);
	}
	return (1);
}

static int	done_has(const char *name)
{
	const char	*p;
	size_t		n;

	n = strlen(name);
	for (p = g_xs.done; (p = strstr(p, name)); p += n)
		if ((p == g_xs.done || p[-1] == ',') && (p[n] == ',' || p[n] == '\0'))
			return (1);
	return (0);
}

static void	done_add(const char *name)
{
	size_t	n;

	if (done_has(name))
		return ;
	n = strlen(g_xs.done);
	if (n + strlen(name) + 2 < sizeof(g_xs.done))
		snprintf(g_xs.done + n, sizeof(g_xs.done) - n, "%s%s", n ? "," : "", name);
}

static int	pick(int level, int avoid)
{
	int	cand[XMAXEXO];
	int	fresh[XMAXEXO];
	int	n;
	int	nf;
	int	i;

	n = 0;
	nf = 0;
	for (i = 0; i < g_nbank; i++)
	{
		if (g_bank[i].level != level || i == avoid)
			continue ;
		cand[n++] = i;
		if (!done_has(g_bank[i].name))
			fresh[nf++] = i;
	}
	if (nf && rand() % 4)
		return (fresh[rand() % nf]);
	if (n)
		return (cand[rand() % n]);
	return (avoid);
}

static void	install(void)
{
	t_xexo	*x;
	char	ts[40];
	time_t	now;

	x = &g_bank[g_st.exo];
	now = time(NULL);
	strftime(ts, sizeof(ts), "%Y%m%d_%H%M%S", localtime(&now));
	sh("mkdir -p %s/archive %s/rendu %s/sujet %s/traces", q(g_work), q(g_work), q(g_work), q(g_work));
	sh("cd %s && for d in rendu/*/; do [ -d \"$d\" ] && mv \"$d\" \"archive/$(basename \"$d\")_%s\"; done; "
		"rm -rf sujet/* 2>/dev/null; true", q(g_work), ts);
	sh("mkdir -p %s/rendu/%s %s/sujet/%s", q(g_work), x->name, q(g_work), x->name);
	sh("cp %s/%s/sujet.fr.txt %s/%s/sujet.en.txt %s/sujet/%s/ 2>/dev/null", q(g_bankdir), x->name,
		q(g_bankdir), x->name, q(g_work), x->name);
}

static int	g_tty;

static void	ecran(void)
{
	if (g_tty)
		printf("\033[H\033[2J\033[3J");
}

static void	pause_x(void)
{
	char	line[200];

	if (!g_tty)
		return ;
	printf("\n  %s[ Entrée pour continuer ]%s ", C_D, C_R);
	fflush(stdout);
	if (!fgets(line, sizeof(line), stdin))
		return ;
}

static const char	*rel(const char *p)
{
	static char	cwd[1100];
	size_t		n;

	if (!getcwd(cwd, sizeof(cwd)))
		return (p);
	n = strlen(cwd);
	if (!strncmp(p, cwd, n) && p[n] == '/')
		return (p + n + 1);
	return (p);
}

static void	show_subject_text(void)
{
	t_xexo	*x;
	char	path[1300];
	char	*txt;
	char	*p;
	char	*e;

	x = &g_bank[g_st.exo];
	snprintf(path, sizeof(path), "%s/%s/sujet.%s.txt", g_bankdir, x->name, g_st.lang ? "en" : "fr");
	txt = slurp(path, NULL);
	if (!txt)
	{
		printf("   (sujet introuvable)\n");
		return ;
	}
	p = txt;
	while (*p)
	{
		e = strchr(p, '\n');
		if (!e)
			e = p + strlen(p);
		printf("   %.*s\n", (int)(e - p), p);
		p = *e ? e + 1 : e;
	}
	free(txt);
}

static long	remaining(void)
{
	if (!g_st.limit)
		return (-1);
	return (g_st.start + g_st.limit - (long)time(NULL));
}

static void	fmt_time(long s, char *b, size_t n)
{
	if (s < 0)
		s = 0;
	snprintf(b, n, "%ld:%02ld:%02ld", s / 3600, s / 60 % 60, s % 60);
}

static int	serie_niveau(int k)
{
	if (g_cfg.diff >= 1 && g_cfg.diff <= XLEVELS)
		return (g_cfg.diff);
	if (k >= g_cfg.serie)
		k = g_cfg.serie - 1;
	return (1 + k * XLEVELS / g_cfg.serie);
}

static void	entete(void)
{
	t_xexo	*x;
	char	t[32];
	int		i;

	x = &g_bank[g_st.exo];
	printf("\n  %s════════════════════════ EXAMSHELL ════════════════════════%s\n", C_AC, C_R);
	if (g_st.mode == 0)
	{
		printf("   %sExamen%s  ", C_B, C_R);
		for (i = 1; i <= XLEVELS; i++)
			printf("%s%s%s ", level_color(i), i < g_st.level ? "■" : i == g_st.level ? "▣" : "□", C_R);
		printf(" %s%3d/100%s", C_B, g_st.points, C_R);
	}
	else if (g_st.mode == 2)
	{
		printf("   %sSérie%s  ", C_B, C_R);
		for (i = 0; i < g_cfg.serie; i++)
			printf("%s%s%s ", level_color(serie_niveau(i)), i < g_st.points ? "■" : i == g_st.points ? "▣" : "□", C_R);
		printf(" %s%d/%d%s · %d réussi(s)", C_B, g_st.points + 1 > g_cfg.serie ? g_cfg.serie : g_st.points + 1,
			g_cfg.serie, C_R, g_st.solved);
	}
	else
		printf("   %sEntraînement%s · niveau %d · %d réussi(s)", C_B, C_R, g_st.level, g_st.solved);
	if (g_st.limit)
	{
		fmt_time(remaining(), t, sizeof(t));
		printf("        %sreste %s%s", remaining() < 1800 ? C_WN : C_D, t, C_R);
	}
	printf("\n   Exercice  %s%s%s · niveau %s%d%s", C_AC, x->name, C_R, level_color(x->level), x->level, C_R);
	if (g_st.mode == 0)
		printf(" · %d points", XPOINTS);
	if (g_st.fails)
		printf("   %s%d échec(s)%s", C_WN, g_st.fails, C_R);
	printf("\n  %s═══════════════════════════════════════════════════════════%s\n\n", C_AC, C_R);
}

static void	commandes(void)
{
	printf("\n  %sgrademe%s corriger   %ssujet%s relire   %sstatus%s où j'en suis   %sfinish%s terminer   %shelp%s aide\n\n",
		C_B, C_R, C_B, C_R, C_B, C_R, C_B, C_R, C_B, C_R);
}

static void	show_status(void)
{
	t_xexo	*x;
	char	t[32];
	char	p[1300];
	int		i;

	x = &g_bank[g_st.exo];
	ecran();
	entete();
	snprintf(p, sizeof(p), "%s/sujet/%s/", g_work, x->name);
	printf("   Sujet   %s\n", rel(p));
	snprintf(p, sizeof(p), "%s/rendu/%s/", g_work, x->name);
	printf("   Rendu   %s", rel(p));
	for (i = 0; i < x->nfiles; i++)
		printf("%s%s", i ? ", " : "", x->files[i]);
	printf("\n");
	fmt_time((long)time(NULL) - g_st.start, t, sizeof(t));
	printf("   Temps   %s écoulé\n", t);
	commandes();
}

static void	show_subject(void)
{
	ecran();
	entete();
	show_subject_text();
	commandes();
}

static int	parse_tests(const char *path, char *store, char **argv[XMAXTEST], char *args[XMAXTEST][XMAXARG + 2])
{
	char	*p;
	char	*line;
	char	*tok;
	char	*w;
	char	*r;
	int		n;
	int		a;

	n = 0;
	p = store;
	(void)path;
	while (p && *p && n < XMAXTEST)
	{
		line = p;
		p = strchr(p, '\n');
		if (p)
			*p++ = '\0';
		line[strcspn(line, "\r")] = '\0';
		if (!*line)
			continue ;
		a = 1;
		tok = strcmp(line, "-") ? line : NULL;
		while (tok && a <= XMAXARG)
		{
			w = strchr(tok, '\t');
			if (w)
				*w++ = '\0';
			args[n][a] = tok;
			for (r = tok; *r; r++)
			{
				if (*r != '\\')
					*tok++ = *r;
				else if (*++r == 't')
					*tok++ = '\t';
				else if (*r == 'n')
					*tok++ = '\n';
				else if (*r == '\\')
					*tok++ = '\\';
				else if (*r != 'e')
					break ;
			}
			*tok = '\0';
			a++;
			tok = w;
		}
		args[n][a] = NULL;
		argv[n] = args[n];
		n++;
	}
	return (n);
}

static int	run_bin(const char *bin, char **argv, char *out, size_t cap, size_t *len, const char *errlog)
{
	int				fd[2];
	pid_t			pid;
	fd_set			set;
	struct timeval	tv;
	char			buf[4096];
	ssize_t			r;
	time_t			end;
	int				status;
	int				efd;

	*len = 0;
	if (pipe(fd) < 0)
		return (-1);
	pid = fork();
	if (pid < 0)
		return (-1);
	if (pid == 0)
	{
		dup2(fd[1], 1);
		close(fd[0]);
		close(fd[1]);
		efd = open(errlog ? errlog : "/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0600);
		if (efd >= 0)
			dup2(efd, 2);
		efd = open("/dev/null", O_RDONLY);
		if (efd >= 0)
			dup2(efd, 0);
		argv[0] = (char *)bin;
		execv(bin, argv);
		_exit(127);
	}
	close(fd[1]);
	end = time(NULL) + XTMO;
	while (1)
	{
		FD_ZERO(&set);
		FD_SET(fd[0], &set);
		tv.tv_sec = 0;
		tv.tv_usec = 200000;
		if (select(fd[0] + 1, &set, NULL, NULL, &tv) > 0)
		{
			r = read(fd[0], buf, sizeof(buf));
			if (r <= 0)
				break ;
			if (*len + r <= cap)
				memcpy(out + *len, buf, r);
			*len += r;
		}
		if (time(NULL) > end)
		{
			kill(pid, SIGKILL);
			close(fd[0]);
			waitpid(pid, &status, 0);
			return (-2);
		}
	}
	close(fd[0]);
	while (waitpid(pid, &status, WNOHANG) == 0)
	{
		if (time(NULL) > end)
		{
			kill(pid, SIGKILL);
			waitpid(pid, &status, 0);
			return (-2);
		}
		usleep(10000);
	}
	if (WIFSIGNALED(status))
		return (1000 + WTERMSIG(status));
	return (WEXITSTATUS(status));
}

static void	cat_e(FILE *f, const char *s, size_t n)
{
	size_t			i;
	unsigned char	c;

	for (i = 0; i < n && i < 3000; i++)
	{
		c = s[i];
		if (c == '\n')
			fputs("$\n", f);
		else if (c == '\t')
			fputs("^I", f);
		else if (c < 32)
			fprintf(f, "^%c", c + 64);
		else if (c == 127)
			fputs("^?", f);
		else
			fputc(c, f);
	}
	if (n > 3000)
		fprintf(f, "\n[... coupé : %lu octets en tout]\n", (unsigned long)n);
	else if (n && s[n - 1] != '\n')
		fputc('\n', f);
}

static int	sym_ok(const t_xexo *x, const char *s, const char *defined)
{
	static const char	*sys[] = {"_GLOBAL_OFFSET_TABLE_", "__stack_chk_fail", "__stack_chk_guard",
		"dyld_stub_binder", "__stack_chk_fail_local", "_DYNAMIC", NULL};
	char				pat[80];
	int					i;

	for (i = 0; sys[i]; i++)
		if (!strcmp(s, sys[i]))
			return (1);
	for (i = 0; i < x->nallowed; i++)
	{
		if (!strcmp(s, x->allowed[i]))
			return (1);
		if (!strcmp(x->allowed[i], "printf") && (!strcmp(s, "puts") || !strcmp(s, "putchar")))
			return (1);
	}
	snprintf(pat, sizeof(pat), "\n%s\n", s);
	return (strstr(defined, pat) != NULL);
}

static int	check_symbols(const t_xexo *x, char *bad, size_t nbad)
{
	FILE	*p;
	char	cmd[2600];
	char	line[400];
	char	type;
	char	sym[200];
	char	*defined;
	char	*undef;
	char	*tok;
	size_t	nd;
	size_t	nu;

	defined = calloc(1, 60000);
	undef = calloc(1, 60000);
	if (!defined || !undef)
		return (free(defined), free(undef), 0);
	nd = 1;
	nu = 0;
	defined[0] = '\n';
	snprintf(cmd, sizeof(cmd), "nm %s/u_*.o 2>/dev/null", q(g_build));
	p = popen(cmd, "r");
	while (p && fgets(line, sizeof(line), p))
	{
		if (sscanf(line, " %c %199s", &type, sym) == 2 && type == 'U')
		{
			if (nu + strlen(sym) + 2 < 60000)
				nu += sprintf(undef + nu, "%s\n", sym);
		}
		else if (sscanf(line, "%*x %c %199s", &type, sym) == 2 && nd + strlen(sym) + 2 < 60000)
			nd += sprintf(defined + nd, "%s\n", sym);
	}
	if (p)
		pclose(p);
	bad[0] = '\0';
	for (tok = strtok(undef, "\n"); tok; tok = strtok(NULL, "\n"))
	{
#ifdef __APPLE__
		if (tok[0] == '_')
			tok++;
#endif
		if (!sym_ok(x, tok, defined) && !strstr(bad, tok) && strlen(bad) + strlen(tok) + 2 < nbad)
			sprintf(bad + strlen(bad), "%s ", tok);
	}
	free(defined);
	free(undef);
	return (bad[0] == '\0');
}

static int	build(const t_xexo *x, const char *srcdir, const char *maindir, int san, char *why, size_t nwhy)
{
	char	objs[2000];
	char	errf[1300];
	char	*err;
	int		i;
	int		r;

	why[0] = '\0';
	objs[0] = '\0';
	snprintf(errf, sizeof(errf), "%s/cc.err", g_build);
	sh("rm -f %s/u_*.o %s/bin%s", q(g_build), q(g_build), san ? "_san" : "");
	for (i = 0; i < x->nfiles; i++)
	{
		if (strcmp(x->files[i] + strlen(x->files[i]) - 2, ".c"))
			continue ;
		r = sh("cc -std=gnu17 -Wall -Wextra -Werror %s -I %s -c %s/%s -o %s/u_%d.o 2> %s",
				san ? "-g -fsanitize=address,undefined -fno-omit-frame-pointer" : "",
				q(srcdir), q(srcdir), x->files[i], q(g_build), i, q(errf));
		if (r != 0)
		{
			err = slurp(errf, NULL);
			snprintf(why, nwhy, "%s", err ? err : "la compilation a échoué");
			free(err);
			return (1);
		}
		snprintf(objs + strlen(objs), sizeof(objs) - strlen(objs), " %s/u_%d.o", q(g_build), i);
	}
	if (!san && !check_symbols(x, errf, sizeof(errf)))
	{
		snprintf(why, nwhy, "fonction(s) non autorisée(s) : %s", errf);
		return (2);
	}
	snprintf(errf, sizeof(errf), "%s/cc.err", g_build);
	if (x->prog)
		r = sh("cc %s %s -o %s/bin%s 2> %s", san ? "-fsanitize=address,undefined" : "", objs, q(g_build),
				san ? "_san" : "", q(errf));
	else
		r = sh("cc -std=gnu17 -w %s -I %s %s/%s/main.c %s -o %s/bin%s 2> %s", san ? "-fsanitize=address,undefined" : "",
				q(srcdir), q(maindir), x->name, objs, q(g_build), san ? "_san" : "", q(errf));
	if (r != 0)
	{
		err = slurp(errf, NULL);
		snprintf(why, nwhy, "%s", err ? err : "l'édition de liens a échoué");
		free(err);
		return (1);
	}
	return (0);
}

static void	trace_write(const t_xexo *x, const char *title, char **argv, const char *exp, size_t ne,
		const char *got, size_t ng, const char *note)
{
	FILE	*f;
	char	path[1300];
	int		i;

	if (!g_st.trace)
		return ;
	snprintf(path, sizeof(path), "%s/traces/trace_%s.txt", g_work, x->name);
	f = fopen(path, "w");
	if (!f)
		return ;
	fprintf(f, "= %s · %s\n", x->name, title);
	if (argv)
	{
		fprintf(f, "\n$> ./%s", x->prog ? x->name : "test");
		for (i = 1; argv[i]; i++)
			fprintf(f, " \"%s\"", argv[i]);
		fprintf(f, " | cat -e\n");
	}
	if (note && *note)
		fprintf(f, "\n%s\n", note);
	if (exp)
	{
		for (i = 0; (size_t)i < ne && (size_t)i < ng && exp[i] == got[i]; i++)
			;
		fprintf(f, "\nattendu : %lu octet(s) · obtenu : %lu octet(s) · première différence à l'octet %d\n",
			(unsigned long)ne, (unsigned long)ng, i + 1);
		fprintf(f, "\n--- attendu\n");
		cat_e(f, exp, ne);
		fprintf(f, "--- obtenu\n");
		cat_e(f, got, ng);
	}
	fclose(f);
}

static void	trace_show(const t_xexo *x)
{
	char	path[1300];
	char	*txt;

	snprintf(path, sizeof(path), "%s/traces/trace_%s.txt", g_work, x->name);
	if (g_st.trace == 2)
	{
		txt = slurp(path, NULL);
		if (txt)
			printf("%s%s%s\n", C_D, txt, C_R);
		free(txt);
		printf("  trace : %s\n", rel(path));
	}
	else if (g_st.trace == 1)
		printf("  %sla trace sera lisible à la fin de l'exam%s\n", C_D, C_R);
}

static int	expected_next(const char **p, const char *end, const char **data, size_t *len)
{
	char	*e;

	if (*p >= end || **p != '#')
		return (0);
	*len = strtoul(*p + 1, &e, 10);
	if (*e != '\n' || e + 1 + *len > end)
		return (0);
	*data = e + 1;
	*p = *data + *len + 1;
	return (1);
}

static int	memory_bonus(const t_xexo *x, const char *srcdir, char **argv[XMAXTEST], int ntests)
{
	static char	out[XOUT];
	char		why[400];
	char		bin[1300];
	char		log[1300];
	size_t		len;
	int			bad;
	int			i;
	int			r;

	if (build(x, srcdir, g_bankdir, 1, why, sizeof(why)) != 0)
		return (-1);
	snprintf(bin, sizeof(bin), "%s/bin_san", g_build);
	snprintf(log, sizeof(log), "%s/san.log", g_build);
	setenv("ASAN_OPTIONS", "exitcode=77:detect_leaks=1:abort_on_error=0", 1);
	setenv("UBSAN_OPTIONS", "exitcode=77:halt_on_error=1", 1);
	setenv("LSAN_OPTIONS", "exitcode=77", 1);
	bad = 0;
	for (i = 0; i < ntests; i++)
	{
		r = run_bin(bin, argv[i], out, sizeof(out), &len, log);
		bad += (r == 77 || r >= 1000);
	}
	return (bad);
}

static int	grade(const t_xexo *x, const char *srcdir, int generate)
{
	static char	out[XOUT];
	static char	*args[XMAXTEST][XMAXARG + 2];
	char		**argv[XMAXTEST];
	char		why[3000];
	char		path[1300];
	char		bin[1300];
	char		*tests;
	char		*expd;
	const char	*p;
	const char	*end;
	const char	*data;
	size_t		len;
	size_t		elen;
	size_t		total;
	FILE		*g;
	int			ntests;
	int			i;
	int			r;

	sh("mkdir -p %s", q(g_build));
	for (i = 0; i < x->nfiles; i++)
	{
		snprintf(path, sizeof(path), "%s/%s", srcdir, x->files[i]);
		if (access(path, R_OK) != 0)
		{
			snprintf(why, sizeof(why), "fichier attendu introuvable : %s", rel(path));
			trace_write(x, "fichier manquant", NULL, NULL, 0, NULL, 0, why);
			printf("  %s%s%s\n", C_KO, why, C_R);
			return (1);
		}
	}
	r = build(x, srcdir, g_bankdir, 0, why, sizeof(why));
	if (r != 0)
	{
		trace_write(x, r == 2 ? "fonction non autorisée" : "erreur de compilation", NULL, NULL, 0, NULL, 0, why);
		if (generate)
			printf("%s\n", why);
		return (1);
	}
	snprintf(path, sizeof(path), "%s/%s/tests", g_bankdir, x->name);
	tests = x->prog ? slurp(path, NULL) : NULL;
	ntests = 1;
	args[0][1] = NULL;
	argv[0] = args[0];
	if (x->prog)
		ntests = parse_tests(path, tests, argv, args);
	if (ntests < 1)
		return (free(tests), printf("  tests introuvables pour %s\n", x->name), 1);
	snprintf(bin, sizeof(bin), "%s/bin", g_build);
	snprintf(path, sizeof(path), "%s/%s/attendu", g_bankdir, x->name);
	if (generate)
	{
		g = fopen(path, "wb");
		if (!g)
			return (free(tests), 1);
		for (i = 0; i < ntests; i++)
		{
			r = run_bin(bin, argv[i], out, sizeof(out), &len, NULL);
			if (r != 0 || len > sizeof(out))
				printf("  attention %s test %d : code %d, %lu octets\n", x->name, i + 1, r, (unsigned long)len);
			fprintf(g, "#%lu\n", (unsigned long)len);
			fwrite(out, 1, len, g);
			fputc('\n', g);
		}
		fclose(g);
		free(tests);
		return (0);
	}
	expd = slurp(path, &total);
	if (!expd)
		return (free(tests), printf("  sorties attendues introuvables pour %s\n", x->name), 1);
	p = expd;
	end = expd + total;
	for (i = 0; i < ntests; i++)
	{
		if (!expected_next(&p, end, &data, &elen))
		{
			printf("  sorties attendues incomplètes pour %s : préviens splatch_afk\n", x->name);
			break ;
		}
		r = run_bin(bin, argv[i], out, sizeof(out), &len, NULL);
		if (r == -2)
		{
			trace_write(x, "boucle infinie", argv[i], NULL, 0, NULL, 0, "aucune réponse après 5 secondes");
			break ;
		}
		if (r >= 1000)
		{
			snprintf(why, sizeof(why), "le programme s'est arrêté sur le signal %d (%s)", r - 1000,
				r - 1000 == SIGSEGV ? "segfault" : r - 1000 == SIGABRT ? "abort" : r - 1000 == SIGBUS ? "bus error" : "signal");
			trace_write(x, "plantage", argv[i], data, elen, out, len > sizeof(out) ? sizeof(out) : len, why);
			break ;
		}
		if (len != elen || memcmp(out, data, elen))
		{
			trace_write(x, "sortie différente", argv[i], data, elen, out, len > sizeof(out) ? sizeof(out) : len, "");
			break ;
		}
	}
	r = i < ntests;
	if (!r)
	{
		i = memory_bonus(x, srcdir, argv, ntests);
		if (i == 0)
			printf("  %smémoire : propre (ASan, UBSan, fuites)%s\n", C_D, C_R);
		else if (i > 0)
			printf("  %smémoire : %d test(s) avec une erreur mémoire, une fuite ou un comportement indéfini%s\n"
				"  %s(le vrai exam ne le vérifie pas, mais une évaluation, oui)%s\n", C_WN, i, C_R, C_D, C_R);
	}
	free(tests);
	free(expd);
	return (r);
}

static void	reward(long octets, const char *why)
{
	if (octets <= 0)
		return ;
	g_save.octets += octets;
	g_save.xp += octets * 2;
	printf("  %s+%ld Octets · +%ld XP%s  %s%s%s\n", C_OK, octets, octets * 2, C_R, C_D, why, C_R);
}

static void	finish(int timeout)
{
	char	p[1300];
	char	t[32];
	long	elapsed;
	long	gain;
	int		i;

	elapsed = (long)time(NULL) - g_st.start;
	fmt_time(elapsed, t, sizeof(t));
	ecran();
	printf("\n%s══════════════════════════════════════════════════════════════%s\n", C_AC, C_R);
	if (timeout)
		printf("  %sTemps écoulé.%s\n", C_KO, C_R);
	if (g_st.mode == 0)
	{
		printf("  %sExamen terminé%s · note : %s%d/100%s · temps : %s · échecs : %d\n", C_B, C_R,
			g_st.points == 100 ? C_OK : C_WN, g_st.points, C_R, t, g_st.tfails);
		printf("  %s\n", g_st.points == 100 ? "Validé." : "Non validé : il faut 100/100.");
		g_xs.runs++;
		if (g_st.cheats)
			printf("  %scheat utilisé pendant la session : aucun Octet pour cet exam%s\n", C_WN, C_R);
		else
		{
			gain = (g_st.points / XPOINTS) * 100;
			if (g_st.points == 100)
			{
				gain += 200;
				if (elapsed < 5400)
					gain += 150;
				if (g_st.tfails == 0)
					gain += 100;
				if (!g_xs.passed)
					gain += 1000;
			}
			if ((long)time(NULL) - g_xs.lastreward < 21600 && g_xs.lastreward)
				gain /= 4;
			reward(gain, g_st.points == 100 && !g_xs.passed ? "premier exam validé" : "");
			if (gain > 0)
				g_xs.lastreward = (long)time(NULL);
		}
		if (g_st.points > g_xs.best)
			g_xs.best = g_st.points;
		if (g_st.points == 100)
		{
			if (!g_st.cheats && (!g_xs.besttime || elapsed < g_xs.besttime))
				g_xs.besttime = elapsed;
			g_xs.passed += !g_st.cheats;
		}
		tdf_stats_begin("exam");
		for (i = g_st.points / XPOINTS + 1; i <= XLEVELS; i++)
			tdf_stats_fail(i == g_st.level ? g_bank[g_st.exo].name : "niveau", "ECHEC");
		tdf_stats_end(g_st.points / XPOINTS, XLEVELS);
	}
	else if (g_st.mode == 2)
	{
		printf("  %sSérie terminée%s · %s%d/%d%s réussi(s) · temps : %s · échecs : %d\n", C_B, C_R,
			g_st.solved == g_cfg.serie ? C_OK : C_WN, g_st.solved, g_cfg.serie, C_R, t, g_st.tfails);
		tdf_stats_begin("exam");
		tdf_stats_end(g_st.solved, g_cfg.serie);
	}
	else
		printf("  %sEntraînement terminé%s · %d exercice(s) réussi(s) · temps : %s\n", C_B, C_R, g_st.solved, t);
	snprintf(p, sizeof(p), "%s/traces/", g_work);
	if (g_st.trace == 1)
		printf("  traces : %s\n", rel(p));
	snprintf(p, sizeof(p), "%s/archive/", g_work);
	printf("  tes rendus sont gardés dans %s\n", rel(p));
	printf("%s══════════════════════════════════════════════════════════════%s\n\n", C_AC, C_R);
	sh("cd %s && mkdir -p archive && for d in rendu/*/; do [ -d \"$d\" ] && mv \"$d\" \"archive/$(basename \"$d\")_fin_%ld\"; done; "
		"rm -rf sujet/* 2>/dev/null; true", q(g_work), (long)time(NULL));
	g_st.active = 0;
	state_save();
	save_write();
}

static void	view_exo(void)
{
	t_xexo	*x;
	char	p[1300];
	int		i;

	x = &g_bank[g_st.exo];
	ecran();
	entete();
	snprintf(p, sizeof(p), "%s/rendu/%s/", g_work, x->name);
	printf("   Ton rendu : %s%s", C_B, rel(p));
	for (i = 0; i < x->nfiles; i++)
		printf("%s%s", i ? ", " : "", x->files[i]);
	printf("%s\n   %sÉcris ton code dans un autre terminal, puis tape grademe ici.%s\n\n", C_R, C_D, C_R);
	show_subject_text();
	commandes();
}

static void	next_exo(void)
{
	g_st.exo = pick(g_st.level, g_st.exo);
	g_st.fails = 0;
	g_st.lastfail = 0;
	install();
	state_save();
	view_exo();
}

static int	do_grademe(void)
{
	t_xexo	*x;
	char	src[1300];
	long	wait;
	int		first;

	x = &g_bank[g_st.exo];
	if (g_st.wait && g_st.fails)
	{
		wait = g_st.lastfail + 60 * (g_st.fails > 10 ? 10 : g_st.fails) - (long)time(NULL);
		if (wait > 0)
		{
			printf("  %sencore %ld s avant de pouvoir relancer grademe (%d échec(s) sur cet exercice)%s\n",
				C_WN, wait, g_st.fails, C_R);
			return (0);
		}
	}
	snprintf(src, sizeof(src), "%s/rendu/%s", g_work, x->name);
	if (g_tty)
	{
		printf("  Rendre %s%s%s maintenant ? (o/n) ", C_AC, x->name, C_R);
		fflush(stdout);
		if (!fgets(src, sizeof(src), stdin) || !(src[0] == 'o' || src[0] == 'O' || src[0] == 'y' || src[0] == 'Y'))
			return (0);
		snprintf(src, sizeof(src), "%s/rendu/%s", g_work, x->name);
	}
	ecran();
	entete();
	printf("   Correction de %s%s%s en cours...\n", C_AC, x->name, C_R);
	fflush(stdout);
	if (grade(x, src, 0) != 0)
	{
		g_st.fails++;
		g_st.tfails++;
		g_st.lastfail = (long)time(NULL);
		printf("\n  %s╔══════════════════════════════╗%s\n", C_KO, C_R);
		printf("  %s║           FAILURE            ║%s\n", C_KO, C_R);
		printf("  %s╚══════════════════════════════╝%s\n\n", C_KO, C_R);
		trace_show(x);
		tdf_stats_begin("exam");
		tdf_stats_fail(x->name, "ECHEC");
		tdf_stats_end(0, 1);
		state_save();
		if (g_st.wait)
			printf("  %sprochain grademe possible dans %d min%s\n", C_D, g_st.fails > 10 ? 10 : g_st.fails, C_R);
		pause_x();
		if (g_tty)
			view_exo();
		return (0);
	}
	printf("\n  %s╔══════════════════════════════╗%s\n", C_OK, C_R);
	printf("  %s║           SUCCESS            ║%s\n", C_OK, C_R);
	printf("  %s╚══════════════════════════════╝%s\n\n", C_OK, C_R);
	first = !done_has(x->name);
	done_add(x->name);
	g_st.solved++;
	if (g_st.mode == 1 || g_st.mode == 2)
	{
		if (!g_st.cheats)
			reward(first ? 15 * x->level : 2, first ? "nouvel exercice réussi" : "exercice déjà réussi");
		if (g_st.mode == 1)
		{
			tdf_stats_begin("exam");
			tdf_stats_end(1, 1);
		}
		save_write();
		if (g_st.mode == 2 && ++g_st.points >= g_cfg.serie)
			return (1);
		if (g_st.mode == 2)
		{
			printf("  %sexercice suivant : %d/%d%s\n", C_OK, g_st.points + 1, g_cfg.serie, C_R);
			g_st.level = serie_niveau(g_st.points);
		}
		pause_x();
		next_exo();
		return (0);
	}
	g_st.points += XPOINTS;
	save_write();
	if (g_st.level == XLEVELS)
		return (1);
	printf("  %s+%d points · niveau suivant%s\n", C_OK, XPOINTS, C_R);
	pause_x();
	g_st.level++;
	next_exo();
	return (0);
}

int	tdf_code_ok(const char *login, const char *code);

static int	code_ok(void)
{
	return (g_xs.code[0] && tdf_code_ok(g_save.login, g_xs.code));
}

static void	enter_code(const char *c)
{
	if (!c)
	{
		printf("  usage : code TON_CODE (à demander à splatch_afk sur Discord, pour ton login : %s)\n", g_save.login);
		return ;
	}
	if (!tdf_code_ok(g_save.login, c))
	{
		printf("  %scode refusé%s : il ne correspond pas au login %s.\n", C_KO, C_R, g_save.login);
		return ;
	}
	snprintf(g_xs.code, sizeof(g_xs.code), "%s", c);
	save_write();
	printf("  %scode accepté%s : skip, reroll et trace sont débloqués pour %s.\n", C_OK, C_R, g_save.login);
	printf("  %sun exam passé avec un cheat ne rapporte aucun Octet.%s\n", C_D, C_R);
}

static int	do_cheat(const char *cmd)
{
	if (g_cfg.real)
	{
		printf("  conditions réelles : aucun cheat pendant cette session\n");
		return (0);
	}
	if (!code_ok())
	{
		printf("  %scommande verrouillée%s : il faut un code, à demander à splatch_afk sur Discord.\n", C_WN, C_R);
		return (0);
	}
	g_st.cheats++;
	g_xs.cheats++;
	if (!strcmp(cmd, "trace"))
	{
		g_st.trace = 2;
		printf("  trace affichée à chaque échec\n");
	}
	else if (!strcmp(cmd, "reroll"))
		next_exo();
	else if (!strcmp(cmd, "skip"))
	{
		if (g_st.mode == 0 && g_st.level == XLEVELS)
			return (1);
		if (g_st.mode == 0)
			g_st.level++;
		if (g_st.mode == 2 && ++g_st.points >= g_cfg.serie)
			return (1);
		if (g_st.mode == 2)
			g_st.level = serie_niveau(g_st.points);
		next_exo();
	}
	state_save();
	return (0);
}

static void	show_help(void)
{
	ecran();
	entete();
	printf("  %sgrademe%s   corriger l'exercice en cours\n", C_B, C_R);
	printf("  %sstatus%s    où j'en suis, temps restant\n", C_B, C_R);
	printf("  %ssujet%s     relire le sujet (sujet en : en anglais, sujet fr : en français)\n", C_B, C_R);
	printf("  %sfinish%s    terminer la session maintenant\n", C_B, C_R);
	printf("  %shelp%s      cette aide\n", C_B, C_R);
	printf("  %sskip, reroll, trace : avec un code (commande : code TON_CODE)%s\n\n", C_D, C_R);
	printf("  Écris ton code dans un autre terminal ou dans ton éditeur, dans le dossier rendu/.\n\n");
}

static void	shell(void)
{
	char	line[1000];
	char	t[32];
	char	*cmd;
	int		end;

	end = 0;
	while (!end)
	{
		if (g_st.limit && remaining() <= 0)
			return (finish(1));
		(void)t;
		printf("%sexamshell%s> ", C_AC, C_R);
		fflush(stdout);
		if (!fgets(line, sizeof(line), stdin))
		{
			printf("\n  session gardée : relance l'exam pour la reprendre.\n");
			return ;
		}
		cmd = strtok(line, " \t\r\n");
		if (!cmd)
			continue ;
		if (g_st.limit && remaining() <= 0)
			return (finish(1));
		if (!strcmp(cmd, "grademe"))
			end = do_grademe();
		else if (!strcmp(cmd, "status"))
			show_status();
		else if (!strcmp(cmd, "sujet") || !strcmp(cmd, "subject"))
		{
			cmd = strtok(NULL, " \t\r\n");
			if (cmd && (!strcmp(cmd, "en") || !strcmp(cmd, "fr")))
				g_st.lang = !strcmp(cmd, "en");
			show_subject();
		}
		else if (!strcmp(cmd, "help") || !strcmp(cmd, "aide"))
			show_help();
		else if (!strcmp(cmd, "skip") || !strcmp(cmd, "reroll") || !strcmp(cmd, "trace"))
			end = do_cheat(cmd);
		else if (!strcmp(cmd, "code"))
			enter_code(strtok(NULL, " \t\r\n"));
		else if (!strcmp(cmd, "clear"))
			view_exo();
		else if (!strcmp(cmd, "finish") || !strcmp(cmd, "exit") || !strcmp(cmd, "quit"))
		{
			printf("  Terminer la session ? (o/n) ");
			fflush(stdout);
			if (fgets(line, sizeof(line), stdin) && (line[0] == 'o' || line[0] == 'O' || line[0] == 'y'))
				end = 1;
		}
		else
			printf("  commande inconnue : %s (help pour la liste)\n", cmd);
	}
	finish(0);
}

static const char	*onoff(int v, const char *a, const char *b)
{
	return (v ? a : b);
}

static int	touche(void)
{
	struct termios	old;
	struct termios	t;
	unsigned char	c[3];
	int				k;
	fd_set			fs;
	struct timeval	tv;

	fflush(stdout);
	tcgetattr(0, &old);
	t = old;
	t.c_lflag &= ~(ICANON | ECHO);
	t.c_cc[VMIN] = 1;
	t.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &t);
	k = -1;
	if (read(0, c, 1) == 1)
	{
		k = c[0];
		if (k == 27)
		{
			FD_ZERO(&fs);
			FD_SET(0, &fs);
			tv.tv_sec = 0;
			tv.tv_usec = 60000;
			if (select(1, &fs, NULL, NULL, &tv) > 0 && read(0, c + 1, 2) == 2 && (c[1] == '[' || c[1] == 'O'))
				k = c[2] == 'A' ? 1000 : c[2] == 'B' ? 1001 : c[2] == 'C' ? 1002 : c[2] == 'D' ? 1003 : 0;
		}
		else if (k == 'z' || k == 'w' || k == 'k')
			k = 1000;
		else if (k == 's' || k == 'j')
			k = 1001;
		else if (k == 'd' || k == 'l')
			k = 1002;
		else if (k == 'a' || k == 'h')
			k = 1003;
	}
	tcsetattr(0, TCSANOW, &old);
	return (k);
}

static int	largeur(const char *s)
{
	int	n;

	n = 0;
	while (*s)
		if ((*s++ & 0xC0) != 0x80)
			n++;
	return (n);
}

static int	reglage_ligne(int r, int sel, const char *nom, const char *val, const char *desc, const char **d)
{
	int	pad;

	pad = 24 - largeur(nom);
	if (pad < 1)
		pad = 1;
	if (r == sel)
	{
		printf("  %s❯ %s%*s%s %s◀ %s ▶%s\n", C_AC, nom, pad, "", C_R, C_B, val, C_R);
		*d = desc;
	}
	else
		printf("    %s%*s %s\n", nom, pad, "", val);
	return (r + 1);
}

static int	reglages_fleches(void)
{
	static const char	*tr[3] = {"jamais", "à la fin de la session", "à chaque échec"};
	static const char	*modes[3] = {"examen blanc", "entraînement", "série"};
	char				v[80];
	const char			*desc;
	int					sel;
	int					n;
	int					k;
	int					id[9];

	sel = 0;
	while (1)
	{
		if (g_cfg.real)
		{
			g_cfg.mode = 0;
			g_cfg.chrono = 1;
			g_cfg.wait = 1;
			g_cfg.trace = 1;
		}
		ecran();
		printf("\n  %s════════════════════════ EXAMSHELL ════════════════════════%s\n", C_AC, C_R);
		printf("   %sLe testeur des fous · Exam en C%s\n", C_B, C_R);
		printf("   %s%d exercices · 4 niveaux%s\n", C_D, g_nbank, C_R);
		printf("  %s═══════════════════════════════════════════════════════════%s\n\n", C_AC, C_R);
		desc = "";
		n = 0;
		id[n] = 0;
		n = reglage_ligne(n, sel, "Mode", modes[g_cfg.mode],
				g_cfg.mode == 0 ? "Comme le vrai : 4 niveaux, un exercice par niveau, 25 points chacun, 100 pour valider."
				: g_cfg.mode == 1 ? "Des exercices d'un seul niveau, à la suite, sans note."
				: "Tu choisis combien d'exercices (1 à 5) et leur difficulté.", &desc);
		if (g_cfg.mode == 2)
		{
			snprintf(v, sizeof(v), "%d", g_cfg.serie);
			id[n] = 1;
			n = reglage_ligne(n, sel, "Exercices", v, "Combien d'exercices dans la série, de 1 à 5.", &desc);
			if (g_cfg.diff)
				snprintf(v, sizeof(v), "niveau %d", g_cfg.diff);
			else
				snprintf(v, sizeof(v), "croissante");
			id[n] = 2;
			n = reglage_ligne(n, sel, "Difficulté", v,
					"Croissante : du niveau 1 vers le niveau 4. Sinon, tous les exercices au même niveau.", &desc);
		}
		else if (g_cfg.mode == 1)
		{
			snprintf(v, sizeof(v), "%d", g_cfg.level);
			id[n] = 3;
			n = reglage_ligne(n, sel, "Niveau", v, "Le niveau des exercices proposés, de 1 à 4.", &desc);
		}
		else
		{
			id[n] = 4;
			n = reglage_ligne(n, sel, "Chronomètre", g_cfg.chrono ? "3 heures" : "libre",
					"3 heures comme le vrai examen, ou sans limite.", &desc);
		}
		id[n] = 5;
		n = reglage_ligne(n, sel, "Attente après un échec", g_cfg.wait ? "progressive" : "aucune",
				"Progressive : 1 minute de plus à chaque échec, 10 au plus, comme le vrai.", &desc);
		id[n] = 6;
		n = reglage_ligne(n, sel, "Trace", tr[g_cfg.trace], "Quand afficher le détail des tests ratés.", &desc);
		id[n] = 7;
		n = reglage_ligne(n, sel, "Langue du sujet", g_cfg.lang ? "anglais" : "français", "La langue des sujets.", &desc);
		id[n] = 8;
		n = reglage_ligne(n, sel, "Conditions réelles", g_cfg.real ? "oui" : "non",
				"Oui : examen blanc, 3 heures, attente, trace à la fin, aucun cheat. Verrouille les autres réglages.", &desc);
		printf("\n");
		if (sel == n)
		{
			printf("  %s❯ Commencer%s\n", C_OK, C_R);
			desc = "Lance la session avec ces réglages.";
		}
		else
			printf("    Commencer\n");
		if (sel == n + 1)
		{
			printf("  %s❯ Quitter%s\n", C_AC, C_R);
			desc = "Revenir sans lancer d'exam.";
		}
		else
			printf("    Quitter\n");
		printf("\n  %s%s%s\n", C_D, desc, C_R);
		if (g_cfg.real && sel < n && id[sel] != 8)
			printf("  %sconditions réelles : ce réglage est verrouillé%s\n", C_WN, C_R);
		printf("\n  %s↑ ↓ choisir   ◀ ▶ changer   Entrée valider   q quitter%s\n", C_D, C_R);
		k = touche();
		if (k < 0 || k == 'q' || k == 'Q' || (k == 27))
			return (0);
		if (k == 1000)
			sel = (sel + n + 1) % (n + 2);
		else if (k == 1001)
			sel = (sel + 1) % (n + 2);
		else if ((k == '\n' || k == '\r' || k == ' ') && sel == n)
			break ;
		else if ((k == '\n' || k == '\r' || k == ' ') && sel == n + 1)
			return (0);
		else if ((k == '\n' || k == '\r' || k == ' ' || k == 1002 || k == 1003) && sel < n)
		{
			if (id[sel] == 8)
				g_cfg.real = !g_cfg.real;
			else if (g_cfg.real)
				continue ;
			else if (id[sel] == 0)
				g_cfg.mode = (g_cfg.mode + (k == 1003 ? 2 : 1)) % 3;
			else if (id[sel] == 1)
				g_cfg.serie = k == 1003 ? (g_cfg.serie + 3) % 5 + 1 : g_cfg.serie % 5 + 1;
			else if (id[sel] == 2)
				g_cfg.diff = (g_cfg.diff + (k == 1003 ? XLEVELS : 1)) % (XLEVELS + 1);
			else if (id[sel] == 3)
				g_cfg.level = k == 1003 ? (g_cfg.level + XLEVELS - 2) % XLEVELS + 1 : g_cfg.level % XLEVELS + 1;
			else if (id[sel] == 4)
				g_cfg.chrono = !g_cfg.chrono;
			else if (id[sel] == 5)
				g_cfg.wait = !g_cfg.wait;
			else if (id[sel] == 6)
				g_cfg.trace = (g_cfg.trace + (k == 1003 ? 2 : 1)) % 3;
			else if (id[sel] == 7)
				g_cfg.lang = !g_cfg.lang;
			if (sel >= n)
				sel = 0;
		}
	}
	cfg_save();
	return (1);
}

static int	settings(void)
{
	static const char	*tr[3] = {"jamais", "à la fin de la session", "à chaque échec"};
	char				line[80];
	int					c;

	if (g_tty)
		return (reglages_fleches());
	while (1)
	{
		if (g_cfg.real)
		{
			g_cfg.mode = 0;
			g_cfg.chrono = 1;
			g_cfg.wait = 1;
			g_cfg.trace = 1;
		}
		ecran();
		printf("\n  %s════════════════════════ EXAMSHELL ════════════════════════%s\n", C_AC, C_R);
		printf("   %sLe testeur des fous · Exam en C%s\n", C_B, C_R);
		printf("   %s%d exercices · 4 niveaux · 25 points par niveau · 100 pour valider%s\n", C_D, g_nbank, C_R);
		printf("  %s═══════════════════════════════════════════════════════════%s\n\n", C_AC, C_R);
		printf("  %s1%s  Mode                    %s\n", C_AC, C_R, g_cfg.mode == 2 ? "série (nombre d'exercices et difficulté au choix)"
			: onoff(g_cfg.mode, "entraînement sur un niveau", "examen (4 niveaux, un exercice par niveau)"));
		if (g_cfg.mode == 2)
		{
			printf("  %s2%s  Exercices               %d\n", C_AC, C_R, g_cfg.serie);
			printf("  %s7%s  Difficulté              ", C_AC, C_R);
			if (g_cfg.diff)
				printf("%sniveau %d%s, fixe\n", level_color(g_cfg.diff), g_cfg.diff, C_R);
			else
				printf("croissante (du facile au difficile)\n");
		}
		else if (g_cfg.mode)
			printf("  %s2%s  Niveau                  %s%d%s\n", C_AC, C_R, level_color(g_cfg.level), g_cfg.level, C_R);
		else
			printf("  %s2%s  Chronomètre             %s\n", C_AC, C_R, onoff(g_cfg.chrono, "3 heures", "libre"));
		printf("  %s3%s  Attente après un échec  %s\n", C_AC, C_R,
			onoff(g_cfg.wait, "progressive (1 min de plus à chaque échec, 10 au plus)", "aucune"));
		printf("  %s4%s  Trace                   %s\n", C_AC, C_R, tr[g_cfg.trace]);
		printf("  %s5%s  Langue du sujet         %s\n", C_AC, C_R, onoff(g_cfg.lang, "anglais", "français"));
		printf("  %s6%s  Conditions réelles      %s\n\n", C_AC, C_R,
			onoff(g_cfg.real, "oui : examen, 3 heures, attente, trace à la fin, aucun cheat", "non"));
		printf("  %sEntrée%s commencer   %s1 à %d%s changer un réglage   %sq%s quitter\n\n  > ", C_B, C_R, C_B,
			g_cfg.mode == 2 ? 7 : 6, C_R, C_B, C_R);
		fflush(stdout);
		if (!fgets(line, sizeof(line), stdin))
			return (0);
		c = line[0];
		if (c == 'q' || c == 'Q')
			return (0);
		if (c == '\n' || c == '\r')
			break ;
		if (c == '6')
			g_cfg.real = !g_cfg.real;
		else if (g_cfg.real)
			printf("  %sconditions réelles : ce réglage est verrouillé (6 pour les quitter)%s\n", C_WN, C_R);
		else if (c == '1')
			g_cfg.mode = (g_cfg.mode + 1) % 3;
		else if (c == '2' && g_cfg.mode == 2)
			g_cfg.serie = g_cfg.serie % 5 + 1;
		else if (c == '7' && g_cfg.mode == 2)
			g_cfg.diff = (g_cfg.diff + 1) % (XLEVELS + 1);
		else if (c == '2' && g_cfg.mode)
			g_cfg.level = g_cfg.level % XLEVELS + 1;
		else if (c == '2')
			g_cfg.chrono = !g_cfg.chrono;
		else if (c == '3')
			g_cfg.wait = !g_cfg.wait;
		else if (c == '4')
			g_cfg.trace = (g_cfg.trace + 1) % 3;
		else if (c == '5')
			g_cfg.lang = !g_cfg.lang;
	}
	cfg_save();
	return (1);
}

static void	start(void)
{
	memset(&g_st, 0, sizeof(g_st));
	g_st.active = 1;
	g_st.mode = g_cfg.mode;
	if (g_cfg.serie < 1 || g_cfg.serie > 5)
		g_cfg.serie = 4;
	g_st.level = g_cfg.mode == 2 ? serie_niveau(0) : (g_cfg.mode ? g_cfg.level : 1);
	g_st.start = (long)time(NULL);
	g_st.limit = (!g_cfg.mode && g_cfg.chrono) ? 3 * 3600 : 0;
	g_st.wait = g_cfg.wait;
	g_st.trace = g_cfg.trace;
	g_st.lang = g_cfg.lang;
	g_st.exo = -1;
	sh("rm -rf %s/traces/* 2>/dev/null; true", q(g_work));
	next_exo();
}

static int	generate(const char *soldir, int ac, char **av)
{
	char	src[1300];
	int		i;
	int		k;
	int		bad;

	bad = 0;
	g_st.trace = 0;
	for (i = 0; i < g_nbank; i++)
	{
		for (k = 0; k < ac; k++)
			if (!strcmp(av[k], g_bank[i].name))
				break ;
		if (ac && k == ac)
			continue ;
		snprintf(src, sizeof(src), "%s/%s", soldir, g_bank[i].name);
		if (grade(&g_bank[i], src, 1) != 0)
		{
			printf("ECHEC %s\n", g_bank[i].name);
			bad++;
		}
		else
			printf("ok    %s\n", g_bank[i].name);
	}
	return (bad != 0);
}

static int	check(const char *soldir, int ac, char **av)
{
	char	src[1300];
	int		i;
	int		k;
	int		bad;

	bad = 0;
	g_st.trace = 2;
	for (i = 0; i < g_nbank; i++)
	{
		for (k = 0; k < ac; k++)
			if (!strcmp(av[k], g_bank[i].name))
				break ;
		if (ac && k == ac)
			continue ;
		snprintf(src, sizeof(src), "%s/%s", soldir, g_bank[i].name);
		k = grade(&g_bank[i], src, 0);
		printf("%s %s\n", k ? "ECHEC" : "ok   ", g_bank[i].name);
		bad += k != 0;
	}
	return (bad != 0);
}

typedef struct s_bexo
{
	char	src[8];
	char	name[48];
	int		niv;
	int		ex;
	char	notion[120];
}	t_bexo;

static t_bexo	g_b[260];
static int		g_nb;
static int		g_vue[260];
static int		g_nvue;
static int		g_bcur = -1;
static int		g_blang;

static int	b_done(const t_bexo *b)
{
	char		key[64];
	const char	*p;
	size_t		n;

	snprintf(key, sizeof(key), "%s:%s", b->src, b->name);
	n = strlen(key);
	for (p = g_xs.bdone; (p = strstr(p, key)); p += n)
		if ((p == g_xs.bdone || p[-1] == ',') && (p[n] == ',' || p[n] == '\0'))
			return (1);
	return (!strcmp(b->src, "exam") && done_has(b->name));
}

static void	b_mark(const t_bexo *b)
{
	size_t	n;

	if (b_done(b) && strcmp(b->src, "exam"))
		return ;
	if (!strcmp(b->src, "exam"))
	{
		done_add(b->name);
		return ;
	}
	n = strlen(g_xs.bdone);
	if (n + strlen(b->name) + 12 < sizeof(g_xs.bdone))
		snprintf(g_xs.bdone + n, sizeof(g_xs.bdone) - n, "%s%s:%s", n ? "," : "", b->src, b->name);
}

static void	b_load_ext(const char *src)
{
	DIR				*d;
	struct dirent	*e;
	FILE			*f;
	char			path[1300];
	char			line[400];
	t_bexo			*b;

	snprintf(path, sizeof(path), "%s/banque/%s", g_root, src);
	d = opendir(path);
	while (d && (e = readdir(d)) && g_nb < 260)
	{
		if (e->d_name[0] == '.')
			continue ;
		snprintf(path, sizeof(path), "%s/banque/%s/%s/meta", g_root, src, e->d_name);
		f = fopen(path, "r");
		if (!f)
			continue ;
		b = &g_b[g_nb];
		memset(b, 0, sizeof(*b));
		snprintf(b->src, sizeof(b->src), "%s", src);
		snprintf(b->name, sizeof(b->name), "%s", e->d_name);
		b->ex = -1;
		while (fgets(line, sizeof(line), f))
		{
			line[strcspn(line, "\n")] = '\0';
			if (!strncmp(line, "niveau=", 7))
				b->niv = atoi(line + 7);
			else if (!strncmp(line, "notion=", 7))
				snprintf(b->notion, sizeof(b->notion), "%s", line + 7);
		}
		fclose(f);
		g_nb++;
	}
	if (d)
		closedir(d);
}

static int	b_cmp(const void *a, const void *b)
{
	const t_bexo	*x = a;
	const t_bexo	*y = b;
	int				r;

	r = strcmp(x->src, y->src);
	if (r)
		return (r);
	if (x->niv != y->niv)
		return (x->niv - y->niv);
	return (strcmp(x->name, y->name));
}

static void	b_load(void)
{
	int	i;

	for (i = 0; i < g_nbank && g_nb < 260; i++)
	{
		memset(&g_b[g_nb], 0, sizeof(t_bexo));
		snprintf(g_b[g_nb].src, sizeof(g_b[g_nb].src), "exam");
		snprintf(g_b[g_nb].name, sizeof(g_b[g_nb].name), "%s", g_bank[i].name);
		g_b[g_nb].niv = g_bank[i].level;
		g_b[g_nb].ex = i;
		snprintf(g_b[g_nb].notion, sizeof(g_b[g_nb].notion), "%s", g_bank[i].prog ? "programme" : "fonction");
		g_nb++;
	}
	b_load_ext("libft");
	b_load_ext("printf");
	qsort(g_b, g_nb, sizeof(t_bexo), b_cmp);
	for (i = 0; i < g_nb; i++)
		if (!strcmp(g_b[i].src, "exam"))
			g_b[i].ex = exo_find(g_b[i].name);
}

static const char	*b_srcname(const char *src)
{
	if (!strcmp(src, "exam"))
		return ("Exam");
	if (!strcmp(src, "libft"))
		return ("Libft");
	return ("printf");
}

static void	b_entete(const char *titre)
{
	int	tot;
	int	ok;
	int	i;

	for (tot = 0, ok = 0, i = 0; i < g_nb; i++)
	{
		tot++;
		ok += b_done(&g_b[i]);
	}
	ecran();
	printf("\n  %s═════════════════════ BANQUE D'EXERCICES ══════════════════════%s\n", C_AC, C_R);
	printf("   %s%s%s", C_B, titre, C_R);
	printf("   %s%d/%d réussis%s\n", C_D, ok, tot, C_R);
	printf("  %s════════════════════════════════════════════════════════════════%s\n\n", C_AC, C_R);
}

static void	b_commandes(void)
{
	printf("\n  %sliste%s [exam|libft|printf] [niveau]   %scherche%s MOT   %souvre%s N   %sgrademe%s   %ssujet%s   %sfait%s   %saide%s   %squitter%s\n\n",
		C_B, C_R, C_B, C_R, C_B, C_R, C_B, C_R, C_B, C_R, C_B, C_R, C_B, C_R, C_B, C_R);
}

static void	b_accueil(void)
{
	static const char	*src[3] = {"exam", "libft", "printf"};
	int					s;
	int					lv;
	int					t;
	int					o;
	int					i;

	b_entete("Tous les exercices du testeur, à faire dans l'ordre que tu veux");
	for (s = 0; s < 3; s++)
	{
		for (t = 0, o = 0, i = 0; i < g_nb; i++)
			if (!strcmp(g_b[i].src, src[s]))
			{
				t++;
				o += b_done(&g_b[i]);
			}
		printf("   %s%-8s%s %3d exercices   %s%d réussis%s", C_B, b_srcname(src[s]), C_R, t, o == t && t ? C_OK : C_D, o, C_R);
		if (s == 0)
		{
			printf("   ");
			for (lv = 1; lv <= XLEVELS; lv++)
			{
				for (t = 0, o = 0, i = 0; i < g_nb; i++)
					if (!strcmp(g_b[i].src, "exam") && g_b[i].niv == lv)
					{
						t++;
						o += b_done(&g_b[i]);
					}
				printf(" %sniv.%d%s %d/%d", level_color(lv), lv, C_R, o, t);
			}
		}
		printf("\n");
	}
	printf("\n   %sExemples : « liste exam 2 », « cherche split », « ouvre 3 », puis « grademe ».%s\n", C_D, C_R);
	if (g_bcur >= 0)
		printf("   Exercice ouvert : %s%s%s (%s)\n", C_AC, g_b[g_bcur].name, C_R, b_srcname(g_b[g_bcur].src));
	b_commandes();
}

static void	b_liste(const char *src, int niv, const char *mot)
{
	char	titre[200];
	int		i;

	g_nvue = 0;
	for (i = 0; i < g_nb; i++)
	{
		if (src && strcmp(g_b[i].src, src))
			continue ;
		if (niv > 0 && g_b[i].niv != niv)
			continue ;
		if (mot && !strstr(g_b[i].name, mot) && !strstr(g_b[i].notion, mot))
			continue ;
		g_vue[g_nvue++] = i;
	}
	if (mot)
		snprintf(titre, sizeof(titre), "Recherche « %s » : %d résultat(s)", mot, g_nvue);
	else
		snprintf(titre, sizeof(titre), "%s%s%s", src ? b_srcname(src) : "Tout", niv > 0 ? " · niveau " : "",
			niv > 0 ? (const char *[]){"", "1", "2", "3", "4", "5", "6", "7", "8", "9"}[niv < 10 ? niv : 0] : "");
	b_entete(titre);
	for (i = 0; i < g_nvue; i++)
	{
		t_bexo	*b;

		b = &g_b[g_vue[i]];
		printf("   %s%3d%s  %s%-22s%s %-7s niv. %-2d  %s%s%s\n", C_AC, i + 1, C_R, b_done(b) ? C_OK : "", b->name,
			C_R, b_srcname(b->src), b->niv, C_D, b->notion, C_R);
	}
	if (!g_nvue)
		printf("   rien ne correspond\n");
	b_commandes();
}

static void	b_workdir(const t_bexo *b, char *p, size_t n)
{
	snprintf(p, n, "%s/banque_travail/%s/%s", g_root, b->src, b->name);
}

static void	b_sujet(void)
{
	t_bexo	*b;
	char	path[1300];
	char	*txt;
	char	*s;
	char	*e;

	if (g_bcur < 0)
	{
		printf("  aucun exercice ouvert : « ouvre N » après une liste\n");
		return ;
	}
	b = &g_b[g_bcur];
	b_entete(b->name);
	b_workdir(b, path, sizeof(path));
	printf("   Ton rendu : %s%s/", C_B, rel(path));
	if (!strcmp(b->src, "exam") && b->ex >= 0)
	{
		int	i;

		for (i = 0; i < g_bank[b->ex].nfiles; i++)
			printf("%s%s", i ? ", " : "", g_bank[b->ex].files[i]);
	}
	else
		printf("%s.c", b->name);
	printf("%s\n", C_R);
	printf("   %sÉcris ton code dans un autre terminal, puis tape grademe ici.%s\n\n", C_D, C_R);
	snprintf(path, sizeof(path), "%s/banque/%s/%s/sujet.%s.txt", g_root, b->src, b->name, g_blang ? "en" : "fr");
	txt = slurp(path, NULL);
	for (s = txt; s && *s; s = *e ? e + 1 : e)
	{
		e = strchr(s, '\n');
		if (!e)
			e = s + strlen(s);
		printf("   %.*s\n", (int)(e - s), s);
	}
	free(txt);
	b_commandes();
}

static void	b_ouvre(const char *arg)
{
	char	path[1300];
	int		i;
	int		k;

	k = -1;
	if (arg && arg[0] >= '0' && arg[0] <= '9')
	{
		i = atoi(arg) - 1;
		if (i >= 0 && i < g_nvue)
			k = g_vue[i];
	}
	for (i = 0; arg && k < 0 && i < g_nb; i++)
		if (!strcmp(g_b[i].name, arg))
			k = i;
	if (k < 0)
	{
		printf("  exercice introuvable : %s (fais d'abord « liste » ou « cherche »)\n", arg ? arg : "");
		return ;
	}
	g_bcur = k;
	b_workdir(&g_b[k], path, sizeof(path));
	sh("mkdir -p %s && cp %s/banque/%s/%s/sujet.*.txt %s/ 2>/dev/null; true", q(path), q(g_root), g_b[k].src, g_b[k].name, q(path));
	b_sujet();
}

static int	b_ext_grade(const t_bexo *b)
{
	char	work[1300];
	char	dest[1300];
	char	res[1300];
	char	line[600];
	FILE	*f;
	int		ok;

	b_workdir(b, work, sizeof(work));
	snprintf(dest, sizeof(dest), "%s/modules/%s/entrainement/niveau_%02d", g_root, b->src, b->niv);
	snprintf(res, sizeof(res), "%s/banque_res.txt", g_build);
	snprintf(line, sizeof(line), "%s/%s.c", work, b->name);
	if (access(line, R_OK) != 0)
	{
		printf("  fichier attendu introuvable : %s\n", rel(line));
		return (0);
	}
	sh("mkdir -p %s && cp %s %s/", q(dest), q(line), q(dest));
	if (!strcmp(b->src, "libft") && !getenv("TDF_LIBFT"))
		printf("  %s(pas de libft trouvée à côté du testeur : seules tes fonctions de ce fichier sont utilisables)%s\n", C_D, C_R);
	sh("rm -f %s; TDF_PROJET=%s bash %s/modules/%s/run.sh --exo %02d --out %s >/dev/null 2>&1", q(res),
		q(getenv("TDF_LIBFT") ? getenv("TDF_LIBFT") : g_root), q(g_root), b->src, b->niv, q(res));
	f = fopen(res, "r");
	ok = 0;
	while (f && fgets(line, sizeof(line), f))
	{
		line[strcspn(line, "\n")] = '\0';
		if (!strncmp(line, "X\t", 2))
		{
			static const char	*k[][2] = {{"ko", "résultat faux"}, {"norme", "Norme non respectée"},
				{"build", "ne compile pas"}, {"crash", "plantage ou erreur mémoire"}, {"leak", "fuite mémoire"},
				{"fuite", "fuite mémoire"}, {"time", "aucune réponse : boucle infinie ?"},
				{"ub", "comportement indéfini"}, {"interdit", "fonction interdite"}, {"absent", "fichier absent"}};
			size_t				j;
			size_t				l;

			ok = !strncmp(line + 2, "ok", 2);
			l = strcspn(line + 2, "\t");
			for (j = 0; !ok && j < sizeof(k) / sizeof(k[0]); j++)
				if (strlen(k[j][0]) == l && !strncmp(line + 2, k[j][0], l))
					break ;
			if (!ok)
				printf("   %s%s%s\n", C_KO, j < sizeof(k) / sizeof(k[0]) ? k[j][1] : line + 2, C_R);
		}
		else if (!strncmp(line, "D\t", 2))
			printf("   %s\n", line + 2);
	}
	if (f)
		fclose(f);
	else
		printf("  le testeur %s n'a rien renvoyé\n", b->src);
	return (ok);
}

static void	b_grademe(void)
{
	t_bexo	*b;
	char	src[1300];
	int		ok;
	int		first;

	if (g_bcur < 0)
	{
		printf("  aucun exercice ouvert : « ouvre N » après une liste\n");
		return ;
	}
	b = &g_b[g_bcur];
	b_entete(b->name);
	printf("   Correction de %s%s%s en cours...\n", C_AC, b->name, C_R);
	fflush(stdout);
	if (!strcmp(b->src, "exam"))
	{
		b_workdir(b, src, sizeof(src));
		g_st.trace = 2;
		g_st.exo = b->ex;
		ok = b->ex >= 0 && grade(&g_bank[b->ex], src, 0) == 0;
		if (!ok)
			trace_show(&g_bank[b->ex]);
	}
	else
		ok = b_ext_grade(b);
	if (ok)
	{
		printf("\n  %s╔══════════════════════════════╗%s\n", C_OK, C_R);
		printf("  %s║           SUCCESS            ║%s\n", C_OK, C_R);
		printf("  %s╚══════════════════════════════╝%s\n\n", C_OK, C_R);
		first = !b_done(b);
		b_mark(b);
		if (first)
			reward(10 + 5 * b->niv, "nouvel exercice de la banque");
		save_write();
	}
	else
	{
		printf("\n  %s╔══════════════════════════════╗%s\n", C_KO, C_R);
		printf("  %s║           FAILURE            ║%s\n", C_KO, C_R);
		printf("  %s╚══════════════════════════════╝%s\n", C_KO, C_R);
	}
	b_commandes();
}

static void	b_aide(void)
{
	b_entete("Aide");
	printf("   %sliste%s                 tous les exercices\n", C_B, C_R);
	printf("   %sliste exam 2%s          l'exam, niveau 2 (aussi : liste libft, liste printf)\n", C_B, C_R);
	printf("   %scherche MOT%s           par nom ou par notion (ex. : cherche lst, cherche bits)\n", C_B, C_R);
	printf("   %souvre N%s               ouvre l'exercice n°N de la dernière liste (ou son nom)\n", C_B, C_R);
	printf("   %sgrademe%s               corrige l'exercice ouvert\n", C_B, C_R);
	printf("   %ssujet%s                 relire le sujet (sujet en : en anglais)\n", C_B, C_R);
	printf("   %sfait%s                  ce que tu as déjà réussi\n", C_B, C_R);
	printf("   %squitter%s               revenir\n\n", C_B, C_R);
	printf("   Les exercices Libft et printf sont corrigés par leur propre testeur :\n");
	printf("   Norme obligatoire, sanitizers, échecs de malloc.\n");
	b_commandes();
}

static int	banque(void)
{
	char	line[400];
	char	*cmd;
	char	*a1;
	char	*a2;

	b_load();
	sh("mkdir -p %s/banque_travail %s/traces", q(g_root), q(g_work));
	g_blang = g_cfg.lang;
	b_accueil();
	while (1)
	{
		printf("%sbanque%s> ", C_AC, C_R);
		fflush(stdout);
		if (!fgets(line, sizeof(line), stdin))
			break ;
		cmd = strtok(line, " \t\r\n");
		if (!cmd)
			continue ;
		a1 = strtok(NULL, " \t\r\n");
		a2 = strtok(NULL, " \t\r\n");
		if (!strcmp(cmd, "liste") || !strcmp(cmd, "ls"))
		{
			if (a1 && a1[0] >= '0' && a1[0] <= '9')
				b_liste(NULL, atoi(a1), NULL);
			else
				b_liste(a1, a2 ? atoi(a2) : 0, NULL);
		}
		else if (!strcmp(cmd, "cherche"))
			b_liste(NULL, 0, a1 ? a1 : "");
		else if (!strcmp(cmd, "ouvre") || !strcmp(cmd, "ouvrir"))
			b_ouvre(a1);
		else if (!strcmp(cmd, "grademe"))
			b_grademe();
		else if (!strcmp(cmd, "sujet") || !strcmp(cmd, "subject"))
		{
			if (a1)
				g_blang = !strcmp(a1, "en");
			b_sujet();
		}
		else if (!strcmp(cmd, "fait"))
		{
			int	i;

			g_nvue = 0;
			for (i = 0; i < g_nb; i++)
				if (b_done(&g_b[i]))
					g_vue[g_nvue++] = i;
			b_entete("Déjà réussis");
			for (i = 0; i < g_nvue; i++)
				printf("   %s%3d%s  %s%-22s%s %s\n", C_AC, i + 1, C_R, C_OK, g_b[g_vue[i]].name, C_R, b_srcname(g_b[g_vue[i]].src));
			if (!g_nvue)
				printf("   rien pour l'instant\n");
			b_commandes();
		}
		else if (!strcmp(cmd, "aide") || !strcmp(cmd, "help"))
			b_aide();
		else if (!strcmp(cmd, "accueil") || !strcmp(cmd, "clear"))
			b_accueil();
		else if (!strcmp(cmd, "quitter") || !strcmp(cmd, "exit") || !strcmp(cmd, "quit") || !strcmp(cmd, "q"))
			break ;
		else
			printf("  commande inconnue : %s (aide pour la liste)\n", cmd);
	}
	save_write();
	return (0);
}

int	main(int ac, char **av)
{
	static char	savedir[1000];
	char		line[40];
	int			i;
	int			direct;

	srand((unsigned int)(time(NULL) ^ getpid()));
	for (i = 1; i + 1 < ac; i += 2)
	{
		if (!strcmp(av[i], "--racine"))
			snprintf(g_root, sizeof(g_root), "%s", av[i + 1]);
		else if (!strcmp(av[i], "--home"))
			snprintf(g_home, sizeof(g_home), "%s", av[i + 1]);
		else
			break ;
	}
	if (!g_root[0] || !g_home[0])
		die("usage : exam --racine DOSSIER --home DOSSIER");
	snprintf(g_bankdir, sizeof(g_bankdir), "%s/banque/exam", g_root);
	snprintf(g_work, sizeof(g_work), "%s/exam", g_root);
	snprintf(g_build, sizeof(g_build), "%s/.build", g_work);
	g_color = isatty(1) && !getenv("NO_COLOR");
	g_tty = isatty(0) && isatty(1);
	bank_load();
	sh("mkdir -p %s %s", q(g_work), q(g_build));
	if (i < ac && !strcmp(av[i], "--generer") && i + 1 < ac)
		return (generate(av[i + 1], ac - i - 2, av + i + 2));
	if (i < ac && !strcmp(av[i], "--verifier") && i + 1 < ac)
		return (check(av[i + 1], ac - i - 2, av + i + 2));
	if (i < ac && !strcmp(av[i], "--liste"))
	{
		for (i = 0; i < g_nbank; i++)
			printf("%d %s %s\n", g_bank[i].level, g_bank[i].prog ? "programme" : "fonction ", g_bank[i].name);
		return (0);
	}
	snprintf(savedir, sizeof(savedir), "%s", g_home);
	g_savedir = savedir;
	save_load();
	if (g_save.valid == -1)
		printf("  %ssauvegarde modifiée à la main : remise à zéro (ancienne copie : save.txt.triche)%s\n", C_KO, C_R);
	cfg_load();
	if (i < ac && !strcmp(av[i], "--banque"))
		return (banque());
	direct = 0;
	for (; i + 1 < ac; i += 2)
	{
		if (!strcmp(av[i], "--serie") && atoi(av[i + 1]) >= 1 && atoi(av[i + 1]) <= 5)
			g_cfg.serie = atoi(av[i + 1]);
		else if (!strcmp(av[i], "--difficulte") && !strcmp(av[i + 1], "croissante"))
			g_cfg.diff = 0;
		else if (!strcmp(av[i], "--difficulte") && atoi(av[i + 1]) >= 1 && atoi(av[i + 1]) <= XLEVELS)
			g_cfg.diff = atoi(av[i + 1]);
		else
			die("options : --serie 1..5   --difficulte croissante|1..4");
		g_cfg.mode = 2;
		g_cfg.real = 0;
		direct = 1;
	}
	if (i < ac)
		die("options : --serie 1..5   --difficulte croissante|1..4");
	if (direct)
		cfg_save();
	if (state_load())
	{
		printf("\n  Une session est en cours (%s, niveau %d). La reprendre ? (o/n) ", g_bank[g_st.exo].name, g_st.level);
		fflush(stdout);
		if (fgets(line, sizeof(line), stdin) && (line[0] == 'n' || line[0] == 'N'))
			g_st.active = 0;
		else
			view_exo();
	}
	if (!g_st.active)
	{
		if (!direct && !settings())
			return (0);
		start();
	}
	shell();
	return (0);
}
