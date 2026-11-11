/* Le testeur des fous · version printf - en-tête commun */
#ifndef TDF_H
# define TDF_H

# define _GNU_SOURCE
# include <signal.h>
# include <stddef.h>
# include <stdint.h>
# include <stdio.h>
# include <sys/types.h>

int		ft_printf(const char *format, ...);

/* ---------- modes (fixés à la compilation par run.sh) ---------- */
# ifndef TDF_MODE
#  define TDF_MODE "asan+ubsan"
# endif
# ifndef TDF_GUARD          /* garde anti-débordement maison (mode sans sanitizer) */
#  define TDF_GUARD 0
# endif
# ifndef TDF_VALGRIND
#  define TDF_VALGRIND 0
# endif
# ifndef TDF_TRACK          /* -Wl,--wrap disponible (Linux) */
#  define TDF_TRACK 1
# endif

/* ---------- traceur d'allocations (tdf_alloc.c) ---------- */
# define TDF_MAX_REPORT 8

typedef struct s_site
{
	uintptr_t	off;      /* adresse de retour - base de l'exécutable */
	size_t		size;
	size_t		used;
}	t_site;

typedef struct s_shm
{
	/* résultat d'un appel */
	volatile int	returned;
	volatile int	ret;
	volatile int	nalloc;
	/* problèmes mémoire relevés par le traceur pendant/après l'appel */
	int		nleak;
	size_t	leak_bytes;
	t_site	leak[TDF_MAX_REPORT];
	int		nover;               /* allocations trop grandes (avertissement) */
	t_site	over[TDF_MAX_REPORT];
	int		novf;                /* débordement détecté par la garde */
	t_site	ovf;
	int		ndfree;              /* double free */
	t_site	dfree;
	int		nbadfree;            /* free d'un pointeur jamais alloué */
	t_site	badfree;
	int		nuaf;                /* écriture après free (quarantaine) */
	t_site	uaf;
	int		nforbid;             /* calloc/realloc appelés */
	/* plantage intercepté (mode sans sanitizer) */
	int			crashed;
	uintptr_t	crash_off;           /* pc fautif (relatif à l'exécutable) */
	uintptr_t	crash_addr;
	int			crash_kind;          /* 1 NULL, 2 use-after-free, 3 débordement tas, 4 autre */
	size_t		crash_blk;
	/* moteur par lots */
	volatile int	cur;
	volatile int	done;
	int		nfail;
	int		fail_idx[64];
	int		fail_ret[64];
	int		fail_len[64];
	char	fail_out[64][160];
	int		vg_err_idx[64];
	int		nvg_err;
}	t_shm;

extern t_shm	*g_shm;
extern int		g_fail_at;      /* n-ième malloc qui renvoie NULL (0 = jamais) */

void	tdf_track_reset(void);
void	tdf_install_segv(void);
void	tdf_track_arm(void);
void	tdf_track_disarm(void);   /* vérifie fuites/gardes et remplit g_shm */
void	tdf_shm_clear_mem(void);
int		tdf_shm_mem_problem(void);

/* ---------- moteur (tdf_harness.c) ---------- */
typedef struct s_buf
{
	char	*p;
	size_t	len;
	size_t	cap;
}	t_buf;

typedef struct s_opts
{
	const char	*only;
	unsigned	seed;
	int			seed_set;
	int			stress_n;
	double		timeout;
	int			bonus;          /* 0 auto, 1 forcé, -1 désactivé */
	int			verbose;
	const char	*project;
	const char	*srclist;
	const char	*vglogdir;
	const char	*exe;
	int			help;           /* 0 cas seul, 1 + ligne, 2 + conseil */
	const char	*machine;       /* fichier de résultats lisible par le jeu */
}	t_opts;

extern t_opts	g_opt;
extern int		g_out;          /* fd d'affichage (stdout d'origine) */

/* catégories */
void	tdf_category(const char *key, const char *title);
int		tdf_selected(const char *key);
void	tdf_category_end(void);
int		tdf_summary(void);

/* sévérité d'un test */
# define SEV_STRICT 0          /* différence = KO */
# define SEV_UB 1              /* différence = avertissement (comportement indéfini) */
# define SEV_LONE 2            /* %% final : crash/timeout = KO (lecture hors format), différence = avert. */

/* déroulé d'un test unitaire (macros dans tdf_tests.h) */
int		tdf_begin(const char *file, int line, const char *fmt, const char *args,
			int sev);
void	tdf_init(void);
void	tdf_exp_start(void);
void	tdf_exp_end(int ret);
int		tdf_child_start(void);
void	tdf_child_end(int ret);
void	tdf_finish(void);
const char	*tdf_edge(const char *fmt);      /* copie le format au bord d'une page protégée */

/* résultats spéciaux */
void	tdf_result(int ko, int warn, const char *file, int line, const char *desc,
			const char *fmt, ...);
void	tdf_ok(void);

/* accès au dernier résultat (tests spéciaux) */
typedef struct s_run
{
	t_buf	out;
	t_buf	err;
	int		status;
	int		timed_out;
	pid_t	pid;
	int		ret;
	int		returned;
}	t_run;

extern t_run	g_run;
extern t_buf	g_exp;
extern int		g_exp_ret;
extern int		g_child_redirect;   /* 0 pipe, 1 fd fermé, 2 /dev/full */
extern int		g_cur_sev;

void	buf_add(t_buf *b, const void *p, size_t n);
void	buf_reset(t_buf *b);
void	tdf_escape(t_buf *dst, const char *s, size_t n, size_t max);
int		tdf_explain_crash(t_buf *msg);
void	tdf_record_cur(int ko, int warn, t_buf *msg);
void	tdf_record_at(int ko, int warn, const char *file, int line,
			const char *desc, t_buf *msg);
void	tdf_bprintf(t_buf *b, const char *fmt, ...);
int		tdf_errfd(void);
void	tdf_collect_batch(pid_t pid);

/* arrêt demandé (Ctrl+C, SIGTERM, SIGHUP) : le gestionnaire ne pose qu'un drapeau */
extern volatile sig_atomic_t	g_quit_requested;
void	tdf_quit_check(pid_t child);   /* tue le fils en cours puis sort, si demandé */
void	tdf_in_child(void);
/* avancement d'un lot long : barre dans le terminal et ligne « P » pour le jeu */
void	tdf_progress(const char *label, int cur, int total, int end);            /* à appeler dans chaque fils de test */
const char	*tdf_cur_desc(void);
void	tdf_cur_where(const char **file, int *line);
void	tdf_symbolize(uintptr_t off, char *dst, size_t n);
void	tdf_memreport(t_buf *msg, int *ko, int *warn);

/* suites */
void	tdf_tests_mandatory(void);
void	tdf_tests_edge(void);
void	tdf_tests_stress(void);
void	tdf_tests_bonus(void);

/* dispatch pour lots */
typedef union u_val
{
	int				i;
	unsigned int	u;
	const char		*s;
	void			*p;
}	t_val;

# define K_INT 0
# define K_UNS 1
# define K_STR 2
# define K_PTR 3

typedef int	(*t_pf)(const char *, ...);
int		tdf_dispatch(t_pf fn, const char *fmt, int n, const int *k, const t_val *v);

/* moteur par lots (stress, bonus) */
typedef struct s_bc
{
	char	fmt[192];
	int		n;
	int		k[4];
	t_val	v[4];
	int		sev;
	size_t	eoff;
	size_t	elen;
	int		eret;
}	t_bc;

void	tdf_batch(t_bc *c, int n, const char *file, int line, const char *label);
void	tdf_bc_desc(t_bc *c, t_buf *dst);

/* injection d'échec malloc */
void	tdf_mf_begin(void);
void	tdf_mf_step(int k);
void	tdf_mf_end(int nalloc);


#endif
