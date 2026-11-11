#include "tdf.h"
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#if defined(__linux__)
# include <ucontext.h>
#endif
#if TDF_VALGRIND
# include <valgrind/memcheck.h>
#endif

#if TDF_TRACK

void	*__real_malloc(size_t n);
void	__real_free(void *p);
void	*__real_calloc(size_t a, size_t b);
void	*__real_realloc(void *p, size_t n);

#define MAXB 16384
#define GUARD 32
#define FILL 0xA5
#define GUARDB 0xC3
#define KEY ((uintptr_t)0xA5A5000000000000ULL ^ (uintptr_t)0x5A5A5A5A5A5AULL)

typedef struct s_blk
{
	uintptr_t	sp;
	size_t		size;
	uintptr_t	site;
	int			state;
	uintptr_t	map;
	size_t		maplen;
}	t_blk;

static t_blk	g_b[MAXB];
static int		g_nb;
static int		g_armed;
static int		g_count;
int				g_fail_at;
uintptr_t		g_exe_base;

static uintptr_t	scr(void *p)
{
	return ((uintptr_t)p ^ KEY);
}

static void	*unscr(uintptr_t s)
{
	return ((void *)(s ^ KEY));
}

static uintptr_t	site_off(void *ra)
{
	return ((uintptr_t)ra - g_exe_base);
}

void	tdf_track_reset(void)
{
	Dl_info	di;

	if (!g_exe_base && dladdr((void *)&tdf_track_reset, &di))
		g_exe_base = (uintptr_t)di.dli_fbase;
	g_nb = 0;
	g_count = 0;
	g_armed = 0;
}

static t_blk	*find(void *p)
{
	int			i;
	uintptr_t	s;

	s = scr(p);
	i = g_nb;
	while (--i >= 0)
		if (g_b[i].sp == s)
			return (&g_b[i]);
	return (NULL);
}

static size_t	used_size(unsigned char *p, size_t size)
{
	while (size > 0 && p[size - 1] == FILL)
		size--;
	return (size);
}

static void	add_site(t_site *arr, int *n, t_blk *b, size_t used)
{
	if (*n < TDF_MAX_REPORT)
	{
		arr[*n].off = b->site;
		arr[*n].size = b->size;
		arr[*n].used = used;
	}
	(*n)++;
}

#if TDF_GUARD
static size_t	round16(size_t n)
{
	return ((n + 15) & ~(size_t)15);
}
#endif

static void	check_guard(t_blk *b)
{
#if TDF_GUARD
	unsigned char	*p;
	size_t			i;
	size_t			slack;

	p = (unsigned char *)unscr(b->sp) + b->size;
	slack = round16(b->size ? b->size : 1) - b->size;
	i = 0;
	while (i < slack && p[i] == GUARDB)
		i++;
	if (i < slack && !g_shm->novf)
	{
		g_shm->novf = 1;
		g_shm->ovf.off = b->site;
		g_shm->ovf.size = b->size;
	}
#else
	(void)b;
#endif
}

#if TDF_GUARD

static void	*guard_alloc(size_t n, uintptr_t *map, size_t *maplen)
{
	static long		psz;
	size_t			total;
	size_t			pages;
	unsigned char	*base;
	unsigned char	*p;

	if (!psz)
		psz = sysconf(_SC_PAGESIZE);
	total = round16(n ? n : 1);
	pages = (total + psz - 1) / psz + 1;
	base = mmap(NULL, pages * psz, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (base == MAP_FAILED)
		return (NULL);
	mprotect(base + (pages - 1) * psz, psz, PROT_NONE);
	p = base + (pages - 1) * psz - total;
	memset(p + n, GUARDB, total - n);
	*map = scr(base);
	*maplen = pages * psz;
	return (p);
}
#endif

static void	*track_alloc(size_t n, void *ra)
{
	unsigned char	*p;
	uintptr_t		map;
	size_t			maplen;

	g_count++;
	if (g_shm)
		g_shm->nalloc = g_count;
	if (g_fail_at && g_count == g_fail_at)
		return (NULL);
	map = 0;
	maplen = 0;
#if TDF_GUARD
	p = guard_alloc(n, &map, &maplen);
#else
	p = __real_malloc(n);
#endif
	if (!p)
		return (NULL);
	memset(p, FILL, n);
#if TDF_VALGRIND
	VALGRIND_MAKE_MEM_UNDEFINED(p, n);
#endif
	if (g_nb < MAXB)
		g_b[g_nb++] = (t_blk){scr(p), n, site_off(ra), 1, map, maplen};
	return (p);
}

static void	real_release(t_blk *b)
{
#if TDF_GUARD
	munmap(unscr(b->map), b->maplen);
#else
	__real_free(unscr(b->sp));
#endif
}

static void	track_free(void *p, void *ra)
{
	t_blk	*b;
	size_t	used;

	b = find(p);
	if (!b)
	{
		if (!g_shm->nbadfree)
			g_shm->badfree.off = site_off(ra);
		g_shm->nbadfree++;
		__real_free(p);
		return ;
	}
	if (b->state == 2)
	{
		if (!g_shm->ndfree)
			g_shm->dfree.off = site_off(ra);
		g_shm->ndfree++;
		if (!TDF_GUARD)
			__real_free(p);
		return ;
	}
	check_guard(b);
#if TDF_VALGRIND
	VALGRIND_DISABLE_ERROR_REPORTING;
#endif
	used = used_size(p, b->size);
#if TDF_VALGRIND
	VALGRIND_ENABLE_ERROR_REPORTING;
#endif
	if (used < b->size)
		add_site(g_shm->over, &g_shm->nover, b, used);
	b->state = 2;
#if TDF_GUARD
	mprotect(unscr(b->map), b->maplen, PROT_NONE);
#else
	__real_free(p);
#endif
}

void	*__wrap_malloc(size_t n)
{
	if (!g_armed)
		return (__real_malloc(n));
	return (track_alloc(n, __builtin_return_address(0)));
}

void	__wrap_free(void *p)
{
	if (!g_armed || !p)
	{
		__real_free(p);
		return ;
	}
	track_free(p, __builtin_return_address(0));
}

void	*__wrap_calloc(size_t a, size_t b)
{
	void	*p;

	if (!g_armed)
		return (__real_calloc(a, b));
	g_shm->nforbid++;
	if (b && a > (size_t)-1 / b)
		return (NULL);
	p = track_alloc(a * b, __builtin_return_address(0));
	if (p)
		memset(p, 0, a * b);
	return (p);
}

void	*__wrap_realloc(void *old, size_t n)
{
	void	*p;
	t_blk	*b;

	if (!g_armed)
		return (__real_realloc(old, n));
	g_shm->nforbid++;
	p = track_alloc(n, __builtin_return_address(0));
	if (p && old)
	{
		b = find(old);
		if (b && b->state == 1)
			memcpy(p, old, b->size < n ? b->size : n);
	}
	if (p && old)
		track_free(old, __builtin_return_address(0));
	return (p);
}

#if TDF_GUARD
static int	in_exe(uintptr_t pc)
{
	Dl_info	di;

	return (pc && dladdr((void *)pc, &di) && (uintptr_t)di.dli_fbase == g_exe_base);
}

static void	on_segv(int sig, siginfo_t *si, void *uc)
{
	uintptr_t	pc;
	uintptr_t	sp;
	uintptr_t	*fp;
	uintptr_t	a;
	int			i;
	int			k;

	pc = 0;
	sp = 0;
	fp = NULL;
# if defined(__linux__) && defined(__x86_64__)
	pc = (uintptr_t)((ucontext_t *)uc)->uc_mcontext.gregs[REG_RIP];
	sp = (uintptr_t)((ucontext_t *)uc)->uc_mcontext.gregs[REG_RSP];
	fp = (uintptr_t *)((ucontext_t *)uc)->uc_mcontext.gregs[REG_RBP];
# elif defined(__linux__) && defined(__aarch64__)
	pc = (uintptr_t)((ucontext_t *)uc)->uc_mcontext.pc;
	fp = (uintptr_t *)((ucontext_t *)uc)->uc_mcontext.regs[29];
# else
	(void)uc;
# endif
	if (!in_exe(pc) && sp && in_exe(*(uintptr_t *)sp))
		pc = *(uintptr_t *)sp;
	k = 0;
	while (!in_exe(pc) && fp && k++ < 16)
	{
		pc = fp[1];
		fp = (uintptr_t *)fp[0];
	}
	a = (uintptr_t)si->si_addr;
	g_shm->crashed = sig;
	g_shm->crash_off = in_exe(pc) ? pc - g_exe_base + 1 : 0;
	g_shm->crash_addr = a;
	g_shm->crash_kind = a < 4096 ? 1 : 4;
	i = -1;
	while (++i < g_nb)
	{
		if (a >= (uintptr_t)unscr(g_b[i].map)
			&& a < (uintptr_t)unscr(g_b[i].map) + g_b[i].maplen)
		{
			g_shm->crash_kind = g_b[i].state == 2 ? 2 : 3;
			g_shm->crash_blk = g_b[i].size;
			g_shm->ovf.off = g_b[i].site;
		}
	}
	signal(sig, SIG_DFL);
}
#endif

void	tdf_install_segv(void)
{
#if TDF_GUARD
	struct sigaction	sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = on_segv;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);
#endif
}

void	tdf_track_arm(void)
{
	g_nb = 0;
	g_count = 0;
	g_armed = 1;
}

void	tdf_track_disarm(void)
{
	int				i;
	unsigned char	*p;

	g_armed = 0;
	i = -1;
	while (++i < g_nb)
	{
		p = unscr(g_b[i].sp);
		if (g_b[i].state == 1)
		{
			check_guard(&g_b[i]);
#if TDF_VALGRIND
			VALGRIND_DISABLE_ERROR_REPORTING;
#endif
			add_site(g_shm->leak, &g_shm->nleak, &g_b[i], used_size(p, g_b[i].size));
#if TDF_VALGRIND
			VALGRIND_ENABLE_ERROR_REPORTING;
#endif
			g_shm->leak_bytes += g_b[i].size;
		}
		else if (TDF_GUARD)
			real_release(&g_b[i]);
	}
	g_nb = 0;
}

#else

int			g_fail_at;
uintptr_t	g_exe_base;

void	tdf_track_reset(void)
{
}

#if TDF_GUARD
static int	in_exe(uintptr_t pc)
{
	Dl_info	di;

	return (pc && dladdr((void *)pc, &di) && (uintptr_t)di.dli_fbase == g_exe_base);
}

static void	on_segv(int sig, siginfo_t *si, void *uc)
{
	uintptr_t	pc;
	uintptr_t	sp;
	uintptr_t	*fp;
	uintptr_t	a;
	int			i;
	int			k;

	pc = 0;
	sp = 0;
	fp = NULL;
# if defined(__linux__) && defined(__x86_64__)
	pc = (uintptr_t)((ucontext_t *)uc)->uc_mcontext.gregs[REG_RIP];
	sp = (uintptr_t)((ucontext_t *)uc)->uc_mcontext.gregs[REG_RSP];
	fp = (uintptr_t *)((ucontext_t *)uc)->uc_mcontext.gregs[REG_RBP];
# elif defined(__linux__) && defined(__aarch64__)
	pc = (uintptr_t)((ucontext_t *)uc)->uc_mcontext.pc;
	fp = (uintptr_t *)((ucontext_t *)uc)->uc_mcontext.regs[29];
# else
	(void)uc;
# endif
	if (!in_exe(pc) && sp && in_exe(*(uintptr_t *)sp))
		pc = *(uintptr_t *)sp;
	k = 0;
	while (!in_exe(pc) && fp && k++ < 16)
	{
		pc = fp[1];
		fp = (uintptr_t *)fp[0];
	}
	a = (uintptr_t)si->si_addr;
	g_shm->crashed = sig;
	g_shm->crash_off = in_exe(pc) ? pc - g_exe_base + 1 : 0;
	g_shm->crash_addr = a;
	g_shm->crash_kind = a < 4096 ? 1 : 4;
	i = -1;
	while (++i < g_nb)
	{
		if (a >= (uintptr_t)unscr(g_b[i].map)
			&& a < (uintptr_t)unscr(g_b[i].map) + g_b[i].maplen)
		{
			g_shm->crash_kind = g_b[i].state == 2 ? 2 : 3;
			g_shm->crash_blk = g_b[i].size;
			g_shm->ovf.off = g_b[i].site;
		}
	}
	signal(sig, SIG_DFL);
}
#endif

void	tdf_install_segv(void)
{
#if TDF_GUARD
	struct sigaction	sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = on_segv;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);
#endif
}

void	tdf_track_arm(void)
{
}

void	tdf_track_disarm(void)
{
}

void	tdf_install_segv(void)
{
}

#endif

void	tdf_shm_clear_mem(void)
{
	size_t	off;

	off = offsetof(t_shm, nleak);
	memset((char *)g_shm + off, 0, offsetof(t_shm, cur) - off);
	g_shm->nalloc = 0;
	g_shm->returned = 0;
	g_shm->ret = 0;
}

int	tdf_shm_mem_problem(void)
{
	return (g_shm->nleak || g_shm->novf || g_shm->ndfree || g_shm->nbadfree
		|| g_shm->nuaf);
}
