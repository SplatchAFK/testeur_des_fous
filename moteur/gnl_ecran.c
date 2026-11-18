#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

#define GMAX 160

typedef struct s_gl
{
	int		section;
	char	st[12];
	char	titre[200];
	char	detail[300];
}	t_gl;

static t_gl	g_gl[GMAX];
static int	g_ngl;
static char	g_gsec[24][120];
static int	g_ngsec;
static char	g_gfatal[200];
static int	g_gok;
static int	g_gko;
static int	g_ginfo;
static char	g_gtailles[120];
static char	g_gprojet[300];

static void	gnl_lire(const char *path)
{
	FILE	*f;
	char	l[1200];
	char	*c[6];
	char	*p;
	int		n;

	g_ngl = 0;
	g_ngsec = 0;
	g_gfatal[0] = '\0';
	g_gok = -1;
	f = fopen(path, "r");
	while (f && fgets(l, sizeof(l), f))
	{
		l[strcspn(l, "\r\n")] = '\0';
		for (n = 0, p = l; n < 6; n++)
		{
			c[n] = p;
			p = p ? strchr(p, '\t') : NULL;
			if (p)
				*p++ = '\0';
		}
		if (!strcmp(c[0], "S") && c[1] && g_ngsec < 24)
			snprintf(g_gsec[g_ngsec++], 120, "%s", c[1]);
		else if (!strcmp(c[0], "L") && c[1] && c[2] && g_ngl < GMAX)
		{
			g_gl[g_ngl].section = g_ngsec - 1;
			snprintf(g_gl[g_ngl].st, 12, "%s", c[1]);
			snprintf(g_gl[g_ngl].titre, 200, "%s", c[2]);
			snprintf(g_gl[g_ngl].detail, 300, "%s", c[3] ? c[3] : "");
			g_ngl++;
		}
		else if (!strcmp(c[0], "X") && c[1])
			snprintf(g_gfatal, sizeof(g_gfatal), "%s", c[1]);
		else if (!strcmp(c[0], "F") && c[1] && c[2] && c[3])
		{
			g_gok = atoi(c[1]);
			g_gko = atoi(c[2]);
			g_ginfo = atoi(c[3]);
			snprintf(g_gtailles, sizeof(g_gtailles), "%s", c[4] ? c[4] : "");
			snprintf(g_gprojet, sizeof(g_gprojet), "%s", c[5] ? c[5] : "");
		}
	}
	if (f)
		fclose(f);
}

static t_rgb	gnl_couleur(const char *st)
{
	if (!strcmp(st, "OK"))
		return (rgb(120, 230, 150));
	if (!strcmp(st, "INFO") || !strcmp(st, "NOTE"))
		return (rgb(255, 215, 120));
	return (rgb(255, 110, 110));
}

static const char	*gnl_sens(const char *st)
{
	if (!strcmp(st, "OK"))
		return (L("Tout est bon pour cette vérification.", "Everything is fine for this check."));
	if (!strcmp(st, "INFO") || !strcmp(st, "NOTE"))
		return (L("Pour info : ce point ne compte pas dans le résultat.", "For info: this point does not count in the result."));
	if (!strcmp(st, "CRASH"))
		return (L("Plantage (segfault, abort...) : active le mode debug (Tester mon projet › Options du test) pour voir la trace complète.",
				"Crash (segfault, abort...): turn on debug mode (Test my project › Test options) to see the full trace."));
	if (!strcmp(st, "FUITE"))
		return (L("Fuite mémoire : un malloc n'est jamais libéré.", "Memory leak: a malloc is never freed."));
	if (!strcmp(st, "BOUCLE"))
		return (L("Boucle infinie ou bien trop lent : le temps limite est dépassé.", "Infinite loop or far too slow: the time limit is exceeded."));
	return (L("Résultat faux : à corriger.", "Wrong result: to fix."));
}

static void	gnl_anneaux(int x, int y)
{
	char	b[64];
	char	p[700];
	FILE	*f;
	int		i;

	snprintf(p, sizeof(p), "%s/puits.txt", g_savedir);
	b[0] = '\0';
	f = fopen(p, "r");
	if (f && fgets(b, sizeof(b), f))
		b[strcspn(b, "\r\n")] = '\0';
	if (f)
		fclose(f);
	if (!b[0] || getenv("TDF_SANS_STATS"))
		return ;
	text_at(x, y, theme()->dim, L("Le Puits :", "The Well:"));
	at(x + 12, y);
	for (i = 0; b[i] && i < 22; i++)
	{
		fg(b[i] == 'O' ? rgb(118, 222, 238) : rgb(220, 70, 60));
		out("●");
		out(" ");
	}
}

void	tdf_ecran_gnl(const char *path)
{
	char	tmp[400];
	char	np[700];
	int		sel;
	int		top;
	int		h;
	int		lw;
	int		w;
	int		x;
	int		y;
	int		i;
	int		k;
	int		nb;
	int		nf;
	int		sec;

	gnl_lire(path);
	snprintf(np, sizeof(np), "%s/derniere/norme_gnl.txt", g_savedir);
	sel = 0;
	for (i = 0; i < g_ngl; i++)
		if (strcmp(g_gl[i].st, "OK") && strcmp(g_gl[i].st, "INFO") && strcmp(g_gl[i].st, "NOTE"))
		{
			sel = i;
			break ;
		}
	top = 0;
	tdf_key_vider();
	cls();
	while (1)
	{
		w = term_cols() - 4 < 130 ? term_cols() - 4 : 130;
		x = (term_cols() - w) / 2;
		lw = w * 55 / 100;
		bgc(theme()->bg);
		frame();
		if (g_gok < 0)
			snprintf(tmp, sizeof(tmp), "%s", L("RÉSULTATS GET_NEXT_LINE · test interrompu", "GET_NEXT_LINE RESULTS · test interrupted"));
		else
			snprintf(tmp, sizeof(tmp), L("RÉSULTATS GET_NEXT_LINE  ·  %d OK  ·  %d en échec  ·  %d pour info",
					"GET_NEXT_LINE RESULTS  ·  %d OK  ·  %d failing  ·  %d for info"), g_gok, g_gko, g_ginfo);
		center(1, g_gok >= 0 && g_gko == 0 && !g_gfatal[0] ? rgb(120, 230, 150) : rgb(255, 110, 110), tmp);
		if (g_gfatal[0])
			center(2, rgb(255, 110, 110), g_gfatal);
		else if (g_gtailles[0])
		{
			snprintf(tmp, sizeof(tmp), L("BUFFER_SIZE testés : %s, et sans -D", "BUFFER_SIZE tested: %s, and without -D"), g_gtailles);
			center(2, theme()->dim, tmp);
		}
		gnl_anneaux(x, 3);
		nb = tdf_norme_compte(np, &nf);
		if (nb > 0)
			snprintf(tmp, sizeof(tmp), L("NORMINETTE : %d erreur(s) dans %d fichier(s) · n pour les voir",
					"NORMINETTE: %d error(s) in %d file(s) · n to see them"), nb, nf);
		else
			snprintf(tmp, sizeof(tmp), "%s", nb == 0 ? L("norminette : aucune erreur", "norminette: no error")
				: L("norminette : pas lancée", "norminette: not run"));
		text_at(x, 4, nb > 0 ? rgb(255, 110, 110) : nb == 0 ? rgb(120, 230, 150) : theme()->dim, tmp);
		h = term_rows() - 9;
		if (h < 4)
			h = 4;
		if (sel < top)
			top = sel;
		if (sel >= top + h - 4)
			top = sel - h + 5;
		if (top < 0)
			top = 0;
		y = 6;
		sec = top > 0 ? g_gl[top - 1].section : -2;
		for (i = top; i < g_ngl && y < 6 + h; i++)
		{
			if (g_gl[i].section != sec)
			{
				sec = g_gl[i].section;
				if (sec >= 0)
				{
					if (y > 6)
						y++;
					if (y >= 6 + h)
						break ;
					text_clip(x, y++, theme()->accent2, g_gsec[sec], lw);
					if (y >= 6 + h)
						break ;
				}
			}
			at(x, y);
			if (i == sel)
				bgc(mix(theme()->bg, theme()->accent, 0.22));
			fg(gnl_couleur(g_gl[i].st));
			outf(" %s %-6s ", i == sel ? "▶" : " ", g_gl[i].st);
			bgc(theme()->bg);
			text_clip(x + 11, y++, i == sel ? theme()->accent2 : theme()->text, g_gl[i].titre, lw - 12);
		}
		if (g_ngl)
		{
			y = 6;
			text_at(x + lw + 3, y++, gnl_couleur(g_gl[sel].st), g_gl[sel].st);
			y = text_wrap(x + lw + 3, y, y + 4, w - lw - 4, theme()->text, g_gl[sel].titre) + 1;
			if (g_gl[sel].detail[0])
				y = text_wrap(x + lw + 3, y, y + 8, w - lw - 4, theme()->dim, g_gl[sel].detail) + 1;
			text_wrap(x + lw + 3, y, y + 4, w - lw - 4, gnl_couleur(g_gl[sel].st), gnl_sens(g_gl[sel].st));
		}
		else
			text_at(x, 6, theme()->dim, L("Aucun résultat : relance le test de ton get_next_line.", "No result: run the test of your get_next_line again."));
		foot_line(x, L("▲ ▼ choisir   n : norminette   Échap ou Entrée : retour au menu",
				"▲ ▼ pick   n: norminette   Esc or Enter: back to the menu"));
		flush();
		k = key_dir(key_read(200));
		if (k == 1000 && sel > 0)
			sel--;
		else if (k == 1001 && sel < g_ngl - 1)
			sel++;
		else if (k == 'n' || k == 'N')
			tdf_norme_voir(np, "get_next_line");
		else if (k == '\r' || k == '\n' || key_quit(k) || k == 127 || k == 8)
			break ;
		if (k > 0)
			cls();
	}
	cls();
}
