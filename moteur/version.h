#ifndef TDF_VERSION_H
# define TDF_VERSION_H

# define TDF_VERSION "3.4.0"
# define TDF_DATE "2026-11-20"
# define TDF_NEW_ACTIFS " 3.3 3.4 3.5 "

# if defined(TDF_PRINTF)
#  define TDF_GAME "printf"
# elif defined(TDF_EXAM)
#  define TDF_GAME "exam"
# else
#  define TDF_GAME "libft"
# endif

#endif
