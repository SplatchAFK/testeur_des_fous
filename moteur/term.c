#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "fous.h"

#define SEQ_ENTER "\033[?1049h\033[?25l\033[?7l"
#define SEQ_LEAVE "\033[0m\033[?7h\033[?25h\033[?1049l"

static struct termios			g_old;
static volatile sig_atomic_t	g_raw;
static char						g_buf[1 << 18];
static size_t					g_len;
static pid_t					g_child;
static int						g_forked;
static long						g_ctrlc_at;

volatile sig_atomic_t			g_quit_requested;

static void	on_signal(int s)
{
	if (g_quit_requested)
	{
		if (g_raw)
		{
			(void)!write(1, SEQ_LEAVE, sizeof(SEQ_LEAVE) - 1);
			tcsetattr(0, TCSANOW, &g_old);
		}
		_exit(128 + s);
	}
	g_quit_requested = s;
}

void	term_child(int pid)
{
	g_child = pid;
}

void	term_forked(void)
{
	g_forked = 1;
	g_child = 0;
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGHUP, SIG_DFL);
}

void	term_poll(void)
{
	int	s;

	s = g_quit_requested;
	if (!s)
		return ;
	if (g_forked)
		_exit(128 + s);
	if (g_child)
		kill(g_child, SIGTERM);
	g_len = 0;
	term_end();
	signal(s, SIG_DFL);
	raise(s);
	_exit(128 + s);
}

int	key_quit(int k)
{
	if (k == 27)
		return (1);
	return ((k == 'q' || k == 'Q') && key_dir(k) == k);
}

int	key_stop(int k)
{
	return (k == 27 || k == 'q' || k == 'Q');
}

#define MINW 80
#define MINH 24

static int	g_cols;
static int	g_rows;
static long	g_paused;
static int	g_fast;
static int	g_col16;

static int	read_byte(int timeout_ms);

static int	size_refresh(void)
{
	struct winsize	w;
	int				c;
	int				r;

	c = 80;
	r = 24;
	if (ioctl(1, TIOCGWINSZ, &w) == 0 && w.ws_col > 0 && w.ws_row > 0)
	{
		c = w.ws_col;
		r = w.ws_row;
	}
	if (c == g_cols && r == g_rows)
		return (0);
	g_cols = c;
	g_rows = r;
	return (1);
}

int	term_cols(void)
{
	if (!g_cols)
		size_refresh();
	return (g_cols);
}

int	term_rows(void)
{
	if (!g_rows)
		size_refresh();
	return (g_rows);
}

static long	real_ms(void)
{
	struct timespec	t;

	clock_gettime(CLOCK_MONOTONIC, &t);
	return (t.tv_sec * 1000L + t.tv_nsec / 1000000L);
}

long	now_ms(void)
{
	return (real_ms() - g_paused);
}

typedef struct s_cell
{
	uint32_t	ch;
	uint32_t	fg;
	uint32_t	bg;
	uint8_t		at;
	uint8_t		w;
}	t_cell;

#define A_UNDER 8
#define A_REV 32

static t_cell	*g_bk;
static t_cell	*g_fr;
static t_cell	*g_tm;
static int		g_vw;
static int		g_vh;
static int		g_cx;
static int		g_cy;
static int		g_inval;
static int		g_sync;
static t_cell	g_pen;
static char		g_ob[1 << 16];
static size_t	g_on;
static int		g_rx = -1;
static int		g_ry = -1;
static int		g_rp_ok;
static t_cell	g_rp;
static int		g_ps;
static int		g_pn[16];
static int		g_np;
static int		g_priv;
static int		g_bad;
static char		g_seq[48];
static int		g_ns;
static uint32_t	g_u;
static uint32_t	g_ucp;
static int		g_un;
static int		g_uneed;

static void	raw_flush(void)
{
	size_t	o;
	ssize_t	w;

	o = 0;
	while (o < g_on)
	{
		w = write(1, g_ob + o, g_on - o);
		if (w < 0 && errno == EINTR)
			continue ;
		if (w <= 0)
			break ;
		o += w;
	}
	g_on = 0;
}

static void	rawn(const char *s, size_t n)
{
	if (g_on + n > sizeof(g_ob))
		raw_flush();
	if (n > sizeof(g_ob))
		return ;
	memcpy(g_ob + g_on, s, n);
	g_on += n;
}

static t_cell	blank(uint32_t bg)
{
	t_cell	c;

	c.ch = ' ';
	c.fg = 0;
	c.bg = bg;
	c.at = 0;
	c.w = 1;
	return (c);
}

static int	cell_eq(const t_cell *a, const t_cell *b)
{
	return (a->ch == b->ch && a->fg == b->fg && a->bg == b->bg && a->at == b->at && a->w == b->w);
}

static void	vs_fit(void)
{
	t_cell	*nb;
	int		x;
	int		y;
	int		n;

	if (g_bk && g_cols == g_vw && g_rows == g_vh)
		return ;
	n = g_cols * g_rows;
	nb = malloc(n * sizeof(t_cell));
	if (!nb)
		return ;
	for (x = 0; x < n; x++)
		nb[x] = blank(g_pen.bg);
	for (y = 0; g_bk && y < g_vh && y < g_rows; y++)
		for (x = 0; x < g_vw && x < g_cols; x++)
			nb[y * g_cols + x] = g_bk[y * g_vw + x];
	free(g_bk);
	g_bk = nb;
	g_fr = realloc(g_fr, n * sizeof(t_cell));
	g_tm = realloc(g_tm, n * sizeof(t_cell));
	g_vw = g_cols;
	g_vh = g_rows;
	g_inval = 1;
}

static void	vs_erase(int from, int to)
{
	while (from < to)
		g_bk[from++] = blank(g_pen.bg);
}

static int	cp_width(uint32_t c)
{
	if ((c >= 0x300 && c <= 0x36F) || (c >= 0x200B && c <= 0x200F) || (c >= 0xFE00 && c <= 0xFE0F)
		|| c == 0xFEFF)
		return (0);
	if ((c >= 0x1100 && c <= 0x115F) || (c >= 0x2E80 && c <= 0xA4CF) || (c >= 0xAC00 && c <= 0xD7A3)
		|| (c >= 0xF900 && c <= 0xFAFF) || (c >= 0xFE30 && c <= 0xFE4F) || (c >= 0xFF00 && c <= 0xFF60)
		|| (c >= 0xFFE0 && c <= 0xFFE6) || (c >= 0x1F300 && c <= 0x1FAFF) || (c >= 0x20000 && c <= 0x3FFFD))
		return (2);
	return (1);
}

static void	vs_put(uint32_t ch, int w)
{
	t_cell	*c;

	if (!w)
		return ;
	if (g_cy < 0 || g_cy >= g_vh)
	{
		g_cx += w;
		return ;
	}
	if (g_cx < 0)
		g_cx = 0;
	if (g_cx >= g_vw)
		g_cx = g_vw - 1;
	if (w == 2 && g_cx == g_vw - 1)
	{
		ch = ' ';
		w = 1;
	}
	c = &g_bk[g_cy * g_vw + g_cx];
	if (c->w == 0 && g_cx > 0 && c[-1].w == 2)
		c[-1] = blank(c[-1].bg);
	if (c->w == 2 && g_cx + 1 < g_vw)
		c[1] = blank(c[1].bg);
	*c = g_pen;
	c->ch = ch;
	c->w = w;
	if (ch == ' ' && !(c->at & (A_UNDER | A_REV)))
	{
		c->fg = 0;
		c->at = 0;
	}
	if (w == 2)
	{
		if (c[1].w == 2 && g_cx + 2 < g_vw)
			c[2] = blank(c[2].bg);
		c[1] = *c;
		c[1].ch = 0;
		c[1].w = 0;
	}
	g_cx += w;
}

static void	vs_sgr(void)
{
	int			i;
	int			p;
	uint32_t	c;

	if (!g_np)
		g_pn[g_np++] = 0;
	for (i = 0; i < g_np; i++)
	{
		p = g_pn[i];
		if (p == 0)
			g_pen = blank(0);
		else if (p >= 1 && p <= 7 && p != 6)
			g_pen.at |= 1 << (p > 6 ? 5 : p - 1);
		else if (p == 22)
			g_pen.at &= ~3;
		else if (p >= 23 && p <= 27)
			g_pen.at &= ~(1 << (p == 27 ? 5 : p - 21));
		else if ((p >= 30 && p <= 37) || (p >= 90 && p <= 97))
			g_pen.fg = 3u << 24 | (p < 90 ? p - 30 : p - 82);
		else if ((p >= 40 && p <= 47) || (p >= 100 && p <= 107))
			g_pen.bg = 3u << 24 | (p < 100 ? p - 40 : p - 92);
		else if (p == 39)
			g_pen.fg = 0;
		else if (p == 49)
			g_pen.bg = 0;
		else if (p == 38 || p == 48)
		{
			c = 0;
			if (i + 2 < g_np && g_pn[i + 1] == 5)
			{
				c = 2u << 24 | (g_pn[i + 2] & 255);
				i += 2;
			}
			else if (i + 4 < g_np && g_pn[i + 1] == 2)
			{
				c = 1u << 24 | (g_pn[i + 2] & 255) << 16 | (g_pn[i + 3] & 255) << 8 | (g_pn[i + 4] & 255);
				i += 4;
			}
			else
				break ;
			if (p == 38)
				g_pen.fg = c;
			else
				g_pen.bg = c;
		}
	}
}

static void	vs_csi(char f)
{
	int	a;
	int	b;
	int	row;

	a = g_np > 0 ? g_pn[0] : 0;
	b = g_np > 1 ? g_pn[1] : 0;
	row = (g_cy < 0 ? 0 : (g_cy >= g_vh ? g_vh - 1 : g_cy)) * g_vw;
	if (g_bad)
		return ;
	if (g_priv || !strchr("HfABCDGJKm", f))
	{
		rawn(g_seq, g_ns);
		if (g_priv && a == 1049)
			g_inval = 1;
		return ;
	}
	if (f == 'H' || f == 'f')
	{
		g_cy = (a ? a : 1) - 1;
		g_cx = (b ? b : 1) - 1;
	}
	else if (f == 'A')
		g_cy -= a ? a : 1;
	else if (f == 'B')
		g_cy += a ? a : 1;
	else if (f == 'C')
		g_cx += a ? a : 1;
	else if (f == 'D')
		g_cx -= a ? a : 1;
	else if (f == 'G')
		g_cx = (a ? a : 1) - 1;
	else if (f == 'J' && a >= 2)
		vs_erase(0, g_vw * g_vh);
	else if (f == 'J' && a == 0)
		vs_erase(row + (g_cx < g_vw ? (g_cx < 0 ? 0 : g_cx) : g_vw), g_vw * g_vh);
	else if (f == 'J')
		vs_erase(0, row + (g_cx < g_vw ? g_cx + 1 : g_vw));
	else if (f == 'K' && a == 0)
		vs_erase(row + (g_cx < g_vw ? (g_cx < 0 ? 0 : g_cx) : g_vw), row + g_vw);
	else if (f == 'K' && a == 1)
		vs_erase(row, row + (g_cx < g_vw ? g_cx + 1 : g_vw));
	else if (f == 'K')
		vs_erase(row, row + g_vw);
	else if (f == 'm')
		vs_sgr();
	if (g_cy < 0)
		g_cy = 0;
	if (g_cx < 0)
		g_cx = 0;
}

static void	vs_byte(unsigned char c)
{
	if (g_ps == 1)
	{
		g_seq[g_ns++] = c;
		g_ps = c == '[' ? 2 : 0;
		if (c != '[')
			rawn(g_seq, g_ns);
		g_np = 0;
		g_priv = 0;
		g_bad = 0;
		memset(g_pn, 0, sizeof(g_pn));
		return ;
	}
	if (g_ps == 2)
	{
		if (g_ns < (int)sizeof(g_seq) - 1)
			g_seq[g_ns++] = c;
		if (c >= '0' && c <= '9')
		{
			if (!g_np)
				g_np = 1;
			if (g_pn[g_np - 1] < 100000)
				g_pn[g_np - 1] = g_pn[g_np - 1] * 10 + (c - '0');
		}
		else if (c == ';' || c == ':')
		{
			if (!g_np)
				g_np = 1;
			if (g_np < 16)
				g_pn[g_np++] = 0;
		}
		else if (c >= 0x3C && c <= 0x3F)
			g_priv = 1;
		else if (c >= 0x20 && c <= 0x2F)
			g_bad = 1;
		else
		{
			g_ps = 0;
			if (c >= 0x40 && c <= 0x7E)
				vs_csi(c);
		}
		return ;
	}
	if (c == 27)
	{
		g_ps = 1;
		g_ns = 0;
		g_seq[g_ns++] = c;
	}
	else if (c == '\n')
	{
		g_cx = 0;
		if (g_cy < g_vh - 1)
			g_cy++;
	}
	else if (c == '\r')
		g_cx = 0;
	else if (c == '\b')
		g_cx -= g_cx > 0;
	else if (c == '\t')
		g_cx = (g_cx + 8) & ~7;
	else if (c == '\a')
		rawn("\a", 1);
	else if (c < 32)
		return ;
	else if (c < 0x80)
		vs_put(c, 1);
	else if ((c & 0xC0) == 0x80)
	{
		if (!g_uneed)
			return ;
		g_u |= (uint32_t)c << (8 * g_un++);
		g_ucp = g_ucp << 6 | (c & 0x3F);
		if (g_un == g_uneed)
		{
			g_uneed = 0;
			vs_put(g_u, cp_width(g_ucp));
		}
	}
	else
	{
		g_uneed = c >= 0xF0 ? 4 : (c >= 0xE0 ? 3 : 2);
		g_ucp = c & (c >= 0xF0 ? 0x07 : (c >= 0xE0 ? 0x0F : 0x1F));
		g_u = c;
		g_un = 1;
	}
}

static int	col_seq(char *b, uint32_t c, int bg)
{
	int	v;

	v = c & 255;
	if (c >> 24 == 1)
		return (sprintf(b, ";%d;2;%d;%d;%d", bg ? 48 : 38, (c >> 16) & 255, (c >> 8) & 255, v));
	if (c >> 24 == 2)
		return (sprintf(b, ";%d;5;%d", bg ? 48 : 38, v));
	if (c >> 24 == 3)
		return (sprintf(b, ";%d", (v < 8 ? 30 + v : 82 + v) + (bg ? 10 : 0)));
	return (sprintf(b, ";%d", bg ? 49 : 39));
}

static void	sync_begin(void)
{
	if (!g_sync)
		rawn("\033[?2026h", 8);
	g_sync = 1;
}

static void	emit_cell(int x, int y, const t_cell *c)
{
	char		b[96];
	int			n;
	int			i;
	uint32_t	v;

	sync_begin();
	if (g_ry != y || g_rx != x)
		rawn(b, sprintf(b, "\033[%d;%dH", y + 1, x + 1));
	n = 0;
	if (!g_rp_ok || g_rp.at != c->at)
	{
		n = sprintf(b, ";0");
		for (i = 0; i < 6; i++)
			if (c->at & (1 << i))
				n += sprintf(b + n, ";%d", i == 5 ? 7 : i + 1);
		n += col_seq(b + n, c->fg, 0);
		n += col_seq(b + n, c->bg, 1);
	}
	else
	{
		if (g_rp.fg != c->fg)
			n += col_seq(b + n, c->fg, 0);
		if (g_rp.bg != c->bg)
			n += col_seq(b + n, c->bg, 1);
	}
	if (n)
	{
		rawn("\033[", 2);
		rawn(b + 1, n - 1);
		rawn("m", 1);
	}
	g_rp = *c;
	g_rp_ok = 1;
	n = 0;
	for (v = c->ch; v && n < 4; v >>= 8)
		b[n++] = v & 255;
	if (!n)
		b[n++] = ' ';
	rawn(b, n);
	g_ry = y;
	g_rx = x + (c->w == 2 ? 2 : 1);
	if (g_rx >= g_vw)
		g_rx = -1;
}

static void	vs_present(const t_cell *src)
{
	int	x;
	int	y;
	int	i;

	if (g_inval)
	{
		sync_begin();
		rawn("\033[0m\033[2J", 8);
		g_rp_ok = 0;
		g_rx = -1;
		for (i = 0; i < g_vw * g_vh; i++)
			g_fr[i] = blank(0);
		g_inval = 0;
	}
	for (y = 0; y < g_vh; y++)
		for (x = 0; x < g_vw; x++)
		{
			i = y * g_vw + x;
			if (cell_eq(&src[i], &g_fr[i]))
				continue ;
			if (src[i].w == 0)
			{
				if (x > 0 && src[i - 1].w == 2)
				{
					emit_cell(x - 1, y, &src[i - 1]);
					g_fr[i - 1] = src[i - 1];
				}
				g_fr[i] = src[i];
				continue ;
			}
			emit_cell(x, y, &src[i]);
			g_fr[i] = src[i];
			if (src[i].w == 2 && x + 1 < g_vw)
				g_fr[i + 1] = src[i + 1];
		}
	if (g_sync)
		rawn("\033[?2026l", 8);
	g_sync = 0;
	raw_flush();
}

void	term_invalidate(void)
{
	g_inval = 1;
}

static void	size_guard(void)
{
	char	b[400];
	long	t0;
	int		c;
	int		lc;
	int		lr;
	int		y;

	if (g_cols >= MINW && g_rows >= MINH)
		return ;
	t0 = real_ms();
	lc = 0;
	lr = 0;
	while (g_cols < MINW || g_rows < MINH)
	{
		if (lc != g_cols || lr != g_rows)
		{
			lc = g_cols;
			lr = g_rows;
			y = lr / 2 > 1 ? lr / 2 - 1 : 1;
			c = snprintf(b, sizeof(b), "\033[0m\033[2J\033[%d;1H\033[1;38;5;220m  %s\033[0m\033[%d;1H"
					"  %d x %d minimum  (%s %d x %d)\033[%d;1H\033[2m  %s\033[0m", y,
					L("AGRANDIS TON TERMINAL", "MAKE YOUR TERMINAL BIGGER"), y + 1, MINW, MINH,
					L("actuel :", "now:"), lc, lr, y + 2,
					L("le testeur reprend tout seul  ·  Q : quitter", "the tester resumes by itself  ·  Q: quit"));
			(void)!write(1, b, c);
		}
		term_poll();
		c = read_byte(120);
		if (c == 3 || c == 'q' || c == 'Q')
		{
			g_quit_requested = SIGINT;
			term_poll();
		}
		size_refresh();
	}
	g_paused += real_ms() - t0;
	g_inval = 1;
}

void	term_resume(void)
{
	struct termios	t;
	const char		*e;

	t = g_old;
	t.c_lflag &= ~(ICANON | ECHO | ISIG);
	t.c_iflag &= ~(IXON | ICRNL);
	t.c_cc[VMIN] = 0;
	t.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &t);
	g_raw = 1;
	g_inval = 1;
	g_len = 0;
	e = getenv("TDF_COLORS");
	if (!e && getenv("TERM") && (!strcmp(getenv("TERM"), "linux") || !strncmp(getenv("TERM"), "vt", 2)))
		e = "16";
	if (!e && getenv("TERM_PROGRAM") && !strcmp(getenv("TERM_PROGRAM"), "Apple_Terminal"))
		e = "256";
	g_col16 = e && !strcmp(e, "16");
	if (e && !strcmp(e, "256"))
		g_save.cfg.col256 = 1;
	(void)!write(1, SEQ_ENTER, sizeof(SEQ_ENTER) - 1);
}

void	term_init(void)
{
	struct sigaction	sa;

	tcgetattr(0, &g_old);
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = on_signal;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
	term_resume();
}

void	term_pause(void)
{
	if (!g_raw)
		return ;
	g_len = 0;
	g_on = 0;
	(void)!write(1, SEQ_LEAVE, sizeof(SEQ_LEAVE) - 1);
	tcsetattr(0, TCSANOW, &g_old);
	g_raw = 0;
}

void	term_end(void)
{
	term_pause();
}

void	term_fast(int on)
{
	g_fast = on;
}

void	sleep_ms(int ms)
{
	struct timespec	t;

	term_poll();
	if (ms <= 0)
		return ;
	t.tv_sec = ms / 1000;
	t.tv_nsec = (ms % 1000) * 1000000L;
	nanosleep(&t, NULL);
	term_poll();
}

static int	read_byte(int timeout_ms)
{
	fd_set			s;
	struct timeval	tv;
	unsigned char	c;

	FD_ZERO(&s);
	FD_SET(0, &s);
	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;
	if (select(1, &s, NULL, NULL, &tv) <= 0)
		return (-1);
	if (read(0, &c, 1) != 1)
		return (-1);
	return (c);
}

int	key_read(int timeout_ms)
{
	int	c;
	int	d;

	term_poll();
	if (g_raw && !g_forked && g_bk && size_refresh())
	{
		size_guard();
		vs_fit();
		vs_present(g_bk);
	}
	c = read_byte(timeout_ms < 0 ? 0 : timeout_ms);
	term_poll();
	if (c == 3)
	{
		if (g_ctrlc_at && now_ms() - g_ctrlc_at < 450)
		{
			if (g_forked)
			{
				kill(getppid(), SIGINT);
				_exit(130);
			}
			g_quit_requested = SIGINT;
			term_poll();
		}
		g_ctrlc_at = now_ms();
		return (27);
	}
	if (c != 27)
		return (c);
	d = read_byte(15);
	if (d != '[' && d != 'O')
		return (27);
	d = read_byte(15);
	if (d == 'A')
		return (1000);
	if (d == 'B')
		return (1001);
	if (d == 'C')
		return (1002);
	if (d == 'D')
		return (1003);
	while (d >= 0 && !(d >= 64 && d <= 126))
		d = read_byte(5);
	return (-1);
}

static void	drain(void)
{
	size_t	i;

	if (g_raw && !g_forked)
	{
		if (!g_bk)
		{
			size_refresh();
			vs_fit();
		}
		for (i = 0; g_bk && i < g_len; i++)
			vs_byte((unsigned char)g_buf[i]);
		if (g_bk)
		{
			g_len = 0;
			return ;
		}
	}
	rawn(g_buf, g_len > sizeof(g_ob) ? 0 : g_len);
	if (g_len > sizeof(g_ob))
		(void)!write(1, g_buf, g_len);
	raw_flush();
	g_len = 0;
}

void	outn(const char *s, size_t n)
{
	if (g_len + n >= sizeof(g_buf))
		drain();
	if (n >= sizeof(g_buf))
		return ;
	memcpy(g_buf + g_len, s, n);
	g_len += n;
}

void	out(const char *s)
{
	outn(s, strlen(s));
}

void	outf(const char *fmt, ...)
{
	char	tmp[2048];
	va_list	ap;
	int		n;

	va_start(ap, fmt);
	n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
	va_end(ap);
	if (n > 0)
		outn(tmp, (size_t)n < sizeof(tmp) ? (size_t)n : sizeof(tmp) - 1);
}

void	flush(void)
{
	if (g_raw && !g_forked)
	{
		size_refresh();
		size_guard();
		vs_fit();
	}
	drain();
	if (!g_raw || g_forked || !g_bk)
		return ;
	vs_present(g_bk);
}

void	frame(void)
{
	drain();
	if (!g_raw || g_forked || !g_bk)
		return ;
	vs_erase(0, g_vw * g_vh);
	g_cx = 0;
	g_cy = 0;
}

void	at(int x, int y)
{
	outf("\033[%d;%dH", y + 1, x + 1);
}

int	color_seq(char *buf, t_rgb c, int bg)
{
	int	i;

	if (g_col16)
	{
		i = (c.r > 110) | (c.g > 110) << 1 | (c.b > 110) << 2;
		if (c.r > 200 || c.g > 200 || c.b > 200)
			i += 60;
		return (sprintf(buf, "\033[%dm", (bg ? 40 : 30) + i));
	}
	if (g_save.cfg.col256)
	{
		i = 16 + 36 * ((c.r * 5 + 127) / 255) + 6 * ((c.g * 5 + 127) / 255)
			+ (c.b * 5 + 127) / 255;
		return (sprintf(buf, "\033[%d;5;%dm", bg ? 48 : 38, i));
	}
	return (sprintf(buf, "\033[%d;2;%d;%d;%dm", bg ? 48 : 38, c.r, c.g, c.b));
}

void	fg(t_rgb c)
{
	char	b[32];

	outn(b, color_seq(b, c, 0));
}

void	bgc(t_rgb c)
{
	char	b[32];

	outn(b, color_seq(b, c, 1));
}

void	bell(void)
{
	if (g_save.cfg.sound)
	{
		out("\a");
		flush();
	}
}

int	anim_off(void)
{
	return (g_fast || g_save.cfg.anim == 2);
}

int	anim_ms(int ms)
{
	if (g_fast || g_save.cfg.anim == 2)
		return (0);
	if (g_save.cfg.anim == 1)
		return (ms * 2 / 5);
	return (ms);
}

int	key_dir(int k)
{
	if (k == 1000 || k == 1001 || k == 1002 || k == 1003)
		return (k);
	if (g_save.cfg.keys == 2)
	{
		if (k == 'k')
			return (1000);
		if (k == 'j')
			return (1001);
		if (k == 'l')
			return (1002);
		if (k == 'h')
			return (1003);
		return (k);
	}
	if (k == (g_save.cfg.keys ? 'w' : 'z'))
		return (1000);
	if (k == 's')
		return (1001);
	if (k == 'd')
		return (1002);
	if (k == (g_save.cfg.keys ? 'a' : 'q'))
		return (1003);
	return (k);
}

void	reset(void)
{
	out("\033[0m");
}

void	cls(void)
{
	reset();
	bgc(theme()->bg);
	frame();
}

t_rgb	rgb(int r, int g, int b)
{
	t_rgb	c;

	c.r = r < 0 ? 0 : (r > 255 ? 255 : r);
	c.g = g < 0 ? 0 : (g > 255 ? 255 : g);
	c.b = b < 0 ? 0 : (b > 255 ? 255 : b);
	return (c);
}

t_rgb	mix(t_rgb a, t_rgb b, double t)
{
	if (t < 0)
		t = 0;
	if (t > 1)
		t = 1;
	return (rgb(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t,
			a.b + (b.b - a.b) * t));
}

t_rgb	hue(double h)
{
	h = fmod(h, 1.0);
	if (h < 0)
		h += 1.0;
	return (rgb(127 + 127 * sin(6.2831 * h), 127 + 127 * sin(6.2831 * (h
					+ 0.333)), 127 + 127 * sin(6.2831 * (h + 0.666))));
}

int	ulen(const char *s)
{
	int	n;

	n = 0;
	for (; *s; s++)
		if (((unsigned char)*s & 0xC0) != 0x80)
			n++;
	return (n);
}

void	text_clip(int x, int y, t_rgb c, const char *s, int maxw)
{
	char	buf[1024];
	int		cols;
	size_t	i;
	size_t	cut;

	if (maxw <= 0)
		return ;
	if (ulen(s) <= maxw)
	{
		text_at(x, y, c, s);
		return ;
	}
	cols = 0;
	cut = 0;
	for (i = 0; s[i] && i < sizeof(buf) - 4; i++)
	{
		if (((unsigned char)s[i] & 0xC0) != 0x80)
		{
			if (cols == maxw - 1)
				break ;
			cols++;
		}
		cut = i + 1;
	}
	memcpy(buf, s, cut);
	memcpy(buf + cut, "…", 4);
	text_at(x, y, c, buf);
}

int	text_wrap(int x, int y, int maxy, int w, t_rgb c, const char *s)
{
	char		buf[1024];
	const char	*p;
	const char	*e;
	const char	*sp;
	int			cols;

	if (w < 1)
		return (y);
	p = s;
	while (*p && y < maxy)
	{
		cols = 0;
		sp = NULL;
		e = p;
		while (*e && *e != '\n' && (cols < w || (((unsigned char)*e & 0xC0) == 0x80)))
		{
			if (*e == ' ')
				sp = e;
			if (((unsigned char)*e & 0xC0) != 0x80)
				cols++;
			e++;
		}
		if (*e && *e != '\n' && *e != ' ' && sp && sp > p)
			e = sp;
		snprintf(buf, sizeof(buf), "%.*s", (int)(e - p), p);
		for (sp = buf; *sp; sp++)
			if (*sp == '\t')
				*(char *)sp = ' ';
		text_at(x, y++, c, buf);
		p = e;
		if (*p == ' ' || *p == '\n')
			p++;
	}
	return (y);
}

void	text_at(int x, int y, t_rgb c, const char *s)
{
	at(x, y);
	fg(c);
	out(s);
}

void	center(int y, t_rgb c, const char *s)
{
	int	x;

	x = (term_cols() - ulen(s)) / 2;
	if (x < 0)
		text_clip(0, y, c, s, term_cols());
	else
		text_at(x, y, c, s);
}

void	box(int x, int y, int w, int h, t_rgb c)
{
	int	i;

	fg(c);
	at(x, y);
	out("╭");
	for (i = 0; i < w - 2; i++)
		out("─");
	out("╮");
	for (i = 1; i < h - 1; i++)
	{
		at(x, y + i);
		out("│");
		at(x + w - 1, y + i);
		out("│");
	}
	at(x, y + h - 1);
	out("╰");
	for (i = 0; i < w - 2; i++)
		out("─");
	out("╯");
}

void	fill(int x, int y, int w, int h, t_rgb c)
{
	int	i;

	bgc(c);
	for (i = 0; i < h; i++)
	{
		at(x, y + i);
		outf("%*s", w, "");
	}
	bgc(theme()->bg);
}

void	bar(int x, int y, int w, double v, t_rgb a, t_rgb b)
{
	int		i;
	double	f;

	if (v < 0)
		v = 0;
	if (v > 1)
		v = 1;
	at(x, y);
	for (i = 0; i < w; i++)
	{
		f = (double)i / (w > 1 ? w - 1 : 1);
		if (i < (int)(v * w))
		{
			fg(mix(a, b, f));
			out("█");
		}
		else if (i == (int)(v * w) && v * w - (int)(v * w) > 0.5)
		{
			fg(mix(a, b, f));
			out("▌");
		}
		else
		{
			fg(mix(theme()->dim, theme()->bg, 0.5));
			out("░");
		}
	}
}
