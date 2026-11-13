#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

#define NCASES 4096

typedef struct s_trad
{
	char	*en;
	char	*tr;
}	t_trad;

static const char	*g_codes[NLANG] = {"fr", "en", "es", "de", "it", "pt", "tr", "ru"};
static const char	*g_noms[NLANG] = {"Français", "English", "Español", "Deutsch", "Italiano",
	"Português", "Türkçe", "Русский"};
static t_trad		g_cases[NCASES];
static int			g_chargee = -1;

const char	*langue_code(int l)
{
	return (g_codes[(l >= 0 && l < NLANG) ? l : 0]);
}

const char	*langue_nom(int l)
{
	return (g_noms[(l >= 0 && l < NLANG) ? l : 0]);
}

int	langue_auto(void)
{
	const char	*v;
	int			l;

	v = getenv("LC_ALL");
	if (!v || !*v)
		v = getenv("LC_MESSAGES");
	if (!v || !*v)
		v = getenv("LANG");
	if (!v || strlen(v) < 2)
		return (0);
	for (l = 0; l < NLANG; l++)
		if (!strncmp(v, g_codes[l], 2))
			return (l);
	return (0);
}

static unsigned long	hacher(const char *s)
{
	unsigned long	h;

	h = 1469598103934665603UL;
	while (*s)
		h = (h ^ (unsigned char)*s++) * 1099511628211UL;
	return (h);
}

static int	hexa(int c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	return (-1);
}

static char	*decoder(const char *s, size_t n)
{
	char	*o;
	size_t	i;
	size_t	j;
	int		v;

	if (!(o = malloc(n + 1)))
		return (NULL);
	for (i = 0, j = 0; i < n; i++)
	{
		if (s[i] != '\\' || i + 1 >= n)
		{
			o[j++] = s[i];
			continue ;
		}
		i++;
		if (s[i] == 'n')
			o[j++] = '\n';
		else if (s[i] == 't')
			o[j++] = '\t';
		else if (s[i] == 'x' && i + 2 < n && hexa(s[i + 1]) >= 0 && hexa(s[i + 2]) >= 0)
		{
			o[j++] = (char)(hexa(s[i + 1]) * 16 + hexa(s[i + 2]));
			i += 2;
		}
		else if (s[i] >= '0' && s[i] <= '7')
		{
			for (v = 0; i < n && s[i] >= '0' && s[i] <= '7' && v < 32; i++)
				v = v * 8 + s[i] - '0';
			o[j++] = (char)v;
			i--;
		}
		else
			o[j++] = s[i];
	}
	o[j] = 0;
	return (o);
}

static void	vider(void)
{
	int	i;

	for (i = 0; i < NCASES; i++)
	{
		free(g_cases[i].en);
		free(g_cases[i].tr);
		g_cases[i].en = NULL;
		g_cases[i].tr = NULL;
	}
}

static void	formats(const char *s, char *o, size_t n)
{
	size_t	j;

	j = 0;
	while (*s && j + 1 < n)
	{
		if (*s++ != '%')
			continue ;
		while (*s && strchr("-+ #0123456789.*lhzjt", *s))
			o[j < n - 1 ? j++ : j] = *s++;
		if (*s)
			o[j < n - 1 ? j++ : j] = *s++;
		o[j < n - 1 ? j++ : j] = '|';
	}
	o[j] = 0;
}

static int	compatibles(const char *en, const char *tr)
{
	char	a[128];
	char	b[128];

	formats(en, a, sizeof(a));
	formats(tr, b, sizeof(b));
	return (!strcmp(a, b));
}

static void	ranger(char *en, char *tr)
{
	unsigned long	h;
	int				k;

	if (!en || !tr || !compatibles(en, tr))
	{
		free(en);
		free(tr);
		return ;
	}
	h = hacher(en) % NCASES;
	for (k = 0; k < NCASES && g_cases[h].en; k++)
	{
		if (!strcmp(g_cases[h].en, en))
			break ;
		h = (h + 1) % NCASES;
	}
	if (k == NCASES || g_cases[h].en)
	{
		free(en);
		free(tr);
		return ;
	}
	g_cases[h].en = en;
	g_cases[h].tr = tr;
}

static FILE	*ouvrir(int l)
{
	char	p[700];
	FILE	*f;

	f = NULL;
	if (getenv("TDF_RACINE"))
	{
		snprintf(p, sizeof(p), "%s/moteur/langues/%s.txt", getenv("TDF_RACINE"), g_codes[l]);
		f = fopen(p, "r");
	}
	return (f);
}

static void	charger(int l)
{
	FILE	*f;
	char	*ligne;
	size_t	cap;
	ssize_t	n;
	char	*tab;

	vider();
	g_chargee = l;
	if (l < 2 || !(f = ouvrir(l)))
		return ;
	ligne = NULL;
	cap = 0;
	while ((n = getline(&ligne, &cap, f)) > 0)
	{
		while (n > 0 && (ligne[n - 1] == '\n' || ligne[n - 1] == '\r'))
			ligne[--n] = 0;
		if (!n || ligne[0] == '#' || !(tab = strchr(ligne, '\t')) || !tab[1])
			continue ;
		ranger(decoder(ligne, tab - ligne), decoder(tab + 1, strlen(tab + 1)));
	}
	free(ligne);
	fclose(f);
}

const char	*tdf_tr(const char *fr, const char *en)
{
	int				l;
	unsigned long	h;
	int				k;

	l = g_save.cfg.lang;
	if (l <= 0 || l >= NLANG)
		return (fr);
	if (l == 1 || !en)
		return (en);
	if (g_chargee != l)
		charger(l);
	h = hacher(en) % NCASES;
	for (k = 0; k < NCASES && g_cases[h].en; k++)
	{
		if (!strcmp(g_cases[h].en, en))
			return (g_cases[h].tr);
		h = (h + 1) % NCASES;
	}
	return (en);
}
