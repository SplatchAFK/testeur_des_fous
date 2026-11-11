#include <stdio.h>
#include <stddef.h>

size_t	ft_strspn(const char *s, const char *accept);

int	main(void)
{
	char	*t[][2] = {{"aaabbc", "a"}, {"aaabbc", "ab"}, {"aaabbc", "abc"}, {"bonjour", ""}, {"", "abc"}, {"   x", " "},
		{"123abc", "0123456789"}, {"testeur", "set"}, {0, 0}};
	int		i;

	for (i = 0; t[i][0]; i++)
		printf("%zu\n", ft_strspn(t[i][0], t[i][1]));
	return (0);
}
