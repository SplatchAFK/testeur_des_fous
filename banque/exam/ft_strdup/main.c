#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char	*ft_strdup(char *src);

int	main(void)
{
	char	*t[] = {"bonjour", "", "a", "une phrase avec des espaces", "tab\tet\nretour", 0};
	char	src[40];
	char	*d;
	int		i;

	for (i = 0; t[i]; i++)
	{
		d = ft_strdup(t[i]);
		printf("[%s] %s\n", d ? d : "(null)", d && d != t[i] ? "copie" : "MEME ZONE");
		free(d);
	}
	strcpy(src, "modifiable");
	d = ft_strdup(src);
	src[0] = 'X';
	printf("[%s]\n", d);
	free(d);
	return (0);
}
