#include <stdio.h>

int	ft_atoi_base(const char *str, int str_base);

int	main(void)
{
	printf("%d\n", ft_atoi_base("101", 2));
	printf("%d\n", ft_atoi_base("ff", 16));
	printf("%d\n", ft_atoi_base("FF", 16));
	printf("%d\n", ft_atoi_base("-2a", 16));
	printf("%d\n", ft_atoi_base("777", 8));
	printf("%d\n", ft_atoi_base("789", 8));
	printf("%d\n", ft_atoi_base("12fdb3", 16));
	printf("%d\n", ft_atoi_base("0", 10));
	printf("%d\n", ft_atoi_base("", 10));
	printf("%d\n", ft_atoi_base("zz", 16));
	printf("%d\n", ft_atoi_base("-11111111", 2));
	printf("%d\n", ft_atoi_base("7fffffff", 16));
	printf("%d\n", ft_atoi_base("-80000000", 16));
	printf("%d\n", ft_atoi_base("123", 10));
	printf("%d\n", ft_atoi_base("Ab", 12));
	return (0);
}
