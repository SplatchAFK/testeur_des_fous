# Le testeur des fous

> Version 3.0.0 en construction · [historique](CHANGELOG.md) · créé par **Splatch** (`romasant`) pour les étudiants de 42 Next.

## Commandes

```bash
bash testeur_des_fous/run.sh              # détecter tes projets et lancer le testeur
bash testeur_des_fous/run.sh libft        # tester ta Libft
bash testeur_des_fous/run.sh printf       # tester ton ft_printf
bash testeur_des_fous/run.sh --no-lore    # les tests seuls, sans histoire
bash testeur_des_fous/run.sh -a           # sans animations pour ce lancement
bash testeur_des_fous/run.sh --anim off   # couper les animations pour de bon (on pour les remettre)
bash testeur_des_fous/run.sh --debug      # tous les rapports détaillés
bash testeur_des_fous/run.sh --version    # le numéro de version
```

## Installation

Clone le testeur **à côté** de tes projets, pas dedans :

```bash
git clone https://github.com/SplatchAFK/testeur_des_fous.git
```

```text
ton_dossier/
├── testeur_des_fous/
├── libft/
└── ft_printf/
```

Le testeur trouve tout seul les projets rangés à côté de lui. Sinon : `--projet CHEMIN`.

Il te faut `cc`, `make` et `bash`. Rien à installer, aucun `sudo`. `norminette` et `valgrind` sont utilisés s'ils sont là.

Chaque module a ses options : `bash testeur_des_fous/run.sh libft --help` ou `printf --help`.

Ta progression est dans `~/.testeur_des_fous/save.txt`, hors du dossier du testeur. Une sauvegarde des anciens testeurs est reprise telle quelle, et une copie en est gardée.

## Ce qu'il teste

| Module | État |
|---|---|
| Libft | intégré : 43 fonctions, mémoire, Norme, audit du Makefile |
| ft_printf | intégré : conversions, retour, `malloc` en échec, stress |
| Exam (C) | à venir |
| Banque d'exercices | à venir |
| get_next_line | plus tard |
| push_swap | plus tard |

Le testeur ne contient aucune solution et n'écrit rien à ta place : il casse ton code et te dit où regarder.

## Discord

Entraide, bugs, idées : [discord.gg/SwAmfRcXCH](https://discord.gg/SwAmfRcXCH)

## Crédits et licence

Créé et vibe-codé par Splatch (`romasant`). Licence MIT, voir [LICENSE](LICENSE).
