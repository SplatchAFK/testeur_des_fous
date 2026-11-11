#include <limits.h>
#include <stdio.h>

int	max(int *tab, unsigned int len);

int	main(void)
{
	int	a[] = {1, 5, 3};
	int	b[] = {-4, -2, -9};
	int	c[] = {42};
	int	d[] = {INT_MIN, INT_MAX, 0};
	int	e[] = {7, 7, 7, 7};
	int	f[] = {3, 2, 1, 0, -1, 10};

	printf("%d\n%d\n%d\n%d\n%d\n%d\n%d\n", max(a, 3), max(b, 3), max(c, 1), max(d, 3), max(e, 4), max(f, 6), max(a, 0));
	printf("%d\n", max(f, 5));
	return (0);
}
