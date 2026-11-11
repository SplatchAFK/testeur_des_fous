#include <stdio.h>
#include <unistd.h>

void	ft_putstr(char *str);

int	main(void)
{
	char	*t[] = {"bonjour", "", "deux mots", "avec\ttab\net retour", "%d %s %%", "0123456789", "z", 0};
	char	big[3001];
	int		i;

	setvbuf(stdout, NULL, _IONBF, 0);
	for (i = 0; t[i]; i++)
	{
		write(1, "[", 1);
		ft_putstr(t[i]);
		write(1, "]\n", 2);
	}
	for (i = 0; i < 3000; i++)
		big[i] = 'a' + i % 26;
	big[3000] = '\0';
	ft_putstr(big);
	write(1, "\n", 1);
	return (0);
}
