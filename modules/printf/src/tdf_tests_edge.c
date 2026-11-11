/* Le testeur des fous · version printf - cas limites, write en échec, malloc qui échoue */
#include "tdf_tests.h"
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#define TL(fmt, ...) TDF_RUN(SEV_LONE, fmt, ##__VA_ARGS__)

/* ft_printf(NULL) : le vrai printf plante (macOS) ou renvoie -1 (glibc).
   On vérifie seulement que ft_printf ne boucle pas à l'infini. */
static void	test_null_format(void)
{
	t_buf	msg;
	int		ko;
	int		warn;
	int		r;

	if (!tdf_begin(__FILE__, __LINE__, "NULL", "", SEV_UB))
		return ;
	memset(&msg, 0, sizeof(msg));
	ko = 0;
	warn = 0;
	if (tdf_child_start())
	{
		tdf_track_arm();
		r = ft_printf(NULL);
		tdf_track_disarm();
		tdf_child_end(r);
	}
	if (g_run.timed_out)
	{
		ko = 1;
		tdf_bprintf(&msg, "          \033[1;31mTIMEOUT\033[0m : ft_printf(NULL) boucle à l'infini\n");
	}
	else if (!g_run.returned)
	{
		warn = 1;
		tdf_bprintf(&msg, "          plante (%s) : comportement indéfini, le printf de "
			"macOS plante aussi (glibc renvoie -1)\n",
			WIFSIGNALED(g_run.status) ? strsignal(WTERMSIG(g_run.status)) : "erreur");
	}
	else if (g_run.ret != -1)
	{
		warn = 1;
		tdf_bprintf(&msg, "          renvoie %d (glibc renvoie -1)\n", g_run.ret);
	}
	tdf_memreport(&msg, &ko, &warn);
	tdf_record_cur(ko, warn && !ko, &msg);
	free(msg.p);
}

static void	tests_limits(void)
{
	tdf_category("limites", "cas limites (NULL, % final, conversion inconnue)");
	test_null_format();
	TL("%");
	TL("abc%");
	TL("%d%", 42);
	TL("%%%");
	TL("%s%", "x");
	TL("%c%", 'c');
	TU("%k");
	TU("%k%d", 42);
	TU("a%yb");
	TU("%v%s", "s");
	TU("%!");
	TU("%K%c", 'z');
	TU("%r%r%r");
	TU("[%y]");
}

static void	tests_write(void)
{
	int	local;

	tdf_category("write", "write en échec (fd fermé, /dev/full) : retour -1");
	TW(1, "hello");
	TW(1, "%c", 'a');
	TW(1, "%s", "abc");
	TW(1, "%s", NULL);
	TW(1, "%d", 42);
	TW(1, "%d", INT_MIN);
	TW(1, "%u", 42u);
	TW(1, "%x", 0xabcu);
	TW(1, "%X", 0xabcu);
	TW(1, "%p", (void *)&local);
	TW(1, "%%");
	TW(1, "ab%dcd%s", 1, "x");
	TW(2, "hello");
	TW(2, "%c", 'a');
	TW(2, "%s", "abc");
	TW(2, "%d", -42);
	TW(2, "%u", UINT_MAX);
	TW(2, "%x", 255u);
	TW(2, "%p", NULL);
	TW(2, "%%");
	TW(2, "%s%d%c", "abc", 12, 'z');
}

static void	tests_malloc(void)
{
	int		local;

	tdf_category("malloc", "malloc qui échoue (chaque malloc, un par un)");
	TM("hello");
	TM("%c", 'a');
	TM("%s", "chaine");
	TM("%s", NULL);
	TM("%d", INT_MIN);
	TM("%d", 123456);
	TM("%i", -42);
	TM("%u", UINT_MAX);
	TM("%x", 0xdeadbeefu);
	TM("%X", 0xdeadbeefu);
	TM("%p", (void *)&local);
	TM("%p", NULL);
	TM("%%");
	TM("%s %d %x %p %u %c %% %X %i", "s", -1, 255u, (void *)1, 7u, 'c', 10u, 0);
}

void	tdf_tests_edge(void)
{
	tests_limits();
	tests_write();
	tests_malloc();
}
