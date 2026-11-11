#include <stdio.h>
#include <stdlib.h>
#include "ft_list.h"

int	ft_list_size(t_list *begin_list);

int	main(void)
{
	t_list	n[50];
	int		k;
	int		i;

	printf("%d\n", ft_list_size(NULL));
	for (k = 1; k <= 50; k += 7)
	{
		for (i = 0; i < k; i++)
		{
			n[i].data = &n[i];
			n[i].next = i + 1 < k ? &n[i + 1] : NULL;
		}
		printf("%d\n", ft_list_size(&n[0]));
	}
	return (0);
}
