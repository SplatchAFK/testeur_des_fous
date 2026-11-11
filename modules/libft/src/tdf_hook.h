#ifndef TDF_HOOK_H
# define TDF_HOOK_H
# include <stdlib.h>

void	*tdf_malloc_s(size_t n);
void	tdf_free_s(void *p);
# define malloc(n) tdf_malloc_s(n)
# define free(p) tdf_free_s(p)

#endif
