#include <stdio.h>

int	is_power_of_2(unsigned int n);

int	main(void)
{
	unsigned int	t[] = {0, 1, 2, 3, 4, 5, 6, 8, 12, 16, 31, 32, 64, 100, 1024, 1023, 65536, 2147483648u, 4294967295u, 3221225472u};
	int				i;

	for (i = 0; i < 20; i++)
		printf("%u %d\n", t[i], is_power_of_2(t[i]));
	return (0);
}
