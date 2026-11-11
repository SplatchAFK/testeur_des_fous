#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

static void	unescape(char *s)
{
	char	*w;

	w = s;
	while (*s)
	{
		if (s[0] == '\\' && s[1] == 'n')
		{
			*w++ = '\n';
			s += 2;
		}
		else if (s[0] == '\\' && s[1] == '\\')
		{
			*w++ = '\\';
			s += 2;
		}
		else
			*w++ = *s++;
	}
	*w = '\0';
}

static int	split(char *s, char **f, int max)
{
	int	n;

	n = 0;
	f[n++] = s;
	while (*s && n < max)
	{
		if (*s == '\t')
		{
			*s = '\0';
			f[n++] = s + 1;
		}
		s++;
	}
	return (n);
}

static void	cpy(char *dst, size_t n, const char *s)
{
	snprintf(dst, n, "%s", s ? s : "");
	unescape(dst);
}

static void	parse(char *line)
{
	char	*f[8];
	int		n;
	t_pcat	*c;

	n = split(line, f, 8);
	if (!strcmp(f[0], "S") && n >= 3 && g_pr.nst < MAXSTAT)
	{
		g_pr.st[g_pr.nst].st = !strcmp(f[1], "OK") ? 1 : (!strcmp(f[1], "KO") ? 2 : 3);
		cpy(g_pr.st[g_pr.nst].title, sizeof(g_pr.st[0].title), f[2]);
		cpy(g_pr.st[g_pr.nst].detail, sizeof(g_pr.st[0].detail), n > 3 ? f[3] : "");
		g_pr.nst++;
	}
	else if (!strcmp(f[0], "D") && n >= 2 && g_pr.nst > 0)
	{
		size_t l = strlen(g_pr.st[g_pr.nst - 1].detail);
		snprintf(g_pr.st[g_pr.nst - 1].detail + l, sizeof(g_pr.st[0].detail) - l, "%s%s", l ? "\n" : "", f[1]);
	}
	else if (!strcmp(f[0], "C") && n >= 6 && g_pr.ncat < 24)
	{
		c = pf_cat(f[1]);
		if (!c)
			c = &g_pr.cat[g_pr.ncat++];
		cpy(c->key, sizeof(c->key), f[1]);
		cpy(c->title, sizeof(c->title), f[2]);
		c->ok = atoi(f[3]);
		c->ko = atoi(f[4]);
		c->warn = atoi(f[5]);
		if (!strcmp(f[1], "bonus"))
			g_pr.bonus = 1;
	}
	else if (!strcmp(f[0], "R") && n >= 6 && g_pr.ncs < MAXCASE)
	{
		cpy(g_pr.cs[g_pr.ncs].cat, sizeof(g_pr.cs[0].cat), f[1]);
		g_pr.cs[g_pr.ncs].ko = !strcmp(f[2], "KO");
		cpy(g_pr.cs[g_pr.ncs].where, sizeof(g_pr.cs[0].where), f[3]);
		cpy(g_pr.cs[g_pr.ncs].desc, sizeof(g_pr.cs[0].desc), f[4]);
		cpy(g_pr.cs[g_pr.ncs].msg, sizeof(g_pr.cs[0].msg), f[5]);
		g_pr.ncs++;
	}
	else if (!strcmp(f[0], "R") && n >= 6)
		g_pr.truncated = 1;
	else if (!strcmp(f[0], "K") && n >= 6 && g_pr.nca < MAXCAUSE)
	{
		cpy(g_pr.ca[g_pr.nca].cat, sizeof(g_pr.ca[0].cat), f[1]);
		cpy(g_pr.ca[g_pr.nca].title, sizeof(g_pr.ca[0].title), f[2]);
		cpy(g_pr.ca[g_pr.nca].where, sizeof(g_pr.ca[0].where), f[3]);
		g_pr.ca[g_pr.nca].count = atoi(f[4]);
		cpy(g_pr.ca[g_pr.nca].hint, sizeof(g_pr.ca[0].hint), f[5]);
		g_pr.nca++;
	}
	else if (!strcmp(f[0], "T") && n >= 5)
	{
		cpy(g_pr.mode, sizeof(g_pr.mode), f[1]);
		g_pr.tok += atoi(f[2]);
		g_pr.tko += atoi(f[3]);
		g_pr.twarn += atoi(f[4]);
	}
	else if (!strcmp(f[0], "P") && n >= 4)
	{
		cpy(g_pr.plabel, sizeof(g_pr.plabel), f[1]);
		g_pr.pcur = atoi(f[2]);
		g_pr.ptot = atoi(f[3]);
	}
	else if (!strcmp(f[0], "E"))
		g_pr.done = 1;
}

void	pf_load(const char *path, long *off)
{
	FILE		*f;
	static char	line[4096];
	size_t		l;

	f = fopen(path, "r");
	if (!f)
		return ;
	g_pr.loaded = 1;
	fseek(f, *off, SEEK_SET);
	while (fgets(line, sizeof(line), f))
	{
		l = strlen(line);
		if (l == 0 || line[l - 1] != '\n')
		{
			if (l == sizeof(line) - 1)
			{
				*off += l;
				continue ;
			}
			break ;
		}
		*off += l;
		line[l - 1] = '\0';
		parse(line);
	}
	fclose(f);
}
