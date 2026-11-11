#include <stdio.h>
#include <stdlib.h>

char	**ft_split(char *str);

static void	show(char *s)
{
	char	**t;
	int		i;

	t = ft_split(s);
	if (!t)
	{
		printf("NULL\n");
		return ;
	}
	for (i = 0; t[i]; i++)
	{
		printf("[%s]", t[i]);
		free(t[i]);
	}
	printf(" %d\n", i);
	free(t);
}

int	main(void)
{
	show("le testeur des fous");
	show("   espaces  autour   ");
	show("");
	show("     ");
	show("unseul");
	show("tab\tet\nretour   ensemble");
	show("a b c d e f g h");
	return (0);
}
