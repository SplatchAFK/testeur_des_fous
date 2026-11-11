#include <stdio.h>
#include <stddef.h>

size_t	ft_strcspn(const char *s, const char *reject);

int	main(void)
{
	char	*t[][2] = {{"bonjour", "j"}, {"bonjour", "xyz"}, {"bonjour", ""}, {"", "abc"}, {"abc", "cba"}, {"le chat", " "},
		{"aaaab", "b"}, {"testeur", "rt"}, {"hello world", "ow"}, {0, 0}};
	int		i;

	for (i = 0; t[i][0]; i++)
		printf("%zu\n", ft_strcspn(t[i][0], t[i][1]));
	return (0);
}
