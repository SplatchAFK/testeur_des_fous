#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fous.h"

t_ft		g_ft;
t_shm		*g_shm;
int			g_quar[NFN];
int			g_absent[NFN];

const char	*g_fn_name[NFN] = {
	"isalpha", "isdigit", "isalnum", "isascii", "isprint", "strlen",
	"memset", "bzero", "memcpy", "memmove", "strlcpy", "strlcat",
	"toupper", "tolower", "strchr", "strrchr", "strncmp", "memchr",
	"memcmp", "strnstr", "atoi", "calloc", "strdup",
	"substr", "strjoin", "strtrim", "split", "itoa", "strmapi", "striteri",
	"putchar_fd", "putstr_fd", "putendl_fd", "putnbr_fd",
	"lstnew", "lstadd_front", "lstsize", "lstlast", "lstadd_back",
	"lstdelone", "lstclear", "lstiter", "lstmap"
};

const char	*g_fn_zone[NFN] = {
	"les touches de la scène", "les chiffres tapés au clavier",
	"le filtre des touches", "le filtre des octets du clavier", "le texte des dialogues",
	"le centrage des textes", "le nettoyage de l'image", "la remise à zéro de l'image",
	"la copie du ciel étoilé", "le ciel qui défile", "le nom du boss",
	"la barre de statut", "le nom du boss en majuscules", "les touches en minuscules",
	"la lecture des dialogues", "la fin des dialogues", "la lecture des réglages du boss",
	"la recherche des retours à la ligne", "l'affichage (seules les lignes changées sont redessinées)",
	"les variables des dialogues", "les points de vie du boss", "les images de la scène",
	"les noms des boss", "le découpage des dialogues", "le texte des points de vie",
	"le nettoyage des dialogues", "le découpage des dialogues en lignes",
	"tous les nombres de la scène", "le nom du boss en majuscules", "l'effet glitch",
	"le cœur qui bat", "tout l'affichage", "le journal des combats", "les chiffres du journal",
	"chaque étincelle", "les étincelles prioritaires", "le compteur d'étincelles",
	"la dernière étincelle", "l'ajout d'étincelles", "les étincelles qui s'éteignent",
	"le nettoyage des étincelles", "le dessin des étincelles", "l'explosion finale du boss"
};

void	tdf_track(int i)
{
	if (g_shm)
	{
		g_shm->cur = i;
		g_shm->used[i]++;
	}
}

int	fn_index(const char *name)
{
	int	i;

	if (strncmp(name, "ft_", 3) == 0)
		name += 3;
	for (i = 0; i < NFN; i++)
		if (strcmp(g_fn_name[i], name) == 0)
			return (i);
	return (-1);
}

static int	fb_isalpha(int c) { return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')); }
static int	fb_isdigit(int c) { return (c >= '0' && c <= '9'); }
static int	fb_isalnum(int c) { return (fb_isalpha(c) || fb_isdigit(c)); }
static int	fb_isascii(int c) { return (c >= 0 && c <= 127); }
static int	fb_isprint(int c) { return (c >= 32 && c <= 126); }
static size_t	fb_strlen(const char *s) { return (strlen(s)); }
static void	*fb_memset(void *s, int c, size_t n) { return (memset(s, c, n)); }
static void	fb_bzero(void *s, size_t n) { memset(s, 0, n); }
static void	*fb_memcpy(void *d, const void *s, size_t n) { return (memcpy(d, s, n)); }
static void	*fb_memmove(void *d, const void *s, size_t n) { return (memmove(d, s, n)); }
static int	fb_toupper(int c) { return ((c >= 'a' && c <= 'z') ? c - 32 : c); }
static int	fb_tolower(int c) { return ((c >= 'A' && c <= 'Z') ? c + 32 : c); }
static char	*fb_strchr(const char *s, int c) { return (strchr(s, c)); }
static char	*fb_strrchr(const char *s, int c) { return (strrchr(s, c)); }
static int	fb_strncmp(const char *a, const char *b, size_t n) { return (strncmp(a, b, n)); }
static void	*fb_memchr(const void *s, int c, size_t n) { return (memchr(s, c, n)); }
static int	fb_memcmp(const void *a, const void *b, size_t n) { return (memcmp(a, b, n)); }
static int	fb_atoi(const char *s) { return (atoi(s)); }
static void	*fb_calloc(size_t n, size_t s) { return (calloc(n ? n : 1, s ? s : 1)); }
static char	*fb_strdup(const char *s) { return (strdup(s)); }

static size_t	fb_strlcpy(char *d, const char *s, size_t n)
{
	size_t	l;

	l = strlen(s);
	if (n)
	{
		if (l >= n)
		{
			memcpy(d, s, n - 1);
			d[n - 1] = '\0';
		}
		else
			memcpy(d, s, l + 1);
	}
	return (l);
}

static size_t	fb_strlcat(char *d, const char *s, size_t n)
{
	size_t	dl;

	dl = strnlen(d, n);
	if (dl == n)
		return (n + strlen(s));
	return (dl + fb_strlcpy(d + dl, s, n - dl));
}

static char	*fb_strnstr(const char *b, const char *l, size_t n)
{
	size_t	ll;
	size_t	i;

	ll = strlen(l);
	if (!ll)
		return ((char *)b);
	for (i = 0; b[i] && i + ll <= n; i++)
		if (strncmp(b + i, l, ll) == 0)
			return ((char *)b + i);
	return (NULL);
}

static char	*fb_substr(char const *s, unsigned int st, size_t n)
{
	size_t	l;

	if (!s)
		return (NULL);
	l = strlen(s);
	if (st >= l)
		return (strdup(""));
	if (n > l - st)
		n = l - st;
	return (strndup(s + st, n));
}

static char	*fb_strjoin(char const *a, char const *b)
{
	char	*r;
	size_t	la;
	size_t	lb;

	if (!a || !b)
		return (NULL);
	la = strlen(a);
	lb = strlen(b);
	r = malloc(la + lb + 1);
	if (!r)
		return (NULL);
	memcpy(r, a, la);
	memcpy(r + la, b, lb + 1);
	return (r);
}

static char	*fb_strtrim(char const *s, char const *set)
{
	size_t	a;
	size_t	b;

	if (!s || !set)
		return (NULL);
	a = 0;
	b = strlen(s);
	while (s[a] && strchr(set, s[a]))
		a++;
	while (b > a && strchr(set, s[b - 1]))
		b--;
	return (strndup(s + a, b - a));
}

static char	**fb_split(char const *s, char c)
{
	char	**r;
	size_t	n;
	size_t	i;
	size_t	k;

	if (!s)
		return (NULL);
	n = 0;
	for (i = 0; s[i]; i++)
		if (s[i] != c && (i == 0 || s[i - 1] == c))
			n++;
	r = calloc(n + 1, sizeof(char *));
	if (!r)
		return (NULL);
	n = 0;
	i = 0;
	while (s[i])
	{
		while (s[i] == c)
			i++;
		k = i;
		while (s[i] && s[i] != c)
			i++;
		if (i > k)
			r[n++] = strndup(s + k, i - k);
	}
	return (r);
}

static char	*fb_itoa(int n)
{
	char	buf[16];
	long	v;
	int		i;
	int		neg;

	v = n;
	neg = v < 0;
	if (neg)
		v = -v;
	i = 15;
	buf[i] = '\0';
	do
	{
		buf[--i] = '0' + v % 10;
		v /= 10;
	} while (v);
	if (neg)
		buf[--i] = '-';
	return (strdup(buf + i));
}

static char	*fb_strmapi(char const *s, char (*f)(unsigned int, char))
{
	char			*r;
	unsigned int	i;

	if (!s || !f)
		return (NULL);
	r = strdup(s);
	if (!r)
		return (NULL);
	for (i = 0; r[i]; i++)
		r[i] = f(i, r[i]);
	return (r);
}

static void	fb_striteri(char *s, void (*f)(unsigned int, char *))
{
	unsigned int	i;

	if (!s || !f)
		return ;
	for (i = 0; s[i]; i++)
		f(i, s + i);
}

static void	fb_putchar_fd(char c, int fd) { (void)!write(fd, &c, 1); }
static void	fb_putstr_fd(char *s, int fd) { if (s) (void)!write(fd, s, strlen(s)); }
static void	fb_putendl_fd(char *s, int fd) { fb_putstr_fd(s, fd); fb_putchar_fd('\n', fd); }

static void	fb_putnbr_fd(int n, int fd)
{
	char	*s;

	s = fb_itoa(n);
	fb_putstr_fd(s, fd);
	free(s);
}

static t_list	*fb_lstnew(void *content)
{
	t_list	*n;

	n = malloc(sizeof(t_list));
	if (!n)
		return (NULL);
	n->content = content;
	n->next = NULL;
	return (n);
}

static void	fb_lstadd_front(t_list **l, t_list *n)
{
	if (!l || !n)
		return ;
	n->next = *l;
	*l = n;
}

static unsigned int	fb_lstsize(t_list *l)
{
	unsigned int	n;

	n = 0;
	for (; l; l = l->next)
		n++;
	return (n);
}

static t_list	*fb_lstlast(t_list *l)
{
	while (l && l->next)
		l = l->next;
	return (l);
}

static void	fb_lstadd_back(t_list **l, t_list *n)
{
	if (!l || !n)
		return ;
	if (!*l)
		*l = n;
	else
		fb_lstlast(*l)->next = n;
}

static void	fb_lstdelone(t_list *l, void (*del)(void *))
{
	if (!l || !del)
		return ;
	del(l->content);
	free(l);
}

static void	fb_lstclear(t_list **l, void (*del)(void *))
{
	t_list	*nx;

	if (!l || !del)
		return ;
	while (*l)
	{
		nx = (*l)->next;
		fb_lstdelone(*l, del);
		*l = nx;
	}
}

static void	fb_lstiter(t_list *l, void (*f)(void *))
{
	if (!f)
		return ;
	for (; l; l = l->next)
		f(l->content);
}

static t_list	*fb_lstmap(t_list *l, void *(*f)(void *), void (*d)(void *))
{
	t_list	*r;
	t_list	*n;
	void	*c;

	r = NULL;
	if (!f || !d)
		return (NULL);
	for (; l; l = l->next)
	{
		c = f(l->content);
		n = fb_lstnew(c);
		if (!n)
		{
			d(c);
			fb_lstclear(&r, d);
			return (NULL);
		}
		fb_lstadd_back(&r, n);
	}
	return (r);
}


/*
** Définitions faibles : si ta libft définit ft_x, c'est la tienne qui
** est liée. Si elle manque ou ne compile pas, la version de secours la
** remplace, y compris quand une autre de tes fonctions l'appelle :
** le jeu se lance toujours.
*/
#define WEAK __attribute__((weak))

WEAK int	ft_isalpha(int c)
{
	return (fb_isalpha(c));
}

WEAK int	ft_isdigit(int c)
{
	return (fb_isdigit(c));
}

WEAK int	ft_isalnum(int c)
{
	return (fb_isalnum(c));
}

WEAK int	ft_isascii(int c)
{
	return (fb_isascii(c));
}

WEAK int	ft_isprint(int c)
{
	return (fb_isprint(c));
}

WEAK size_t	ft_strlen(const char *s)
{
	return (fb_strlen(s));
}

WEAK void	*ft_memset(void *s, int c, size_t n)
{
	return (fb_memset(s, c, n));
}

WEAK void	ft_bzero(void *s, size_t n)
{
	fb_bzero(s, n);
}

WEAK void	*ft_memcpy(void *d, const void *s, size_t n)
{
	return (fb_memcpy(d, s, n));
}

WEAK void	*ft_memmove(void *d, const void *s, size_t n)
{
	return (fb_memmove(d, s, n));
}

WEAK size_t	ft_strlcpy(char *d, const char *s, size_t n)
{
	return (fb_strlcpy(d, s, n));
}

WEAK size_t	ft_strlcat(char *d, const char *s, size_t n)
{
	return (fb_strlcat(d, s, n));
}

WEAK int	ft_toupper(int c)
{
	return (fb_toupper(c));
}

WEAK int	ft_tolower(int c)
{
	return (fb_tolower(c));
}

WEAK char	*ft_strchr(const char *s, int c)
{
	return (fb_strchr(s, c));
}

WEAK char	*ft_strrchr(const char *s, int c)
{
	return (fb_strrchr(s, c));
}

WEAK int	ft_strncmp(const char *a, const char *b, size_t n)
{
	return (fb_strncmp(a, b, n));
}

WEAK void	*ft_memchr(const void *s, int c, size_t n)
{
	return (fb_memchr(s, c, n));
}

WEAK int	ft_memcmp(const void *a, const void *b, size_t n)
{
	return (fb_memcmp(a, b, n));
}

WEAK char	*ft_strnstr(const char *b, const char *l, size_t n)
{
	return (fb_strnstr(b, l, n));
}

WEAK int	ft_atoi(const char *s)
{
	return (fb_atoi(s));
}

WEAK void	*ft_calloc(size_t n, size_t s)
{
	return (fb_calloc(n, s));
}

WEAK char	*ft_strdup(const char *s)
{
	return (fb_strdup(s));
}

WEAK char	*ft_substr(char const *s, unsigned int st, size_t n)
{
	return (fb_substr(s, st, n));
}

WEAK char	*ft_strjoin(char const *a, char const *b)
{
	return (fb_strjoin(a, b));
}

WEAK char	*ft_strtrim(char const *s, char const *set)
{
	return (fb_strtrim(s, set));
}

WEAK char	**ft_split(char const *s, char c)
{
	return (fb_split(s, c));
}

WEAK char	*ft_itoa(int n)
{
	return (fb_itoa(n));
}

WEAK char	*ft_strmapi(char const *s, char (*f)(unsigned int, char))
{
	return (fb_strmapi(s, f));
}

WEAK void	ft_striteri(char *s, void (*f)(unsigned int, char *))
{
	fb_striteri(s, f);
}

WEAK void	ft_putchar_fd(char c, int fd)
{
	fb_putchar_fd(c, fd);
}

WEAK void	ft_putstr_fd(char *s, int fd)
{
	fb_putstr_fd(s, fd);
}

WEAK void	ft_putendl_fd(char *s, int fd)
{
	fb_putendl_fd(s, fd);
}

WEAK void	ft_putnbr_fd(int n, int fd)
{
	fb_putnbr_fd(n, fd);
}

WEAK t_list	*ft_lstnew(void *content)
{
	return (fb_lstnew(content));
}

WEAK void	ft_lstadd_front(t_list **lst, t_list *n)
{
	fb_lstadd_front(lst, n);
}

WEAK unsigned int	ft_lstsize(t_list *lst)
{
	return (fb_lstsize(lst));
}

WEAK t_list	*ft_lstlast(t_list *lst)
{
	return (fb_lstlast(lst));
}

WEAK void	ft_lstadd_back(t_list **lst, t_list *n)
{
	fb_lstadd_back(lst, n);
}

WEAK void	ft_lstdelone(t_list *lst, void (*del)(void *))
{
	fb_lstdelone(lst, del);
}

WEAK void	ft_lstclear(t_list **lst, void (*del)(void *))
{
	fb_lstclear(lst, del);
}

WEAK void	ft_lstiter(t_list *lst, void (*f)(void *))
{
	fb_lstiter(lst, f);
}

WEAK t_list	*ft_lstmap(t_list *l, void *(*f)(void *), void (*d)(void *))
{
	return (fb_lstmap(l, f, d));
}

/* absente = fichier manquant (skip sans détail) ou qui ne compile pas ;
** une fonction juste « non sélectionnée » reste la tienne */
#define PICK(f) do { \
	g_absent[E_##f] = (g_run.fn[E_##f].st == ST_BUILD \
		|| (g_run.fn[E_##f].st == ST_SKIP && g_run.fn[E_##f].nd == 0)); \
	g_ft.f = (g_absent[E_##f] || g_quar[E_##f]) ? fb_##f : ft_##f; \
} while (0)

void	lib_init(void)
{
	PICK(isalpha); PICK(isdigit); PICK(isalnum); PICK(isascii);
	PICK(isprint); PICK(strlen); PICK(memset); PICK(bzero); PICK(memcpy);
	PICK(memmove); PICK(strlcpy); PICK(strlcat); PICK(toupper);
	PICK(tolower); PICK(strchr); PICK(strrchr); PICK(strncmp); PICK(memchr);
	PICK(memcmp); PICK(strnstr); PICK(atoi); PICK(calloc); PICK(strdup);
	PICK(substr); PICK(strjoin); PICK(strtrim); PICK(split); PICK(itoa);
	PICK(strmapi); PICK(striteri); PICK(putchar_fd); PICK(putstr_fd);
	PICK(putendl_fd); PICK(putnbr_fd); PICK(lstnew); PICK(lstadd_front);
	PICK(lstsize); PICK(lstlast); PICK(lstadd_back); PICK(lstdelone);
	PICK(lstclear); PICK(lstiter); PICK(lstmap);
}
