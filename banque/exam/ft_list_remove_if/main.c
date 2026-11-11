#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ft_list.h"

void	ft_list_remove_if(t_list **begin_list, void *data_ref, int (*cmp)());

static t_list	*make(char **w)
{
	t_list	*h;
	t_list	**p;
	int		i;

	h = NULL;
	p = &h;
	for (i = 0; w[i]; i++)
	{
		*p = malloc(sizeof(t_list));
		(*p)->data = w[i];
		(*p)->next = NULL;
		p = &(*p)->next;
	}
	return (h);
}

static void	show_free(t_list *l)
{
	t_list	*n;

	printf("[");
	while (l)
	{
		n = l->next;
		printf("%s%s", (char *)l->data, n ? " " : "");
		free(l);
		l = n;
	}
	printf("]\n");
}

int	main(void)
{
	char	*a[] = {"x", "a", "x", "b", "x", 0};
	char	*b[] = {"x", "x", "x", 0};
	char	*c[] = {"a", "b", "c", 0};
	char	*d[] = {0};
	t_list	*l;

	l = make(a);
	ft_list_remove_if(&l, "x", strcmp);
	show_free(l);
	l = make(b);
	ft_list_remove_if(&l, "x", strcmp);
	show_free(l);
	l = make(c);
	ft_list_remove_if(&l, "x", strcmp);
	show_free(l);
	l = make(c);
	ft_list_remove_if(&l, "a", strcmp);
	show_free(l);
	l = make(c);
	ft_list_remove_if(&l, "c", strcmp);
	show_free(l);
	l = make(d);
	ft_list_remove_if(&l, "x", strcmp);
	show_free(l);
	return (0);
}
