#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_point.h"

void	flood_fill(char **tab, t_point size, t_point begin);

static void	run(const char **src, int w, int h, int bx, int by)
{
	char	**tab;
	t_point	size;
	t_point	begin;
	int		i;

	tab = malloc(sizeof(char *) * h);
	for (i = 0; i < h; i++)
		tab[i] = strdup(src[i]);
	size.x = w;
	size.y = h;
	begin.x = bx;
	begin.y = by;
	flood_fill(tab, size, begin);
	for (i = 0; i < h; i++)
	{
		printf("%s\n", tab[i]);
		free(tab[i]);
	}
	printf("\n");
	free(tab);
}

int	main(void)
{
	const char	*a[] = {"11111111", "10001001", "10010001", "10110001", "11100001"};
	const char	*b[] = {"0000", "0110", "0000"};
	const char	*c[] = {"x"};
	const char	*d[] = {"1010101", "0101010", "1010101"};
	const char	*e[] = {"FFF0", "F00F", "0F00"};

	run(a, 8, 5, 7, 4);
	run(a, 8, 5, 1, 1);
	run(b, 4, 3, 0, 0);
	run(b, 4, 3, 1, 1);
	run(c, 1, 1, 0, 0);
	run(d, 7, 3, 3, 1);
	run(e, 4, 3, 0, 0);
	run(e, 4, 3, 1, 1);
	return (0);
}
