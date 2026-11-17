#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "fous.h"

#define NORME_MAX 400

typedef struct s_nerr
{
	char	fich[64];
	int		ligne;
	int		col;
	char	regle[40];
	char	msg[160];
}	t_nerr;

static t_nerr	g_ne[NORME_MAX];
static int		g_nne;
static int		g_nfich;
static char		g_npath[700];
static long		g_nmtime = -1;
static int		g_nlu;

static void	norme_charger(const char *path)
{
	struct stat	st;
	FILE		*f;
	char		l[600];
	char		fich[64];
	char		*p;
	t_nerr		*e;

	if (!path || stat(path, &st) != 0)
	{
		g_nlu = 0;
		g_nne = 0;
		g_nfich = 0;
		g_npath[0] = '\0';
		return ;
	}
	if (g_nlu && !strcmp(path, g_npath) && (long)st.st_mtime == g_nmtime)
		return ;
	snprintf(g_npath, sizeof(g_npath), "%s", path);
	g_nmtime = (long)st.st_mtime;
	g_nlu = 1;
	g_nne = 0;
	g_nfich = 0;
	fich[0] = '\0';
	f = fopen(path, "r");
	while (f && fgets(l, sizeof(l), f))
	{
		l[strcspn(l, "\r\n")] = '\0';
		if ((p = strstr(l, ": Error!")) && p[8] == '\0')
		{
			*p = '\0';
			snprintf(fich, sizeof(fich), "%s", l);
			g_nfich++;
			continue ;
		}
		if (strncmp(l, "Error:", 6) || g_nne >= NORME_MAX)
			continue ;
		e = &g_ne[g_nne++];
		memset(e, 0, sizeof(*e));
		snprintf(e->fich, sizeof(e->fich), "%s", fich);
		p = l + 6;
		while (*p == ' ')
			p++;
		sscanf(p, "%39s", e->regle);
		if ((p = strstr(l, "line:")))
			e->ligne = atoi(p + 5);
		if ((p = strstr(l, "col:")))
			e->col = atoi(p + 4);
		p = strchr(l, ')');
		p = p ? p + 1 : l + 6;
		while (*p == ':' || *p == ' ' || *p == '\t')
			p++;
		snprintf(e->msg, sizeof(e->msg), "%s", p);
	}
	if (f)
		fclose(f);
}

int	tdf_norme_compte(const char *path, int *nfich)
{
	norme_charger(path);
	if (nfich)
		*nfich = g_nfich;
	return (g_nlu ? g_nne : -1);
}

int	tdf_norme_de(const char *path, const char *fichier, char out[][200], int max)
{
	int	i;
	int	n;

	norme_charger(path);
	for (n = 0, i = 0; i < g_nne && n < max; i++)
		if (!strcmp(g_ne[i].fich, fichier))
			snprintf(out[n++], 200, L("Norme · ligne %d, col %d · %s · %s", "Norm · line %d, col %d · %s · %s"),
				g_ne[i].ligne, g_ne[i].col, g_ne[i].regle, g_ne[i].msg);
	return (n);
}

void	tdf_norme_voir(const char *path, const char *projet)
{
	char	tmp[400];
	int		top;
	int		h;
	int		n;
	int		i;
	int		k;
	int		x;
	int		y;
	int		w;
	int		r;

	top = 0;
	cls();
	while (1)
	{
		norme_charger(path);
		w = term_cols() - 6 < 130 ? term_cols() - 6 : 130;
		x = (term_cols() - w) / 2;
		bgc(theme()->bg);
		frame();
		snprintf(tmp, sizeof(tmp), L("N O R M I N E T T E   ·   %s", "N O R M I N E T T E   ·   %s"), projet);
		center(1, theme()->accent2, tmp);
		if (!g_nlu)
			snprintf(tmp, sizeof(tmp), "%s", L("La norminette n'a pas tourné pour ce projet (introuvable ou désactivée).",
					"The norminette did not run for this project (not found or turned off)."));
		else if (!g_nne)
			snprintf(tmp, sizeof(tmp), "%s", L("Aucune erreur : la Norme est respectée.", "No error: the Norm is respected."));
		else
			snprintf(tmp, sizeof(tmp), L("%d erreur(s) dans %d fichier(s) · la moulinette vérifie la Norme avant les tests (Prerequisites)",
					"%d error(s) in %d file(s) · the moulinette checks the Norm before the tests (Prerequisites)"), g_nne, g_nfich);
		center(3, !g_nlu ? theme()->dim : g_nne ? rgb(255, 110, 110) : rgb(120, 230, 150), tmp);
		h = term_rows() - 8;
		if (h < 3)
			h = 3;
		n = 0;
		y = 5;
		for (i = 0, r = 0; i < g_nne && y < 5 + h; i++)
		{
			if (i == 0 || strcmp(g_ne[i].fich, g_ne[i - 1].fich))
			{
				if (r++ >= top)
				{
					snprintf(tmp, sizeof(tmp), "✘ %s", g_ne[i].fich[0] ? g_ne[i].fich : "?");
					text_at(x, y++, rgb(255, 120, 120), tmp);
				}
			}
			if (y >= 5 + h)
				break ;
			if (r++ < top)
				continue ;
			snprintf(tmp, sizeof(tmp), L("  ligne %-4d col %-3d", "  line %-4d col %-3d"), g_ne[i].ligne, g_ne[i].col);
			text_at(x, y, theme()->dim, tmp);
			text_clip(x + 22, y, rgb(255, 215, 120), g_ne[i].regle, 28);
			text_clip(x + 51, y++, theme()->text, g_ne[i].msg, w - 51);
			n++;
		}
		foot_line(x, L("▲ ▼ défiler   Échap ou Entrée : retour", "▲ ▼ scroll   Esc or Enter: back"));
		flush();
		k = key_dir(key_read(200));
		if (k == 1000 && top > 0)
			top--;
		else if (k == 1001 && n > 0 && y >= 5 + h)
			top++;
		else if (k == '\r' || k == '\n' || key_quit(k) || k == 127 || k == 8)
			break ;
		if (k > 0)
			cls();
	}
	cls();
}
