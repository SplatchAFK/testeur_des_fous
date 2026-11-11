/* Le testeur des fous · version printf - partie obligatoire */
#include "tdf_tests.h"
#include <stdlib.h>
#include <string.h>

static int	g_global_var = 42;
static char	g_long_str[100001];
static char	g_long_fmt[20001];

static void	init_data(void)
{
	int	i;

	i = -1;
	while (++i < 100000)
		g_long_str[i] = (char)('a' + i % 26);
	g_long_str[100000] = '\0';
	i = -1;
	while (++i < 20000)
		g_long_fmt[i] = (char)('A' + i % 26);
	memcpy(g_long_fmt + 9000, "%d|%s|%x|%%", 11);
	g_long_fmt[20000] = '\0';
}

static void	tests_c(void)
{
	tdf_category("c", "%c  caractères");
	T("%c", 'a');
	T("%c", 'Z');
	T("%c", '0');
	T("%c", ' ');
	T("%c", '\n');
	T("%c", '\t');
	T("%c", '~');
	T("%c", 127);
	T("%c", '\0');
	T("%c", 0);
	T("[%c]", 0);
	T("%c%c%c", 0, 0, 0);
	T("a%cb", 0);
	T("%c", 128);
	T("%c", 200);
	T("%c", 255);
	T("%c", -1);
	T("%c", -128);
	T("%c", (char)-42);
	T("%c", 'a' + 256);
	T("%c", '0' - 256);
	T("%c%c", 'a', 'b');
	T("%c%c%c%c%c", 'h', 'e', 'l', 'l', 'o');
	T(" %c %c %c ", '1', '2', '3');
	T(" %c %c %c ", 0, '1', 0);
	T("%c%c%c%c%c%c%c%c%c%c", '0', '1', '2', '3', '4', '5', '6', '7', '8', '9');
	T("%c=%c, %c=%c", 'x', 0, 255, 'y');
}

static void	tests_s(void)
{
	char	*heap;

	tdf_category("s", "%s  chaînes");
	heap = strdup("sur le tas");
	T("%s", "hello");
	T("%s", "");
	T("[%s]", "");
	T("%s%s", "", "");
	T(" %s ", "-");
	T("%s", "a");
	T("%s", NULL);
	T("[%s]", NULL);
	T("%s %s", NULL, NULL);
	T("%s|%s", "(null)", NULL);
	T("%s", "ab\0cd");
	T("%s", "\0abc");
	T("%s", "\t\n\v\f\r");
	T("%s", "100% sûr");
	T("%s", "%d %s %x");
	T("%s", "%");
	T("%s", "é à ü ç 漢字 🙂");
	T("%s", "\xff\xfe\x80\x01");
	T("%s", heap);
	T("%s", g_long_str + 99000);
	T("%s", g_long_str);
	T("%s%s", g_long_str, g_long_str);
	T(">%s<", g_long_str + 30000);
	T("%s %s %s %s %s", " - ", "", "4", "", "fin");
	T("%s%c%s", "a", 0, "b");
	free(heap);
}

static void	tests_p(void)
{
	int		local;
	char	*heap;

	tdf_category("p", "%p  pointeurs (NULL : glibc \"(nil)\", macOS \"0x0\")");
	heap = malloc(16);
	T("%p", (void *)&local);
	T("%p", (void *)heap);
	T("%p", (void *)&g_global_var);
	T("%p", (void *)tests_p);
	T("%p", (void *)"littéral");
	T("%p", NULL);
	T("%p", (void *)0);
	T("[%p]", NULL);
	T("%p %p", NULL, NULL);
	T("%p", (void *)1);
	T("%p", (void *)15);
	T("%p", (void *)16);
	T("%p", (void *)17);
	T("%p", (void *)255);
	T("%p", (void *)0xabcdefUL);
	T("%p", (void *)0xdeadbeefUL);
	T("%p", (void *)0x7fffffffffffUL);
	T("%p", (void *)0x7fffffffffffffffUL);
	T("%p", (void *)0x8000000000000000UL);
	T("%p", (void *)0xffffffff00000000UL);
	T("%p", (void *)0x00000000ffffffffUL);
	T("%p", (void *)0x100000000UL);
	T("%p", (void *)-1L);
	T("%p", (void *)-2L);
	T("%p%p", (void *)&local, (void *)heap);
	T("%p|%p|%p", (void *)1, NULL, (void *)-1L);
	free(heap);
}

static void	tests_di(const char *key)
{
	int	d;

	d = key[0] == 'd';
	tdf_category(key, d ? "%d  entiers signés" : "%i  entiers signés");
#define TD(v) do { if (d) TDF_RUNS(SEV_STRICT, #v, "%d", v); else TDF_RUNS(SEV_STRICT, #v, "%i", v); } while (0)
	TD(0);
	TD(1);
	TD(-1);
	TD(9);
	TD(-9);
	TD(10);
	TD(-10);
	TD(42);
	TD(-42);
	TD(99);
	TD(100);
	TD(-100);
	TD(101);
	TD(123456789);
	TD(-123456789);
	TD(999999999);
	TD(1000000000);
	TD(-1000000000);
	TD(1999999999);
	TD(INT_MAX);
	TD(INT_MAX - 1);
	TD(INT_MIN);
	TD(INT_MIN + 1);
	TD(-2147483647);
	TD(SHRT_MAX);
	TD(SHRT_MIN);
	TD(CHAR_MIN);
	TD((int)0x80000000U);
	TD((int)4294967295U);
#undef TD
	if (d)
	{
		T("[%d]", 0);
		T("%d%d", 1, -1);
		T("%d %d %d", INT_MIN, 0, INT_MAX);
		T("%d|%d|%d|%d|%d|%d|%d|%d", 1, -2, 3, -4, 5, -6, 7, INT_MIN);
	}
	else
	{
		T("[%i]", 0);
		T("%i%d%i", -1, 2, -3);
		T("%i %i %i", INT_MIN, 0, INT_MAX);
		T("%i|%i|%i|%i|%i|%i|%i|%i", 1, -2, 3, -4, 5, -6, 7, INT_MIN);
	}
}

static void	tests_u(void)
{
	tdf_category("u", "%u  entiers non signés");
	T("%u", 0u);
	T("%u", 1u);
	T("%u", 9u);
	T("%u", 10u);
	T("%u", 42u);
	T("%u", 100u);
	T("%u", 4294967295u);
	T("%u", UINT_MAX);
	T("%u", UINT_MAX - 1);
	T("%u", 3000000000u);
	T("%u", 2147483647u);
	T("%u", 2147483648u);
	T("%u", 999999999u);
	T("%u", 1000000000u);
	T("%u", 4000000000u);
	T("%u", (unsigned int)-1);
	T("%u", (unsigned int)-42);
	T("%u", (unsigned int)INT_MIN);
	T("[%u]", 0u);
	T("%u %u %u", 0u, UINT_MAX, 3000000000u);
	T("%u|%u|%u|%u|%u|%u|%u|%u", 1u, 2u, 3u, 4u, 5u, 6u, 7u, UINT_MAX);
	T("%u%d", UINT_MAX, -1);
}

static void	tests_x(int up)
{
	tdf_category(up ? "X" : "x", up ? "%X  hexadécimal MAJUSCULE" : "%x  hexadécimal minuscule");
#define TX(v) do { if (up) TDF_RUNS(SEV_STRICT, #v, "%X", v); else TDF_RUNS(SEV_STRICT, #v, "%x", v); } while (0)
	TX(0u);
	TX(1u);
	TX(9u);
	TX(10u);
	TX(11u);
	TX(15u);
	TX(16u);
	TX(17u);
	TX(42u);
	TX(255u);
	TX(256u);
	TX(4095u);
	TX(4096u);
	TX(0xabcdefu);
	TX(0xABCDEFu);
	TX(0xdeadbeefu);
	TX(0x0f0f0f0fu);
	TX(0xf0f0f0f0u);
	TX(0x10000000u);
	TX(0x7fffffffu);
	TX(0x80000000u);
	TX(UINT_MAX);
	TX((unsigned int)-1);
	TX((unsigned int)-42);
	TX((unsigned int)INT_MIN);
	TX(1234567890u);
#undef TX
	if (up)
	{
		T("[%X]", 0u);
		T("%X%x", 0xabcu, 0xabcu);
		T("%X|%X|%X|%X|%X|%X|%X|%X", 1u, 0xau, 0xbu, 0xcu, 0xdu, 0xeu, 0xfu, UINT_MAX);
	}
	else
	{
		T("[%x]", 0u);
		T("%x%X", 0xabcu, 0xabcu);
		T("%x|%x|%x|%x|%x|%x|%x|%x", 1u, 0xau, 0xbu, 0xcu, 0xdu, 0xeu, 0xfu, UINT_MAX);
		T("%x %x %x %x %x %x %x %x %x %x", 0xau, 0xbu, 0xcu, 0xdu, 0xeu, 0xfu,
			0xffffffffu, 0x80000000u, 0x7fffffffu, 0u);
	}
}

static void	tests_percent(void)
{
	tdf_category("%", "%  pourcent");
	T("%%");
	T("%%%%");
	T("%%%%%%");
	T(" %% ");
	T("a%%b");
	T("%%c");
	T("%%d%%");
	T("%%%d", 42);
	T("%d%%", 42);
	T("%%%c%%", 'x');
	T("%%%s%%", "s");
	T("100%%");
	T("%% %% %%");
	T("%%  %%  %%");
	T("%c%%%c", 0, 0);
}

static void	tests_mix(void)
{
	int		local;
	char	*heap;

	tdf_category("mix", "mélanges, format vide/long");
	heap = malloc(1);
	T("");
	T("hello");
	T("hello world\n");
	T("\n");
	T("\t\v\f\r");
	T("é漢🙂");
	T("%c%s%p%d%i%u%x%X%%", 'A', "42", (void *)&local, -42, 42, 42u, 42u, 42u);
	T("c=%c s=%s p=%p d=%d i=%i u=%u x=%x X=%X %%", 'z', "str", NULL, INT_MIN, INT_MAX,
		UINT_MAX, 0xdeadu, 0xbeefu);
	T("%%%c%%%s%%%d%%%i%%%u%%%x%%%X%%%%", 'A', "42", 42, 42, 42u, 42u, 42u);
	T("%s%d%s", "", 0, "");
	T("%d%s%d", 1, NULL, 2);
	T("%p%s%p", NULL, NULL, (void *)heap);
	T("%c%c%c%s%c", 'a', 0, 'b', "", 0);
	T("%d %i %u %x %X %c %s %p %%", -1, -1, (unsigned int)-1, (unsigned int)-1,
		(unsigned int)-1, -1, "-1", (void *)-1L);
	T("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, INT_MIN);
	T("%s%s%s%s%s%s%s%s%s%s", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10");
	T("%x %x %x %x %x %x %x %x", 1u, 2u, 3u, 4u, 5u, 6u, 0xffffffffu, 0xabcdefu);
	T("%p %p %p %p %p %p %p", (void *)1, (void *)2, (void *)3, (void *)4, (void *)5,
		(void *)6, (void *)-1L);
	T("%c%d%c%u%c%x%c%s%c%p", 'a', 1, 'b', 2u, 'c', 3u, 'd', "e", 'f', NULL);
	T(g_long_fmt, -42, "milieu", 0xcafeu);
	T("%s|%s|%d", g_long_str, "x", 42);
	free(heap);
}

static void	tests_ret(void)
{
	tdf_category("ret", "valeur de retour (avec \\0, gros volumes)");
	T("%c", 0);
	T("%c%c", 0, 'a');
	T("abc%cdef", 0);
	T("%s", "");
	T("%s", NULL);
	T("%p", NULL);
	T("%d", INT_MIN);
	T("%d", -1);
	T("%u", UINT_MAX);
	T("%x", 0u);
	T("%%");
	T("");
	T("%s%s%s", g_long_str, g_long_str, g_long_str);
	T("%c%s%c", 0, g_long_str, 0);
}

void	tdf_tests_mandatory(void)
{
	init_data();
	tests_c();
	tests_s();
	tests_p();
	tests_di("d");
	tests_di("i");
	tests_u();
	tests_x(0);
	tests_x(1);
	tests_percent();
	tests_mix();
	tests_ret();
}
