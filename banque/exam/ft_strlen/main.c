#include <stdio.h>

int	ft_strlen(char *str);

int	main(void)
{
	char	*t[] = {"", "a", "bonjour", "Hello World!", "   \t  ", "0123456789012345678901234567890123456789",
		"accent \xc3\xa9", "fin\n", 0};
	char	big[5001];
	int		i;

	for (i = 0; t[i]; i++)
		printf("%d\n", ft_strlen(t[i]));
	for (i = 0; i < 5000; i++)
		big[i] = 'x';
	big[5000] = '\0';
	printf("%d\n", ft_strlen(big));
	return (0);
}
