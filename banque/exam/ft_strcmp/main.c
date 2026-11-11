#include <stdio.h>

int	ft_strcmp(char *s1, char *s2);

static int	sg(int v)
{
	return ((v > 0) - (v < 0));
}

int	main(void)
{
	char	*t[][2] = {{"abc", "abc"}, {"abc", "abd"}, {"abd", "abc"}, {"", ""}, {"", "a"}, {"a", ""}, {"abc", "ab"},
		{"ab", "abc"}, {"Chat", "chat"}, {"\200", "a"}, {"a", "\200"}, {"testeur", "testeur des fous"}, {0, 0}};
	int		i;

	for (i = 0; t[i][0]; i++)
		printf("%d\n", sg(ft_strcmp(t[i][0], t[i][1])));
	return (0);
}
