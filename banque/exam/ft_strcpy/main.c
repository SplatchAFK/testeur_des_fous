#include <stdio.h>
#include <string.h>

char	*ft_strcpy(char *s1, char *s2);

int	main(void)
{
	char	*t[] = {"bonjour", "", "a", "une phrase plus longue, avec de la ponctuation !", "tab\tet\nretour", 0};
	char	dst[200];
	char	*r;
	int		i;
	int		k;

	for (i = 0; t[i]; i++)
	{
		memset(dst, 'X', sizeof(dst));
		r = ft_strcpy(dst, t[i]);
		printf("copie=[%s] retour=%s", dst, r == dst ? "s1" : "AUTRE");
		k = strlen(t[i]);
		printf(" nul=%d suite=%c%c\n", dst[k] == '\0', dst[k + 1], dst[k + 2]);
	}
	return (0);
}
