#ifndef TDF_VERSION_H
# define TDF_VERSION_H

# define TDF_VERSION "3.0.0-dev"
# define TDF_DATE "2026-11-11"

# if defined(TDF_PRINTF)
#  define TDF_GAME "printf"
# elif defined(TDF_EXAM)
#  define TDF_GAME "exam"
# else
#  define TDF_GAME "libft"
# endif

#endif
