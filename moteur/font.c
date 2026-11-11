#include <math.h>
#include <string.h>
#include "fous.h"

static const char	*g_glyph[][5] = {
	{" ### ", "#   #", "#####", "#   #", "#   #"},
	{"#### ", "#   #", "#### ", "#   #", "#### "},
	{" ####", "#    ", "#    ", "#    ", " ####"},
	{"#### ", "#   #", "#   #", "#   #", "#### "},
	{"#####", "#    ", "#### ", "#    ", "#####"},
	{"#####", "#    ", "#### ", "#    ", "#    "},
	{" ####", "#    ", "#  ##", "#   #", " ### "},
	{"#   #", "#   #", "#####", "#   #", "#   #"},
	{"#####", "  #  ", "  #  ", "  #  ", "#####"},
	{"#####", "   # ", "   # ", "#  # ", " ##  "},
	{"#   #", "#  # ", "###  ", "#  # ", "#   #"},
	{"#    ", "#    ", "#    ", "#    ", "#####"},
	{"#   #", "## ##", "# # #", "#   #", "#   #"},
	{"#   #", "##  #", "# # #", "#  ##", "#   #"},
	{" ### ", "#   #", "#   #", "#   #", " ### "},
	{"#### ", "#   #", "#### ", "#    ", "#    "},
	{" ### ", "#   #", "# # #", "#  # ", " ## #"},
	{"#### ", "#   #", "#### ", "#  # ", "#   #"},
	{" ####", "#    ", " ### ", "    #", "#### "},
	{"#####", "  #  ", "  #  ", "  #  ", "  #  "},
	{"#   #", "#   #", "#   #", "#   #", " ### "},
	{"#   #", "#   #", "#   #", " # # ", "  #  "},
	{"#   #", "#   #", "# # #", "## ##", "#   #"},
	{"#   #", " # # ", "  #  ", " # # ", "#   #"},
	{"#   #", " # # ", "  #  ", "  #  ", "  #  "},
	{"#####", "   # ", "  #  ", " #   ", "#####"},
};

static const char	*g_digit[][5] = {
	{" ### ", "#  ##", "# # #", "##  #", " ### "},
	{"  #  ", " ##  ", "  #  ", "  #  ", " ### "},
	{" ### ", "#   #", "  ## ", " #   ", "#####"},
	{"#### ", "    #", " ### ", "    #", "#### "},
	{"#   #", "#   #", "#####", "    #", "    #"},
	{"#####", "#    ", "#### ", "    #", "#### "},
	{" ### ", "#    ", "#### ", "#   #", " ### "},
	{"#####", "   # ", "  #  ", " #   ", " #   "},
	{" ### ", "#   #", " ### ", "#   #", " ### "},
	{" ### ", "#   #", " ####", "    #", " ### "},
};

static const char	*g_slash[5] = {"    #", "   # ", "  #  ", " #   ", "#    "};
static const char	*g_bang[5] = {"  #  ", "  #  ", "  #  ", "     ", "  #  "};

static const char	**glyph(char c)
{
	if (c >= 'a' && c <= 'z')
		c -= 32;
	if (c >= 'A' && c <= 'Z')
		return ((const char **)g_glyph[c - 'A']);
	if (c >= '0' && c <= '9')
		return ((const char **)g_digit[c - '0']);
	if (c == '/')
		return ((const char **)g_slash);
	if (c == '!')
		return ((const char **)g_bang);
	return (NULL);
}

int	big_width(const char *s)
{
	int	n;

	n = (int)strlen(s);
	return (n ? n * 6 - 1 : 0);
}

void	big_text(int x, int y, const char *s, double t, int mode)
{
	const char	**g;
	int			i;
	int			r;
	int			c;
	int			dy;
	double		k;
	t_rgb		col;

	for (i = 0; s[i]; i++)
	{
		g = glyph(s[i]);
		if (!g)
			continue ;
		dy = 0;
		if (mode == 2)
		{
			k = t - i * 0.06;
			if (k < 0)
				continue ;
			if (k < 0.5)
				dy = (int)(-8 * (1 - k / 0.5) * (1 - k / 0.5)
						+ 2 * sin(k * 12) * (1 - k / 0.5));
		}
		for (r = 0; r < 5; r++)
		{
			for (c = 0; c < 5; c++)
			{
				if (g[r][c] != '#' || y + r + dy < 0)
					continue ;
				if (mode == 1)
					col = mix(rgb(255, 40, 60), rgb(255, 220, 220),
							0.5 + 0.5 * sin(t * 25 + r + c * 3 + i));
				else if (mode == 3)
					col = mix(theme()->accent, theme()->accent2,
							0.5 + 0.5 * sin(t * 3 + (i * 6 + c) * 0.15));
				else
					col = hue((i * 6 + c + r * 0.5) / 70.0 + t * 0.25);
				at(x + i * 6 + c, y + r + dy);
				fg(col);
				out("█");
			}
		}
	}
}

void	hd_big(t_hd *d, int x, int y, const char *s, double t, int mode)
{
	const char	**g;
	int			i;
	int			r;
	int			c;
	int			dy;
	double		k;
	t_rgb		col;

	for (i = 0; s[i]; i++)
	{
		g = glyph(s[i]);
		if (!g)
			continue ;
		dy = 0;
		if (mode == 2)
		{
			k = t - i * 0.06;
			if (k < 0)
				continue ;
			if (k < 0.5)
				dy = (int)(-8 * (1 - k / 0.5) * (1 - k / 0.5) + 2 * sin(k * 12) * (1 - k / 0.5));
		}
		for (r = 0; r < 5; r++)
			for (c = 0; c < 5; c++)
			{
				if (g[r][c] != '#' || y + r + dy < 0)
					continue ;
				if (mode == 1)
					col = mix(rgb(255, 40, 60), rgb(255, 220, 220), 0.5 + 0.5 * sin(t * 25 + r + c * 3 + i));
				else
					col = hue((i * 6 + c + r * 0.5) / 70.0 + t * 0.25);
				hd_text(d, x + i * 6 + c, y + r + dy, col, col, "█");
			}
	}
}

