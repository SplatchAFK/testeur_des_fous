#include <stdio.h>

unsigned char	reverse_bits(unsigned char octet);

int	main(void)
{
	int	i;

	for (i = 0; i < 256; i += 7)
		printf("%d %d\n", i, reverse_bits((unsigned char)i));
	printf("%d %d\n", 255, reverse_bits(255));
	printf("%d %d\n", 38, reverse_bits(38));
	return (0);
}
