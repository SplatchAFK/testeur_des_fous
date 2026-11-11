#ifndef TDF_TESTS_H
# define TDF_TESTS_H
# include "tdf.h"
# include <limits.h>

# pragma GCC diagnostic ignored "-Wformat"
# pragma GCC diagnostic ignored "-Wformat-extra-args"
# pragma GCC diagnostic ignored "-Wformat-security"
# pragma GCC diagnostic ignored "-Wformat-zero-length"
# pragma GCC diagnostic ignored "-Wformat-overflow"
# pragma GCC diagnostic ignored "-Wpragmas"
# pragma GCC diagnostic ignored "-Wunknown-warning-option"
# pragma GCC diagnostic ignored "-Wnonnull"

# define TDF_FT(fmt, ...) do { \
		const char *_tf = tdf_edge(fmt); \
		int _tr; \
		tdf_track_arm(); \
		_tr = ft_printf(_tf, ##__VA_ARGS__); \
		tdf_track_disarm(); \
		tdf_child_end(_tr); \
	} while (0)

# define TDF_RUN(sev, fmt, ...) TDF_RUNS(sev, #__VA_ARGS__, fmt, ##__VA_ARGS__)
# define TDF_RUNS(sev, argstr, fmt, ...) do { \
		if (tdf_begin(__FILE__, __LINE__, #fmt, argstr, sev)) { \
			tdf_exp_start(); \
			{ int _te = printf(fmt, ##__VA_ARGS__); tdf_exp_end(_te); } \
			if (tdf_child_start()) \
				TDF_FT(fmt, ##__VA_ARGS__); \
			tdf_finish(); \
		} \
	} while (0)

# define T(fmt, ...)  TDF_RUN(SEV_STRICT, fmt, ##__VA_ARGS__)
# define TU(fmt, ...) TDF_RUN(SEV_UB, fmt, ##__VA_ARGS__)
# define TW(mode, fmt, ...) do { \
		g_child_redirect = (mode); \
		TDF_RUN(SEV_STRICT, fmt, ##__VA_ARGS__); \
		g_child_redirect = 0; \
	} while (0)

# define TM(fmt, ...) do { \
		int _tk; \
		int _tn; \
		if (!tdf_begin(__FILE__, __LINE__, #fmt, #__VA_ARGS__, SEV_STRICT)) \
			break ; \
		tdf_exp_start(); \
		{ int _te = printf(fmt, ##__VA_ARGS__); tdf_exp_end(_te); } \
		g_fail_at = 0; \
		if (tdf_child_start()) \
			TDF_FT(fmt, ##__VA_ARGS__); \
		_tn = g_shm->nalloc; \
		tdf_mf_begin(); \
		for (_tk = 1; _tk <= _tn && _tk <= 64; _tk++) { \
			g_fail_at = _tk; \
			if (tdf_child_start()) \
				TDF_FT(fmt, ##__VA_ARGS__); \
			tdf_mf_step(_tk); \
		} \
		g_fail_at = 0; \
		tdf_mf_end(_tn); \
	} while (0)

#endif
