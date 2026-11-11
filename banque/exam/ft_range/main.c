#include <stdio.h>
#include <stdlib.h>

int	*ft_range(int start, int end);

static void	show(int a, int b)
{
	int	*t;
	int	n;
	int	i;

	n = (a > b ? a - b : b - a) + 1;
	t = ft_range(a, b);
	if (!t)
	{
		printf("NULL\n");
		return ;
	}
	for (i = 0; i < n; i++)
		printf("%d%s", t[i], i + 1 < n ? " " : "\n");
	free(t);
}

int	main(void)
{
	show(1, 3);
	show(-1, 2);
	show(0, 0);
	show(0, -3);
	show(5, 1);
	show(-10, -5);
	show(40, 42);
	return (0);
}
