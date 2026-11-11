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


static const char *const g_it1[] = {
	"QQQQQQQQQQQQQQQQQQQQQQQQQQQQ",
	"QQQQQQQQQQQQQQQQQQQQQQQQQQQQ",
	"QQQQQQQQQQQQhQQQQQQQQhQQQQQQ",
	"QQQhQQQQQQQQQQQQQQQQhQQQQQQQ",
	"QQQQQQQQQQQQQQQQQQQhhQQQQQQQ",
	"QQQQQQQQQQQQQQQQQQQhhQQQQQQQ",
	"QQQQQQQQQQQQQQQQQQQQhhQQQQQQ",
	"QQQQQQQQQQQQQQQQQQQQQhhQQQQQ",
	"QQQQQQQQQQQQQQQQQQQQQQQQQQQQ",
	"QQQQQQQQQQQQQQQQQhQQQQQQQQQQ",
	"QQQQQQQQQQQQQQQQQQQQQQQQQQQQ",
	"QQQQQQQQQQQQQQQQQQQQQQQQQQQQ",
	"QQQQQQQQQQQQQQQQQQQQQQQQQhQQ",
	"QQQQQQQQQQQQQQQQQQQQQQQQQQQQ",
	"QQQQQQhQQQQQQQQQQQQQQQQQQQQQ",
	"QQQQQQQQQQQQQQQQQQQQQQQQQQQQ",
	"QQQQQQQQQQQQQQQQQQQQQQQQQQQQ",
	"QQQQQQQQQQQQQQQQQQQQQQQQQQQQ",
	"QQQQQQQQQQQQQQQQQQQQQQQQQQQQ",
	"QQQQQQQQQQQQQQQQQQQQQQQhQQQQ",
	"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ",
	"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ",
	"ZZZZZZZZZhZZZZZZZZZZZZZZZZZZ",
	"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ",
	"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ",
	"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ",
	"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ",
	"ZZhZZZZZZZZZZZZZZZZZZZZZZZZZ",
	"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ",
	"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ",
	"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ",
	"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ"
};

static const char *const g_it2[] = {
	"mmmmmmmmmmmmmmmmmmmmmmmmmmmm",
	"mmmmmmmmmmmmmmmmmmmmmmmmmmmm",
	"mmmmmmmmmmmmmmmmmmmmmmmmmmmm",
	"mmmmmmmmmmmmmmmmmmmmmmmmmmmm",
	"mmmmmmmmmmmmmmmmmmmmmmmmmmmm",
	"mmmmmmmmmmmmmmmmmmmmmmmmmmmm",
	"rrrrrrrrrrrrrrrrrrrrrrrrrrrr",
	"rrrrrrrrrrrrrrrrrrrrrrrrrrrr",
	"rrrrrrrrrrrrrrrrrrrrrrrrrrrr",
	"rrrrrrrrrrrrrrrrrrrrrrrrrrrr",
	"rrrrrrrrrrrrrrrrrrrrrrrrrrrr",
	"rrrrrrrrrrrrrrrrrrrrrrrrrrrr",
	"ffffffffffffffffffffffffffff",
	"ffffffffffffffffffffffffffff",
	"ffffffffffffffffffffffffffff",
	"oooooooooooooooooooooooooooo",
	"oooooooooooooooooooooooooooo",
	"ooooooooooooyyyyyooooooooooo",
	"oooooooooooyyyyyyyoooooooooo",
	"ooooooooooyyyyyyyyyooooooooo",
	"oooooooooyyyyyyyyyyyoooooooo",
	"aaaaaaaaayyyyyyyyyyyaaaaaaaa",
	"aaaaaaaaayyyyyyyyyyyaaaaaaaa",
	"aaaaaaaaayyyyyyyyyyyaaaaaaaa",
	"FFFFFFFFFyyyyyyyyyyyFFFFFFFF",
	"FFFFFFFFFFyyyyyyyyyFFFFFFFFF",
	"zzzzpzzzzpzzzzpzzzzpzzzzpzzz",
	"zzzpzzzzpzzzzpzzzzpzzzzpzzzz",
	"zzpzzzzpzzzzpzzzzpzzzzpzzzzp",
	"zpzzzzpzzzzpzzzzpzzzzpzzzzpz",
	"pzzzzpzzzzpzzzzpzzzzpzzzzpzz",
	"zzzzpzzzzpzzzzpzzzzpzzzzpzzz"
};

static const char *const g_it3[] = {
	"rrrrWWWWrrrrWWWWrrrrWWWWrrrr",
	"rrrrWWWWrrrrWWWWrrrrWWWWrrrr",
	"rrrrWWWWrrrrWWWWrrrrWWWWrrrr",
	"rrrrWWWWrrrrWWWWrrrrWWWWrrrr",
	"WWWWrrrrWWWWrrrrWWWWrrrrWWWW",
	"WWWWrrrrWWWWrrrrWWWWrrrrWWWW",
	"WWWWrrrrWWWWrrrrWWWWrrrrWWWW",
	"WWWWrrrrWWWWrrrrWWWWrrrrWWWW",
	"rrrrWWWWrrrrWWWWrrrrWWWWrrrr",
	"rrrrWWWWrrrrWWWWrrrrWWWWrrrr",
	"rrrrWWWWrrrrWWWWrrrrWWWWrrrr",
	"rrrrWWWWrrrrWWWWrrrrWWWWrrrr",
	"WWWWrrrrWWWWrrrrWWWWrrrrWWWW",
	"WWWWrrrrWWWWrrrrWWWWrrrrWWWW",
	"WWWWrrrrWWWWrrrrWWWWrrrrWWWW",
	"WWWWrrrrWWWWrrrrWWWWrrrrWWWW",
	"rrrrWWWWrrrrWWWWrrrrWWWWrrrr",
	"rrrrWWWWrrrrWWWWrrrrWWWWrrrr",
	"rrrrWWWWrrrrWWWWrrrrWWWWrrrr",
	"rrrrWWWWrrrrWWWWrrrrWWWWrrrr",
	"WWWWrrrrWWWWrrrrWWWWrrrrWWWW",
	"WWWWrrrrWWWWrrrrWWWWrrrrWWWW",
	"WWWWrrrrWWWWrrrrWWWWrrrrWWWW",
	"WWWWrrrrWWWWrrrrWWWWrrrrWWWW",
	"rrrrWWWWrrrrWWWWrrrrWWWWrrrr",
	"rrrrWWWWrrrrWWWWrrrrWWWWrrrr",
	"rrrrWWWWrrrrWWWWrrrrWWWWrrrr",
	"rrrrWWWWrrrrWWWWrrrrWWWWrrrr",
	"WWWWrrrrWWWWrrrrWWWWrrrrWWWW",
	"WWWWrrrrWWWWrrrrWWWWrrrrWWWW",
	"WWWWrrrrWWWWrrrrWWWWrrrrWWWW",
	"WWWWrrrrWWWWrrrrWWWWrrrrWWWW"
};

static const char *const g_it4[] = {
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DvvlvvlvvlvDDDDDDDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DvvlvvlvvlvvDDDDDDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DvvlvvlvvlvvlDDDDDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DvvlvvlvvlvvlvDDDDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DvvlvvlvvlvvlvvDDDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DvvlvvlvvlvvlvvlDDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DvvlvvlvvlvvlvvlvDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DvvlvvlvvlvvlvvlvvDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DvvlvvlvvlvvlvvlvvlDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DvvlvvlvvlvvlvvlvvlvDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DDDDDDDDDDDDDDDDDDDDDDDDDDDD",
	"DvvlvvlvvlvvlvvlvvlvvDDDDDDD"
};

static const char *const g_it5[] = {
	"qqqqqqqqqqqqqqqqqqqqqqqqqqqq",
	"qqqqqqqqqqqqqqqqqqqqqqqqqqqq",
	"qqqqqqqqqqqqqqqqqqqqqqqqqqqq",
	"qqqqqqqqqqqqqqqqqqqqqqqqqqqq",
	"qqqqwwwwwqqqqqqqqqqqqqqqqqqq",
	"qqwwWWWWWwwqqqqqqqqqqqqqqqqq",
	"qwWWWWWWWWWwqqqqqqqqqqqqqqqq",
	"qwWWWWWWWWWwqqqqqqqqqqqqqqqq",
	"qqwwWWWWWwwqqqqqqqqqqqqqqqqq",
	"qqqqwwwwwqqqqqqqqwwwwwwwqqqq",
	"qqqqqqqqqqqqqqqwwWWWWWWWwwqq",
	"qqqqqqqqqqqqqqwWWWWWWWWWWWwq",
	"qqqqqqqqqqqqqqwWWWWWWWWWWWwq",
	"qqqqqqqqqqqqqqqwwWWWWWWWwwqq",
	"qqqqqqqqqqqqqqqqqwwwwwwwqqqq",
	"qqqqqqqqqqqqqqqqqqqqqqqqqqqq",
	"qqqqqqqqqqqqqqqqqqqqqqqqqqqq",
	"qqqqqqqqqqqqqqqqqqqqqqqqqqqq",
	"qqqqqqqqqqqqqqqqqqqqqqqqqqqq",
	"qqqqqqqqqqqqqqqqqqqqqqqqqqqq",
	"qqqqqqwwwwwwwqqqqqqqqqqqqqqq",
	"qqqqwwWWWWWWWwwqqqqqqqqqqqqq",
	"qqqwWWWWWWWWWWWwqqqqqqqqqqqq",
	"qqqwWWWWWWWWWWWwqqqqqqqqqqqq",
	"qqqqwwWWWWWWWwwqqqqqqqqqqqqq",
	"qqqqqqwwwwwwwqqqqqqqwwwwwqqq",
	"qqqqqqqqqqqqqqqqqqwwWWWWWwwq",
	"qqqqqqqqqqqqqqqqqwWWWWWWWWWw",
	"qqqqqqqqqqqqqqqqqwWWWWWWWWWw",
	"qqqqqqqqqqqqqqqqqqwwWWWWWwwq",
	"qqqqqqqqqqqqqqqqqqqqwwwwwqqq",
	"qqqqqqqqqqqqqqqqqqqqqqqqqqqq"
};

static const char *const g_it6[] = {
	"iiiiiiiiiiiiiiiiiiiiiiiiiiii",
	"iiiiiiiiiiiiiiiiiiiiiiiiiiii",
	"iiiiiiiiiiiiiiiiiiiiiiiiiiii",
	"iiiiiiiiiiiiiiiiiiiiiiiiiiii",
	"iiKKKKKiiiiiiiiiiiiiiiiiiiii",
	"iiKyyyKiiiiiiiiiiiiiiiiiiiii",
	"iiKyyyKiiiiiiiiiiiiiiiiiiiii",
	"iiKyyyKiiiiiiiiiiiiiiiiiiiii",
	"iiKyyyKiiiiiiiiiiiiiiiiiiiii",
	"iiKKKKKiiiiiiiiiiiiiiiiiiiii",
	"wwwwwwwwwwwwwwwwwwwwwwwwwwww",
	"iiiiiiiiiiiiiiiiiiiiiiiiiiii",
	"iiiiiiiiiiiiiiiiiiiiiiiiiiii",
	"iiiiiiiiiiiiiiiiiiiiiiiiiiii",
	"iiiiiiiiiiiiiiiiiiiiiiKKiiii",
	"iiiiiiiiiiiiiiiiiiiiiKllKiii",
	"iiiiiiiiiiiiiiiiiiiiKllllKii",
	"iiiiiiiiiiiiiiiiiiiiKllllKii",
	"iiiiiiiiiiiiiiiiiiiiKllllKii",
	"iiiiiiiiiiiiiiiiiiiiiKllKiii",
	"iiiiiiiiiiiiiiiiiiiiiiKKiiii",
	"wwwwwwwwwwwwwwwwwwwwwwwwwwww",
	"iiiiiiiiiiiiiiiiiiiiiiiiiiii",
	"iiiKKKKKKKiiiiiiiiiiiiiiiiii",
	"iiiKWWWWWKiiiiiiiiiiiiiiiiii",
	"iiiKWWWWWKiiiiiiiiiiiiiiiiii",
	"iiiKWWWWWKiiiiiiiiiiiiiiiiii",
	"iiiKWWWWwKiiiiiiiiiiiiiiiiii",
	"iiiKWWWwwKiiiiiiiiiiiiiiiiii",
	"iiiKWWwwwKiiiiiiiiiiiiiiiiii",
	"iiiKKKKKKKiiiiiiiiiiiiiiiiii",
	"iiiiiiiiiiiiiiiiiiiiiiiiiiii"
};

static const char *const g_it7[] = {
	"RRRRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRfRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRfffRRRRRRRRRRRR",
	"RRRRRRRRRRRRRfffRRRRRRRRRRRR",
	"RRRRRRRRRRRRfffffRRRRRRRRRRR",
	"RRRRRRRRRRRRKNNNKRRRRRRRRRRR",
	"RRRRRRRRRRRKNNNNNKRRRRRRRRRR",
	"RRRRRRRRRRKNNNNNNNKRRRRRRRRR",
	"RRRRRRRRRRKNNNNNNNKRRRRRRRRR",
	"RRRRRRRRRKNNNNNNNNNKRRRRRRRR",
	"RRRRRRRRKNNNNNNNNNNNKRRRRRRR",
	"RRRRRRRRKNNNNNNNNNNNKRRRRRRR",
	"RRRRRRRKNNNNNNNNNNNNNKRRRRRR",
	"RRRRRRRKNNNNNNNNNNNNNKRRRRRR",
	"RRRRRRKNNNNNNNNNNNNNNNKRRRRR",
	"RRRRRKNNNNNNNNNNNNNNNNNKRRRR",
	"RRRRRKNNNNNNNNNNNNNNNNNKRRRR",
	"ffFfKKKKKKKKKKKKKKKKKKKKKfFf",
	"fFfffFfffFfffFfffFfffFfffFff",
	"FfffFfffFfffFfffFfffFfffFfff",
	"fffFfffFfffFfffFfffFfffFfffF",
	"ffFfffFfffFfffFfffFfffFfffFf",
	"fFfffFfffFfffFfffFfffFfffFff",
	"FfffFfffFfffFfffFfffFfffFfff",
	"fffFfffFfffFfffFfffFfffFfffF",
	"ffFfffFfffFfffFfffFfffFfffFf",
	"fFfffFfffFfffFfffFfffFfffFff"
};


static const char *const g_it9[] = {
	"....KKKKKKKKKK....",
	"....K33331111K....",
	"...K3331111111K...",
	"...K3311111111K...",
	"...K3111111111K...",
	"..K111111111111K..",
	"..K111111111111K..",
	"..K111111111112K..",
	"..K111111111122K..",
	".K11111111112222K.",
	".K11111111222222K.",
	".K11111112222222K.",
	"K1111111222222222K",
	"KKKKKKKKKKKKKKKKKK"
};

static const char *const g_it10[] = {
	".KKKKKKKKKK.....",
	".K11111111K.....",
	".K11111111K.....",
	".33333333333....",
	".K11111111K.....",
	".K11111111K.....",
	".K111111111K....",
	".33333333333....",
	"K1111111111K....",
	"K1111111111K....",
	"K11111111111KKKK",
	"K3333333333KKKKK",
	"K1111111111KWWWK",
	"K1111111111KWWWK",
	"KKKKKKKKKKKKKKKK"
};

static const char *const g_it11[] = {
	"...KKKKKKKKKKKK...",
	"...KW11WW11WW1K...",
	"...KW11WW11WW1K...",
	"..K11WW11WW11WWK..",
	"..K11WW11WW11WWK..",
	"..KWW11WW11WW11K..",
	"..KWW11WW11WW11K..",
	".KW11WW11WW11WW1K.",
	".KW11WW11WW11WW1K.",
	".K1WW11WW11WW11WK.",
	".K1WW11WW11WW11WK.",
	"KWW11WW11WW11WW11K",
	"KWW11WW11WW11WW11K",
	"KKKKKKKKKKKKKKKKKK"
};

static const char *const g_it12[] = {
	".W.1W13W31W13W3.W.",
	"...131313131313...",
	"313131313131313131",
	"313131313131313131",
	"313131313131313131",
	"313131313131313131",
	"313131313131313131",
	"313131313131313131",
	"313131313131313131",
	"313131313131313131",
	"313131313131313131",
	"313131313131313131",
	"313131313131313131",
	"3.3.3.3.3.3.3.3.3."
};

static const char *const g_it13[] = {
	"...KKKKKKKKKKKK...",
	"...K1111111111K...",
	"...K1111111111K...",
	"..K111111111111K..",
	"..K1111WWWW1111K..",
	"..K111WWWWWW111K..",
	"..K111WkWWkW111K..",
	".K1111WWWWWW1111K.",
	".K11111WWWW11111K.",
	".K11111111111111K.",
	".K111WWWWWWWW111K.",
	"K1111111111111111K",
	"K1111111111111111K",
	"KKKKKKKKKKKKKKKKKK"
};

static const char *const g_it14[] = {
	"..KKKKKKKKKKKK..",
	".11111111111111.",
	"..KWWWWWWWWWWK..",
	"..KWWWWWWWWWWK..",
	".11111111111111.",
	".KWWWWWWWWWWWWK.",
	".KWWWWWWWWWWWWK.",
	".11111111111111.",
	".KWWWWWWWWWWWWK.",
	".KWWWWWWWWWWWWK.",
	".11111111111111.",
	"KWWWWWWWWWWWWWWK",
	"KWWWWWWWWWWWWWWK",
	"K11111111111111K",
	"KKKKKKKKKKKKKKKK"
};

static const char *const g_it15[] = {
	"....22..22..22.....",
	"...211221122112....",
	"...211221122112....",
	"...211221122112....",
	"..22222222222222...",
	".2112211221122112..",
	".2112211221122112..",
	".2112211221122112..",
	".222.222.222.222...",
	"211221122112211222.",
	"2112211221122112112",
	"2112211221122112112",
	".22..22..22..222112",
	"................22."
};

static const char *const g_it16[] = {
	".K........................K.",
	".KKK....................KKK.",
	"..K1KK................KK1K..",
	"..K111KKK..........KKK111K..",
	"...K11111KK......KK11111K...",
	"...K111111K......K111111K...",
	"..K1111112K......K1111111K..",
	"..K1111122K......K1111111K..",
	".KKKK11222K......K11111KKKK.",
	"K....K122K........K111K....K",
	"......K2K..........K1K......",
	"......KK............KK......",
	"......K..............K......"
};

static const char *const g_it17[] = {
	".KKK..KKK.",
	".K1K..K1K.",
	".K1K..K1K.",
	".K1K..K1K.",
	".KKK..KKK.",
	"KKKK..KKKK",
	"KKKK..KKKK"
};

static const char *const g_it18[] = {
	"KKKKKKKKKK",
	"K1y111111K",
	"K111y11W1K",
	"KKKKKKKKKK",
	".ss....ss.",
	".ss....ss.",
	".ss....ss.",
	"KKKK..KKKK"
};

static const char *const g_it19[] = {
	".....22222222.....",
	"....2111111112....",
	"...211111111112...",
	".3231313131313132.",
	"222222222222222222",
	".....WW....WW.....",
	".....WW....WW.....",
	".....WW....WW....."
};

static const char *const g_it20[] = {
	"..KKKKKKKK..",
	"..K11KK11K..",
	".K111KK111K.",
	".K111KK111K.",
	".K111KK111K.",
	".K111KK111K.",
	"K1111KK1111K",
	"KKKKKKKKKKKK"
};

static const char *const g_it21[] = {
	"....KKK..KKK....",
	"....KGK..KGK....",
	"....KGK..KGK....",
	"....KKK..KKK....",
	"....KKK..KKK....",
	"...K11K..K11K...",
	".KK1111KK1111KK.",
	"KKKKKKKKKKKKKKKK"
};

static const char *const g_it22[] = {
	"KKK..KKK",
	"333..PPP",
	"K1K..KmK",
	"333..PPP",
	"K1K..KmK",
	"KKK..KKK"
};

static const char *const g_it23[] = {
	".KKK..KKK.",
	".KGK..KGK.",
	".KKK..KKK.",
	"KKKK..KKKK",
	"K11K..K11K",
	"K11K..K11K",
	"K12K..K12K",
	"KKKK..KKKK"
};

static const char *const g_it24[] = {
	".KKK..KKK.",
	".KGK..KGK.",
	".KGK..KGK.",
	".KGK..KGK.",
	"KKKKKKKKKK",
	"K111KK111K",
	"KKKKKKKKKK",
	".K.K..K.K.",
	"y..y.y..y."
};

static const char *const g_it25[] = {
	"....KKKKKKKK....",
	".KKK33311111KKK.",
	"K11K33111111K11K",
	"K11K31111111K11K",
	"K11K11111112K11K",
	".KKKnnnynnnnKKK.",
	"...K11112222K...",
	"....KKKKKKKK...."
};

static const char *const g_it26[] = {
	"....KKKKKKKK....",
	".KKK11111111KKK.",
	"K11KW1W1W1W1K11K",
	"K11K11111111K11K",
	"K11K1v1v1v1vK11K",
	".KKK11111111KKK.",
	"...K111y1111K...",
	"....KKKKKKKK...."
};

static const char *const g_it27[] = {
	"....KKKKKKKK....",
	".KKKKKKKKKKKKKK.",
	"KbbKK111111KKbbK",
	"KbbKK111111KKbbK",
	"KbbKK1r11r1KKbbK",
	".KKKK111111KKKK.",
	"...KK111111KK...",
	"....KKKKKKKK...."
};

static const char *const g_it28[] = {
	"....KKKKKKKK....",
	".KKK333K1111KKK.",
	"KGGK331K1111KGGK",
	"KGGKWWWWWWWWKGGK",
	"KGGK111K1111KGGK",
	".KK1111K11222KK.",
	"..K1111K12222K..",
	"...K111K2222K...",
	"....KKKKKKKK...."
};

static const char *const g_it29[] = {
	"....KKWWWWKK....",
	".KKK11WrrW11KKK.",
	"K11K11WWWW11K11K",
	"K11K111WW111K11K",
	"K11K111WW111K11K",
	".KKK111WW111KKK.",
	"...K111WW111K...",
	"....KKKKKKKK...."
};

static const char *const g_it30[] = {
	"....2KKKKKKK....",
	".KKK32311111KKK.",
	"K11K32111111K11K",
	"K11K11211111K11K",
	"K11K11121111K11K",
	"K11KWWWWWWWWK11K",
	".KK1111122222KK.",
	"...K11112222K...",
	"....KKKKK2KK...."
};

static const char *const g_it31[] = {
	"...KKKKKKKKKKKK...",
	"KKKK1111111111KKKK",
	"K11K1K111111K1K11K",
	"K11K11K1111K11K11K",
	"KKKK111111K122KKKK",
	"...K1111112222K...",
	"...K1111122222K...",
	"...KKKKKKKKKKKK..."
};

static const char *const g_it32[] = {
	"....KKKKKKKK....",
	".KKK11111111KKK.",
	"K11K11WWWW11K11K",
	"K11K11W1WW11K11K",
	".KKK11WW1W11KKK.",
	"...K11WWWW11K...",
	"...K11111111K...",
	"....KKKKKKKK...."
};

static const char *const g_it33[] = {
	"......yy......",
	"...KKKKKKKK...",
	"...K331111K...",
	"..K33111111K..",
	"..K11111111K..",
	"..K11111111K..",
	".K1111111111K.",
	".K1111111111K.",
	".K1111111111K.",
	"K111111111111K",
	"KKKKKKKKKKKKKK"
};

static const char *const g_it34[] = {
	"......KKKKKKKK......",
	".....K1K1111K1K.....",
	"....KK11K11K11KK....",
	"...K1K11K11K11K1K...",
	"..K1KK111KK111KK1K..",
	".K11KrrrrrrrrrrK11K.",
	"KKKKKrrrrrrrrrrKKKKK",
	"......KKKKKKKK......"
};

static const char *const g_it35[] = {
	"....KKKKKK....",
	"...K333111K...",
	"..K33111111K..",
	".K3311111111K.",
	"K331111111111K",
	"K3KKKKKKKKKK1K",
	"K1kkkkkkkkkk2K",
	"K1kkkkkkkkkk2K",
	"K1kkkkkkkkkk2K",
	".KkkkkkkkkkkK.",
	"..K11112222K..",
	"...K112222K...",
	"....KKKKKK...."
};

static const char *const g_it36[] = {
	"...GGGGGGGGGGGGGG...",
	"...KKKKKKKKKKKKKK...",
	"..GK333331111111KG..",
	".G.K333311111111K.G.",
	".G.K333111111111K.G.",
	"G..K3kkkkkkkkkk1K..G",
	"G..K3kkkkkkkkkk2K..G",
	"....KkkkkkkkkkkK....",
	"....KkkkkkkkkkkK....",
	"....K1111111222K....",
	"....K1111122222K....",
	"....K1111222222K....",
	"....KKKKKKKKKKKK...."
};

static const char *const g_it37[] = {
	".KKKKKKKKKKKK.......",
	"K333331111111NNNNNNN",
	"K333311111111NNNNNNN",
	"K333111111111K......",
	"K3kkkkkkkkkk1K......",
	"K1kkkkkkkkkk2K......",
	"K1kkkkkkkkkk2K......",
	"K1kkkkkkkkkk2K......",
	"K111111122222K......",
	"K111112222222K......",
	".KKKKKKKKKKKK......."
};

static const char *const g_it38[] = {
	".......vv.......",
	".......vv.......",
	".......vv.......",
	".....KKvvKK.....",
	"...KK332111KK...",
	"..K3331211111K..",
	".K333112111111K.",
	"K33kkkk21kkkk11K",
	"K31kkk1211kkk11K",
	"K111kk1211kk122K",
	"K111k112111k222K",
	"K11111121112222K",
	".K11kkkkkkkk22K.",
	"..K1111222222K..",
	"...KK112222KK...",
	".....KKKKKK....."
};

static const char *const g_it39[] = {
	".KKKKKKKKKKKK.",
	".KyyyyyyyyyyK.",
	".KKKKKKKKKKKK.",
	".w11111111v1w.",
	"w11111111vvv1w",
	"w111111111v11w",
	"w111111111111w",
	"w1kkkkkkkkkk1w",
	"w1kkkkkkkkkk1w",
	"w1kkkkkkkkkk1w",
	"w1kkkkkkkkkk1w",
	"w11v111111111w",
	"w1vvv11111111w",
	".w1v11111111w.",
	"..wwwwwwwwww.."
};

static const char *const g_it40[] = {
	".....KKKKKK.....",
	"...KKWWW111KK...",
	"..KWWWW111111K..",
	".KWWWW11111111K.",
	".KWW1111111111K.",
	"KWW111111111111K",
	"KW1kkkkkkkkkk11K",
	"K11kkkkkkkkkk11K",
	"K11kkkkkkkkkk11K",
	"K11kkkkkkkkkk11K",
	".K111111111111K.",
	".K111111111111K.",
	"..K1111111111K..",
	"...KKKKKKKKKK...",
	"....KggggggK....",
	"....KKKKKKKK...."
};

static const char *const g_it41[] = {
	".......KK.......",
	".......KK.......",
	"......K11K......",
	"......K11K......",
	".....K1111K.....",
	".....K1111K.....",
	"....WWWWWWWW....",
	"....K111111K....",
	"...KkkkkkkkkK...",
	"...KkkkkkkkkK...",
	"..K1kkkkkkkk1K..",
	"..K1111111111K..",
	".K1WWWWWWWWWW1K.",
	".K111111111111K.",
	"K11111111111111K",
	"KKKKKKKKKKKKKKKK"
};

static const char *const g_it42[] = {
	"......lv........",
	".......vl.......",
	".......v........",
	"KKKKKKKKKKKKKKKK",
	"K11111111111111K",
	"KKKKKKKKKKKKKKKK",
	".K111111111111K.",
	".K111111111111K.",
	"..K1kkkkkkkk1K..",
	"..K1kkkkkkkk1K..",
	"..K1kkkkkkkk1K..",
	"..K1kkkkkkkk2K..",
	"..K1111111222K..",
	"...K11111222K...",
	"...K11122222K...",
	"...KKKKKKKKKK..."
};

static const char *const g_it43[] = {
	"wwwwwwwwwwwwww",
	"K333333111111K",
	"K333311111111K",
	"rrrrrrrrrrrrrr",
	"rrrrrrrrrrrrrr",
	"rrkkkkkkkkkkrr",
	"rrkkkkkkkkkkrr",
	"rrkkkkkkkkkkrr",
	"rrrrrrrrrrrrrr",
	"rrrrrrrrrrrrrr",
	"K111111222222K",
	"K111112222222K",
	"KKKKKKKKKKKKKK"
};

static const char *const g_it44[] = {
	"........KK........",
	".......K11K.......",
	"......KK11KK......",
	"....KKWWKK11KK....",
	"...KWW11111111K...",
	"..KW11111111111K..",
	"..K111111111111K..",
	".K11kkkkkkkkkk22K.",
	".K11kkkkkkkkkk22K.",
	"K111kkkkkkkkkk222K",
	"K111kkkkkkkkkk222K",
	"KKKKKKKKKKKKKKKKKK",
	"gggkkkkkkkkkkkkggg",
	"...kkkkkkkkkkkk...",
	"...kkkkkkkkkkkk..."
};

static const char *const g_it45[] = {
	"EE....EE",
	"EE....EE"
};

static const char *const g_it46[] = {
	"EEEEEEEE"
};

static const char *const g_it47[] = {
	"E.E..E.E",
	".E....E.",
	"E.E..E.E"
};

static const char *const g_it48[] = {
	"E.E..E.E",
	"EEE..EEE",
	".E....E."
};

static const char *const g_it49[] = {
	".E....E.",
	"EEE..EEE",
	".E....E."
};

static const char *const g_it50[] = {
	"rrrrWWcccc",
	"rrrr..cccc",
	"rrrr..cccc"
};

static const char *const g_it51[] = {
	".EEEE.",
	"EEkkEE",
	"EEkkEE",
	"EEkkEE",
	".EEEE."
};

static const char *const g_it52[] = {
	"..........EEE",
	"...........E.",
	"..........EEE",
	".............",
	".............",
	"EEEE..EEEE..."
};

static const char *const g_it53[] = {
	".E.....E.",
	"EE....EE.",
	".EE....EE",
	"EE....EE."
};

static const char *const g_it54[] = {
	"EEEEEEEEEE.........",
	"EEEEEEEEEEEE.......",
	"............EEEE...",
	"................EEE"
};


static const char *const g_it56[] = {
	".1.1.",
	"11y11",
	".111.",
	"..v.."
};

static const char *const g_it57[] = {
	"..KKKKKKK",
	"..K1rK11K",
	"..K111r1K",
	".KKKKKKKK",
	"K........"
};

static const char *const g_it58[] = {
	"11111Krrrrr",
	".....G.....",
	".....G....."
};

static const char *const g_it59[] = {
	"F1",
	"W1",
	"1W",
	"11"
};

static const char *const g_it60[] = {
	".KKKK",
	".K11K",
	".GKK.",
	"G...."
};

static const char *const g_it61[] = {
	"....222",
	"...222.",
	"..22...",
	".22....",
	"22....."
};

static const char *const g_it62[] = {
	".KKKK.",
	"K1111K",
	"K1111K",
	".KKKK.",
	".AaA..",
	"..AA..",
	"..A..."
};

static const char *const g_it63[] = {
	".KkKKK....",
	"oK111KKK..",
	"..KKK111KK",
	"..KK1112KK",
	"....KKKK.."
};

static const char *const g_it64[] = {
	".1.1.1.1",
	"31313131",
	"31313131",
	"31313131",
	"31313131"
};

static const char *const g_it65[] = {
	"K...K...KK",
	"KK.K1K.K1K",
	"K1K111K11K",
	"K111r1111K",
	"KKKKKKKKKK"
};

static const char *const g_it66[] = {
	"...nn...",
	"...nn...",
	"..KKKK..",
	"KK1111KK",
	"KK1111KK",
	"..KKKK.."
};

static const char *const g_it67[] = {
	"..KKKKKK..",
	"KKW111W1KK",
	"KK111111KK",
	"..KxxxxK..",
	"...xxxx..."
};

static const char *const g_it68[] = {
	"K..........K",
	"KKK..22..KKK",
	"K11KK22KK11K",
	"KKK..22..KKK",
	"K..........K"
};

static const char *const g_it69[] = {
	"KKKKKKKKKK",
	"KKKKKKKKKK",
	"K11111111K",
	"KKKKKKKKKK",
	"....G....."
};

static const char *const g_it70[] = {
	"......rrrrrrrr......",
	"....rroooooooorr....",
	"..rrooyyyyyyyyoorr..",
	".r.oyy.vvvvvv.yyo.r.",
	".royvvvbbbbbbvvvyor.",
	"royv.bbppppppbb.vyor",
	"royvbpp......ppbvyor"
};


static const char *const g_it72[] = {
	"..KK..",
	"..KK..",
	".K31K.",
	".K31K.",
	".K31K.",
	".K31K.",
	".K31K.",
	".K31K.",
	".K31K.",
	".K31K.",
	".K31K.",
	".K31K.",
	".K31K.",
	".K31K.",
	".KKKK.",
	"YYYYYY",
	"..nn..",
	"..nn..",
	"..nn.."
};

static const char *const g_it73[] = {
	".KK.",
	"K11K",
	"K11K",
	"Kx1K",
	"K1xK",
	"K11K",
	"K11K",
	"Kx1K",
	"K1xK",
	"K11K",
	"K11K",
	"Kx1K",
	"K1xK",
	"K12K",
	"K12K",
	"Kx2K",
	"K1xK",
	"K12K",
	"K12K",
	".KK."
};

static const char *const g_it74[] = {
	"..KK..",
	".K31K.",
	".K1kK.",
	"K3111K",
	"K3111K",
	"K1111K",
	"K1111K",
	"K1111K",
	"K1112K",
	"K1112K",
	"K1122K",
	".K12K.",
	".K12K.",
	"..KK..",
	"..KK..",
	".K1K..",
	".K11K.",
	"KKKKKK"
};

static const char *const g_it75[] = {
	"1.1.1",
	"1.1.1",
	"1.1.1",
	"1.1.1",
	"1.1.1",
	"11111",
	"11111",
	"11111",
	"..1..",
	"..1..",
	"..1..",
	"..1..",
	"..1..",
	"..1..",
	"..1..",
	"..1..",
	"..1..",
	"..1..",
	"..1..",
	"..1.."
};

static const char *const g_it76[] = {
	"...KKKKK...",
	"..K11111K..",
	".K1111111K.",
	"K111111111K",
	"K111111111K",
	"KKKKKGKKKKK",
	".....G.....",
	".....G.....",
	".....G.....",
	".....G.....",
	".....G.....",
	".....G.....",
	".....G.....",
	".....G.....",
	".....G.....",
	".....G.....",
	".....G.....",
	".....G.....",
	".....G.....",
	".....G.....",
	"....GG....."
};

static const char *const g_it77[] = {
	"..KKKK..",
	".K1111K.",
	"K111111K",
	".K1111K.",
	"..KnnK..",
	"...nn...",
	"...nn...",
	"...nn...",
	"...nn...",
	"...nn...",
	"...nn...",
	"...nn...",
	"...nn...",
	"...nn...",
	"...nn...",
	"...nn...",
	"...nn...",
	"...nn...",
	"...nn..."
};

static const char *const g_it78[] = {
	"...KKKK...",
	"..K1111K..",
	".K111113K.",
	"K111113WWK",
	"K11WW3W11K",
	"KWW131111K",
	"K11311111K",
	".K311111K.",
	"..K1111K..",
	"...KWWK...",
	"....WW....",
	"....WW....",
	"....WW....",
	"....WW....",
	"....WW....",
	"....WW....",
	"....WW....",
	"....WW....",
	"....WW....",
	"....WW...."
};

static const char *const g_it79[] = {
	".KKKKKK.",
	".KkwwkK.",
	".KwwwwK.",
	".KKKKKK.",
	"KKKKKKKK",
	"K333111K",
	"K331111K",
	"K331111K",
	"K311111K",
	"K31b111K",
	"K111111K",
	"K111111K",
	"K111111K",
	"K111111K",
	"K111111K",
	"K111111K",
	"K111111K",
	"KKKKKKKK"
};

static const char *const g_it80[] = {
	"....nn....",
	"....nn....",
	"....nn....",
	"....nn....",
	"....nn....",
	"....nn....",
	"....nn....",
	"....nn....",
	"....nn....",
	"....nn....",
	"....nn....",
	"....nn....",
	"....nn....",
	"....nn....",
	"....nn....",
	"..222222..",
	"..211112..",
	".21111112.",
	".21111112.",
	".21111112.",
	".21111112.",
	"2111111112",
	"2222222222"
};

static const char *const g_it81[] = {
	"....w..",
	"....www",
	"....w1w",
	"...w1w.",
	"...w1w.",
	"...w1w.",
	"...w1w.",
	"..w1w..",
	"..w1w..",
	"..w1w..",
	"..w1w..",
	".w1w...",
	".w1w...",
	".w1w...",
	".w1w...",
	"w1w....",
	"wkw....",
	".k.....",
	"k......",
	"k......",
	"k......"
};

static const char *const g_it82[] = {
	"KKKKKK",
	"K1111K",
	"K2222K",
	"K1111K",
	"K2222K",
	"K1111K",
	"KKKKKK",
	"..kk..",
	"..kk..",
	"..kk..",
	"..kk..",
	"..kk..",
	"..kk..",
	"..kk..",
	"..kk..",
	"..kk..",
	"..kk..",
	"..kk..",
	"..kk.."
};

static const char *const g_it83[] = {
	"....NN....",
	"....NN....",
	"....NN....",
	"....NN....",
	"....NN....",
	"....NN....",
	"....NN....",
	"....NN....",
	"....NN....",
	"....NN....",
	"....NN....",
	"....NN....",
	"...KKKK...",
	"..K1111K..",
	".K111111K.",
	".K111111K.",
	"..KKKKKK..",
	".KK1111KK.",
	"K111k1111K",
	"K11111112K",
	"K11111122K",
	".KK1122KK.",
	"...KKKK..."
};


static const char *const g_it85[] = {
	"KKKKKKKK",
	"K33y111K",
	"K33y111K",
	"K31y111K",
	"KyyyyyyK",
	"K11y112K",
	"K11y112K",
	".K1y12K.",
	"..Ky2K..",
	"..K1K...",
	"...K...."
};

static const char *const g_it86[] = {
	"...KKK...",
	"..K311K..",
	".K31111K.",
	"K3111111K",
	"K311K111K",
	"K11KGK12K",
	"K111K112K",
	"K1111122K",
	".K11122K.",
	"..K122K..",
	"...KKK..."
};

static const char *const g_it87[] = {
	"nn......",
	"AAnnnn..",
	"A111AAnn",
	".A1r11A.",
	".A1111A.",
	".Av11A..",
	".A11rA..",
	".A11A...",
	".A11A...",
	"..AA....",
	"..AA....",
	"..A....."
};

static const char *const g_it88[] = {
	"..KKKK..",
	".K3111K.",
	"K31K111K",
	"K1KFK11K",
	"K11K111K",
	"K111111K",
	".K1111K.",
	"..KKKn..",
	"......n.",
	"......n.",
	".......n"
};

static const char *const g_it89[] = {
	"K3KKKKK",
	"K31111K",
	"K31111K",
	"K31WWWK",
	"K31111K",
	"K31WWWK",
	"K31122K",
	"K31222K",
	"K3KKKKK"
};

static const char *const g_it90[] = {
	"..KKK..",
	".K311K.",
	"K31111K",
	"K11111K",
	"K11112K",
	"K11122K",
	".K122K.",
	"..KKK..",
	"...W...",
	"...W...",
	"....W..",
	"....W..",
	".....W.",
	".....W.",
	"......W",
	"......W"
};

static const char *const g_it91[] = {
	"..GG..",
	"KKKKKK",
	"K1111K",
	"K1111K",
	"K1W11K",
	"K1111K",
	"K1111K",
	"K1111K",
	"KKKKKK"
};

static const char *const g_it92[] = {
	"...w....",
	"..w.....",
	"........",
	"KNNNNK..",
	"K1111KK.",
	"K1111K1K",
	"K1111K1K",
	"K1112KK.",
	"K1122K..",
	"KKKKKK.."
};

static const char *const g_it93[] = {
	"2222222",
	"2111112",
	"2KKKKK2",
	"2111112",
	"2KKKK12",
	"2111112",
	"2222222"
};

static const char *const g_it94[] = {
	"....22...",
	".222112..",
	"21121122.",
	"211222112",
	".222.2112",
	".2112.22.",
	".2112....",
	"..22l....",
	"...ll....",
	"...ll....",
	"...ll....",
	"...ll...."
};


static const char *const g_it96[] = {
	"1....",
	"11...",
	"111..",
	"1111.",
	"11111",
	"..11.",
	"...11"
};

static const char *const g_it97[] = {
	"..KK...",
	".K1kKo.",
	".K11K..",
	"..KKK..",
	"KK111KK",
	"K11111K",
	"KK112KK",
	"..KKK.."
};

static const char *const g_it98[] = {
	".....KK..",
	"....K11K.",
	"K..K1211K",
	".x.K1121K",
	"aaaaaaaa.",
	"axxxxxxa.",
	"aaaaaaaa."
};

static const char *const g_it99[] = {
	"...K...",
	"..K1K..",
	".K111K.",
	"K1k1k1K",
	"K11112K",
	"K11122K",
	"KKKKKKK"
};

static const char *const g_it100[] = {
	".K...K.",
	".KK.KK.",
	".KKKKK.",
	".K111K.",
	"K1k1k1K",
	"K11111K",
	".K111K.",
	"..KKK.."
};

static const char *const g_it101[] = {
	".K....K",
	"..K..K.",
	"...KK..",
	"K.K11KK",
	".K1111K",
	"KK1112K",
	"..K12K.",
	"K..KK.K"
};

static const char *const g_it102[] = {
	"..wwww..",
	".w1111w.",
	"w111111w",
	"w11o111w",
	"w1oooo1w",
	"w11o111w",
	".w1111w.",
	"..wwww.."
};

static const char *const g_it103[] = {
	"..KKKK..",
	".KW111K.",
	"K1111W1K",
	".K1111K.",
	"..aaaa..",
	"..axxa..",
	"..aaaa..",
	"..K..K.."
};

static const char *const g_it104[] = {
	"KKKKKKK",
	"K11111K",
	"K1r1r1K",
	"K11111K",
	"K11111K",
	"KyyyyyK",
	"KKKKKKK"
};

static const char *const g_it105[] = {
	"..gggg..",
	".g1111g.",
	"g111111g",
	".g1111g.",
	"..gggg..",
	"........",
	".q.q.q..",
	"........",
	"..q.q.q."
};









const t_idef	g_items[] = {
	{"fond_aucun", "Aucun", "None", 0, 0, {0, 0, 0}, {0, 0, 0, 0, NULL}},
	{"fond_nuit", "Nuit étoilée", "Starry night", 0, 60, {30, 30, 70}, {28, 32, 0, 0, g_it1}},
	{"fond_couchant", "Coucher de soleil", "Sunset", 0, 80, {255, 120, 60}, {28, 32, 0, 0, g_it2}},
	{"fond_damier", "Damier de cantine", "Cafeteria checks", 0, 50, {230, 230, 240}, {28, 32, 0, 0, g_it3}},
	{"fond_terminal", "Terminal", "Terminal", 0, 70, {0, 40, 10}, {28, 32, 0, 0, g_it4}},
	{"fond_nuages", "Nuages en coton", "Cotton clouds", 0, 60, {140, 200, 255}, {28, 32, 0, 0, g_it5}},
	{"fond_frigo", "Intérieur de frigo", "Inside a fridge", 0, 90, {220, 245, 255}, {28, 32, 0, 0, g_it6}},
	{"fond_lave", "Volcan en promo", "Volcano on sale", 0, 110, {120, 20, 10}, {28, 32, 0, 0, g_it7}},
	{"cape_aucun", "Aucun", "None", 1, 0, {0, 0, 0}, {0, 0, 0, 0, NULL}},
	{"cape_classique", "Cape de chevalier", "Knight cape", 1, 0, {200, 40, 60}, {18, 14, 5, 15, g_it9}},
	{"cape_chaussette", "Chaussette géante", "Giant sock", 1, 70, {90, 160, 240}, {16, 15, 8, 15, g_it10}},
	{"cape_nappe", "Nappe à carreaux", "Checkered tablecloth", 1, 60, {220, 50, 60}, {18, 14, 5, 15, g_it11}},
	{"cape_rideau", "Rideau de douche", "Shower curtain", 1, 80, {120, 220, 230}, {18, 14, 5, 15, g_it12}},
	{"cape_pirate", "Drapeau pirate", "Pirate flag", 1, 120, {30, 30, 40}, {18, 14, 5, 15, g_it13}},
	{"cape_serviette", "Serviette de plage", "Beach towel", 1, 70, {255, 200, 60}, {16, 15, 6, 15, g_it14}},
	{"cape_feuilles", "Cape en feuilles", "Leaf cape", 1, 90, {90, 190, 80}, {19, 14, 5, 15, g_it15}},
	{"cape_chauvesouris", "Ailes de chauve-souris", "Bat wings", 1, 150, {90, 60, 120}, {28, 13, 0, 12, g_it16}},
	{"jambes_jambieres", "Jambières", "Greaves", 2, 0, {120, 130, 150}, {10, 7, 9, 23, g_it17}},
	{"jambes_short", "Short à fleurs", "Flower shorts", 2, 50, {255, 120, 180}, {10, 8, 9, 22, g_it18}},
	{"jambes_tutu", "Tutu", "Tutu", 2, 80, {255, 160, 210}, {18, 8, 5, 22, g_it19}},
	{"jambes_pattes", "Pattes d'eph'", "Bell-bottoms", 2, 60, {90, 120, 220}, {12, 8, 8, 22, g_it20}},
	{"jambes_palmes", "Palmes", "Flippers", 2, 70, {60, 200, 120}, {16, 8, 6, 23, g_it21}},
	{"jambes_chaussettes", "Chaussettes dépareillées", "Odd socks", 2, 40, {255, 210, 60}, {8, 6, 10, 23, g_it22}},
	{"jambes_bottes", "Bottes en caoutchouc", "Rubber boots", 2, 60, {250, 220, 40}, {10, 8, 9, 22, g_it23}},
	{"jambes_rollers", "Rollers", "Roller skates", 2, 110, {230, 60, 160}, {10, 9, 9, 22, g_it24}},
	{"armure_cotte", "Cotte de mailles", "Chainmail", 3, 0, {110, 130, 170}, {16, 8, 6, 15, g_it25}},
	{"armure_noel", "Pull de Noël", "Xmas sweater", 3, 80, {200, 40, 50}, {16, 8, 6, 15, g_it26}},
	{"armure_tablier", "Tablier de cuisine", "Kitchen apron", 3, 50, {255, 255, 255}, {16, 8, 6, 15, g_it27}},
	{"armure_gilet", "Gilet de sauvetage", "Life vest", 3, 70, {255, 120, 30}, {16, 9, 6, 15, g_it28}},
	{"armure_smoking", "Smoking", "Tuxedo", 3, 120, {30, 30, 40}, {16, 8, 6, 15, g_it29}},
	{"armure_peignoir", "Peignoir", "Bathrobe", 3, 60, {255, 180, 210}, {16, 9, 6, 15, g_it30}},
	{"armure_carton", "Armure en carton", "Cardboard armor", 3, 40, {190, 140, 80}, {18, 8, 5, 15, g_it31}},
	{"armure_foot", "Maillot de foot", "Football jersey", 3, 70, {40, 90, 220}, {16, 8, 6, 15, g_it32}},
	{"armure_poubelle", "Sac poubelle", "Trash bag", 3, 30, {40, 40, 50}, {14, 11, 7, 13, g_it33}},
	{"armure_kimono", "Kimono", "Kimono", 3, 100, {240, 240, 245}, {20, 8, 4, 15, g_it34}},
	{"casque_rond", "Casque rond", "Round helmet", 4, 0, {214, 218, 230}, {14, 13, 7, 3, g_it35}},
	{"casque_seau", "Seau", "Bucket", 4, 40, {90, 150, 230}, {20, 13, 4, 3, g_it36}},
	{"casque_casserole", "Casserole", "Saucepan", 4, 60, {170, 175, 190}, {20, 11, 7, 5, g_it37}},
	{"casque_citrouille", "Citrouille", "Pumpkin", 4, 90, {250, 140, 30}, {16, 16, 6, 1, g_it38}},
	{"casque_bocal", "Bocal à cornichons", "Pickle jar", 4, 100, {190, 240, 220}, {14, 15, 7, 2, g_it39}},
	{"casque_ampoule", "Ampoule", "Light bulb", 4, 120, {255, 250, 170}, {16, 16, 6, 1, g_it40}},
	{"casque_cone", "Cône de chantier", "Traffic cone", 4, 70, {255, 120, 30}, {16, 16, 6, 0, g_it41}},
	{"casque_pot", "Pot de fleurs", "Flower pot", 4, 60, {200, 110, 70}, {16, 16, 6, 0, g_it42}},
	{"casque_conserve", "Boîte de conserve", "Tin can", 4, 50, {200, 205, 215}, {14, 13, 7, 3, g_it43}},
	{"casque_cloche", "Cloche de plat", "Food cloche", 4, 140, {230, 230, 240}, {18, 15, 5, 1, g_it44}},
	{"yeux_points", "Points blancs", "White dots", 5, 0, {255, 255, 255}, {8, 2, 10, 9, g_it45}},
	{"yeux_fente", "Fente", "Slit", 5, 30, {255, 220, 80}, {8, 1, 10, 10, g_it46}},
	{"yeux_ko", "Croix (KO)", "Crosses (KO)", 5, 40, {255, 90, 90}, {8, 3, 10, 9, g_it47}},
	{"yeux_coeurs", "Cœurs", "Hearts", 5, 60, {255, 90, 160}, {8, 3, 10, 9, g_it48}},
	{"yeux_etoiles", "Étoiles", "Stars", 5, 60, {255, 230, 80}, {8, 3, 10, 9, g_it49}},
	{"yeux_3d", "Lunettes 3D", "3D glasses", 5, 80, {255, 255, 255}, {10, 3, 9, 9, g_it50}},
	{"yeux_cyclope", "Un seul œil", "One eye", 5, 70, {120, 255, 200}, {6, 5, 11, 8, g_it51}},
	{"yeux_dodo", "Endormis", "Sleepy", 5, 30, {200, 200, 255}, {13, 6, 9, 5, g_it52}},
	{"yeux_dollars", "Dollars", "Dollars", 5, 90, {120, 255, 120}, {9, 4, 9, 8, g_it53}},
	{"yeux_laser", "Laser", "Laser", 5, 150, {255, 40, 40}, {19, 4, 9, 9, g_it54}},
	{"deco_aucun", "Aucun", "None", 6, 0, {0, 0, 0}, {0, 0, 0, 0, NULL}},
	{"deco_fleur", "Fleur", "Flower", 6, 30, {255, 120, 180}, {5, 4, 11, 0, g_it56}},
	{"deco_etiquette", "Étiquette -50%", "-50% tag", 6, 40, {255, 230, 90}, {9, 5, 14, 0, g_it57}},
	{"deco_helice", "Hélice", "Propeller", 6, 60, {80, 160, 255}, {11, 3, 8, 1, g_it58}},
	{"deco_bougie", "Bougie d'anniversaire", "Birthday candle", 6, 50, {120, 200, 255}, {2, 4, 13, 0, g_it59}},
	{"deco_antenne", "Antenne", "Antenna", 6, 40, {255, 80, 80}, {5, 4, 13, 0, g_it60}},
	{"deco_plume", "Plume", "Feather", 6, 50, {255, 230, 120}, {7, 5, 15, 0, g_it61}},
	{"deco_glace", "Cornet de glace", "Ice cream cone", 6, 80, {255, 170, 210}, {6, 7, 11, 0, g_it62}},
	{"deco_canard", "Canard en plastique", "Rubber duck", 6, 90, {255, 220, 40}, {10, 5, 9, 0, g_it63}},
	{"deco_crete", "Crête de punk", "Punk mohawk", 6, 70, {255, 60, 160}, {8, 5, 10, 0, g_it64}},
	{"deco_couronne", "Couronne en carton", "Cardboard crown", 6, 100, {255, 210, 60}, {10, 5, 9, 0, g_it65}},
	{"deco_ventouse", "Ventouse", "Plunger", 6, 60, {230, 50, 60}, {8, 6, 10, 0, g_it66}},
	{"deco_champignon", "Champignon", "Mushroom", 6, 70, {230, 50, 60}, {10, 5, 9, 0, g_it67}},
	{"deco_noeud", "Nœud géant", "Giant bow", 6, 50, {255, 100, 180}, {12, 5, 8, 0, g_it68}},
	{"deco_wip", "Panneau WIP", "WIP sign", 6, 80, {255, 220, 40}, {10, 5, 9, 0, g_it69}},
	{"deco_arcenciel", "Arc-en-ciel", "Rainbow", 6, 160, {255, 255, 255}, {20, 7, 4, 0, g_it70}},
	{"arme_aucun", "Aucun", "None", 7, 0, {0, 0, 0}, {0, 0, 0, 0, NULL}},
	{"arme_epee", "Épée", "Sword", 7, 0, {220, 225, 240}, {6, 19, 20, 3, g_it72}},
	{"arme_baguette", "Baguette de pain", "Baguette", 7, 30, {220, 160, 80}, {4, 20, 21, 3, g_it73}},
	{"arme_poisson", "Poisson", "Fish", 7, 60, {120, 180, 240}, {6, 18, 20, 6, g_it74}},
	{"arme_fourchette", "Fourchette géante", "Giant fork", 7, 50, {210, 215, 230}, {5, 20, 20, 3, g_it75}},
	{"arme_parapluie", "Parapluie", "Umbrella", 7, 80, {230, 60, 120}, {11, 21, 17, 1, g_it76}},
	{"arme_ventouse", "Ventouse de combat", "Battle plunger", 7, 40, {230, 50, 60}, {8, 19, 19, 3, g_it77}},
	{"arme_sucette", "Sucette géante", "Giant lollipop", 7, 70, {255, 120, 200}, {10, 20, 18, 2, g_it78}},
	{"arme_usb", "Clé USB géante", "Giant USB stick", 7, 90, {60, 60, 70}, {8, 18, 19, 3, g_it79}},
	{"arme_balai", "Balai", "Broom", 7, 50, {200, 150, 60}, {10, 23, 18, 2, g_it80}},
	{"arme_plume", "Plume d'écrivain", "Quill", 7, 60, {255, 255, 255}, {7, 21, 21, 2, g_it81}},
	{"arme_spatule", "Spatule", "Spatula", 7, 40, {240, 100, 60}, {6, 19, 20, 3, g_it82}},
	{"arme_guitare", "Guitare", "Guitar", 7, 120, {200, 60, 50}, {10, 23, 18, 1, g_it83}},
	{"main_aucun", "Aucun", "None", 8, 0, {0, 0, 0}, {0, 0, 0, 0, NULL}},
	{"main_bouclier", "Bouclier", "Shield", 8, 0, {200, 60, 70}, {8, 11, 1, 15, g_it85}},
	{"main_poubelle", "Couvercle de poubelle", "Trash can lid", 8, 30, {150, 155, 170}, {9, 11, 0, 14, g_it86}},
	{"main_pizza", "Part de pizza", "Pizza slice", 8, 60, {255, 200, 80}, {8, 12, 1, 14, g_it87}},
	{"main_poele", "Poêle", "Frying pan", 8, 50, {60, 60, 70}, {8, 11, 0, 13, g_it88}},
	{"main_livre", "Livre « man 3 »", "\"man 3\" book", 8, 70, {60, 100, 200}, {7, 9, 1, 15, g_it89}},
	{"main_ballon", "Ballon de baudruche", "Balloon", 8, 40, {255, 70, 90}, {7, 16, 0, 3, g_it90}},
	{"main_lanterne", "Lanterne", "Lantern", 8, 90, {255, 220, 120}, {6, 9, 1, 16, g_it91}},
	{"main_cafe", "Tasse de café", "Coffee mug", 8, 40, {255, 255, 255}, {8, 10, 1, 14, g_it92}},
	{"main_postit", "Post-it", "Sticky note", 8, 20, {255, 240, 100}, {7, 7, 1, 16, g_it93}},
	{"main_bouquet", "Bouquet de brocolis", "Broccoli bouquet", 8, 80, {80, 180, 80}, {9, 12, 0, 13, g_it94}},
	{"familier_aucun", "Aucun", "None", 9, 0, {0, 0, 0}, {0, 0, 0, 0, NULL}},
	{"fam_pointeur", "Pointeur", "Pointer", 9, 60, {255, 255, 255}, {5, 7, 21, 24, g_it96}},
	{"fam_canard", "Canard", "Duck", 9, 50, {255, 220, 40}, {7, 8, 21, 24, g_it97}},
	{"fam_escargot", "Escargot", "Snail", 9, 50, {200, 140, 220}, {9, 7, 19, 25, g_it98}},
	{"fam_slime", "Slime", "Slime", 9, 70, {90, 230, 140}, {7, 7, 21, 25, g_it99}},
	{"fam_chat", "Chat", "Cat", 9, 90, {240, 150, 60}, {7, 8, 21, 24, g_it100}},
	{"fam_bug", "Bug (segfault)", "Bug (segfault)", 9, 80, {60, 200, 80}, {7, 8, 21, 24, g_it101}},
	{"fam_poisson", "Poisson rouge en bocal", "Goldfish bowl", 9, 100, {180, 230, 255}, {8, 8, 20, 24, g_it102}},
	{"fam_champignon", "Champignon qui marche", "Walking mushroom", 9, 60, {230, 60, 70}, {8, 8, 20, 24, g_it103}},
	{"fam_miniboss", "Mini-boss", "Mini boss", 9, 150, {120, 220, 120}, {7, 7, 21, 25, g_it104}},
	{"fam_nuage", "Nuage de pluie", "Rain cloud", 9, 70, {200, 205, 220}, {8, 9, 20, 22, g_it105}},
	{"aura_aucune", "Aucune", "None", 10, 0, {255, 255, 255}, {0, 0, 0, 0, NULL}},
	{"aura_etincelles", "Étincelles", "Sparks", 10, 80, {255, 255, 255}, {0, 0, 0, 0, NULL}},
	{"aura_flammes", "Flammes", "Flames", 10, 120, {255, 255, 255}, {0, 0, 0, 0, NULL}},
	{"aura_bulles", "Bulles", "Bubbles", 10, 60, {255, 255, 255}, {0, 0, 0, 0, NULL}},
	{"aura_notes", "Notes de musique", "Music notes", 10, 90, {255, 255, 255}, {0, 0, 0, 0, NULL}},
	{"aura_coeurs", "Cœurs", "Hearts", 10, 70, {255, 255, 255}, {0, 0, 0, 0, NULL}},
	{"aura_binaire", "Zéros et uns", "Zeros and ones", 10, 100, {255, 255, 255}, {0, 0, 0, 0, NULL}},
	{"aura_flocons", "Flocons", "Snowflakes", 10, 80, {255, 255, 255}, {0, 0, 0, 0, NULL}},
	{"cape_veteran", "Cape du Vétéran", "Veteran cape", 1, -1, {255, 200, 70}, {18, 14, 5, 15, g_it9}},
	{"casque_veteran", "Casque du Vétéran", "Veteran helmet", 4, -1, {255, 205, 90}, {14, 13, 7, 3, g_it35}},
	{"fam_moulinette", "Moulinette (la vraie)", "Moulinette (the real one)", 9, -1, {150, 152, 162}, {7, 8, 21, 24, g_it100}},
	{"cape_format", "Cape du Format", "Format cape", 1, 150, {255, 110, 210}, {18, 14, 5, 15, g_it9}},
	{"main_chocolat", "Chocolat chaud", "Hot chocolate", 8, 60, {120, 70, 40}, {8, 10, 1, 14, g_it92}},
	{"casque_pourcent", "Casque pourcent", "Percent helmet", 4, 120, {200, 120, 255}, {14, 13, 7, 3, g_it35}}
};
const int	g_nitems = 120;
