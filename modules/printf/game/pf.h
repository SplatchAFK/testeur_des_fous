#ifndef PF_H
# define PF_H

/*
** Le testeur des fous · version printf : données propres à ce jeu.
** Inclus par fous.h quand TDF_PRINTF est défini.
*/

# define NCHAIN 9
# define NDLC 6
# define NPEXO 16
# define NPACH 25
# define NPHASE 7
# define MAXCASE 700
# define MAXCAUSE 160
# define MAXSTAT 24

# define pf_load tdf_pf_load
# define pf_chain_ok tdf_pf_chain_ok
# define pf_chain_ko tdf_pf_chain_ko
# define pf_phase_ok tdf_pf_phase_ok
# define pf_all_ok tdf_pf_all_ok
# define pf_cat tdf_pf_cat
# define pf_libft_done tdf_pf_libft_done
# define pf_connected tdf_pf_connected
# define pf_ach tdf_pf_ach
# define pf_badges tdf_pf_badges
# define g_pr tdf_g_pr
# define g_pf tdf_g_pf
# define g_chains tdf_g_chains
# define g_phases tdf_g_phases
# define g_dlcb tdf_g_dlcb
# define g_pach_fr tdf_g_pach_fr
# define g_pach_en tdf_g_pach_en
# define g_pexos tdf_g_pexos
# define pf_scan tdf_pf_scan
# define pf_results tdf_pf_results
# define pf_rewards tdf_pf_rewards
# define pf_record tdf_pf_record
# define pf_profile tdf_pf_profile
# define pf_credits tdf_pf_credits
# define pf_connect_screen tdf_pf_connect_screen
# define pf_grant_veteran tdf_pf_grant_veteran
# define pf_menu tdf_pf_menu
# define pf_dlc_menu tdf_pf_dlc_menu
# define pf_boss_menu tdf_pf_boss_menu
# define pf_train tdf_pf_train
# define pf_exo_done tdf_pf_exo_done
# define story_splash tdf_story_splash
# define story_previously tdf_story_previously
# define story_printf_intro tdf_story_printf_intro
# define story_chains tdf_story_chains
# define story_freed tdf_story_freed
# define story_preboss tdf_story_preboss
# define story_bigboss tdf_story_bigboss
# define story_victory tdf_story_victory
# define story_dlc tdf_story_dlc

typedef struct s_pcat
{
	char	key[16];
	char	title[110];
	int		ok;
	int		ko;
	int		warn;
}	t_pcat;

typedef struct s_pcase
{
	char	cat[16];
	int		ko;
	char	where[64];
	char	desc[200];
	char	msg[700];
}	t_pcase;

typedef struct s_pcause
{
	char	cat[16];
	char	title[160];
	char	where[160];
	int		count;
	char	hint[320];
}	t_pcause;

typedef struct s_pstat
{
	int		st;
	char	title[170];
	char	detail[400];
}	t_pstat;

/* résultats du moteur (fichier --machine) */
typedef struct s_prun
{
	t_pcat		cat[24];
	int			ncat;
	t_pcase		cs[MAXCASE];
	int			ncs;
	t_pcause	ca[MAXCAUSE];
	int			nca;
	t_pstat		st[MAXSTAT];
	int			nst;
	int			tok;
	int			tko;
	int			twarn;
	char		mode[32];
	int			done;
	char		plabel[32];
	int			pcur;
	int			ptot;
	int			loaded;
	int			bonus;
	int			truncated;
}	t_prun;

/* section printf de save.txt */
typedef struct s_psave
{
	int		runs;
	char	chains[NCHAIN + 1];
	int		big;
	char	dlc[NDLC + 1];
	char	exo[NPEXO + 1];
	char	ach[NPACH + 1];
	long	lastreward;
	int		crashes;
	int		vet;
	int		secret;
	int		seen;
}	t_psave;

typedef struct s_chain
{
	const char	*key;
	const char	*conv;
	const char	*fr;
	const char	*en;
	t_rgb		c;
}	t_chain;

typedef struct s_phase
{
	const char	*fr;
	const char	*en;
	const char	*cats;
	const char	*stats;
}	t_phase;

typedef struct s_pexo
{
	const char	*fn;
	const char	*proto;
	const char	*fr;
	const char	*en;
	const char	*notion;
}	t_pexo;

extern t_prun			g_pr;
extern t_psave			g_pf;
extern const t_chain	g_chains[NCHAIN];
extern const t_phase	g_phases[NPHASE];
extern const t_chain	g_dlcb[NDLC];
extern const char		*g_pach_fr[NPACH][2];
extern const char		*g_pach_en[NPACH][2];
extern const t_pexo		g_pexos[NPEXO];

void			pf_load(const char *path, long *off);
t_pcat			*pf_cat(const char *key);
int				pf_chain_ok(int i);
int				pf_chain_ko(int i);
int				pf_phase_ok(int i);
int				pf_all_ok(void);
int				pf_libft_done(void);
int				pf_connected(void);
int				pf_ach(int i);
void			pf_badges(int quiet);

int				pf_scan(const char *path, int pid);
int				pf_results(int after);
void			pf_rewards(void);
int				pf_record(void);
void			pf_profile(void);
void			pf_credits(void);
void			pf_connect_screen(void);
void			pf_grant_veteran(void);
int				pf_menu(void);
int				pf_dlc_menu(void);
void			pf_boss_menu(void);
void			pf_train(void);
int				pf_exo_done(void);

void			story_splash(void);
void			story_previously(void);
void			story_printf_intro(void);
int				story_chains(int dlc);
void			story_freed(void);
void			story_preboss(void);
int				story_bigboss(void);
void			story_victory(int secret);
void			story_dlc(void);

#endif
