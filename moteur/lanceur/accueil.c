#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include "fous.h"

#define MAXI 24

static struct termios	g_old;
static int				g_brut;

static long	maintenant(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000L + tv.tv_usec / 1000);
}

static void	restaurer(void)
{
	if (!g_brut)
		return ;
	tcsetattr(0, TCSANOW, &g_old);
	(void)!write(1, "\033[0m\033[?25h\033[?1049l", 19);
	g_brut = 0;
}

static void	sur_signal(int s)
{
	restaurer();
	signal(s, SIG_DFL);
	raise(s);
}

static int	touche(int ms)
{
	fd_set			fs;
	struct timeval	tv;
	unsigned char	c[4];
	int				n;

	FD_ZERO(&fs);
	FD_SET(0, &fs);
	tv.tv_sec = 0;
	tv.tv_usec = ms * 1000;
	if (select(1, &fs, NULL, NULL, &tv) <= 0)
		return (-1);
	n = read(0, c, 1);
	if (n != 1)
		return (-2);
	if (c[0] != 27)
		return (c[0]);
	FD_ZERO(&fs);
	FD_SET(0, &fs);
	tv.tv_sec = 0;
	tv.tv_usec = 30000;
	if (select(1, &fs, NULL, NULL, &tv) <= 0 || read(0, c + 1, 2) != 2)
		return (27);
	if (c[1] != '[' && c[1] != 'O')
		return (0);
	return (c[2] == 'A' ? 1000 : c[2] == 'B' ? 1001 : c[2] == 'C' ? 1002 : c[2] == 'D' ? 1003 : 0);
}

typedef struct s_acc
{
	const char	*l1;
	const char	*l2;
	const char	*sous;
	const char	*aide;
	const char	*items[MAXI];
	int			n;
	int			sel;
}	t_acc;

static int	largeur_utf8(const char *s, int n)
{
	int	w;
	int	i;

	w = 0;
	for (i = 0; i < n && s[i]; i++)
		if ((s[i] & 0xC0) != 0x80)
			w++;
	return (w);
}

static void	ecrire_ligne(int y, int x, const char *s, int n)
{
	printf("\033[%d;%dH%.*s", y, x, n, s);
}

static void	dessiner(t_acc *a, int haut)
{
	int			cols;
	int			rows;
	int			y;
	int			i;
	int			lw;
	int			x;
	const char	*bar;
	const char	*d;

	entete_taille(&cols, &rows);
	printf("\033[0m\033[2J");
	y = haut + 2;
	lw = 0;
	for (i = 0; i < a->n; i++)
	{
		bar = strchr(a->items[i], '|');
		x = largeur_utf8(a->items[i], bar ? (int)(bar - a->items[i]) : (int)strlen(a->items[i]));
		if (x > lw)
			lw = x;
	}
	x = (cols - lw - 4) / 2;
	if (x < 2)
		x = 2;
	if (a->sous && *a->sous)
	{
		printf("\033[%d;%dH\033[2m%s\033[0m", y, x, a->sous);
		y += 2;
	}
	for (i = 0; i < a->n; i++)
	{
		bar = strchr(a->items[i], '|');
		if (i == a->sel)
			printf("\033[%d;%dH\033[1;36m\xe2\x9d\xaf %.*s\033[0m", y + i, x, bar ? (int)(bar - a->items[i]) : (int)strlen(a->items[i]), a->items[i]);
		else
			printf("\033[%d;%dH  %.*s", y + i, x, bar ? (int)(bar - a->items[i]) : (int)strlen(a->items[i]), a->items[i]);
	}
	bar = strchr(a->items[a->sel], '|');
	d = bar ? bar + 1 : "";
	printf("\033[%d;%dH\033[2m", y + a->n + 1, x);
	ecrire_ligne(y + a->n + 1, x, d, (int)strlen(d));
	printf("\033[0m\033[%d;%dH\033[2m%s\033[0m", rows, x, a->aide);
	fflush(stdout);
}

static int	reste(t_acc *a)
{
	return (a->n + 8 + ((a->sous && *a->sous) ? 2 : 0));
}

static int	boucle(t_acc *a)
{
	long	t0;
	int		k;
	int		haut;
	int		cols;
	int		rows;
	int		pc;
	int		pr;

	t0 = maintenant();
	haut = entete_hauteur(a->l1, a->l2, reste(a));
	dessiner(a, haut);
	entete_taille(&pc, &pr);
	while (1)
	{
		if (haut)
			entete_dessiner(a->l1, a->l2, maintenant() - t0, 2, reste(a));
		k = touche(40);
		entete_taille(&cols, &rows);
		if (cols != pc || rows != pr)
		{
			pc = cols;
			pr = rows;
			haut = entete_hauteur(a->l1, a->l2, reste(a));
			dessiner(a, haut);
		}
		if (k == -1)
			continue ;
		if (k == -2 || k == 'q' || k == 'Q' || k == 27 || k == 1003)
			return (255);
		if (k == '\r' || k == '\n' || k == ' ' || k == 1002)
			return (a->sel);
		if (k == 1000 || k == 'z' || k == 'w' || k == 'k')
			a->sel = (a->sel + a->n - 1) % a->n;
		else if (k == 1001 || k == 's' || k == 'j')
			a->sel = (a->sel + 1) % a->n;
		else if (k >= '1' && k <= '9' && k - '1' < a->n)
			a->sel = k - '1';
		else
			continue ;
		dessiner(a, haut);
	}
}

int	main(int ac, char **av)
{
	struct termios	t;
	t_acc			a;
	int				i;
	int				r;

	memset(&a, 0, sizeof(a));
	a.l1 = "TESTEUR";
	a.l2 = "DES FOUS";
	a.aide = "\xe2\x86\x91 \xe2\x86\x93 choisir   Entr\xc3\xa9" "e valider   \xc3\x89" "chap ou q : retour";
	for (i = 1; i < ac; i++)
	{
		if (!strcmp(av[i], "--titre") && i + 2 < ac)
		{
			a.l1 = av[++i];
			a.l2 = av[++i];
		}
		else if (!strcmp(av[i], "--sous") && i + 1 < ac)
			a.sous = av[++i];
		else if (!strcmp(av[i], "--aide") && i + 1 < ac)
			a.aide = av[++i];
		else if (!strcmp(av[i], "--choix") && i + 1 < ac)
			a.sel = atoi(av[++i]);
		else if (a.n < MAXI)
			a.items[a.n++] = av[i];
	}
	if (a.n == 0 || !isatty(0) || !isatty(1))
		return (254);
	if (a.sel < 0 || a.sel >= a.n)
		a.sel = 0;
	tcgetattr(0, &g_old);
	t = g_old;
	t.c_lflag &= ~(ICANON | ECHO);
	t.c_cc[VMIN] = 1;
	t.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &t);
	g_brut = 1;
	signal(SIGINT, sur_signal);
	signal(SIGTERM, sur_signal);
	signal(SIGHUP, sur_signal);
	(void)!write(1, "\033[?1049h\033[?25l", 14);
	r = boucle(&a);
	restaurer();
	return (r);
}
