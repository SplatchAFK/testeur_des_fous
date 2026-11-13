#include "tdf_tests.h"
#include <stdint.h>
#include <stdlib.h>

static unsigned long long	g_mr;

static unsigned	mr(void)
{
	g_mr ^= g_mr << 13;
	g_mr ^= g_mr >> 7;
	g_mr ^= g_mr << 17;
	return ((unsigned)(g_mr >> 11));
}

static int	mint(void)
{
	return ((int)((mr() << 1) ^ mr()));
}

static const char	*g_mstr[] = {"", "hello", "42 Paris", "a%b%c", "line\nbreak", "tab\there", NULL,
	"  spaces  ", "Moulinette", "%%", "0123456789", "end."};

static void	block_percent(void)
{
	tdf_category("mou_01", "It should print a single percent sign");
	T("%%");
	T("[%%]");
	tdf_category("mou_02", "It should print consecutive literal percent sequences");
	T("%%%%");
	T("%%%%%%");
	T("%% %% %%");
	tdf_category("mou_03", "It should handle formats without conversions");
	T("hello world");
	T("");
	T("no conversion here\n");
	tdf_category("mou_04", "It should print literal percent signs within text");
	T("100%% sure");
	T("a%%b%%c");
	T("%%start and end%%");
}

static void	block_c(void)
{
	int	i;
	int	a;
	int	b;

	tdf_category("mou_05", "It should print percent signs passed as characters");
	T("%c", '%');
	T("%c%c", '%', '%');
	T("[%c]", '%');
	tdf_category("mou_06", "It should handle printable and whitespace characters");
	T("%c", ' ');
	T("%c", '\t');
	T("%c", '\n');
	for (i = 0; i < 6; i++)
	{
		a = 32 + (int)(mr() % 95);
		b = 32 + (int)(mr() % 95);
		T("%c|%c", a, b);
	}
}

static void	block_s(void)
{
	int			i;
	const char	*a;
	const char	*b;

	tdf_category("mou_07", "It should print empty strings");
	T("%s", "");
	T("[%s]", "");
	T("%s%s%s", "", "", "");
	tdf_category("mou_08", "It should render NULL string pointers");
	T("%s", (char *)NULL);
	T("[%s]", (char *)NULL);
	T("%s %s", (char *)NULL, "x");
	tdf_category("mou_09", "It should keep percent signs and newlines inside string arguments");
	T("%s", "100%");
	T("%s", "a%db%s");
	T("%s", "first\nsecond\n");
	tdf_category("mou_10", "It should handle varied string payloads and NULL");
	for (i = 0; i < 6; i++)
	{
		a = g_mstr[mr() % 12];
		b = g_mstr[mr() % 12];
		T("%s - %s", a, b);
	}
}

static void	block_p(void)
{
	int		local;
	int		i;
	void	*a;
	void	*b;

	tdf_category("mou_11", "It should render NULL pointers");
	T("%p", NULL);
	T("[%p]", NULL);
	tdf_category("mou_12", "It should render small pointer values");
	T("%p", (void *)1);
	T("%p", (void *)15);
	T("%p", (void *)255);
	T("%p", (void *)4096);
	tdf_category("mou_13", "It should render large pointer addresses");
	T("%p", (void *)UINTPTR_MAX);
	T("%p", (void *)(UINTPTR_MAX / 2));
	T("%p", (void *)&local);
	tdf_category("mou_14", "It should format NULL, small and large addresses");
	for (i = 0; i < 4; i++)
	{
		a = (void *)(uintptr_t)(mr() % 4096);
		b = (void *)((uintptr_t)mr() << 20);
		T("%p %p %p", NULL, a, b);
	}
}

static void	block_d(void)
{
	int	i;
	int	v1;
	int	v2;

	tdf_category("mou_15", "It should handle INT_MIN");
	T("%d", INT_MIN);
	T("[%d]", INT_MIN);
	tdf_category("mou_16", "It should handle INT_MAX");
	T("%d", INT_MAX);
	T("[%d]", INT_MAX);
	tdf_category("mou_17", "It should handle negative values with surrounding text");
	T("value: %d units", -42);
	T("before %d after", -1);
	T("(%d)", -2147483647);
	tdf_category("mou_18", "It should format signed decimal values across the range");
	for (i = 0; i < 8; i++)
	{
		v1 = mint();
		v2 = mint();
		T("%d %d", v1, v2);
	}
}

static void	block_i(void)
{
	int	i;
	int	v1;
	int	v2;

	tdf_category("mou_19", "It should format zero values");
	T("%i", 0);
	T("%i%i", 0, 0);
	tdf_category("mou_20", "It should format negative values");
	T("%i", -1);
	T("%i", -987654);
	T("%i", INT_MIN);
	tdf_category("mou_21", "It should format positive integers");
	T("%i", 1);
	T("%i", 123456);
	T("%i", INT_MAX);
	tdf_category("mou_22", "It should format signed integer values across the range");
	for (i = 0; i < 8; i++)
	{
		v1 = mint();
		v2 = mint();
		T("%i %i", v1, v2);
	}
}

static void	block_u(void)
{
	int	i;
	unsigned	u1;
	unsigned	u2;

	tdf_category("mou_23", "It should print zero for unsigned");
	T("%u", 0);
	T("[%u]", 0u);
	tdf_category("mou_24", "It should print the unsigned int boundary");
	T("%u", UINT_MAX);
	T("%u", -1);
	tdf_category("mou_25", "It should format unsigned integers across the range");
	for (i = 0; i < 8; i++)
	{
		u1 = mr() ^ (mr() << 16);
		u2 = mr();
		T("%u %u", u1, u2);
	}
}

static void	block_x(void)
{
	int	i;
	unsigned	u1;
	unsigned	u2;

	tdf_category("mou_26", "It should print zero in lowercase hexadecimal");
	T("%x", 0);
	tdf_category("mou_27", "It should print UINT_MAX in lowercase hexadecimal");
	T("%x", UINT_MAX);
	T("%x", -1);
	tdf_category("mou_28", "It should format lowercase hexadecimal values");
	for (i = 0; i < 8; i++)
	{
		u1 = mr() ^ (mr() << 16);
		u2 = mr() % 256;
		T("%x %x", u1, u2);
	}
	tdf_category("mou_29", "It should print zero in uppercase hexadecimal");
	T("%X", 0);
	tdf_category("mou_30", "It should print UINT_MAX in uppercase hexadecimal");
	T("%X", UINT_MAX);
	T("%X", -1);
	tdf_category("mou_31", "It should format uppercase hexadecimal values");
	for (i = 0; i < 8; i++)
	{
		u1 = mr() ^ (mr() << 16);
		u2 = mr() % 256;
		T("%X %X", u1, u2);
	}
}

static void	block_mix(void)
{
	static const char	*sep[] = {" ", " | ", ",", "\t", "--", "\n", ""};
	int					i;
	const char			*a;
	const char			*b;
	const char			*str;
	void				*ptr;
	int					c;
	int					d;
	int					e;
	unsigned			u;
	unsigned			x;
	unsigned			y;

	tdf_category("mou_32", "It should handle randomized mixed conversions with NULL pointers and varied separators");
	for (i = 0; i < 10; i++)
	{
		a = sep[mr() % 7];
		b = sep[mr() % 7];
		c = 33 + (int)(mr() % 90);
		str = g_mstr[mr() % 12];
		ptr = (mr() % 3) ? NULL : (void *)(uintptr_t)mr();
		d = mint();
		e = mint();
		u = mr();
		x = mr();
		y = mr();
		T("%c%s%s%s%p%s%d%s%i%s%u%s%x%s%X%s%%", c, a, str, b, ptr, a, d, b, e, a, u, b, x, a, y, b);
	}
}

void	tdf_tests_moulinette(void)
{
	if (!g_opt.only || !tdf_selected("moulinette"))
		return ;
	g_mr = 0x9E3779B97F4A7C15ULL ^ g_opt.seed;
	block_percent();
	block_c();
	block_s();
	block_p();
	block_d();
	block_i();
	block_u();
	block_x();
	block_mix();
}
