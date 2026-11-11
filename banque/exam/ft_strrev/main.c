#include <stdio.h>
#include <string.h>

char	*ft_strrev(char *str);

int	main(void)
{
	char	*t[] = {"bonjour", "", "a", "ab", "abc", "kayak", "le testeur des fous", "1234567890", 0};
	char	b[100];
	char	*r;
	int		i;

	for (i = 0; t[i]; i++)
	{
		strcpy(b, t[i]);
		r = ft_strrev(b);
		printf("[%s] %s\n", b, r == b ? "str" : "AUTRE");
	}
	return (0);
}
