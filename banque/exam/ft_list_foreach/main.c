#include <stdio.h>
#include "ft_list.h"

void	ft_list_foreach(t_list *begin_list, void (*f)(void *));

static int	g_calls;

static void	show(void *d)
{
	g_calls++;
	printf("[%s]", (char *)d);
}

static void	upper(void *d)
{
	char	*s;

	s = d;
	g_calls++;
	while (*s)
	{
		if (*s >= 'a' && *s <= 'z')
			*s -= 32;
		s++;
	}
}

int	main(void)
{
	char	a[] = "un";
	char	b[] = "deux";
	char	c[] = "trois";
	t_list	n[3];

	n[0].data = a;
	n[0].next = &n[1];
	n[1].data = b;
	n[1].next = &n[2];
	n[2].data = c;
	n[2].next = NULL;
	ft_list_foreach(n, show);
	printf(" %d\n", g_calls);
	ft_list_foreach(n, upper);
	ft_list_foreach(n, show);
	printf(" %d\n", g_calls);
	g_calls = 0;
	ft_list_foreach(NULL, show);
	printf("%d\n", g_calls);
	ft_list_foreach(&n[2], show);
	printf("\n");
	return (0);
}
