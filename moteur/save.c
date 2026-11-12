#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "fous.h"

t_save			g_save;
const char		*g_savedir;

const t_theme	g_themes[] = {
	{"nuit", "Nuit des fous", 0, {120, 200, 255}, {200, 120, 255},
		{70, 90, 160}, {28, 32, 52}, {225, 230, 245}, {110, 116, 140},
		{12, 13, 24}},
	{"synthwave", "Synthwave", 150, {255, 80, 200}, {255, 200, 60},
		{140, 40, 160}, {40, 16, 52}, {255, 225, 245}, {150, 100, 160},
		{18, 6, 28}},
	{"matrix", "Terminal vert", 150, {60, 255, 120}, {170, 255, 60},
		{20, 120, 50}, {8, 30, 14}, {190, 255, 200}, {60, 130, 80},
		{2, 10, 4}},
	{"sakura", "Sakura", 200, {255, 150, 190}, {255, 230, 240},
		{200, 90, 130}, {50, 30, 40}, {255, 235, 240}, {170, 120, 140},
		{24, 14, 20}},
	{"lave", "Volcan", 250, {255, 120, 30}, {255, 230, 80},
		{150, 40, 20}, {40, 14, 8}, {255, 230, 210}, {160, 100, 80},
		{18, 6, 4}},
	{"ocean", "Abysses", 250, {60, 200, 255}, {120, 255, 220},
		{20, 70, 120}, {6, 22, 40}, {210, 240, 255}, {90, 130, 160},
		{2, 8, 18}},
	{"papier", "Papier mâché", 300, {200, 80, 60}, {60, 120, 200},
		{150, 130, 110}, {230, 220, 200}, {40, 34, 30}, {120, 110, 100},
		{246, 240, 228}},
	{"veteran", "Or du Vétéran", -1, {255, 200, 80}, {255, 240, 170},
		{120, 90, 40}, {36, 26, 14}, {255, 244, 225}, {170, 140, 100},
		{20, 14, 6}}
};
int				g_ntheme = sizeof(g_themes) / sizeof(g_themes[0]);

static t_save	*cur;
static struct
{
	int				fmt;
	int				profile;
	int				libft;
	unsigned long	want_leg;
	unsigned long	want_p;
	unsigned long	want_l;
	int				game;
	unsigned long	want_g;
}				pres;
static char		g_other[6000];
static char		g_probe_other[6000];
static char		g_unk[4000];
static char		g_probe_unk[4000];
static char		*g_cur_unk;
static unsigned long	g_hp;
static unsigned long	g_hl;
#ifdef TDF_SECTION
static unsigned long	g_hg;
#endif

#define SAVE_FORMAT 4

static int		copy_file(const char *from, const char *to);
static void		write_file(int refresh);

static unsigned long	hash_of(const char *p)
{
	unsigned long	h;

	h = 1469598103934665603UL;
	for (; *p; p++)
		h = (h ^ (unsigned char)*p) * 1099511628211UL + 0x2a;
	return (h);
}

static unsigned long	hash_line(unsigned long h, const char *k, const char *v)
{
	for (; *k; k++)
		h = (h ^ (unsigned char)*k) * 1099511628211UL + 0x2a;
	h = (h ^ (unsigned char)'=') * 1099511628211UL + 0x2a;
	for (; *v; v++)
		h = (h ^ (unsigned char)*v) * 1099511628211UL + 0x2a;
	return ((h ^ (unsigned char)'\n') * 1099511628211UL + 0x2a);
}

typedef struct s_ancien
{
	const char	*id;
	int			prix;
}	t_ancien;

static const t_ancien	g_anciens[120] = {
	{"fond_aucun", 0},
	{"fond_nuit", 60},
	{"fond_couchant", 80},
	{"fond_damier", 50},
	{"fond_terminal", 70},
	{"fond_nuages", 60},
	{"fond_frigo", 90},
	{"fond_lave", 110},
	{"cape_aucun", 0},
	{"cape_classique", 0},
	{"cape_chaussette", 70},
	{"cape_nappe", 60},
	{"cape_rideau", 80},
	{"cape_pirate", 120},
	{"cape_serviette", 70},
	{"cape_feuilles", 90},
	{"cape_chauvesouris", 150},
	{"jambes_jambieres", 0},
	{"jambes_short", 50},
	{"jambes_tutu", 80},
	{"jambes_pattes", 60},
	{"jambes_palmes", 70},
	{"jambes_chaussettes", 40},
	{"jambes_bottes", 60},
	{"jambes_rollers", 110},
	{"armure_cotte", 0},
	{"armure_noel", 80},
	{"armure_tablier", 50},
	{"armure_gilet", 70},
	{"armure_smoking", 120},
	{"armure_peignoir", 60},
	{"armure_carton", 40},
	{"armure_foot", 70},
	{"armure_poubelle", 30},
	{"armure_kimono", 100},
	{"casque_rond", 0},
	{"casque_seau", 40},
	{"casque_casserole", 60},
	{"casque_citrouille", 90},
	{"casque_bocal", 100},
	{"casque_ampoule", 120},
	{"casque_cone", 70},
	{"casque_pot", 60},
	{"casque_conserve", 50},
	{"casque_cloche", 140},
	{"yeux_points", 0},
	{"yeux_fente", 30},
	{"yeux_ko", 40},
	{"yeux_coeurs", 60},
	{"yeux_etoiles", 60},
	{"yeux_3d", 80},
	{"yeux_cyclope", 70},
	{"yeux_dodo", 30},
	{"yeux_dollars", 90},
	{"yeux_laser", 150},
	{"deco_aucun", 0},
	{"deco_fleur", 30},
	{"deco_etiquette", 40},
	{"deco_helice", 60},
	{"deco_bougie", 50},
	{"deco_antenne", 40},
	{"deco_plume", 50},
	{"deco_glace", 80},
	{"deco_canard", 90},
	{"deco_crete", 70},
	{"deco_couronne", 100},
	{"deco_ventouse", 60},
	{"deco_champignon", 70},
	{"deco_noeud", 50},
	{"deco_wip", 80},
	{"deco_arcenciel", 160},
	{"arme_aucun", 0},
	{"arme_epee", 0},
	{"arme_baguette", 30},
	{"arme_poisson", 60},
	{"arme_fourchette", 50},
	{"arme_parapluie", 80},
	{"arme_ventouse", 40},
	{"arme_sucette", 70},
	{"arme_usb", 90},
	{"arme_balai", 50},
	{"arme_plume", 60},
	{"arme_spatule", 40},
	{"arme_guitare", 120},
	{"main_aucun", 0},
	{"main_bouclier", 0},
	{"main_poubelle", 30},
	{"main_pizza", 60},
	{"main_poele", 50},
	{"main_livre", 70},
	{"main_ballon", 40},
	{"main_lanterne", 90},
	{"main_cafe", 40},
	{"main_postit", 20},
	{"main_bouquet", 80},
	{"familier_aucun", 0},
	{"fam_pointeur", 60},
	{"fam_canard", 50},
	{"fam_escargot", 50},
	{"fam_slime", 70},
	{"fam_chat", 90},
	{"fam_bug", 80},
	{"fam_poisson", 100},
	{"fam_champignon", 60},
	{"fam_miniboss", 150},
	{"fam_nuage", 70},
	{"aura_aucune", 0},
	{"aura_etincelles", 80},
	{"aura_flammes", 120},
	{"aura_bulles", 60},
	{"aura_notes", 90},
	{"aura_coeurs", 70},
	{"aura_binaire", 100},
	{"aura_flocons", 80},
	{"cape_veteran", -1},
	{"casque_veteran", -1},
	{"fam_moulinette", -1},
	{"cape_format", 150},
	{"main_chocolat", 60},
	{"casque_pourcent", 120}
};

static char	g_items_brut[256];

static int	nouvel_objet(const char *id)
{
	static const char	*m[][2] = {{"main_chocolat", "main_braise"}, {"cape_classique", "cape_voyage"},
		{"casque_rond", "casque_cabosse"}, {"yeux_points", "yeux_fente"}, {"deco_plume", "deco_plume"}};
	size_t				i;

	for (i = 0; i < sizeof(m) / sizeof(m[0]); i++)
		if (!strcmp(id, m[i][0]))
			return (item_find(m[i][1]));
	return (item_find(id));
}

static void	migrer_objets(t_save *s)
{
	int		i;
	int		n;
	long	rembourse;

	rembourse = 0;
	memset(s->items, '0', g_nitems);
	s->items[g_nitems] = '\0';
	for (i = 0; i < g_nitems; i++)
		if (g_items[i].price == 0)
			s->items[i] = '1';
	for (i = 0; i < (int)(sizeof(g_anciens) / sizeof(g_anciens[0])) && g_items_brut[i]; i++)
	{
		if (g_items_brut[i] != '1')
			continue ;
		n = nouvel_objet(g_anciens[i].id);
		if (n >= 0)
			s->items[n] = '1';
		else if (g_anciens[i].prix > 0)
			rembourse += g_anciens[i].prix;
	}
	for (i = 0; i < NSLOT; i++)
	{
		n = s->look.eq[i];
		if (n >= 0 && n < (int)(sizeof(g_anciens) / sizeof(g_anciens[0])))
			n = nouvel_objet(g_anciens[n].id);
		else
			n = -1;
		s->look.eq[i] = (n >= 0 && g_items[n].slot == i) ? n : -1;
		s->look.col[i] = 0;
	}
	s->octets += rembourse;
}

static unsigned long	sum_legacy(const t_save *s)
{
	char	tmp[2400];
	int		i;
	size_t	n;

	n = snprintf(tmp, sizeof(tmp), "%s|%ld|%ld|%d|%s|%s|%s|%d|%s|%s|%d%d%d|%s|%s|%ld|%d",
		s->login, s->xp, s->octets, s->runs, s->cleared, g_items_brut[0] ? g_items_brut : s->items, s->titles, s->title,
		s->themes, s->theme, s->boss[0], s->boss[1], s->boss[2], s->exo, s->ach,
		s->lastreward, s->crashes);
	for (i = 0; i < NSLOT && n < sizeof(tmp) - 16; i++)
		n += snprintf(tmp + n, sizeof(tmp) - n, ",%d:%d", s->look.eq[i], s->look.col[i]);
	return (hash_of(tmp));
}

static void	profile_text(char *b, size_t sz, const t_save *s)
{
	size_t	n;
	int		i;

	n = snprintf(b, sz, "login=%s\nbio=%s\npcol=%d\nbadges=%s\nvitrine=%d,%d,%d\nbanner=%s\n"
			"pfp=%s\nxp=%ld\noctets=%ld\nitems=%s\ntitles=%s\ntitle=%d\n"
			"themes=%s\ntheme=%s\neq=", s->login, s->bio, s->pcol, s->badges, s->vitrine[0],
			s->vitrine[1], s->vitrine[2], s->banner, s->pfp, s->xp, s->octets, s->items,
			s->titles, s->title, s->themes, s->theme);
	for (i = 0; i < NSLOT && n < sz - 16; i++)
		n += snprintf(b + n, sz - n, "%d%s", s->look.eq[i], i + 1 < NSLOT ? "," : "\ncol=");
	for (i = 0; i < NSLOT && n < sz - 16; i++)
		n += snprintf(b + n, sz - n, "%d%s", s->look.col[i], i + 1 < NSLOT ? "," : "\n");
}

static void	libft_text(char *b, size_t sz, const t_save *s)
{
	snprintf(b, sz, "runs=%d\ncleared=%s\nboss=%d%d%d\nexo=%s\nach=%s\nlastreward=%ld\n"
		"crashes=%d\n", s->runs, s->cleared, s->boss[0], s->boss[1], s->boss[2], s->exo,
		s->ach, s->lastreward, s->crashes);
}

static void	fill_default(t_save *s)
{
	const char	*u;
	int			i;

	memset(s, 0, sizeof(*s));
	u = getenv("USER");
	snprintf(s->login, sizeof(s->login), "%s", u ? u : "fou");
	memset(s->badges, '0', NBADGE);
	s->vitrine[0] = -1;
	s->vitrine[1] = -1;
	s->vitrine[2] = -1;
	memset(s->cleared, '0', NFN);
	memset(s->ach, '0', 32);
	memset(s->titles, '0', NTITLE);
	memset(s->exo, '0', NEXO);
	memset(s->themes, '0', 15);
	s->themes[0] = '1';
	memset(s->items, '0', g_nitems);
	for (i = 0; i < g_nitems; i++)
		if (g_items[i].price == 0)
			s->items[i] = '1';
	s->titles[1] = '1';
	s->title = 1;
	strcpy(s->theme, "nuit");
	look_default(&s->look);
	s->cfg.tmo = 30;
	s->cfg.asan = 1;
	s->cfg.ubsan = 1;
	s->cfg.norm = 1;
	s->cfg.help = 2;
}

static const char	*save_path(void)
{
	static char	p[600];

	snprintf(p, sizeof(p), "%s/save.txt", g_savedir);
	return (p);
}

static void	set_bits(char *dst, const char *v, int n)
{
	int	i;

	for (i = 0; i < n && v[i]; i++)
		dst[i] = v[i] == '1' ? '1' : '0';
}

static void	clean_text(char *dst, size_t n, const char *v)
{
	size_t	i;

	i = 0;
	while (*v && i + 1 < n)
	{
		if ((unsigned char)*v >= 32 && *v != 127)
			dst[i++] = *v;
		v++;
	}
	dst[i] = '\0';
}

static void	clean_hex(char *dst, size_t n, const char *v)
{
	size_t	i;

	i = 0;
	while (*v && i + 1 < n)
	{
		if ((*v >= '0' && *v <= '9') || (*v >= 'a' && *v <= 'f'))
			dst[i++] = *v;
		v++;
	}
	dst[i] = '\0';
}

void	save_set_bio(const char *s)
{
	clean_text(g_save.bio, 81, s);
}

static void	read_look(const char *v, int *tab)
{
	int		i;
	char	*e;

	for (i = 0; i < NSLOT && *v; i++)
	{
		tab[i] = (int)strtol(v, &e, 10);
		if (e == v)
			break ;
		v = (*e == ',') ? e + 1 : e;
	}
}

static int	*cfg_field(const char *k)
{
	static const char	*n[] = {"anim", "col256", "aide", "tmo", "asan", "ubsan",
		"valg", "norm", "audit", "sound", "lang", "keys", "lore", "rendu"};
	int					i;

	for (i = 0; i < 14; i++)
		if (!strcmp(k, n[i]))
			return (&((int *)&cur->cfg)[i]);
	return (NULL);
}

static int	kv_profile(const char *k, const char *v)
{
	if (!strcmp(k, "login"))
		snprintf(cur->login, sizeof(cur->login), "%s", v);
	else if (!strcmp(k, "bio"))
		clean_text(cur->bio, sizeof(cur->bio), v);
	else if (!strcmp(k, "pcol"))
		cur->pcol = atoi(v);
	else if (!strcmp(k, "badges"))
		set_bits(cur->badges, v, NBADGE);
	else if (!strcmp(k, "vitrine"))
		sscanf(v, "%d,%d,%d", &cur->vitrine[0], &cur->vitrine[1], &cur->vitrine[2]);
	else if (!strcmp(k, "banner"))
		clean_hex(cur->banner, sizeof(cur->banner), v);
	else if (!strcmp(k, "pfp"))
		clean_hex(cur->pfp, sizeof(cur->pfp), v);
	else if (!strcmp(k, "xp"))
		cur->xp = atol(v);
	else if (!strcmp(k, "octets"))
		cur->octets = atol(v);
	else if (!strcmp(k, "items"))
	{
		snprintf(g_items_brut, sizeof(g_items_brut), "%s", v);
		set_bits(cur->items, v, g_nitems);
	}
	else if (!strcmp(k, "titles"))
		set_bits(cur->titles, v, NTITLE);
	else if (!strcmp(k, "title"))
		cur->title = atoi(v);
	else if (!strcmp(k, "themes"))
		set_bits(cur->themes, v, 15);
	else if (!strcmp(k, "theme"))
		snprintf(cur->theme, sizeof(cur->theme), "%s", v);
	else if (!strcmp(k, "eq"))
		read_look(v, cur->look.eq);
	else if (!strcmp(k, "col"))
		read_look(v, cur->look.col);
	else
		return (0);
	return (1);
}

static int	kv_libft(const char *k, const char *v)
{
	if (!strcmp(k, "runs"))
		cur->runs = atoi(v);
	else if (!strcmp(k, "cleared"))
		set_bits(cur->cleared, v, NFN);
	else if (!strcmp(k, "boss") && strlen(v) >= 3)
	{
		cur->boss[0] = v[0] == '1';
		cur->boss[1] = v[1] == '1';
		cur->boss[2] = v[2] == '1';
	}
	else if (!strcmp(k, "exo"))
		set_bits(cur->exo, v, NEXO);
	else if (!strcmp(k, "ach"))
		set_bits(cur->ach, v, 32);
	else if (!strcmp(k, "lastreward"))
		cur->lastreward = atol(v);
	else if (!strcmp(k, "crashes"))
		cur->crashes = atoi(v);
	else
		return (0);
	return (1);
}

static void	kv(const char *k, const char *v)
{
	int	*f;

	if (!strcmp(k, "format"))
		pres.fmt = atoi(v);
	else if (!strcmp(k, "sum"))
		pres.want_leg = strtoul(v, NULL, 16);
	else if (!strcmp(k, "sum_profile"))
		pres.want_p = strtoul(v, NULL, 16);
	else if (!strcmp(k, "sum_libft"))
		pres.want_l = strtoul(v, NULL, 16);
	else if (!strncmp(k, "cfg_", 4))
	{
		f = cfg_field(k + 4);
		if (f)
			*f = atoi(v);
	}
	else if (kv_libft(k, v))
	{
		pres.libft = 1;
		g_hl = hash_line(g_hl, k, v);
	}
	else
	{
		pres.profile = 1;
		g_hp = hash_line(g_hp, k, v);
		if (!kv_profile(k, v) && strlen(g_cur_unk) + strlen(k) + strlen(v) + 3 < sizeof(g_unk))
			sprintf(g_cur_unk + strlen(g_cur_unk), "%s=%s\n", k, v);
	}
}

static void	save_fix(t_save *s)
{
	int	i;
	int	e;

	for (i = 0; i < NSLOT; i++)
	{
		e = s->look.eq[i];
		if (e < 0 || e >= g_nitems || g_items[e].slot != i || s->items[e] != '1')
		{
			t_look	d;

			look_default(&d);
			s->look.eq[i] = d.eq[i];
		}
		if (s->look.col[i] < 0 || s->look.col[i] > 9)
			s->look.col[i] = 0;
	}
	if (s->pcol < 0 || s->pcol > 9)
		s->pcol = 0;
	for (i = 0; i < 3; i++)
		if (s->vitrine[i] < -1 || s->vitrine[i] >= NBADGE
			|| (s->vitrine[i] >= 0 && s->badges[s->vitrine[i]] != '1'))
			s->vitrine[i] = -1;
	if (strlen(s->banner) != BAN_W * BAN_H * 3)
		s->banner[0] = '\0';
	if (strlen(s->pfp) != PFP_W * PFP_H * 3)
		s->pfp[0] = '\0';
	if (s->title < 0 || s->title >= NTITLE || s->titles[s->title] != '1')
		s->title = 1;
	if (s->cfg.tmo < 5 || s->cfg.tmo > 300)
		s->cfg.tmo = 30;
	if (s->cfg.lore < 0 || s->cfg.lore > 2)
		s->cfg.lore = 0;
	if (s->cfg.rendu < 0 || s->cfg.rendu > 4)
		s->cfg.rendu = 0;
	if (s->cfg.help < 0 || s->cfg.help > 2)
		s->cfg.help = 2;
}

static int	parse_file(const char *path, t_save *out, char *other, size_t on, char *unk)
{
	FILE		*f;
	static char	line[9000];
	char		*eq;
	size_t		used;
	size_t		len;
	int			r;

	f = fopen(path, "r");
	if (!f)
		return (-1);
	g_items_brut[0] = '\0';
	fill_default(out);
	cur = out;
	memset(&pres, 0, sizeof(pres));
	other[0] = '\0';
	unk[0] = '\0';
	g_cur_unk = unk;
	g_hp = 1469598103934665603UL;
	g_hl = 1469598103934665603UL;
#ifdef TDF_SECTION
	g_hg = 1469598103934665603UL;
	game_reset();
#endif
	used = 0;
	while (fgets(line, sizeof(line), f))
	{
		line[strcspn(line, "\r\n")] = '\0';
#ifdef TDF_SECTION
		if (!strncmp(line, TDF_GAME ".", strlen(TDF_GAME) + 1) && strchr(line, '='))
		{
			eq = strchr(line, '=');
			*eq = '\0';
			g_hg = hash_line(g_hg, line, eq + 1);
			game_kv(line + strlen(TDF_GAME) + 1, eq + 1);
			pres.game = 1;
			continue ;
		}
		if (!strncmp(line, "sum_" TDF_GAME "=", strlen(TDF_GAME) + 5))
		{
			pres.want_g = strtoul(line + strlen(TDF_GAME) + 5, NULL, 16);
			continue ;
		}
		if ((strchr(line, '.') && strchr(line, '=') && strchr(line, '.') < strchr(line, '='))
			|| (!strncmp(line, "sum_", 4) && strncmp(line, "sum_profile=", 12)
				&& strncmp(line, "sum_libft=", 10)))
#else
		if (!strncmp(line, "printf.", 7) || !strncmp(line, "sum_printf=", 11)
			|| (strchr(line, '.') && strchr(line, '=') && strchr(line, '.') < strchr(line, '='))
			|| (!strncmp(line, "sum_", 4) && strncmp(line, "sum_profile=", 12)
				&& strncmp(line, "sum_libft=", 10) && strncmp(line, "sum=", 4)))
#endif
		{
			len = strlen(line);
			if (used + len + 2 < on)
			{
				memcpy(other + used, line, len);
				other[used + len] = '\n';
				used += len + 1;
				other[used] = '\0';
			}
			continue ;
		}
		eq = strchr(line, '=');
		if (!eq)
			continue ;
		*eq = '\0';
		kv(line, eq + 1);
	}
	fclose(f);
	r = 0;
	if (!pres.fmt)
		r = pres.want_leg == sum_legacy(out) ? 0 : -2;
	if (pres.fmt && pres.profile && pres.want_p != g_hp)
		r = -2;
	if (pres.fmt && pres.libft && pres.want_l != g_hl)
		r = -2;
#ifdef TDF_SECTION
	if (pres.fmt && pres.game && pres.want_g != g_hg)
		r = -2;
#endif
	if (r == 0 && pres.fmt < 4 && pres.profile)
		migrer_objets(out);
	g_items_brut[0] = '\0';
	save_fix(out);
	return (r);
}

static void	save_migrate(void)
{
	char		dir[620];
	char		dst[700];
	struct stat	st;

	snprintf(dir, sizeof(dir), "%s/copies", g_savedir);
	mkdir(dir, 0700);
	chmod(dir, 0700);
	snprintf(dst, sizeof(dst), "%s/save_avant_v%d.txt", dir, SAVE_FORMAT);
	if (stat(dst, &st) != 0 && copy_file(save_path(), dst) == 0)
		chmod(dst, 0600);
	write_file(0);
}

void	save_load(void)
{
	static t_save	tmp;
	t_cfg			keep;
	char			bad[620];
	int				r;
	int				fmt;

	fill_default(&g_save);
	g_other[0] = '\0';
	mkdir(g_savedir, 0700);
	chmod(g_savedir, 0700);
	r = parse_file(save_path(), &tmp, g_other, sizeof(g_other), g_unk);
	if (r == -1)
		return ;
	fmt = pres.fmt;
	keep = tmp.cfg;
	if (r == 0)
	{
		g_save = tmp;
		g_save.valid = 1;
		if (fmt < SAVE_FORMAT)
			save_migrate();
		return ;
	}
	snprintf(bad, sizeof(bad), "%s.triche", save_path());
	rename(save_path(), bad);
	fill_default(&g_save);
	g_other[0] = '\0';
	g_unk[0] = '\0';
	g_save.cfg = keep;
	g_save.valid = -1;
	save_fix(&g_save);
}

static void	other_refresh(void)
{
	static t_save	tmp;
	static char		unk[sizeof(g_unk)];
	char			keep[sizeof(g_other)];

#ifdef TDF_SECTION
	game_hold(0);
#endif
	memcpy(keep, g_other, sizeof(keep));
	if (parse_file(save_path(), &tmp, g_other, sizeof(g_other), unk) == -1)
		memcpy(g_other, keep, sizeof(g_other));
#ifdef TDF_SECTION
	game_hold(1);
#endif
}

static void	write_file(int refresh)
{
	FILE				*f;
	char				tmp[620];
	static char			b[13000];
	unsigned long		hp;
	unsigned long		hl;

	if (refresh)
		other_refresh();
	snprintf(tmp, sizeof(tmp), "%s.tmp", save_path());
	f = fopen(tmp, "w");
	if (!f)
		return ;
	chmod(tmp, 0600);
	fprintf(f, "format=%d\n", SAVE_FORMAT);
	profile_text(b, sizeof(b), &g_save);
	if (strlen(b) + strlen(g_unk) < sizeof(b))
		strcat(b, g_unk);
	fputs(b, f);
	hp = hash_of(b);
	libft_text(b, sizeof(b), &g_save);
	fputs(b, f);
	hl = hash_of(b);
	fprintf(f, "cfg_anim=%d\ncfg_col256=%d\ncfg_aide=%d\ncfg_tmo=%d\ncfg_asan=%d\n"
		"cfg_ubsan=%d\ncfg_valg=%d\ncfg_norm=%d\ncfg_audit=%d\ncfg_sound=%d\n"
		"cfg_lang=%d\ncfg_keys=%d\ncfg_lore=%d\ncfg_rendu=%d\nsum_profile=%lx\nsum_libft=%lx\n", g_save.cfg.anim,
		g_save.cfg.col256, g_save.cfg.help, g_save.cfg.tmo, g_save.cfg.asan,
		g_save.cfg.ubsan, g_save.cfg.valg, g_save.cfg.norm, g_save.cfg.audit,
		g_save.cfg.sound, g_save.cfg.lang, g_save.cfg.keys, g_save.cfg.lore, g_save.cfg.rendu, hp, hl);
#ifdef TDF_SECTION
	game_text(b, sizeof(b));
	fputs(b, f);
	fprintf(f, "sum_%s=%lx\n", TDF_GAME, hash_of(b));
#endif
	fputs(g_other, f);
	fclose(f);
	rename(tmp, save_path());
	cfg_export();
}

void	save_write(void)
{
	write_file(1);
}

#ifdef TDF_PRINTF
static t_psave	g_probe_pf;
#endif

int	save_probe(const char *path, t_save *out)
{
	int		r;
#ifdef TDF_PRINTF
	t_psave	pf;

	pf = g_pf;
#endif
	r = parse_file(path, out, g_probe_other, sizeof(g_probe_other), g_probe_unk);
#ifdef TDF_PRINTF
	g_probe_pf = g_pf;
	g_pf = pf;
#endif
	return (r);
}

static int	copy_file(const char *from, const char *to)
{
	FILE	*a;
	FILE	*b;
	char	buf[4096];
	size_t	n;

	a = fopen(from, "rb");
	if (!a)
		return (-1);
	b = fopen(to, "wb");
	if (!b)
	{
		fclose(a);
		return (-1);
	}
	while ((n = fread(buf, 1, sizeof(buf), a)) > 0)
		fwrite(buf, 1, n, b);
	fclose(a);
	fclose(b);
	return (0);
}

void	save_adopt(const t_save *s)
{
	t_cfg	keep;
	char	bak[620];

	snprintf(bak, sizeof(bak), "%s.bak", save_path());
	copy_file(save_path(), bak);
	keep = g_save.cfg;
	g_save = *s;
	g_save.cfg = keep;
	g_save.valid = 1;
	g_save.badges[10] = '1';
	memcpy(g_other, g_probe_other, sizeof(g_other));
	memcpy(g_unk, g_probe_unk, sizeof(g_unk));
#ifdef TDF_PRINTF
	g_pf = g_probe_pf;
#endif
	write_file(0);
}

int	save_export(const char *dest, int force)
{
	FILE	*f;

	save_write();
	f = fopen(dest, "r");
	if (f)
	{
		fclose(f);
		if (!force)
			return (-2);
	}
	return (copy_file(save_path(), dest));
}

void	cfg_export(void)
{
	FILE	*f;
	char	p[620];

	snprintf(p, sizeof(p), "%s/reglages.txt", g_savedir);
	f = fopen(p, "w");
	if (!f)
		return ;
	fprintf(f, "TIMEOUT=%d\nASAN=%d\nUBSAN=%d\nVALGRIND=%d\nNORM=%d\nAUDIT=%d\nAIDE=%d\n",
		g_save.cfg.tmo, g_save.cfg.asan, g_save.cfg.ubsan, g_save.cfg.valg,
		g_save.cfg.norm, g_save.cfg.audit, g_save.cfg.help);
	fclose(f);
}

int	item_owned(int i)
{
	return (i >= 0 && i < g_nitems && g_save.items[i] == '1');
}

void	item_own(int i)
{
	if (i >= 0 && i < g_nitems)
		g_save.items[i] = '1';
}

long	xp_for(int level)
{
	if (level <= 1)
		return (0);
	return ((long)(60 * pow(level - 1, 1.6)));
}

int	level_of(long xp)
{
	int	l;

	l = 1;
	while (xp >= xp_for(l + 1))
		l++;
	return (l);
}

int	ach_has(int i)
{
	return (g_save.ach[i] == '1');
}

int	ach_give(int i)
{
	if (i < 0 || i >= NACH || ach_has(i))
		return (0);
	g_save.ach[i] = '1';
	g_save.octets += 50;
	g_save.xp += 50;
	bell();
	toast(L("SUCCÈS DÉBLOQUÉ  +50 ◈", "ACHIEVEMENT UNLOCKED  +50 ◈"),
		L(g_ach_fr[i][0], g_ach_en[i][0]), theme()->accent2);
	return (1);
}

const t_theme	*theme(void)
{
	int	i;

	for (i = 0; i < g_ntheme; i++)
		if (!strcmp(g_themes[i].id, g_save.theme))
			return (&g_themes[i]);
	return (&g_themes[0]);
}
