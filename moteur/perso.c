#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fous.h"

const t_perso	g_perso[] = {
	{"vague_blanche", PC_VAGUE, 0, 0, "Vague de lumière · blanche", "Light wave · white"},
	{"vague_or", PC_VAGUE, 1, 150, "Vague de lumière · or", "Light wave · gold"},
	{"vague_rose", PC_VAGUE, 2, 150, "Vague de lumière · rose", "Light wave · pink"},
	{"vague_verte", PC_VAGUE, 3, 150, "Vague de lumière · verte", "Light wave · green"},
	{"vague_violette", PC_VAGUE, 4, 150, "Vague de lumière · violette", "Light wave · violet"},
	{"vague_arc", PC_VAGUE, 5, 300, "Vague de lumière · arc-en-ciel", "Light wave · rainbow"},
	{"titre_normal", PC_TITRE, 0, 0, "Grand titre · droit", "Big title · upright"},
	{"titre_penche", PC_TITRE, 1, 250, "Grand titre · penché", "Big title · slanted"},
	{"titre_ombre", PC_TITRE, 2, 250, "Grand titre · ombré", "Big title · shadowed"},
	{"titre_trame", PC_TITRE, 3, 250, "Grand titre · tramé", "Big title · dotted"},
	{"fou_origine", PC_FOU, 0, 0, "Costume du fou · d'origine", "Fool's costume · original"},
	{"fou_or", PC_FOU, 1, 200, "Costume du fou · or", "Fool's costume · gold"},
	{"fou_nuit", PC_FOU, 2, 200, "Costume du fou · nuit", "Fool's costume · night"},
	{"fou_neon", PC_FOU, 3, 200, "Costume du fou · néon", "Fool's costume · neon"},
	{"fou_givre", PC_FOU, 4, 200, "Costume du fou · givre", "Fool's costume · frost"},
	{"bulle_sans", PC_BULLE, 0, 0, "Bulles · sans cadre", "Bubbles · no frame"},
	{"bulle_or", PC_BULLE, 1, 120, "Bulles · cadre doré", "Bubbles · golden frame"},
	{"bulle_cyan", PC_BULLE, 2, 120, "Bulles · cadre cyan", "Bubbles · cyan frame"},
	{"bulle_rose", PC_BULLE, 3, 120, "Bulles · cadre rose", "Bubbles · pink frame"},
	{"curseur_fleche", PC_CURSEUR, 0, 0, "Curseur · ▶", "Cursor · ▶"},
	{"curseur_losange", PC_CURSEUR, 1, 80, "Curseur · ◆", "Cursor · ◆"},
	{"curseur_etoile", PC_CURSEUR, 2, 80, "Curseur · ★", "Cursor · ★"},
	{"curseur_pointe", PC_CURSEUR, 3, 80, "Curseur · ➤", "Cursor · ➤"},
	{"curseur_rond", PC_CURSEUR, 4, 80, "Curseur · ●", "Cursor · ●"},
	{"curseur_carreau", PC_CURSEUR, 5, 80, "Curseur · ♦", "Cursor · ♦"},
	{"teinture_rouge", PC_TEINTURE, 1, 200, "Teinture rouge", "Red dye"},
	{"teinture_bleue", PC_TEINTURE, 2, 200, "Teinture bleue", "Blue dye"},
	{"teinture_verte", PC_TEINTURE, 3, 200, "Teinture verte", "Green dye"},
	{"teinture_or", PC_TEINTURE, 4, 200, "Teinture or", "Gold dye"},
	{"teinture_violette", PC_TEINTURE, 5, 200, "Teinture violette", "Violet dye"},
	{"teinture_noire", PC_TEINTURE, 6, 200, "Teinture noire", "Black dye"},
	{"embleme_sans", PC_EMBLEME, 0, 0, "Emblème · aucun", "Emblem · none"},
	{"embleme_etoile", PC_EMBLEME, 1, 100, "Emblème · ★", "Emblem · ★"},
	{"embleme_couronne", PC_EMBLEME, 2, 100, "Emblème · ♛", "Emblem · ♛"},
	{"embleme_lune", PC_EMBLEME, 3, 100, "Emblème · ☾", "Emblem · ☾"},
	{"embleme_fleur", PC_EMBLEME, 4, 100, "Emblème · ✿", "Emblem · ✿"},
	{"embleme_cavalier", PC_EMBLEME, 5, 100, "Emblème · ♞", "Emblem · ♞"},
	{"embleme_coeur", PC_EMBLEME, 6, 100, "Emblème · ♥", "Emblem · ♥"},
	{"cadre_sans", PC_CADRE, 0, 0, "Cadre du profil · aucun", "Profile frame · none"},
	{"cadre_or", PC_CADRE, 1, 150, "Cadre du profil · or", "Profile frame · gold"},
	{"cadre_cyan", PC_CADRE, 2, 150, "Cadre du profil · cyan", "Profile frame · cyan"},
	{"cadre_rose", PC_CADRE, 3, 150, "Cadre du profil · rose", "Profile frame · pink"},
	{"tenue_6", PC_TENUE, 1, 300, "6e emplacement de tenue", "6th outfit slot"},
	{"tenue_7", PC_TENUE, 2, 300, "7e emplacement de tenue", "7th outfit slot"},
	{"tenue_8", PC_TENUE, 3, 300, "8e emplacement de tenue", "8th outfit slot"},
	{"tenue_9", PC_TENUE, 4, 300, "9e emplacement de tenue", "9th outfit slot"},
	{"tenue_10", PC_TENUE, 5, 300, "10e emplacement de tenue", "10th outfit slot"}
};

const int	g_nperso = sizeof(g_perso) / sizeof(g_perso[0]);

int	perso_possede(int i)
{
	if (i < 0 || i >= g_nperso)
		return (0);
	return (g_perso[i].prix == 0 || g_save.perso[i] == '1');
}

void	perso_acheter(int i)
{
	if (i >= 0 && i < g_nperso && i < (int)sizeof(g_save.perso) - 1)
		g_save.perso[i] = '1';
}

int	perso_val(int cat)
{
	char	c;
	int		i;

	if (cat < 0 || cat >= NPC)
		return (0);
	c = g_save.perso_sel[cat];
	if (c < '1' || c > '9')
		return (0);
	for (i = 0; i < g_nperso; i++)
		if (g_perso[i].cat == cat && g_perso[i].val == c - '0')
			return (perso_possede(i) ? c - '0' : 0);
	return (0);
}

void	perso_choisir(int i)
{
	if (i < 0 || i >= g_nperso || g_perso[i].cat == PC_TEINTURE || g_perso[i].cat == PC_TENUE)
		return ;
	g_save.perso_sel[g_perso[i].cat] = (char)('0' + g_perso[i].val);
}

int	perso_choisi(int i)
{
	if (i < 0 || i >= g_nperso || g_perso[i].cat == PC_TEINTURE || g_perso[i].cat == PC_TENUE)
		return (0);
	return (perso_val(g_perso[i].cat) == g_perso[i].val);
}

int	perso_tenues(void)
{
	int	i;
	int	n;

	n = 5;
	for (i = 0; i < g_nperso; i++)
		if (g_perso[i].cat == PC_TENUE && perso_possede(i))
			n++;
	return (n > 10 ? 10 : n);
}

t_rgb	perso_vague(int x)
{
	switch (perso_val(PC_VAGUE))
	{
		case 1:
			return (rgb(255, 215, 90));
		case 2:
			return (rgb(255, 120, 200));
		case 3:
			return (rgb(120, 255, 150));
		case 4:
			return (rgb(190, 130, 255));
		case 5:
			return (hue(x / 40.0));
		default:
			return (rgb(255, 255, 255));
	}
}

t_rgb	perso_fou(t_rgb c)
{
	static const t_rgb	t[5] = {{0, 0, 0}, {240, 196, 78}, {60, 70, 150}, {255, 60, 200}, {170, 220, 255}};
	int					v;

	v = perso_val(PC_FOU);
	if (v <= 0 || v > 4)
		return (c);
	return (mix(c, t[v], 0.5));
}

int	perso_couleur_cadre(int cat, t_rgb *c)
{
	switch (perso_val(cat))
	{
		case 1:
			*c = rgb(240, 196, 78);
			return (1);
		case 2:
			*c = rgb(90, 220, 255);
			return (1);
		case 3:
			*c = rgb(255, 120, 200);
			return (1);
		default:
			return (0);
	}
}

const char	*tdf_curseur(void)
{
	static const char	*c[6] = {"▶", "◆", "★", "➤", "●", "♦"};
	int					v;

	v = perso_val(PC_CURSEUR);
	return (c[v >= 0 && v < 6 ? v : 0]);
}

const char	*perso_embleme(void)
{
	static const char	*e[7] = {"", "★ ", "♛ ", "☾ ", "✿ ", "♞ ", "♥ "};
	int					v;

	v = perso_val(PC_EMBLEME);
	return (e[v >= 0 && v < 7 ? v : 0]);
}

int	perso_teinte_possede(int t)
{
	int	i;

	if (t == 0)
		return (1);
	for (i = 0; i < g_nperso; i++)
		if (g_perso[i].cat == PC_TEINTURE && g_perso[i].val == t)
			return (perso_possede(i));
	return (0);
}

int	perso_teinte_suivante(int t)
{
	int	k;

	for (k = 1; k <= 7; k++)
		if (perso_teinte_possede((t + k) % 7))
			return ((t + k) % 7);
	return (0);
}

void	perso_teindre(unsigned char *rgbv, const unsigned char *a, int n, int t)
{
	static const t_rgb	d[7] = {{0, 0, 0}, {230, 60, 60}, {60, 120, 230}, {60, 190, 90},
		{240, 196, 78}, {160, 80, 220}, {50, 50, 62}};
	double				l;
	int					i;
	int					k;
	int					v;

	if (t <= 0 || t > 6 || !rgbv)
		return ;
	for (i = 0; i < n; i++)
	{
		if (a && !a[i])
			continue ;
		l = (rgbv[i * 3] * 0.30 + rgbv[i * 3 + 1] * 0.59 + rgbv[i * 3 + 2] * 0.11) / 160.0;
		for (k = 0; k < 3; k++)
		{
			v = (int)((k == 0 ? d[t].r : (k == 1 ? d[t].g : d[t].b)) * l * 0.8 + rgbv[i * 3 + k] * 0.2);
			rgbv[i * 3 + k] = (unsigned char)(v > 255 ? 255 : v);
		}
	}
}
