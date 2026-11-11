#include <stddef.h>
#include "libft.h"

#define P(name, type) _Static_assert(_Generic(&ft_##name, type: 1, default: 0), \
	"ft_" #name " : le prototype ne colle pas au sujet")

#ifdef F_isalpha
P(isalpha, int (*)(int));
#endif
#ifdef F_isdigit
P(isdigit, int (*)(int));
#endif
#ifdef F_isalnum
P(isalnum, int (*)(int));
#endif
#ifdef F_isascii
P(isascii, int (*)(int));
#endif
#ifdef F_isprint
P(isprint, int (*)(int));
#endif
#ifdef F_strlen
P(strlen, size_t (*)(const char *));
#endif
#ifdef F_memset
P(memset, void *(*)(void *, int, size_t));
#endif
#ifdef F_bzero
P(bzero, void (*)(void *, size_t));
#endif
#ifdef F_memcpy
P(memcpy, void *(*)(void *, const void *, size_t));
#endif
#ifdef F_memmove
P(memmove, void *(*)(void *, const void *, size_t));
#endif
#ifdef F_strlcpy
P(strlcpy, size_t (*)(char *, const char *, size_t));
#endif
#ifdef F_strlcat
P(strlcat, size_t (*)(char *, const char *, size_t));
#endif
#ifdef F_toupper
P(toupper, int (*)(int));
#endif
#ifdef F_tolower
P(tolower, int (*)(int));
#endif
#ifdef F_strchr
P(strchr, char *(*)(const char *, int));
#endif
#ifdef F_strrchr
P(strrchr, char *(*)(const char *, int));
#endif
#ifdef F_strncmp
P(strncmp, int (*)(const char *, const char *, size_t));
#endif
#ifdef F_memchr
P(memchr, void *(*)(const void *, int, size_t));
#endif
#ifdef F_memcmp
P(memcmp, int (*)(const void *, const void *, size_t));
#endif
#ifdef F_strnstr
P(strnstr, char *(*)(const char *, const char *, size_t));
#endif
#ifdef F_atoi
P(atoi, int (*)(const char *));
#endif
#ifdef F_calloc
P(calloc, void *(*)(size_t, size_t));
#endif
#ifdef F_strdup
P(strdup, char *(*)(const char *));
#endif
#ifdef F_substr
P(substr, char *(*)(char const *, unsigned int, size_t));
#endif
#ifdef F_strjoin
P(strjoin, char *(*)(char const *, char const *));
#endif
#ifdef F_strtrim
P(strtrim, char *(*)(char const *, char const *));
#endif
#ifdef F_split
P(split, char **(*)(char const *, char));
#endif
#ifdef F_itoa
P(itoa, char *(*)(int));
#endif
#ifdef F_strmapi
P(strmapi, char *(*)(char const *, char (*)(unsigned int, char)));
#endif
#ifdef F_striteri
P(striteri, void (*)(char *, void (*)(unsigned int, char *)));
#endif
#ifdef F_putchar_fd
P(putchar_fd, void (*)(char, int));
#endif
#ifdef F_putstr_fd
P(putstr_fd, void (*)(char *, int));
#endif
#ifdef F_putendl_fd
P(putendl_fd, void (*)(char *, int));
#endif
#ifdef F_putnbr_fd
P(putnbr_fd, void (*)(int, int));
#endif
#ifdef F_lstnew
P(lstnew, t_list *(*)(void *));
#endif
#ifdef F_lstadd_front
P(lstadd_front, void (*)(t_list **, t_list *));
#endif
#ifdef F_lstsize
_Static_assert(_Generic(&ft_lstsize, int (*)(t_list *): 1,
	unsigned int (*)(t_list *): 1, default: 0),
	"ft_lstsize : int ou unsigned int ft_lstsize(t_list *lst)");
#endif
#ifdef F_lstlast
P(lstlast, t_list *(*)(t_list *));
#endif
#ifdef F_lstadd_back
P(lstadd_back, void (*)(t_list **, t_list *));
#endif
#ifdef F_lstdelone
P(lstdelone, void (*)(t_list *, void (*)(void *)));
#endif
#ifdef F_lstclear
P(lstclear, void (*)(t_list **, void (*)(void *)));
#endif
#ifdef F_lstiter
P(lstiter, void (*)(t_list *, void (*)(void *)));
#endif
#ifdef F_lstmap
P(lstmap, t_list *(*)(t_list *, void *(*)(void *), void (*)(void *)));
#endif
#ifdef F_t_list
_Static_assert(_Generic(((t_list *)0)->content, void *: 1, default: 0)
	&& _Generic(((t_list *)0)->next, struct s_list *: 1, default: 0)
	&& offsetof(t_list, content) == 0,
	"t_list doit etre { void *content; struct s_list *next; }");
#endif

int	g_protos_unused;
