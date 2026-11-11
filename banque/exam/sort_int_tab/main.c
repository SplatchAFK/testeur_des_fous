#include <limits.h>
#include <stdio.h>

void	sort_int_tab(int *tab, unsigned int size);

static void	show(int *t, unsigned int n)
{
	unsigned int	i;

	sort_int_tab(t, n);
	for (i = 0; i < n; i++)
		printf("%d ", t[i]);
	printf("\n");
}

int	main(void)
{
	int	a[] = {5, 3, 9, 1, 7};
	int	b[] = {1, 2, 3};
	int	c[] = {3, 2, 1};
	int	d[] = {4, 4, -1, 4, 0, -1};
	int	e[] = {INT_MAX, INT_MIN, 0, -5, 5};
	int	f[] = {42};

	show(a, 5);
	show(b, 3);
	show(c, 3);
	show(d, 6);
	show(e, 5);
	show(f, 1);
	show(f, 0);
	return (0);
}
