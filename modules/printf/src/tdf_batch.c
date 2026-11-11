#include "tdf.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#if TDF_VALGRIND
# include <valgrind/valgrind.h>
# include <valgrind/memcheck.h>
#endif

static t_buf	g_mfmsg;
static int		g_mf_ko;
static int		g_mf_warn;

void	tdf_mf_begin(void)
{
	buf_reset(&g_mfmsg);
	g_mf_ko = 0;
	g_mf_warn = 0;
}

void	tdf_mf_step(int k)
{
	t_buf	m;
	int		ko;
	int		warn;
	int		crash;

	memset(&m, 0, sizeof(m));
	ko = 0;
	warn = 0;
	g_shm->nover = 0;
	crash = tdf_explain_crash(&m);
	tdf_memreport(&m, &ko, &warn);
	if (crash)
		ko = 1;
	else if (!g_run.returned)
		ko = 1;
	else if (g_run.ret == -1)
		;
	else if (g_run.ret == g_exp_ret && g_run.out.len == g_exp.len
		&& !memcmp(g_run.out.p, g_exp.p, g_exp.len))
		;
	else if (g_run.ret >= 0 && (size_t)g_run.ret == g_run.out.len)
	{
		ko = 1;
		tdf_bprintf(&m, "          retour %d (= octets écrits) au lieu de " "\033[1;31m" "-1"
			"\033[0m" " : l'échec de malloc est ignoré, sortie tronquée en silence\n",
			g_run.ret);
	}
	else
	{
		ko = 1;
		tdf_bprintf(&m, "          retour %d " "\033[1;31m" "incohérent" "\033[0m"
			" : %zu octet(s) réellement écrit(s), -1 attendu\n",
			g_run.ret, g_run.out.len);
	}
	if ((ko && !g_mf_ko) || (warn && !g_mf_ko && !g_mf_warn))
	{
		buf_reset(&g_mfmsg);
		tdf_bprintf(&g_mfmsg, "          le malloc n°%d renvoie NULL :\n", k);
		if (m.len)
			buf_add(&g_mfmsg, m.p, m.len);
	}
	g_mf_ko |= ko;
	g_mf_warn |= warn;
	free(m.p);
}

void	tdf_mf_end(int nalloc)
{
	(void)nalloc;
	tdf_record_cur(g_mf_ko, g_mf_warn && !g_mf_ko, &g_mfmsg);
}

void	tdf_bc_desc(t_bc *c, t_buf *dst)
{
	int	i;

	buf_add(dst, "ft_printf(", 10);
	tdf_escape(dst, c->fmt, strlen(c->fmt), 120);
	i = -1;
	while (++i < c->n)
	{
		if (c->k[i] == K_INT)
			tdf_bprintf(dst, ", %d", c->v[i].i);
		else if (c->k[i] == K_UNS)
			tdf_bprintf(dst, ", %uu", c->v[i].u);
		else if (c->k[i] == K_PTR)
			tdf_bprintf(dst, ", (void *)%#lx", (unsigned long)c->v[i].p);
		else if (!c->v[i].s)
			tdf_bprintf(dst, ", NULL");
		else
		{
			buf_add(dst, ", ", 2);
			tdf_escape(dst, c->v[i].s, strlen(c->v[i].s), 40);
		}
	}
	buf_add(dst, ")", 1);
}

static double	now(void)
{
	struct timespec	ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec + ts.tv_nsec / 1e9);
}

static int			g_outfd = -1;
static const char	*g_label = "lot";
static int			g_total;

static void	child_loop(t_bc *c, int n, int start, const char *arena)
{
	int				i;
	int				r;
	off_t			len;
	static char		tmp[1 << 16];
	ssize_t			rd;
	struct rlimit	rl;
#if TDF_VALGRIND
	unsigned		errs;
#endif

	tdf_in_child();
	rl.rlim_cur = 64 << 20;
	rl.rlim_max = 64 << 20;
	setrlimit(RLIMIT_FSIZE, &rl);
	dup2(tdf_errfd(), 2);
	dup2(g_outfd, 1);
	tdf_track_reset();
	tdf_install_segv();
	i = start - 1;
	while (++i < n)
	{
		g_shm->cur = i;
		tdf_shm_clear_mem();
		if (ftruncate(1, 0) < 0)
			_exit(3);
		lseek(1, 0, SEEK_SET);
#if TDF_VALGRIND
		errs = VALGRIND_COUNT_ERRORS;
#endif
		tdf_track_arm();
		r = tdf_dispatch(ft_printf, c[i].fmt, c[i].n, c[i].k, c[i].v);
		tdf_track_disarm();
		g_shm->ret = r;
		g_shm->returned = 1;
#if TDF_VALGRIND
		if (VALGRIND_COUNT_ERRORS != errs)
			exit(0);
#endif
		if (tdf_shm_mem_problem())
			exit(0);
		len = lseek(1, 0, SEEK_END);
		rd = pread(1, tmp, sizeof(tmp), 0);
		if (r != c[i].eret || len != (off_t)c[i].elen || rd < 0
			|| memcmp(tmp, arena + c[i].eoff,
				c[i].elen < sizeof(tmp) ? c[i].elen : sizeof(tmp)))
		{
			g_shm->fail_idx[g_shm->nfail] = i;
			g_shm->fail_ret[g_shm->nfail] = r;
			g_shm->fail_len[g_shm->nfail] = (int)len;
			memcpy(g_shm->fail_out[g_shm->nfail], tmp,
				len < 159 ? (size_t)len : 159);
			if (++g_shm->nfail >= 64)
				exit(0);
		}
	}
	g_shm->done = 1;
	exit(0);
}

static void	report_fail(t_bc *c, int i, const char *arena, int *st,
		const char *file, int line, int ret, int len, const char *out)
{
	t_buf	msg;
	t_buf	desc;
	int		ko;

	memset(&msg, 0, sizeof(msg));
	memset(&desc, 0, sizeof(desc));
	tdf_bc_desc(&c[i], &desc);
	tdf_bprintf(&msg, "          attendu : ");
	tdf_escape(&msg, arena + c[i].eoff, c[i].elen, 120);
	tdf_bprintf(&msg, "  ret=%d\n          obtenu  : ", c[i].eret);
	tdf_escape(&msg, out, len < 159 ? (size_t)(len < 0 ? 0 : len) : 159, 120);
	if (len > 159)
		tdf_bprintf(&msg, "\033[2m…(%d octets au total)\033[0m", len);
	tdf_bprintf(&msg, "  ret=%d\n", ret);
	if (c[i].sev == SEV_UB)
		tdf_bprintf(&msg, "          \033[2m(comportement indéfini : simple avertissement)\033[0m\n");
	ko = c[i].sev != SEV_UB;
	st[i] = ko ? 1 : 2;
	(void)file;
	(void)line;
	tdf_record_at(ko, !ko, g_label, i, desc.p, &msg);
	free(msg.p);
	free(desc.p);
}

static void	report_abnormal(t_bc *c, int i, int *st, const char *file, int line)
{
	t_buf	msg;
	t_buf	desc;
	int		ko;
	int		warn;

	memset(&msg, 0, sizeof(msg));
	memset(&desc, 0, sizeof(desc));
	ko = 0;
	warn = 0;
	tdf_bc_desc(&c[i], &desc);
	if (tdf_explain_crash(&msg))
		ko = 1;
	g_shm->nover = 0;
	tdf_memreport(&msg, &ko, &warn);
	if (!ko && !warn)
	{
		ko = 1;
		tdf_bprintf(&msg, "          erreur mémoire détectée (voir valgrind)\n");
	}
	if (c[i].sev == SEV_UB && ko)
	{
		ko = 0;
		warn = 1;
		tdf_bprintf(&msg, "          \033[2m(comportement indéfini : simple avertissement)\033[0m\n");
	}
	st[i] = ko ? 1 : 2;
	(void)file;
	(void)line;
	tdf_record_at(ko, warn, g_label, i, desc.p, &msg);
	free(msg.p);
	free(desc.p);
}

static void	compute_expected(t_bc *c, int n, t_buf *arena)
{
	int	i;
	int	r;

	i = -1;
	while (++i < n)
	{
		if ((i & 255) == 0)
			tdf_quit_check(0);
		tdf_exp_start();
		r = tdf_dispatch(printf, c[i].fmt, c[i].n, c[i].k, c[i].v);
		tdf_exp_end(r);
		c[i].eoff = arena->len;
		c[i].elen = g_exp.len;
		c[i].eret = r;
		buf_add(arena, g_exp.p, g_exp.len);
	}
}

static int	wait_child(pid_t pid, int *timed_out)
{
	int		status;
	int		last;
	double	deadline;

	last = -1;
	deadline = now() + g_opt.timeout;
	*timed_out = 0;
	while (waitpid(pid, &status, WNOHANG) == 0)
	{
		tdf_quit_check(pid);
		tdf_progress(g_label, g_shm->cur, g_total, 0);
		if (g_shm->cur != last)
		{
			last = g_shm->cur;
			deadline = now() + g_opt.timeout;
		}
		else if (now() > deadline)
		{
			kill(pid, SIGKILL);
			waitpid(pid, &status, 0);
			*timed_out = 1;
			break ;
		}
		usleep(300);
	}
	return (status);
}

void	tdf_batch(t_bc *c, int n, const char *file, int line, const char *label)
{
	t_buf	arena;
	int		*st;
	int		start;
	int		j;
	pid_t	pid;
	FILE	*f;
	int		crashes;
	int		timeouts;

	g_label = label;
	g_total = n;
	if (n <= 0)
		return ;
	memset(&arena, 0, sizeof(arena));
	buf_add(&arena, "", 0);
	compute_expected(c, n, &arena);
	st = calloc((size_t)n, sizeof(int));
	if (g_outfd < 0)
	{
		f = tmpfile();
		g_outfd = dup(fileno(f));
		fclose(f);
	}
	start = 0;
	crashes = 0;
	timeouts = 0;
	while (start < n)
	{
		if (crashes >= 25 || timeouts >= 3)
		{
			t_buf	msg;

			memset(&msg, 0, sizeof(msg));
			tdf_bprintf(&msg, "          lot interrompu après %d plantages / erreurs mémoire "
				"dont %d timeout(s) : %d cas non exécutés (corrige d'abord les KO ci-dessus)\n",
				crashes, timeouts, n - start);
			tdf_record_at(1, 0, file, line, "arrêt du lot", &msg);
			free(msg.p);
			break ;
		}
		g_shm->nfail = 0;
		g_shm->done = 0;
		g_shm->cur = start;
		if (ftruncate(tdf_errfd(), 0) < 0)
			perror("ftruncate");
		lseek(tdf_errfd(), 0, SEEK_SET);
		fflush(NULL);
		pid = fork();
		if (pid == 0)
			child_loop(c, n, start, arena.p);
		g_run.status = wait_child(pid, &g_run.timed_out);
		tdf_collect_batch(pid);
		g_run.returned = g_shm->returned;
		g_run.ret = g_shm->ret;
		j = -1;
		while (++j < g_shm->nfail)
			report_fail(c, g_shm->fail_idx[j], arena.p, st, file, line,
				g_shm->fail_ret[j], g_shm->fail_len[j], g_shm->fail_out[j]);
		if (g_shm->done && WIFEXITED(g_run.status) && WEXITSTATUS(g_run.status) == 0
			&& !g_run.timed_out)
			break ;
		if (g_shm->done)
		{
			t_buf	msg;

			memset(&msg, 0, sizeof(msg));
			tdf_explain_crash(&msg);
			tdf_record_at(1, 0, file, line, "fin du lot (fuite détectée à la sortie)", &msg);
			free(msg.p);
			break ;
		}
		if (st[g_shm->cur] == 0)
		{
			report_abnormal(c, g_shm->cur, st, file, line);
			crashes++;
			timeouts += g_run.timed_out;
		}
		start = g_shm->cur + 1;
	}
	tdf_progress(g_label, n, n, 1);
	if (crashes < 25 && timeouts < 3)
		start = n;
	j = -1;
	while (++j < start && j < n)
		if (st[j] == 0)
			tdf_ok();
	free(st);
	free(arena.p);
}
