#include <stdio.h>

char	*ft_strpbrk(const char *s1, const char *s2);

int	main(void)
{
	char	*t[][2] = {{"bonjour", "j"}, {"bonjour", "xyz"}, {"bonjour", ""}, {"", "abc"}, {"abc", "cba"}, {"le chat", " "},
		{"testeur", "rt"}, {"hello world", "ow"}, {0, 0}};
	char	*r;
	int		i;

	for (i = 0; t[i][0]; i++)
	{
		r = ft_strpbrk(t[i][0], t[i][1]);
		if (r)
			printf("%ld [%s]\n", (long)(r - t[i][0]), r);
		else
			printf("NULL\n");
	}
	return (0);
}
