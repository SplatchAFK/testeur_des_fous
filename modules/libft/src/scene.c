#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "fous.h"

#define CW 80
#define CH 36
#define ROWS 24
#define LMAX 12000

typedef struct s_line
{
	char	s[LMAX];
	size_t	n;
}	t_line;

typedef struct s_part
{
	double	x;
	double	y;
	double	vx;
	double	vy;
	int		life;
	t_rgb	c;
}	t_part;

static t_line	g_cur[ROWS];
static t_line	g_prev[ROWS];
static t_canvas	g_cv;
static t_rgb	*g_sky;
static t_list	*g_parts;
static char		*g_bname;
static char		*g_lines[2];
static char		g_log[96];
static char		g_cheat[16];
static int		g_ox;
static int		g_oy;
static int		g_hp;
static int		g_maxhp;
static long		g_flash_boss;
static long		g_flash_red;
static long		g_dash;
static long		g_lunge;
static long		g_dmg_at;
static int		g_dead;
static int		g_boss;
static int		g_logfd = -1;

static void	ap(t_line *l, const char *s)
{
	size_t	n;

	n = strlen(s);
	if (l->n + n >= LMAX)
		return ;
	memcpy(l->s + l->n, s, n + 1);
	l->n += n;
}

static void	apc(t_line *l, t_rgb c, int bg)
{
	char	b[32];

	color_seq(b, c, bg);
	ap(l, b);
}

static void	part_free(void *p)
{
	free(p);
}

static void	*part_clone(void *p)
{
	t_part	*a;
	t_part	*b;

	a = p;
	b = FT(calloc, 1, sizeof(t_part));
	if (!b)
		return (NULL);
	*b = *a;
	b->vx = -a->vx * 0.6;
	b->vy = a->vy * 0.6 - 0.4;
	b->c = mix(a->c, rgb(255, 255, 255), 0.4);
	return (b);
}

static void	spawn(double x, double y, t_rgb c, int n, double sp, int front)
{
	t_part	*p;
	t_list	*node;
	int		i;

	for (i = 0; i < n && (int)FT(lstsize, g_parts) < 1400; i++)
	{
		p = FT(calloc, 1, sizeof(t_part));
		if (!p)
			return ;
		p->x = x;
		p->y = y;
		p->vx = ((rand() % 200) - 100) / 100.0 * sp;
		p->vy = ((rand() % 200) - 130) / 100.0 * sp;
		p->life = 12 + rand() % 22;
		p->c = c;
		node = FT(lstnew, p);
		if (front)
			FT(lstadd_front, &g_parts, node);
		else
			FT(lstadd_back, &g_parts, node);
	}
}

static void	part_draw(void *c)
{
	t_part	*p;

	p = c;
	p->x += p->vx;
	p->y += p->vy;
	p->vy += 0.06;
	p->life--;
	if (p->life > 0)
		cv_px(&g_cv, (int)p->x, (int)p->y, mix(p->c, rgb(10, 10, 20),
				1.0 - p->life / 30.0));
}

static void	parts_prune(void)
{
	t_list	*cur;
	t_list	*nx;

	while (g_parts && ((t_part *)g_parts->content)->life <= 0)
	{
		nx = g_parts->next;
		FT(lstdelone, g_parts, part_free);
		g_parts = nx;
	}
	cur = g_parts;
	while (cur && cur->next)
	{
		if (((t_part *)cur->next->content)->life <= 0)
		{
			nx = cur->next->next;
			FT(lstdelone, cur->next, part_free);
			cur->next = nx;
		}
		else
			cur = cur->next;
	}
}

static char	up_f(unsigned int i, char c)
{
	(void)i;
	if ((unsigned char)c < 128)
		return (FT(toupper, (unsigned char)c));
	return (c);
}

static void	utf_up_f(unsigned int i, char *c)
{
	if (i > 0 && (unsigned char)c[-1] == 0xC3 && (unsigned char)*c >= 0xA0
		&& (unsigned char)*c <= 0xBE && (unsigned char)*c != 0xB7)
		*c = (char)((unsigned char)*c - 0x20);
}

static void	glitch_f(unsigned int i, char *c)
{
	if (g_flash_red > now_ms() && (i * 5 + now_ms() / 50) % 7 == 0
		&& (unsigned char)*c < 128 && *c != ' ')
		*c = "#%&@$!?"[(i + now_ms() / 40) % 7];
}

static void	sanit_f(unsigned int i, char *c)
{
	(void)i;
	if ((unsigned char)*c < 128 && !FT(isprint, (unsigned char)*c))
		*c = ' ';
}

static void	say(const char *text, int n)
{
	char	**parts;
	char	*num;
	char	*hole;
	char	*a;
	char	*b;
	char	*tmp;
	int		i;

	free(g_lines[0]);
	free(g_lines[1]);
	g_lines[0] = NULL;
	g_lines[1] = NULL;
	parts = FT(split, text, '\n');
	for (i = 0; parts && parts[i] && i < 2; i++)
	{
		hole = FT(strnstr, parts[i], "{n}", FT(strlen, parts[i]));
		if (hole)
		{
			num = FT(itoa, n);
			a = FT(substr, parts[i], 0, hole - parts[i]);
			tmp = FT(strjoin, a, num);
			b = FT(strjoin, tmp, hole + 3);
			free(a);
			free(tmp);
			free(num);
			g_lines[i] = FT(strtrim, b, " ");
			free(b);
		}
		else
			g_lines[i] = FT(strtrim, parts[i], " ");
		if (g_lines[i])
			FT(striteri, g_lines[i], sanit_f);
	}
	for (i = 0; parts && parts[i]; i++)
		free(parts[i]);
	free(parts);
}

static void	sky_init(void)
{
	int		x;
	int		y;
	double	t;

	g_sky = FT(calloc, CW * CH, sizeof(t_rgb));
	if (!g_sky)
		return ;
	for (y = 0; y < CH; y++)
	{
		t = (double)y / CH;
		for (x = 0; x < CW; x++)
			g_sky[y * CW + x] = mix(rgb(10, 8, 26), mix(theme()->bg, g_bosses[g_boss].glow, 0.18), t);
		if (y < 26)
			for (x = (y * 37) % 11; x < CW; x += 9 + (y * 7) % 13)
				g_sky[y * CW + x] = mix(rgb(200, 200, 230), g_sky[y * CW + x], (y % 3) * 0.3);
	}
	for (x = 0; x < CW; x++)
		for (y = 30; y < CH; y++)
			g_sky[y * CW + x] = mix(rgb(30, 26, 44), rgb(14, 12, 22), (y - 30) / 6.0);
}

static void	sky_scroll(void)
{
	t_rgb	keep;
	int		y;

	for (y = 0; y < 26; y++)
	{
		keep = g_sky[y * CW + CW - 1];
		FT(memmove, g_sky + y * CW + 1, g_sky + y * CW, (CW - 1) * sizeof(t_rgb));
		g_sky[y * CW] = keep;
	}
}

static void	draw_world(long t)
{
	double	fl;
	int		bx;
	int		by;
	int		kx;
	int		shake;
	t_rgb	eye;

	FT(bzero, g_cv.a, (size_t)CW * CH);
	if (g_sky)
	{
		FT(memcpy, g_cv.px, g_sky, (size_t)CW * CH * sizeof(t_rgb));
		FT(memset, g_cv.a, 1, (size_t)CW * CH);
	}
	shake = (g_flash_red > t) ? (int)(2 * sin(t / 20.0)) : 0;
	kx = 6 + (g_dash > t ? (int)(14 * sin((g_dash - t) / 300.0 * 3.14)) : 0) + shake;
	bx = 76 - g_boss_spr[g_boss].w + (g_lunge > t ? -(int)(10 * sin((g_lunge - t) / 300.0 * 3.14)) : 0);
	by = CH - g_boss_spr[g_boss].h - 1;
	if (g_dead == 1)
		by += 0;
	fl = g_flash_boss > t ? (g_flash_boss - t) / 220.0 : 0;
	eye = mix(g_bosses[g_boss].glow, rgb(255, 255, 255), 0.5);
	if (g_dead != 2 && !gardien_sprite(&g_cv, g_boss, bx + shake, by, g_boss_spr[g_boss].w, g_boss_spr[g_boss].h, fl))
		cv_spr(&g_cv, &g_boss_spr[g_boss], bx + shake, by, eye, eye, fl > 1 ? 1 : fl);
	avatar_draw(&g_cv, kx, CH - 33, &g_save.look, t);
	if (g_dash > t && g_dash - t < 160)
	{
		int	i;

		for (i = 0; i < 22; i++)
			cv_px(&g_cv, bx + 6 + i, by + 4 + i, rgb(255, 255, 255));
	}
	FT(lstiter, g_parts, part_draw);
	parts_prune();
}

static void	text_row(int r, t_rgb c, const char *s)
{
	char	buf[400];
	size_t	n;
	size_t	w;

	g_cur[r].n = 0;
	g_cur[r].s[0] = '\0';
	apc(&g_cur[r], theme()->bg, 1);
	if (!s)
		return ;
	n = FT(strlcpy, buf, s, sizeof(buf));
	(void)n;
	w = (size_t)ulen(buf);
	while (w < CW && (CW - w) / 2 > 0 && g_cur[r].n < 60)
	{
		ap(&g_cur[r], " ");
		w += 2;
	}
	apc(&g_cur[r], c, 0);
	ap(&g_cur[r], buf);
}

static void	hud(long t)
{
	char	st[256];
	char	*pv;
	char	*mx;
	char	*a;
	char	*b;
	int		i;
	int		w;

	pv = FT(itoa, g_hp);
	mx = FT(itoa, g_maxhp);
	a = FT(strjoin, " PV ", pv);
	b = a ? FT(strjoin, a, "/") : NULL;
	FT(strlcpy, st, g_bname ? g_bname : "?", sizeof(st));
	FT(striteri, st, glitch_f);
	g_cur[0].n = 0;
	g_cur[0].s[0] = '\0';
	apc(&g_cur[0], theme()->bg, 1);
	apc(&g_cur[0], g_bosses[g_boss].glow, 0);
	ap(&g_cur[0], " ");
	ap(&g_cur[0], st);
	apc(&g_cur[0], theme()->text, 0);
	ap(&g_cur[0], b ? b : " PV ?");
	ap(&g_cur[0], mx ? mx : "?");
	ap(&g_cur[0], "  ");
	w = 30;
	for (i = 0; i < w; i++)
	{
		apc(&g_cur[0], i < w * g_hp / (g_maxhp ? g_maxhp : 1)
			? mix(rgb(255, 60, 80), rgb(255, 170, 60), (double)i / w) : rgb(50, 40, 60), 0);
		ap(&g_cur[0], "█");
	}
	if (g_dmg_at > t)
	{
		apc(&g_cur[0], rgb(255, 255, 120), 0);
		ap(&g_cur[0], "  -1 !");
	}
	free(pv);
	free(mx);
	free(a);
	free(b);
	snprintf(st, sizeof(st), " %s", g_save.login);
	g_cur[1].n = 0;
	g_cur[1].s[0] = '\0';
	apc(&g_cur[1], theme()->bg, 1);
	apc(&g_cur[1], theme()->accent, 0);
	ap(&g_cur[1], st);
	apc(&g_cur[1], theme()->dim, 0);
	ap(&g_cur[1], L("   contre ", "   versus "));
	apc(&g_cur[1], g_bosses[g_boss].glow, 0);
	ap(&g_cur[1], L(g_bosses[g_boss].name, g_bosses[g_boss].en));
}

static void	compose(long t)
{
	int		r;
	char	buf[LMAX];

	draw_world(t);
	hud(t);
	for (r = 0; r < CH / 2; r++)
	{
		cv_row(&g_cv, r, theme()->bg, buf, sizeof(buf));
		g_cur[2 + r].n = 0;
		g_cur[2 + r].s[0] = '\0';
		ap(&g_cur[2 + r], buf);
	}
	text_row(20, theme()->dim, "");
	text_row(21, theme()->text, g_lines[0]);
	text_row(22, theme()->text, g_lines[1]);
	text_row(23, g_flash_red > t ? rgb(255, 90, 100) : theme()->dim, g_log);
}

static void	present(void)
{
	char	mv[48];
	char	bgs[48];
	int		y;

	for (y = 0; y < ROWS; y++)
	{
		if (g_cur[y].n == g_prev[y].n
			&& FT(memcmp, g_cur[y].s, g_prev[y].s, g_cur[y].n) == 0)
			continue ;
		snprintf(mv, sizeof(mv), "\033[%d;%dH\033[0m", g_oy + y + 1, g_ox + 1);
		color_seq(bgs, theme()->bg, 1);
		FT(putstr_fd, mv, 1);
		FT(putstr_fd, bgs, 1);
		FT(putstr_fd, "\033[K", 1);
		FT(putstr_fd, g_cur[y].s, 1);
		FT(memcpy, g_prev[y].s, g_cur[y].s, g_cur[y].n + 1);
		g_prev[y].n = g_cur[y].n;
	}
	snprintf(mv, sizeof(mv), "\033[%d;%dH\033[38;2;255;80;110m", g_oy + 1, g_ox + CW);
	FT(putstr_fd, mv, 1);
	FT(putchar_fd, (now_ms() / 400) % 2 ? '*' : ' ', 1);
}

static int	keys(void)
{
	int		k;
	char	one[2];
	int		skip;

	skip = 0;
	while ((k = key_read(0)) >= 0)
	{
		if (key_stop(k) || k == 'x')
		{
			g_shm->quit = 1;
			return (2);
		}
		if (k == '\r' || k == '\n' || k == ' ')
			skip = 1;
		if (k >= 1000 || !FT(isascii, k))
			continue ;
		if (FT(isalnum, k))
		{
			one[0] = FT(tolower, k);
			one[1] = '\0';
			if (FT(strlen, g_cheat) + 2 >= sizeof(g_cheat))
				FT(bzero, g_cheat, sizeof(g_cheat));
			FT(strlcat, g_cheat, one, sizeof(g_cheat));
			if (FT(strnstr, g_cheat, "fous", sizeof(g_cheat)))
			{
				spawn(40, 10, hue((now_ms() % 1000) / 1000.0), 80, 2.5, 1);
				FT(bzero, g_cheat, sizeof(g_cheat));
			}
		}
		if (FT(isdigit, k) && FT(atoi, one) >= 0)
			skip = skip || (k == '0');
		if (FT(isalpha, k))
			(void)FT(tolower, k);
	}
	return (skip);
}

static int	frame_wait(long until)
{
	long	t;
	int		k;

	while ((t = now_ms()) < until)
	{
		g_shm->beat++;
		if ((g_shm->beat & 3) == 0)
			sky_scroll();
		compose(t);
		present();
		k = keys();
		if (k)
			return (k);
		sleep_ms(30);
	}
	return (0);
}

static void	log_line(const char *what, int ok)
{
	if (g_logfd < 0)
		return ;
	FT(putstr_fd, (char *)what, g_logfd);
	FT(putstr_fd, ok ? " OK  pv=" : " FAUX  pv=", g_logfd);
	FT(putnbr_fd, g_hp, g_logfd);
	FT(putendl_fd, "", g_logfd);
}

static void	explode(void)
{
	const t_spr	*s;
	int			i;
	int			j;
	int			bx;
	int			by;
	t_list		*copy;
	t_list		*last;

	s = &g_boss_spr[g_boss];
	bx = 76 - s->w;
	by = CH - s->h - 1;
	for (j = 0; j < s->h; j += 1)
		for (i = 0; i < s->w; i += 1)
			if (s->rows[j][i] != '.' && rand() % 2 == 0)
				spawn(bx + s->ox + i, by + s->oy + j,
					pal(s->rows[j][i], g_bosses[g_boss].glow, g_bosses[g_boss].glow), 1, 1.4, 0);
	copy = FT(lstmap, g_parts, part_clone, part_free);
	last = FT(lstlast, g_parts);
	if (last)
		last->next = copy;
	else
		g_parts = copy;
}

int	scene_child(int boss, int replay)
{
	char	*name;
	int		f;
	int		k;
	int		ok;
	int		fails;
	long	step;
	char	path[600];

	(void)replay;
	srand(now_ms() ^ boss);
	g_boss = boss;
	g_ox = (term_cols() - CW) / 2;
	g_oy = (term_rows() - ROWS) / 2;
	if (g_ox < 0)
		g_ox = 0;
	if (g_oy < 0)
		g_oy = 0;
	cv_init(&g_cv, CW, CH);
	sky_init();
	FT(bzero, g_prev, sizeof(g_prev));
	name = FT(strdup, L(g_bosses[boss].name, g_bosses[boss].en));
	g_bname = name ? FT(strmapi, name, up_f) : NULL;
	if (g_bname)
		FT(striteri, g_bname, utf_up_f);
	free(name);
	snprintf(path, sizeof(path), "%s/journal_des_boss.log", g_savedir);
	g_logfd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
	g_maxhp = g_bosses[boss].last - g_bosses[boss].first + 1;
	g_hp = g_maxhp;
	FT(putstr_fd, "\033[0m\033[2J", 1);
	FT(strlcpy, g_log, L(" Entrée : passer   Q / Échap : quitter les combats",
			" Enter: skip   Q / Esc: leave the fights"), sizeof(g_log));
	say(texte_trad(g_bosses[boss].intro), 0);
	k = frame_wait(now_ms() + anim_ms(2600));
	if (k == 2)
		return (0);
	step = anim_ms(420);
	fails = 0;
	for (f = g_bosses[boss].first; f <= g_bosses[boss].last && k != 2; f++)
	{
		ok = st_pass(g_run.fn[f].st);
		snprintf(g_log, sizeof(g_log), " ft_%s : %s", g_fn_name[f],
			ok ? "OK" : st_label(g_run.fn[f].st));
		if (ok)
		{
			g_hp--;
			g_dash = now_ms() + 300;
			g_flash_boss = now_ms() + 220;
			g_dmg_at = now_ms() + 300;
			spawn(60, 16, rgb(255, 255, 200), 14, 1.6, 1);
		}
		else
		{
			fails++;
			g_lunge = now_ms() + 300;
			g_flash_red = now_ms() + 360;
			spawn(14, 20, rgb(255, 60, 60), 18, 1.8, 0);
		}
		log_line(g_fn_name[f], ok);
		if (k != 1)
			k = frame_wait(now_ms() + step);
	}
	if (k == 2)
		return (0);
	if (g_hp == 0)
	{
		g_dead = 2;
		explode();
		g_shm->beaten[boss] = 1;
		say(texte_trad(g_bosses[boss].defeat), 0);
		snprintf(g_log, sizeof(g_log), L(" %s est vaincu !  (Entrée)", " %s is defeated!  (Enter)"),
			L(g_bosses[boss].name, g_bosses[boss].en));
	}
	else
	{
		say(texte_trad(g_bosses[boss].taunt), fails);
		snprintf(g_log, sizeof(g_log), L(" Il reste %d PV au boss.  (Entrée)", " The boss has %d HP left.  (Enter)"), g_hp);
	}
	frame_wait(now_ms() + 600000);
	FT(lstclear, &g_parts, part_free);
	if (g_logfd >= 0)
		close(g_logfd);
	free(g_bname);
	free(g_lines[0]);
	free(g_lines[1]);
	free(g_sky);
	cv_free(&g_cv);
	return (0);
}

void	scene_play(t_scene_out *g, int boss, int replay)
{
	pid_t			pid;
	int				status;
	unsigned long	beat;
	long			seen;
	long			start;
	int				i;

	memset(g, 0, sizeof(*g));
	g->fn = -1;
	g_shm = mmap(NULL, sizeof(t_shm), PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (g_shm == MAP_FAILED)
	{
		g_shm = NULL;
		return ;
	}
	memset((void *)g_shm, 0, sizeof(t_shm));
	g_shm->cur = -1;
	reset();
	frame();
	flush();
	pid = fork();
	if (pid == 0)
	{
		term_forked();
		lib_init();
		scene_child(boss, replay);
		_exit(0);
	}
	term_child(pid > 0 ? pid : 0);
	beat = 0;
	seen = now_ms();
	start = seen;
	status = 0;
	while (pid > 0 && waitpid(pid, &status, WNOHANG) == 0)
	{
		if (g_shm->beat != beat)
		{
			beat = g_shm->beat;
			seen = now_ms();
		}
		else if (now_ms() - seen > 3000 && now_ms() - start > 1500)
		{
			kill(pid, SIGKILL);
			waitpid(pid, &status, 0);
			g->frozen = 1;
			break ;
		}
		sleep_ms(20);
	}
	term_child(0);
	if (pid > 0 && !g->frozen && WIFSIGNALED(status))
	{
		g->crashed = 1;
		g->sig = WTERMSIG(status);
	}
	term_invalidate();
	g->fn = g_shm->cur;
	g->quit = g_shm->quit;
	for (i = 0; i < 3; i++)
		if (g_shm->beaten[i] && part_clean(i))
			g_save.boss[i] = 1;
	munmap((void *)g_shm, sizeof(t_shm));
	g_shm = NULL;
	while (key_read(0) >= 0)
		;
	cls();
}
