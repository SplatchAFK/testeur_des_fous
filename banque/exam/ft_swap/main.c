#include <limits.h>
#include <stdio.h>

void	ft_swap(int *a, int *b);

int	main(void)
{
	int	t[][2] = {{1, 2}, {0, 0}, {-5, 5}, {INT_MAX, INT_MIN}, {42, 42}, {-1, 0}, {1000000, -7}};
	int	x;
	int	i;

	for (i = 0; i < 7; i++)
	{
		ft_swap(&t[i][0], &t[i][1]);
		printf("%d %d\n", t[i][0], t[i][1]);
	}
	x = 9;
	ft_swap(&x, &x);
	printf("%d\n", x);
	return (0);
}
