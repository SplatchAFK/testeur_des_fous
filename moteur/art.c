#include <string.h>
#include <stddef.h>
#include "fous.h"

const char	g_pal_keys[] = "KkWwgGrRoyYlvVcbBpmPnNsxqQuhzZtTjaAdDfFi";
const t_rgb	g_pal_vals[] = {
	{24, 20, 36},
	{8, 8, 14},
	{246, 246, 250},
	{204, 206, 218},
	{146, 148, 164},
	{86, 88, 104},
	{232, 64, 74},
	{150, 32, 44},
	{247, 146, 44},
	{252, 218, 66},
	{196, 152, 34},
	{156, 232, 86},
	{64, 174, 86},
	{30, 104, 54},
	{86, 222, 232},
	{74, 124, 232},
	{40, 62, 142},
	{164, 96, 226},
	{232, 74, 184},
	{255, 158, 204},
	{154, 98, 58},
	{92, 56, 30},
	{242, 204, 164},
	{255, 242, 212},
	{128, 204, 255},
	{42, 42, 74},
	{255, 120, 120},
	{255, 252, 200},
	{70, 50, 110},
	{36, 26, 60},
	{110, 220, 170},
	{40, 120, 110},
	{210, 170, 255},
	{255, 196, 90},
	{170, 100, 20},
	{60, 64, 80},
	{30, 32, 44},
	{255, 90, 40},
	{255, 200, 60},
	{190, 255, 250}
};

static const char *const g_boss0[] = {
	"..........KKKKKKKWKKKKKKKKKKKK.........",
	".........KPPPPPPPPPPPPPPPPPWPPK........",
	".........KPPWPPPPPPPPPPPPPPPPPK........",
	".........KPPPPPPPPPPPPPWPPPPPmK........",
	"..........KPPPPPPPPPWPPPPPmmmK.........",
	"..........KKKPPPPPPPPPPmmmmKKK.........",
	".........KWWWKKKPPPPmmmmKKKWWWK........",
	".........KWWWWKKKKKKKKKKKKWWWWK........",
	"........KWWWWKWWWKWWWWKWWWKWWWwK.......",
	"........KWWWKWWWWWKWWKEEWWWKWwwK.......",
	"........KWWWKWWEEWKWWKEEWWWKwwwK.......",
	"........KWWWKWWEEWKWWKWWWWWKwwwK.......",
	".........KWWWKWWWKWWWWKWWWKwwwK........",
	".........KWWWWKKKWWWWWWKKKwwwwK........",
	"..........KWWWWWWWWWWwwwwwwwwK.........",
	"...........KKWWKKKKKKKKKKwwKK..........",
	"...........yKKKKyWywywywKKKKy..........",
	"............KlKKKKKKKKKKDvvK...........",
	"...........yKlKKlDDvKKvDDvvKy..........",
	"....KKKKK...KllllvvvvvvvvvvK...KKKKK...",
	"..KKvvvvvKKyKlDDvKKvDDvKKvvKyKKvvvvvKK.",
	".KvvvvvvvvvKKlDDvKKvDDvKKvvKKvvvvvvvvvK",
	".KvvvvvvvVVKKlvvvvvvvvvvvvVKKvvvvvvvVVK",
	"..KKvvvVVKK.KvKKvDDvKKvDDVVK.KKvvvVVKK.",
	"oKkyKKKKK..yKvKKvDDvKKvDDVVKy..KKKKK...",
	"KyyyyK......KvvvvvvvvvvVVVVK...........",
	"KyyyYK.....yKvDDvKKvDDVKKVVKy..........",
	".KyYK.......KvDDvKKvDDVKKVVK...........",
	"..KK.......yKvvvvvvvvVVVVVVKy..........",
	"............KvKKvDDvKKVDDVVK...........",
	"...........yKvKKvDDVKKVDDVVKy..........",
	"..........mmmmmmmmmmmmmmmmmmmm.........",
	".........mPPPPPPPPPPPPPPPPPPPPm........",
	".......mmPPPPPPPPPPPPPPPPPPPPPPmm......",
	"......mmmmmmmmmmmmmmmmmmmmmmmmmmmm.....",
	"............mmmmmm....mmmmmm..........."
};

static const char *const g_boss1[] = {
	"..............cccccKKKrrrrr............",
	"...................KpK.................",
	"...................KpK.................",
	"..................KpppK................",
	".................KpppppK...............",
	".................KyppppK...............",
	"................KpppppppK..............",
	"................KppppWpzK..............",
	"...............KppppppzzzK.............",
	"..............KpcppppzzyzzK............",
	"............KKKKKKKKKKKKKKKKK..........",
	"..KKK...KKKKpppppppppppppppppKKKK......",
	"..KnK...KKKKppppppKKKKKppzzzzKKKK......",
	"..KnK.......KKKKKKZZZZZKKKKKK..........",
	".wKKKw........KKZZZZZZZZZKK............",
	"wiw.wiw.......KEEEEZZEEEEZK............",
	"wiw.wiw......KZEZZEZZEZZEZZK...........",
	"wiw.wiw......KZEZEEZZEZEEZZK...KKK.....",
	"wlllllw......KZEZZZZZEZZZZZK..KykyK....",
	"wlllllw.......KEEEEEZEEEEEK..Kyyyyyo...",
	"wlWlllw.......KKZZZZZZZZZKK..KyyyyyK...",
	"wlllllw........KKKZZZZZKKK....KyyyK....",
	"wlllllw.......KKKKKKKKKKKKK....KKK.....",
	"wlllllw..KKKKKhhzzzzzzzzzyyKKKKK.......",
	"wlllllw.KhhhhzzzzzzzzzzzzzzzyyyyK......",
	".ww.ww.KhyyyzzzzzzzzzzzzzzzzzyYYYK.....",
	"........KyyyyzzzzzzzzzzzzzzzYYYYK......",
	".........KKKKKyyzzzzzzzzzYYKKKKK.......",
	"...........KzzKKKKKKKKKKKKKZZK.........",
	"..........KzzzzzzzzzzzzZZZZZZZK........",
	".........KzzzzzzzzzzzZZZZZZZZZZK.......",
	".........KzzzzzzzzzzZZZZZZZZZZZK.......",
	"........KzzzzzzzzzzZZZZZZZZZZZZZK......",
	"........KzzzzzzzzzZZZZZZZZZZZZZZK......",
	".......KKKKKKKKKKKKKKKKKKKKKKKKKKK.....",
	"..KKKKKKKKKKKKK...........KKKKKKKKKKKKK"
};

static const char *const g_boss2[] = {
	"...............wwwwwwwww...............",
	".....m.........wWWWWWWWw...............",
	".....K..........KKKKKKK..........K.....",
	"....KyK.........KKKKKKK.........KoK....",
	"....KyK.........KvvvvvK.........KoK....",
	"...KyyyK.......KvvvvvvvK.......WWWWW...",
	"...KyyyK......KvvEvvvEvvK.....KoooooK..",
	"..KKKKKKK.....KvvvvvvvVVK.....KoooooK..",
	"..KrrrrrK......KvvvvvVVK.....KKKKKKKKK.",
	".KrrrrrrrK......KkkkkkK.......KbbbbbK..",
	"KrrErrrErrK......KKPKK.......KbbbbbbbK.",
	"KrrrrrrrRRK.......KcK.......KbbEbbbEbbK",
	".KrrrrrRRK........KGK.......KbbbbbbbBBK",
	"..KkkkkkK.........KKK........KbbbbbBBK.",
	"...KKPKK..........KKK.........KkkkkkK..",
	".....KKK..........KgK..........KKPKK...",
	".....KKK..........KKK..........KKK.....",
	".....KgK..........KKK..........KgK.....",
	".....KKKK.........KGK..........KKK.....",
	"......KGK.........KKK.........KKK......",
	"......KKK.........KKK.........KGK......",
	"......KKK.........KgK.........KKK......",
	"......KgK.........KKK.........KgK......",
	".KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK..",
	".KKKKKKKGKKKKKKKKKKGKKKKKKKKKKGKKKKKK..",
	"...KKWWKKKWWWWWWWWKKKWWWWWWWWKKKWKK....",
	"...KWWWWWWWWqWWWWWWWWWWWWWWqWWWWWWK....",
	"..KWWWWWWWWqqqWWWWWWWWWWWWqqqWWWWWWK...",
	"..KWWWWWWWWWqWWWWWqWWWWWWWWqWWWwwwwK...",
	"..KWWWWWqWWWWWWWWqqqWWWWqWwwwwwwwwwK...",
	"...KWWWqqqWWWWWWWWqWWWwqqqwwwwwwwwK....",
	"...KKWWWqWWWWWWWWWwwwwwwqwwwwwwwwKK....",
	".....KKKKKKKKKKKKKKKKKKKKKKKKKKKK......",
	".....YYY........................YYY....",
	".....YYY........................YYY....",
	".....YYY........................YYY...."
};

const t_spr	g_boss_spr[3] = {
	{39, 36, 0, 0, g_boss0},
	{39, 36, 0, 0, g_boss1},
	{39, 36, 1, 0, g_boss2}
};

const t_idef	g_items[] = {
	{"fond_aucun", "Aucun", "None", 0, 0, {0, 0, 0}, {0, 0, 0, 0, NULL}},
	{"fond_cite", "La Cité", "The City", 0, 60, {30, 44, 66}, {0, 0, 0, 0, NULL}},
	{"fond_fonderie", "La Fonderie", "The Foundry", 0, 80, {120, 50, 20}, {0, 0, 0, 0, NULL}},
	{"fond_puits", "Le Puits", "The Well", 0, 80, {30, 80, 90}, {0, 0, 0, 0, NULL}},
	{"fond_nuit_tour", "Nuit sur la Tour", "Night over the Tower", 0, 100, {30, 40, 70}, {0, 0, 0, 0, NULL}},
	{"cape_aucun", "Aucune", "None", 1, 0, {0, 0, 0}, {0, 0, 0, 0, NULL}},
	{"cape_voyage", "Cape de voyage", "Travel cloak", 1, 0, {64, 86, 112}, {0, 0, 0, 0, NULL}},
	{"cape_lisiere", "Cape usée de la lisière", "Worn edge cloak", 1, 70, {70, 92, 78}, {0, 0, 0, 0, NULL}},
	{"cape_brume", "Cape de brume", "Mist cloak", 1, 120, {200, 214, 222}, {0, 0, 0, 0, NULL}},
	{"cape_format", "Cape du Format", "Format cloak", 1, 150, {190, 64, 54}, {0, 0, 0, 0, NULL}},
	{"cape_veteran", "Cape du vétéran", "Veteran cloak", 1, -1, {46, 56, 80}, {0, 0, 0, 0, NULL}},
	{"jambes_jambieres", "Jambières", "Greaves", 2, 0, {170, 190, 205}, {0, 0, 0, 0, NULL}},
	{"jambes_bottes", "Bottes de marche", "Walking boots", 2, 50, {130, 90, 60}, {0, 0, 0, 0, NULL}},
	{"jambes_greves", "Grèves d'acier", "Steel greaves", 2, 90, {196, 210, 222}, {0, 0, 0, 0, NULL}},
	{"armure_cotte", "Cotte de mailles", "Chainmail", 3, 0, {150, 168, 184}, {0, 0, 0, 0, NULL}},
	{"armure_gambison", "Gambison", "Gambeson", 3, 60, {176, 164, 140}, {0, 0, 0, 0, NULL}},
	{"armure_pierre", "Plastron de pierre moussue", "Mossy stone plate", 3, 110, {90, 110, 90}, {0, 0, 0, 0, NULL}},
	{"armure_fonderie", "Armure de la Fonderie", "Foundry armour", 3, 140, {110, 90, 84}, {0, 0, 0, 0, NULL}},
	{"casque_cabosse", "Casque cabossé", "Dented helm", 4, 0, {176, 196, 210}, {0, 0, 0, 0, NULL}},
	{"casque_heaume", "Heaume à fente", "Slit great helm", 4, 90, {184, 200, 214}, {0, 0, 0, 0, NULL}},
	{"casque_capuche", "Capuche de veilleur", "Watcher's hood", 4, 70, {70, 78, 96}, {0, 0, 0, 0, NULL}},
	{"casque_pourcent", "Casque pourcent", "Percent helm", 4, 120, {234, 142, 72}, {0, 0, 0, 0, NULL}},
	{"casque_veteran", "Casque du vétéran", "Veteran helm", 4, -1, {230, 186, 70}, {0, 0, 0, 0, NULL}},
	{"yeux_fente", "Fente blanche", "White slit", 5, 0, {234, 232, 222}, {0, 0, 0, 0, NULL}},
	{"yeux_ame", "Lueur d'âme", "Soul glow", 5, 60, {118, 222, 238}, {0, 0, 0, 0, NULL}},
	{"yeux_braise", "Lueur de braise", "Ember glow", 5, 60, {234, 142, 72}, {0, 0, 0, 0, NULL}},
	{"deco_aucun", "Aucune", "None", 6, 0, {0, 0, 0}, {0, 0, 0, 0, NULL}},
	{"deco_plume", "Plume de braise", "Ember feather", 6, 0, {234, 142, 72}, {0, 0, 0, 0, NULL}},
	{"deco_plume_ame", "Plume d'âme", "Soul feather", 6, 60, {118, 222, 238}, {0, 0, 0, 0, NULL}},
	{"deco_echarpe", "Écharpe de fil d'or", "Gold thread scarf", 6, 80, {246, 210, 110}, {0, 0, 0, 0, NULL}},
	{"arme_aucun", "Aucune", "None", 7, 0, {0, 0, 0}, {0, 0, 0, 0, NULL}},
	{"arme_epee", "Épée de la lisière", "Edge sword", 7, 0, {230, 236, 240}, {0, 0, 0, 0, NULL}},
	{"arme_lance", "Lance", "Spear", 7, 80, {200, 210, 220}, {0, 0, 0, 0, NULL}},
	{"arme_marteau", "Marteau de forge", "Forge hammer", 7, 100, {150, 150, 156}, {0, 0, 0, 0, NULL}},
	{"arme_fil", "Fil du Tisseur", "Weaver's thread", 7, 130, {255, 214, 120}, {0, 0, 0, 0, NULL}},
	{"main_aucun", "Aucune", "None", 8, 0, {0, 0, 0}, {0, 0, 0, 0, NULL}},
	{"main_lanterne", "Lanterne à âme", "Soul lantern", 8, 0, {118, 222, 238}, {0, 0, 0, 0, NULL}},
	{"main_bouclier", "Bouclier de chêne", "Oak shield", 8, 0, {140, 90, 50}, {0, 0, 0, 0, NULL}},
	{"main_norme", "Livre de la Norme", "Book of the Norm", 8, 70, {70, 100, 180}, {0, 0, 0, 0, NULL}},
	{"main_braise", "Braise de la Fonderie", "Foundry ember", 8, 60, {234, 142, 72}, {0, 0, 0, 0, NULL}},
	{"familier_aucun", "Aucun", "None", 9, 0, {0, 0, 0}, {0, 0, 0, 0, NULL}},
	{"fam_moulinette", "Moulinette", "Moulinette", 9, -1, {124, 130, 144}, {0, 0, 0, 0, NULL}},
	{"fam_luciole", "Luciole d'âme", "Soul firefly", 9, 80, {200, 255, 170}, {0, 0, 0, 0, NULL}},
	{"fam_rouage", "Petit rouage", "Little cog", 9, 100, {210, 170, 90}, {0, 0, 0, 0, NULL}},
	{"fam_corbeau", "Corbeau de la Cité", "City crow", 9, 120, {50, 56, 70}, {0, 0, 0, 0, NULL}},
	{"aura_aucune", "Aucune", "None", 10, 0, {0, 0, 0}, {0, 0, 0, 0, NULL}},
	{"aura_spores", "Spores d'âme", "Soul spores", 10, 90, {118, 222, 238}, {0, 0, 0, 0, NULL}},
	{"aura_braises", "Braises", "Embers", 10, 110, {234, 142, 72}, {0, 0, 0, 0, NULL}},
	{"aura_brume", "Brume", "Mist", 10, 100, {200, 214, 222}, {0, 0, 0, 0, NULL}}
};

const int	g_nitems = 49;

int	item_find(const char *id)
{
	int	i;

	for (i = 0; i < g_nitems; i++)
		if (!strcmp(g_items[i].id, id))
			return (i);
	return (-1);
}

void	look_default(t_look *lk)
{
	static const char	*def[NSLOT] = {"fond_aucun", "cape_voyage",
		"jambes_jambieres", "armure_cotte", "casque_cabosse", "yeux_fente",
		"deco_plume", "arme_epee", "main_lanterne", "familier_aucun", "aura_aucune"};
	int					i;

	for (i = 0; i < NSLOT; i++)
	{
		lk->eq[i] = item_find(def[i]);
		lk->col[i] = 0;
	}
}
