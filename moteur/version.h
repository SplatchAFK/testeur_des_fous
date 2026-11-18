#ifndef TDF_VERSION_H
# define TDF_VERSION_H

# define TDF_VERSION "3.3.0"
# define TDF_DATE "2026-11-19"
# define TDF_NEW_ACTIFS " 3.2 3.3 "

# if defined(TDF_PRINTF)
#  define TDF_GAME "printf"
# elif defined(TDF_EXAM)
#  define TDF_GAME "exam"
# else
#  define TDF_GAME "libft"
# endif

#endif
