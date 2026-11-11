#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct s_list
{
	void			*content;
	struct s_list	*next;
}	t_list;

#define W __attribute__((weak))

W int		ft_isalpha(int c);
W int		ft_isdigit(int c);
W int		ft_isalnum(int c);
W int		ft_isascii(int c);
W int		ft_isprint(int c);
W size_t	ft_strlen(const char *s);
W void		*ft_memset(void *s, int c, size_t n);
W void		ft_bzero(void *s, size_t n);
W void		*ft_memcpy(void *d, const void *s, size_t n);
W void		*ft_memmove(void *d, const void *s, size_t n);
W size_t	ft_strlcpy(char *d, const char *s, size_t n);
W size_t	ft_strlcat(char *d, const char *s, size_t n);
W int		ft_toupper(int c);
W int		ft_tolower(int c);
W char		*ft_strchr(const char *s, int c);
W char		*ft_strrchr(const char *s, int c);
W int		ft_strncmp(const char *a, const char *b, size_t n);
W void		*ft_memchr(const void *s, int c, size_t n);
W int		ft_memcmp(const void *a, const void *b, size_t n);
W char		*ft_strnstr(const char *b, const char *l, size_t n);
W int		ft_atoi(const char *s);
W void		*ft_calloc(size_t n, size_t s);
W char		*ft_strdup(const char *s);
W char		*ft_substr(char const *s, unsigned int st, size_t n);
W char		*ft_strjoin(char const *a, char const *b);
W char		*ft_strtrim(char const *s, char const *set);
W char		**ft_split(char const *s, char c);
W char		*ft_itoa(int n);
W char		*ft_strmapi(char const *s, char (*f)(unsigned int, char));
W void		ft_striteri(char *s, void (*f)(unsigned int, char *));
W void		ft_putchar_fd(char c, int fd);
W void		ft_putstr_fd(char *s, int fd);
W void		ft_putendl_fd(char *s, int fd);
W void		ft_putnbr_fd(int n, int fd);
W t_list	*ft_lstnew(void *content);
W void		ft_lstadd_front(t_list **l, t_list *n);
W unsigned	ft_lstsize(t_list *l);
W t_list	*ft_lstlast(t_list *l);
W void		ft_lstadd_back(t_list **l, t_list *n);
W void		ft_lstdelone(t_list *l, void (*d)(void *));
W void		ft_lstclear(t_list **l, void (*d)(void *));
W void		ft_lstiter(t_list *l, void (*f)(void *));
W t_list	*ft_lstmap(t_list *l, void *(*f)(void *), void (*d)(void *));

#define KO(...) do { printf("KO\t"); printf(__VA_ARGS__); printf("\n"); return (1); } while (0)

static int	ctype_eq(int (*f)(int), int (*ref)(int), const char *n)
{
	int	c;

	for (c = -1; c < 256; c++)
		if (!!f(c) != !!ref(c))
			KO("%s(%d) : %d, la libc dit %d", n, c, f(c), ref(c));
	return (0);
}

static int	t_isalpha(void) { return (ctype_eq(ft_isalpha, isalpha, "ft_isalpha")); }
static int	t_isdigit(void) { return (ctype_eq(ft_isdigit, isdigit, "ft_isdigit")); }
static int	t_isalnum(void) { return (ctype_eq(ft_isalnum, isalnum, "ft_isalnum")); }
static int	t_isascii(void) { return (ctype_eq(ft_isascii, isascii, "ft_isascii")); }
static int	t_isprint(void) { return (ctype_eq(ft_isprint, isprint, "ft_isprint")); }

static int	t_toupper(void)
{
	int	c;

	for (c = -1; c < 256; c++)
		if (ft_toupper(c) != toupper(c))
			KO("ft_toupper(%d) : %d, attendu %d", c, ft_toupper(c), toupper(c));
	return (0);
}

static int	t_tolower(void)
{
	int	c;

	for (c = -1; c < 256; c++)
		if (ft_tolower(c) != tolower(c))
			KO("ft_tolower(%d) : %d, attendu %d", c, ft_tolower(c), tolower(c));
	return (0);
}

static int	t_strlen(void)
{
	static char	big[1001];

	memset(big, 'a', 1000);
	if (ft_strlen("") != 0 || ft_strlen("abc") != 3 || ft_strlen(big) != 1000)
		KO("ft_strlen : mauvaise longueur sur \"\", \"abc\" ou 1000 caractères");
	return (0);
}

static int	t_memset(void)
{
	char	a[20];
	char	b[20];

	memset(a, 'x', 20);
	memset(b, 'x', 20);
	if (ft_memset(a + 2, 200, 10) != a + 2)
		KO("ft_memset : mauvaise valeur de retour");
	memset(b + 2, 200, 10);
	if (memcmp(a, b, 20))
		KO("ft_memset(s, 200, 10) : contenu différent de la libc");
	return (0);
}

static int	t_bzero(void)
{
	char	a[10];

	memset(a, 'x', 10);
	ft_bzero(a + 1, 5);
	if (a[0] != 'x' || a[1] || a[5] || a[6] != 'x')
		KO("ft_bzero(s + 1, 5) : mauvaise zone mise à zéro");
	ft_bzero(a, 0);
	return (0);
}

static int	t_memcpy(void)
{
	char	d[16];

	memset(d, 0, 16);
	if (ft_memcpy(d, "bonjour", 8) != d || strcmp(d, "bonjour"))
		KO("ft_memcpy(d, \"bonjour\", 8) : contenu ou retour faux");
	return (0);
}

static int	t_memmove(void)
{
	char	a[16];
	char	b[16];

	strcpy(a, "abcdefghij");
	strcpy(b, "abcdefghij");
	ft_memmove(a + 2, a, 6);
	memmove(b + 2, b, 6);
	if (strcmp(a, b))
		KO("ft_memmove vers la droite (zones qui se chevauchent) : \"%s\", attendu \"%s\"", a, b);
	strcpy(a, "abcdefghij");
	strcpy(b, "abcdefghij");
	ft_memmove(a, a + 3, 6);
	memmove(b, b + 3, 6);
	if (strcmp(a, b))
		KO("ft_memmove vers la gauche : \"%s\", attendu \"%s\"", a, b);
	return (0);
}

static int	t_strlcpy(void)
{
	char	d[10];

	memset(d, 'x', 10);
	if (ft_strlcpy(d, "hello", 3) != 5 || strcmp(d, "he"))
		KO("ft_strlcpy(d, \"hello\", 3) : attendu \"he\" et 5");
	if (ft_strlcpy(d, "hi", 10) != 2 || strcmp(d, "hi"))
		KO("ft_strlcpy(d, \"hi\", 10) : attendu \"hi\" et 2");
	d[0] = 'z';
	if (ft_strlcpy(d, "abc", 0) != 3 || d[0] != 'z')
		KO("ft_strlcpy(d, \"abc\", 0) : ne doit rien écrire et renvoyer 3");
	return (0);
}

static int	t_strlcat(void)
{
	char	d[12];

	strcpy(d, "ab");
	if (ft_strlcat(d, "cdef", 5) != 6 || strcmp(d, "abcd"))
		KO("ft_strlcat(\"ab\", \"cdef\", 5) : attendu \"abcd\" et 6");
	strcpy(d, "abc");
	if (ft_strlcat(d, "xyz", 2) != 5 || strcmp(d, "abc"))
		KO("ft_strlcat(\"abc\", \"xyz\", 2) : attendu \"abc\" inchangé et 5");
	return (0);
}

static int	t_strchr(void)
{
	const char	*s = "bonjour";

	if (ft_strchr(s, 'o') != s + 1 || ft_strchr(s, 'z') || ft_strchr(s, '\0') != s + 7
		|| ft_strchr(s, 'o' + 256) != s + 1)
		KO("ft_strchr : mauvais pointeur (premier 'o', absent, '\\0', 'o' + 256)");
	return (0);
}

static int	t_strrchr(void)
{
	const char	*s = "bonjour";

	if (ft_strrchr(s, 'o') != s + 4 || ft_strrchr(s, 'z') || ft_strrchr(s, '\0') != s + 7)
		KO("ft_strrchr : mauvais pointeur (dernier 'o', absent, '\\0')");
	return (0);
}

static int	sign(int v)
{
	return ((v > 0) - (v < 0));
}

static int	t_strncmp(void)
{
	if (sign(ft_strncmp("abc", "abd", 3)) != -1 || ft_strncmp("abc", "abd", 2) != 0
		|| sign(ft_strncmp("a\200", "a\0", 2)) != 1 || ft_strncmp("x", "y", 0) != 0)
		KO("ft_strncmp : mauvais signe (octets >= 128 en unsigned char, n = 0)");
	return (0);
}

static int	t_memchr(void)
{
	const char	s[] = "ab\0cd\200";

	if (ft_memchr(s, 'c', 6) != s + 3 || ft_memchr(s, 'z', 6) || ft_memchr(s, 0x80, 6) != s + 5)
		KO("ft_memchr : mauvais pointeur (après un '\\0', absent, octet 0x80)");
	return (0);
}

static int	t_memcmp(void)
{
	if (sign(ft_memcmp("ab\0x", "ab\0y", 4)) != -1 || ft_memcmp("ab", "ac", 1) != 0
		|| sign(ft_memcmp("\200", "\0", 1)) != 1)
		KO("ft_memcmp : mauvais signe (au-delà d'un '\\0', octets >= 128)");
	return (0);
}

static int	t_strnstr(void)
{
	const char	*b = "un chat noir";

	if (ft_strnstr(b, "chat", 12) != b + 3 || ft_strnstr(b, "chat", 6) || ft_strnstr(b, "", 3) != b
		|| ft_strnstr(b, "noir", 11))
		KO("ft_strnstr : mauvais pointeur (trouvé, coupé par len, aiguille vide)");
	return (0);
}

static int	t_atoi(void)
{
	const char	*v[] = {"42", "  -17", "\t\n+8x", "--5", "0", "2147483647", "-2147483648", " +-1", ""};
	int			i;

	for (i = 0; i < 9; i++)
		if (ft_atoi(v[i]) != atoi(v[i]))
			KO("ft_atoi(\"%s\") : %d, attendu %d", v[i], ft_atoi(v[i]), atoi(v[i]));
	return (0);
}

static int	t_calloc(void)
{
	unsigned char	*p;
	int				i;

	p = ft_calloc(7, 3);
	if (!p)
		KO("ft_calloc(7, 3) renvoie NULL");
	for (i = 0; i < 21; i++)
		if (p[i])
			KO("ft_calloc(7, 3) : octet %d non nul", i);
	free(p);
	p = ft_calloc(0, 0);
	free(p);
	return (0);
}

static int	t_strdup(void)
{
	char	*d;

	d = ft_strdup("salut");
	if (!d || strcmp(d, "salut"))
		KO("ft_strdup(\"salut\") : copie fausse");
	free(d);
	return (0);
}

static int	t_substr(void)
{
	char	*s;

	s = ft_substr("bonjour", 3, 2);
	if (!s || strcmp(s, "jo"))
		KO("ft_substr(\"bonjour\", 3, 2) : attendu \"jo\"");
	free(s);
	s = ft_substr("bonjour", 10, 3);
	if (!s || strcmp(s, ""))
		KO("ft_substr(\"bonjour\", 10, 3) : attendu \"\"");
	free(s);
	s = ft_substr("abc", 1, 100);
	if (!s || strcmp(s, "bc"))
		KO("ft_substr(\"abc\", 1, 100) : attendu \"bc\"");
	free(s);
	return (0);
}

static int	t_strjoin(void)
{
	char	*s;

	s = ft_strjoin("abc", "def");
	if (!s || strcmp(s, "abcdef"))
		KO("ft_strjoin(\"abc\", \"def\") : attendu \"abcdef\"");
	free(s);
	s = ft_strjoin("", "");
	if (!s || strcmp(s, ""))
		KO("ft_strjoin(\"\", \"\") : attendu \"\"");
	free(s);
	return (0);
}

static int	t_strtrim(void)
{
	char	*s;

	s = ft_strtrim("  xx bonjour x ", " x");
	if (!s || strcmp(s, "bonjour"))
		KO("ft_strtrim(\"  xx bonjour x \", \" x\") : attendu \"bonjour\"");
	free(s);
	s = ft_strtrim("xxxx", "x");
	if (!s || strcmp(s, ""))
		KO("ft_strtrim(\"xxxx\", \"x\") : attendu \"\"");
	free(s);
	return (0);
}

static int	t_split(void)
{
	const char	*exp[] = {"un", "deux", "trois"};
	char		**t;
	int			i;

	t = ft_split("  un  deux trois ", ' ');
	if (!t)
		KO("ft_split(\"  un  deux trois \", ' ') renvoie NULL");
	for (i = 0; i < 3; i++)
		if (!t[i] || strcmp(t[i], exp[i]))
			KO("ft_split : mot %d \"%s\", attendu \"%s\"", i, t[i] ? t[i] : "NULL", exp[i]);
	if (t[3])
		KO("ft_split : le tableau doit finir par NULL après 3 mots");
	for (i = 0; t[i]; i++)
		free(t[i]);
	free(t);
	t = ft_split("", ' ');
	if (!t || t[0])
		KO("ft_split(\"\", ' ') : attendu un tableau avec seulement NULL");
	free(t);
	return (0);
}

static int	t_itoa(void)
{
	const int	v[] = {0, -1, 42, INT_MAX, INT_MIN};
	char		e[24];
	char		*s;
	int			i;

	for (i = 0; i < 5; i++)
	{
		s = ft_itoa(v[i]);
		snprintf(e, sizeof(e), "%d", v[i]);
		if (!s || strcmp(s, e))
			KO("ft_itoa(%d) : \"%s\", attendu \"%s\"", v[i], s ? s : "NULL", e);
		free(s);
	}
	return (0);
}

static char	up_odd(unsigned int i, char c)
{
	return (i % 2 ? (char)toupper(c) : c);
}

static void	up_even(unsigned int i, char *c)
{
	if (i % 2 == 0)
		*c = (char)toupper(*c);
}

static int	t_strmapi(void)
{
	char	*s;

	s = ft_strmapi("abcd", up_odd);
	if (!s || strcmp(s, "aBcD"))
		KO("ft_strmapi(\"abcd\", majuscule aux index impairs) : attendu \"aBcD\"");
	free(s);
	return (0);
}

static int	t_striteri(void)
{
	char	s[] = "abcd";

	ft_striteri(s, up_even);
	if (strcmp(s, "AbCd"))
		KO("ft_striteri(\"abcd\", majuscule aux index pairs) : attendu \"AbCd\"");
	return (0);
}

static int	fd_out(int which, char *buf, size_t n)
{
	int		p[2];
	ssize_t	r;

	if (pipe(p))
		return (-1);
	if (which == 0)
		ft_putchar_fd('Z', p[1]);
	else if (which == 1)
		ft_putstr_fd("salut", p[1]);
	else if (which == 2)
		ft_putendl_fd("ligne", p[1]);
	else if (which == 3)
		ft_putnbr_fd(INT_MIN, p[1]);
	else
		ft_putnbr_fd(0, p[1]);
	close(p[1]);
	r = read(p[0], buf, n - 1);
	buf[r > 0 ? r : 0] = '\0';
	close(p[0]);
	return (0);
}

static int	t_putchar_fd(void)
{
	char	b[64];

	fd_out(0, b, sizeof(b));
	if (strcmp(b, "Z"))
		KO("ft_putchar_fd('Z', fd) : écrit \"%s\"", b);
	return (0);
}

static int	t_putstr_fd(void)
{
	char	b[64];

	fd_out(1, b, sizeof(b));
	if (strcmp(b, "salut"))
		KO("ft_putstr_fd(\"salut\", fd) : écrit \"%s\"", b);
	return (0);
}

static int	t_putendl_fd(void)
{
	char	b[64];

	fd_out(2, b, sizeof(b));
	if (strcmp(b, "ligne\n"))
		KO("ft_putendl_fd(\"ligne\", fd) : il manque le '\\n' ou le texte est faux");
	return (0);
}

static int	t_putnbr_fd(void)
{
	char	b[64];

	fd_out(3, b, sizeof(b));
	if (strcmp(b, "-2147483648"))
		KO("ft_putnbr_fd(INT_MIN, fd) : écrit \"%s\"", b);
	fd_out(4, b, sizeof(b));
	if (strcmp(b, "0"))
		KO("ft_putnbr_fd(0, fd) : écrit \"%s\"", b);
	return (0);
}

static int	g_del;

static void	del(void *p)
{
	g_del++;
	free(p);
}

static void	*dup_up(void *p)
{
	char	*s;

	s = strdup(p);
	if (s && s[0])
		s[0] = (char)toupper(s[0]);
	return (s);
}

static void	bump(void *p)
{
	((char *)p)[0] = '#';
}

static t_list	*mk(void)
{
	t_list	*l;

	l = NULL;
	ft_lstadd_back(&l, ft_lstnew(strdup("a")));
	ft_lstadd_back(&l, ft_lstnew(strdup("b")));
	ft_lstadd_back(&l, ft_lstnew(strdup("c")));
	return (l);
}

static int	t_lstnew(void)
{
	t_list	*n;
	int		x;

	n = ft_lstnew(&x);
	if (!n || n->content != &x || n->next)
		KO("ft_lstnew : content ou next faux");
	free(n);
	return (0);
}

static int	t_lstadd_front(void)
{
	t_list	*l;
	t_list	*n;

	l = ft_lstnew("b");
	n = ft_lstnew("a");
	ft_lstadd_front(&l, n);
	if (l != n || l->next == NULL || strcmp(l->next->content, "b"))
		KO("ft_lstadd_front : le nouveau maillon doit devenir la tête");
	free(l->next);
	free(l);
	return (0);
}

static int	t_lstsize(void)
{
	t_list	*l;

	if (ft_lstsize(NULL) != 0)
		KO("ft_lstsize(NULL) : attendu 0");
	l = mk();
	if (ft_lstsize(l) != 3)
		KO("ft_lstsize : attendu 3");
	ft_lstclear(&l, free);
	return (0);
}

static int	t_lstlast(void)
{
	t_list	*l;

	if (ft_lstlast(NULL))
		KO("ft_lstlast(NULL) : attendu NULL");
	l = mk();
	if (!ft_lstlast(l) || strcmp(ft_lstlast(l)->content, "c"))
		KO("ft_lstlast : attendu le maillon \"c\"");
	ft_lstclear(&l, free);
	return (0);
}

static int	t_lstadd_back(void)
{
	t_list	*l;

	l = mk();
	if (strcmp(l->content, "a") || strcmp(l->next->next->content, "c") || l->next->next->next)
		KO("ft_lstadd_back : ordre a, b, c attendu");
	ft_lstclear(&l, free);
	return (0);
}

static int	t_lstdelone(void)
{
	t_list	*n;

	g_del = 0;
	n = ft_lstnew(strdup("x"));
	ft_lstdelone(n, del);
	if (g_del != 1)
		KO("ft_lstdelone : del doit être appelée une fois");
	return (0);
}

static int	t_lstclear(void)
{
	t_list	*l;

	g_del = 0;
	l = mk();
	ft_lstclear(&l, del);
	if (g_del != 3 || l)
		KO("ft_lstclear : del 3 fois et liste mise à NULL attendus (del %d fois)", g_del);
	return (0);
}

static int	t_lstiter(void)
{
	t_list	*l;

	l = mk();
	ft_lstiter(l, bump);
	if (strcmp(l->content, "#") || strcmp(l->next->next->content, "#"))
		KO("ft_lstiter : f doit être appliquée à chaque maillon");
	ft_lstclear(&l, free);
	return (0);
}

static int	t_lstmap(void)
{
	t_list	*l;
	t_list	*m;

	l = mk();
	m = ft_lstmap(l, dup_up, free);
	if (!m || strcmp(m->content, "A") || strcmp(m->next->next->content, "C") || m->next->next->next)
		KO("ft_lstmap : nouvelle liste A, B, C attendue");
	if (strcmp(l->content, "a"))
		KO("ft_lstmap : la liste d'origine ne doit pas changer");
	ft_lstclear(&l, free);
	ft_lstclear(&m, free);
	return (0);
}

typedef struct s_t
{
	const char	*name;
	void		*fn;
	int			(*test)(void);
}	t_t;

int	main(void)
{
	const t_t	t[] = {
		{"isalpha", (void *)ft_isalpha, t_isalpha}, {"isdigit", (void *)ft_isdigit, t_isdigit},
		{"isalnum", (void *)ft_isalnum, t_isalnum}, {"isascii", (void *)ft_isascii, t_isascii},
		{"isprint", (void *)ft_isprint, t_isprint}, {"strlen", (void *)ft_strlen, t_strlen},
		{"memset", (void *)ft_memset, t_memset}, {"bzero", (void *)ft_bzero, t_bzero},
		{"memcpy", (void *)ft_memcpy, t_memcpy}, {"memmove", (void *)ft_memmove, t_memmove},
		{"strlcpy", (void *)ft_strlcpy, t_strlcpy}, {"strlcat", (void *)ft_strlcat, t_strlcat},
		{"toupper", (void *)ft_toupper, t_toupper}, {"tolower", (void *)ft_tolower, t_tolower},
		{"strchr", (void *)ft_strchr, t_strchr}, {"strrchr", (void *)ft_strrchr, t_strrchr},
		{"strncmp", (void *)ft_strncmp, t_strncmp}, {"memchr", (void *)ft_memchr, t_memchr},
		{"memcmp", (void *)ft_memcmp, t_memcmp}, {"strnstr", (void *)ft_strnstr, t_strnstr},
		{"atoi", (void *)ft_atoi, t_atoi}, {"calloc", (void *)ft_calloc, t_calloc},
		{"strdup", (void *)ft_strdup, t_strdup}, {"substr", (void *)ft_substr, t_substr},
		{"strjoin", (void *)ft_strjoin, t_strjoin}, {"strtrim", (void *)ft_strtrim, t_strtrim},
		{"split", (void *)ft_split, t_split}, {"itoa", (void *)ft_itoa, t_itoa},
		{"strmapi", (void *)ft_strmapi, t_strmapi}, {"striteri", (void *)ft_striteri, t_striteri},
		{"putchar_fd", (void *)ft_putchar_fd, t_putchar_fd}, {"putstr_fd", (void *)ft_putstr_fd, t_putstr_fd},
		{"putendl_fd", (void *)ft_putendl_fd, t_putendl_fd}, {"putnbr_fd", (void *)ft_putnbr_fd, t_putnbr_fd},
		{"lstnew", (void *)ft_lstnew, t_lstnew}, {"lstadd_front", (void *)ft_lstadd_front, t_lstadd_front},
		{"lstsize", (void *)ft_lstsize, t_lstsize}, {"lstlast", (void *)ft_lstlast, t_lstlast},
		{"lstadd_back", (void *)ft_lstadd_back, t_lstadd_back}, {"lstdelone", (void *)ft_lstdelone, t_lstdelone},
		{"lstclear", (void *)ft_lstclear, t_lstclear}, {"lstiter", (void *)ft_lstiter, t_lstiter},
		{"lstmap", (void *)ft_lstmap, t_lstmap}};
	int			n;
	int			ok;
	int			i;
	int			st;
	pid_t		pid;

	n = sizeof(t) / sizeof(t[0]);
	ok = 0;
	for (i = 0; i < n; i++)
	{
		fflush(stdout);
		if (!t[i].fn || ((i >= 34) && (!ft_lstnew || !ft_lstadd_back || !ft_lstclear)))
		{
			printf("F\tft_%s\tABSENTE\t%s\n", t[i].name, t[i].fn ? "ft_lstnew, ft_lstadd_back ou ft_lstclear manque"
				: "fonction introuvable dans la libft");
			continue ;
		}
		pid = fork();
		if (pid == 0)
		{
			printf("F\tft_%s\t", t[i].name);
			fflush(stdout);
			if (t[i].test())
			{
				fflush(stdout);
				_exit(1);
			}
			printf("OK\t\n");
			fflush(stdout);
			_exit(0);
		}
		waitpid(pid, &st, 0);
		if (WIFEXITED(st) && WEXITSTATUS(st) == 0)
			ok++;
		else if (!WIFEXITED(st) || WEXITSTATUS(st) != 1)
			printf("CRASH\tplantage ou erreur mémoire pendant le test\n");
	}
	printf("T\t%d\t%d\n", ok, n);
	return (ok == n ? 0 : 1);
}
