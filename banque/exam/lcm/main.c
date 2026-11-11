#include <stdio.h>

unsigned int	lcm(unsigned int a, unsigned int b);

int	main(void)
{
	unsigned int	t[][2] = {{4, 6}, {3, 5}, {0, 5}, {5, 0}, {1, 1}, {12, 18}, {7, 7}, {21, 6}, {100, 75}, {65535, 3}};
	int				i;

	for (i = 0; i < 10; i++)
		printf("%u\n", lcm(t[i][0], t[i][1]));
	return (0);
}
