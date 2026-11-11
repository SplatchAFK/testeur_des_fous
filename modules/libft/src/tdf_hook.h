/* Injecté avant chaque fichier de ta libft pendant les tests (cc -include).
** Il redirige malloc et free vers le traqueur du testeur : taille exacte,
** fuites, double free et échecs de malloc, sans dépendre de l'OS.
** Ce fichier n'est jamais compilé dans ta libft.a. */
#ifndef TDF_HOOK_H
# define TDF_HOOK_H
# include <stdlib.h>

void	*tdf_malloc_s(size_t n);
void	tdf_free_s(void *p);
# define malloc(n) tdf_malloc_s(n)
# define free(p) tdf_free_s(p)

#endif
