#include <stdio.h>
#include "list.h"

t_list	*sort_list(t_list *lst, int (*cmp)(int, int));

static int	up(int a, int b)
{
	return (a <= b);
}

static int	down(int a, int b)
{
	return (a >= b);
}

static void	run(int *v, int n, int (*cmp)(int, int))
{
	t_list	node[20];
	t_list	*l;
	int		i;

	for (i = 0; i < n; i++)
	{
		node[i].data = v[i];
		node[i].next = i + 1 < n ? &node[i + 1] : NULL;
	}
	l = sort_list(n ? node : NULL, cmp);
	while (l)
	{
		printf("%d ", l->data);
		l = l->next;
	}
	printf("\n");
}

int	main(void)
{
	int	a[] = {5, 3, 9, 1, 7};
	int	b[] = {4, 4, -1, 4, 0, -1};
	int	c[] = {1};

	run(a, 5, up);
	run(a, 5, down);
	run(b, 6, up);
	run(c, 1, up);
	run(c, 0, up);
	return (0);
}
