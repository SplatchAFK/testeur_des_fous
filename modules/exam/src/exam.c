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
}	t_xsave;

static t_xexo	g_bank[XMAXEXO];
static int		g_nbank;
static t_xcfg	g_cfg = {0, 1, 0, 2, 0, 0, 1};
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
}

void	game_text(char *b, size_t n)
{
	snprintf(b, n, "exam.runs=%d\nexam.best=%d\nexam.passed=%d\nexam.besttime=%ld\nexam.cheats=%d\n"
		"exam.lastreward=%ld\nexam.done=%s\nexam.code=%s\n", g_xs.runs, g_xs.best, g_xs.passed, g_xs.besttime,
		g_xs.cheats, g_xs.lastreward, g_xs.done, g_xs.code);
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
		if (!strcmp(k, "mode"))
			g_cfg.mode = v != 0;
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
	fprintf(f, "mode=%d\nchrono=%d\nattente=%d\ntrace=%d\nlangue=%d\nreel=%d\nniveau=%d\n", g_cfg.mode,
		g_cfg.chrono, g_cfg.wait, g_cfg.trace, g_cfg.lang, g_cfg.real, g_cfg.level);
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
	if (g_st.mode == 1)
	{
		if (!g_st.cheats)
			reward(first ? 15 * x->level : 2, first ? "nouvel exercice réussi" : "exercice déjà réussi");
		tdf_stats_begin("exam");
		tdf_stats_end(1, 1);
		save_write();
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

static int	settings(void)
{
	static const char	*tr[3] = {"jamais", "à la fin de la session", "à chaque échec"};
	char				line[80];
	int					c;

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
		printf("  %s1%s  Mode                    %s\n", C_AC, C_R,
			onoff(g_cfg.mode, "entraînement sur un niveau", "examen (4 niveaux, un exercice par niveau)"));
		if (g_cfg.mode)
			printf("  %s2%s  Niveau                  %s%d%s\n", C_AC, C_R, level_color(g_cfg.level), g_cfg.level, C_R);
		else
			printf("  %s2%s  Chronomètre             %s\n", C_AC, C_R, onoff(g_cfg.chrono, "3 heures", "libre"));
		printf("  %s3%s  Attente après un échec  %s\n", C_AC, C_R,
			onoff(g_cfg.wait, "progressive (1 min de plus à chaque échec, 10 au plus)", "aucune"));
		printf("  %s4%s  Trace                   %s\n", C_AC, C_R, tr[g_cfg.trace]);
		printf("  %s5%s  Langue du sujet         %s\n", C_AC, C_R, onoff(g_cfg.lang, "anglais", "français"));
		printf("  %s6%s  Conditions réelles      %s\n\n", C_AC, C_R,
			onoff(g_cfg.real, "oui : examen, 3 heures, attente, trace à la fin, aucun cheat", "non"));
		printf("  %sEntrée%s commencer   %s1 à 6%s changer un réglage   %sq%s quitter\n\n  > ", C_B, C_R, C_B, C_R, C_B, C_R);
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
			g_cfg.mode = !g_cfg.mode;
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
	g_st.level = g_cfg.mode ? g_cfg.level : 1;
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

int	main(int ac, char **av)
{
	static char	savedir[1000];
	char		line[40];
	int			i;

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
		if (!settings())
			return (0);
		start();
	}
	shell();
	return (0);
}
