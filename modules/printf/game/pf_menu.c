#include <math.h>
#include <stdio.h>
#include <string.h>
#include "fous.h"

int	pf_menu(void)
{
	static int	sel;
	const char	*items[14];
	char		tmp[200];
	int			n;
	int			k;
	int			i;
	int			x;
	int			y;
	int			c;
	long		t;

	n = 0;
	items[n++] = L("Tester                  (r)", "Test                    (r)");
	items[n++] = L("Chevalier", "Knight");
	items[n++] = L("Lore", "Lore");
	items[n++] = L("Stats", "Stats");
	items[n++] = L("Paramètres", "Settings");
	items[n++] = L("Quitter", "Quit");
	cls();
	while (1)
	{
		t = now_ms();
		x = (term_cols() - 78) / 2;
		if (x < 0)
			x = 0;
		y = (term_rows() - 22) / 2;
		if (y < 0)
			y = 0;
		bgc(theme()->bg);
		frame();
		stars_draw();
		avatar_panel(x, y + 1, &g_save.look, g_save.title, t);
		at(x + 34, y + 1);
		for (i = 0; "LE TESTEUR DES FOUS"[i]; i++)
		{
			fg(hue(0.82 + i / 60.0 + t / 4000.0));
			outf("%c", "LE TESTEUR DES FOUS"[i]);
		}
		text_at(x + 34 + 20, y + 1, theme()->dim, "v" TDF_VERSION);
		text_at(x + 34, y + 2, theme()->dim, L("ft_printf · créé et vibe-codé par Splatch",
				"ft_printf · created and vibe-coded by Splatch"));
		for (i = 0; i < n; i++)
		{
			at(x + 34, y + 4 + i * 2);
			if (i == sel)
			{
				bgc(mix(theme()->bg, theme()->accent, 0.18 + 0.08 * sin(t / 150.0)));
				fg(theme()->accent2);
				outf(" ▶ %-34s ", items[i]);
				bgc(theme()->bg);
			}
			else
			{
				fg(i == 0 ? theme()->accent : theme()->text);
				outf("   %-34s ", items[i]);
			}
		}
		for (c = 0, i = 0; i < NCHAIN; i++)
			c += g_pf.chains[i] == '1';
		snprintf(tmp, sizeof(tmp), L("chaînes %d/9 · ft_printf %s · entr. %d/16", "chains %d/9 · ft_printf %s · train %d/16"),
			c, g_pf.big ? "✓" : "✗", pf_exo_done());
		text_at(x + 34, y + 5 + n * 2, theme()->dim, tmp);
		tdf_menu_aide(x + 34, y + 7 + n * 2, sel);
		flush();
		k = key_dir(key_read(40));
		if (k == 1000)
			sel = (sel + n - 1) % n;
		else if (k == 1001)
			sel = (sel + 1) % n;
		else if (k == '\r' || k == '\n' || k == ' ')
		{
			cls();
			return (sel);
		}
		else if (k == 'r' || k == 'R')
		{
			cls();
			return (100);
		}
		else if (key_quit(k))
			sel = n - 1;
	}
}

int	pf_dlc_menu(void)
{
	static int	sel;
	const char	*items[3];
	char		a[120];
	int			open;
	int			r;

	open = g_pf.big || pf_libft_done();
	snprintf(a, sizeof(a), "%s%s", open ? "" : "~", L("Monde des flags · 6 boss (- 0 largeur . # +)",
			"Flag world · 6 bosses (- 0 width . # +)"));
	items[0] = a;
	items[1] = L("Entraînement · 16 niveaux", "Training · 16 levels");
	items[2] = L("Retour", "Back");
	while (1)
	{
		r = tdf_list_menu(L("D L C", "D L C"), items, 3, &sel, L("Le monde des flags teste ce que le sujet v1.1 ne demande pas "
				"(flags, largeur, précision), contre le vrai printf. Il s'ouvre après ft_printf, ou tout de suite pour "
				"les Vétérans de la Libft. L'entraînement montre les notions du projet et celles que le sujet ne montre pas.",
				"The flag world tests what subject v1.1 does not ask for (flags, width, precision), against the real "
				"printf. It opens after ft_printf, or right away for Libft Veterans. Training covers the project's "
				"notions and the ones the subject does not show."));
		if (r < 0 || r == 2)
			return (0);
		if (r == 0 && !open)
		{
			toast(L("VERROUILLÉ", "LOCKED"), L("bats ft_printf, ou connecte une libft validée",
					"beat ft_printf, or log in with a validated libft"), rgb(255, 200, 70));
			continue ;
		}
		if (r == 0)
			return (43);
		pf_train();
	}
}

void	pf_boss_menu(void)
{
	static int	sel;
	const char	*items[8];
	char		b[6][120];
	int			r;

	snprintf(b[0], 120, "%s", L("Introduction · la Machine de la Norminette", "Intro · the Norminette Machine"));
	snprintf(b[1], 120, "%s%s", pf_libft_done() ? "" : "~", L("Précédemment, dans la Libft", "Previously, in the Libft"));
	snprintf(b[2], 120, "%s%s", g_pr.done ? "" : "~", L("Les 9 chaînes (ton dernier run)", "The 9 chains (your last run)"));
	snprintf(b[3], 120, "%s%s", g_pf.big ? "" : "~", L("ft_printf, le big boss", "ft_printf, the big boss"));
	snprintf(b[4], 120, "%s%s", g_pf.big ? "" : "~", L("Victoire : le chocolat chaud", "Victory: hot chocolate"));
	snprintf(b[5], 120, "%s%s", g_pf.secret ? "" : "~", L("Fin secrète", "Secret ending"));
	for (r = 0; r < 6; r++)
		items[r] = b[r];
	items[6] = L("Retour", "Back");
	while (1)
	{
		r = tdf_list_menu(L("B O S S   E T   C I N É M A T I Q U E S", "B O S S E S   A N D   C U T S C E N E S"),
				items, 7, &sel, L("Les lignes grises se débloquent en avançant.", "Grey lines unlock as you progress."));
		if (r < 0 || r == 6)
			return ;
		if (items[r][0] == '~')
		{
			toast(L("PAS ENCORE", "NOT YET"), L("continue à avancer pour la débloquer", "keep going to unlock it"),
				rgb(255, 200, 70));
			continue ;
		}
		if (r == 0)
			story_printf_intro();
		else if (r == 1)
			story_previously();
		else if (r == 2)
			story_chains(0);
		else if (r == 3)
		{
			story_preboss();
			story_bigboss();
		}
		else
			story_victory(r == 5);
	}
}
