#include <stdio.h>

int	ft_atoi(const char *str);

int	main(void)
{
	char	*t[] = {"0", "42", "-42", "+17", "   123abc", "\t\n\v\f\r 99", "--5", "+-5", "-+5", "abc", "", "  -0",
		"2147483647", "-2147483648", "00012", " 12 34", "-", "+", "9a9", 0};
	int		i;

	for (i = 0; t[i]; i++)
		printf("%d\n", ft_atoi(t[i]));
	return (0);
}
