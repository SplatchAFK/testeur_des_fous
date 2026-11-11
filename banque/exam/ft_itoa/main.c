#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

char	*ft_itoa(int nbr);

int	main(void)
{
	int		t[] = {0, 1, -1, 42, -42, 10, -10, 1000000, INT_MAX, INT_MIN, 7, -987654321};
	char	*s;
	int		i;

	for (i = 0; i < 12; i++)
	{
		s = ft_itoa(t[i]);
		printf("[%s]\n", s ? s : "(null)");
		free(s);
	}
	return (0);
}
