#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "fous.h"

void	lire_ecrire(const char *dir, const char *src, int niv, const char *fn, const char *proto,
		const char *fr, const char *en, const char *notion, const char *autorise, const char *note)
{
	char	d[900];
	char	p[1000];
	FILE	*f;
	int		lang;

	snprintf(d, sizeof(d), "%s/%s", dir, fn);
	mkdir(d, 0755);
	snprintf(p, sizeof(p), "%s/meta", d);
	f = fopen(p, "w");
	if (!f)
		return ;
	fprintf(f, "source=%s\nniveau=%d\ntype=fonction\nfichiers=%s.c\nautorise=%s\nnotion=%s\n", src, niv, fn, autorise, notion);
	fclose(f);
	for (lang = 0; lang < 2; lang++)
	{
		snprintf(p, sizeof(p), "%s/sujet.%s.txt", d, lang ? "en" : "fr");
		f = fopen(p, "w");
		if (!f)
			return ;
		fprintf(f, "%s : %s\n", lang ? "Assignment name   " : "Exercice          ", fn);
		fprintf(f, "%s : %s.c\n", lang ? "Expected files    " : "Fichiers attendus ", fn);
		fprintf(f, "%s : %s\n", lang ? "Allowed functions " : "Fonctions permises", autorise);
		fprintf(f, "--------------------------------------------------------------\n\n");
		fprintf(f, "%s\n\n", lang ? en : fr);
		fprintf(f, "%s\n\n%s\n", lang ? "Your function must have this prototype:" : "Ta fonction doit avoir ce prototype :", proto);
		fprintf(f, "\n%s : %s\n", lang ? "Notion" : "Notion", notion);
		fprintf(f, "%s\n", note);
		fclose(f);
	}
}

int	main(int ac, char **av)
{
	char	d[800];
	int		i;

	if (ac < 2)
		return (1);
	snprintf(d, sizeof(d), "%s/libft", av[1]);
	mkdir(d, 0755);
	for (i = 0; i < NEXO; i++)
		lire_ecrire(d, "libft", i + 1, g_exos[i].fn, g_exos[i].proto, g_exos[i].fr, g_exos[i].en, g_exos[i].notion,
			"malloc, free, write, ta libft", "Norme obligatoire. Les fonctions d'aide doivent être static.");
	snprintf(d, sizeof(d), "%s/printf", av[1]);
	mkdir(d, 0755);
	for (i = 0; i < NPEXO; i++)
		lire_ecrire(d, "printf", i + 1, g_pexos[i].fn, g_pexos[i].proto, g_pexos[i].fr, g_pexos[i].en, g_pexos[i].notion,
			"write, malloc, free, va_start, va_arg, va_copy, va_end", "Norme obligatoire. Pas de libft ici.");
	printf("%d + %d exercices exportés\n", NEXO, NPEXO);
	return (0);
}
