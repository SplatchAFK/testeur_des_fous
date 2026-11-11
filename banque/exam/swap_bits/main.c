#include <stdio.h>

unsigned char	swap_bits(unsigned char octet);

int	main(void)
{
	int	i;

	for (i = 0; i < 256; i += 9)
		printf("%d %d\n", i, swap_bits((unsigned char)i));
	printf("%d %d\n", 65, swap_bits(65));
	printf("%d %d\n", 255, swap_bits(255));
	return (0);
}
