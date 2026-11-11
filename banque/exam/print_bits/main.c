#include <unistd.h>

void	print_bits(unsigned char octet);

int	main(void)
{
	unsigned char	t[] = {0, 1, 2, 5, 42, 127, 128, 170, 255, 16};
	int				i;

	for (i = 0; i < 10; i++)
	{
		print_bits(t[i]);
		write(1, "\n", 1);
	}
	return (0);
}
