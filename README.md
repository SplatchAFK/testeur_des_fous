# Le testeur des fous

> Version 3.0.0 en construction · [historique](CHANGELOG.md) · créé par **Splatch** (`romasant`) pour les étudiants de 42 Next.

## Commandes

```bash
bash testeur_des_fous/run.sh              # détecter tes projets et lancer le testeur
bash testeur_des_fous/run.sh libft        # tester ta Libft
bash testeur_des_fous/run.sh printf       # tester ton ft_printf
bash testeur_des_fous/run.sh exam         # passer l'exam en C (4 niveaux, grademe)
bash testeur_des_fous/run.sh --menu       # le menu sans lancer de tests (stats, chevalier, lore)
bash testeur_des_fous/run.sh debug        # le journal de la dernière session (debug aide : tout)
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
| Exam (C) | intégré : 57 exercices sur 4 niveaux, chronomètre de 3 h, `grademe` |
| Banque d'exercices | à venir |
| get_next_line | plus tard |
| push_swap | plus tard |

Le testeur ne contient aucune solution et n'écrit rien à ta place : il casse ton code et te dit où regarder.

## L'exam

Avant de commencer, tu règles ta session : examen ou entraînement sur un niveau, chronomètre, attente après un échec, trace, langue du sujet, ou « conditions réelles ». Ensuite :

- le sujet est dans `testeur_des_fous/exam/sujet/<exercice>/` ;
- tu écris ton code dans `testeur_des_fous/exam/rendu/<exercice>/` ;
- dans l'examshell : `grademe` corrige, `status` donne le temps restant, `sujet` relit le sujet, `finish` termine.

La correction compile avec `cc -Wall -Wextra -Werror`, refuse les fonctions non autorisées et compare ta sortie octet par octet, comme le vrai. En plus, elle te signale les erreurs mémoire et les fuites. `skip`, `reroll` et `trace` demandent un code, à demander à splatch_afk sur Discord.

## Un problème ?

Chaque lancement est noté dans `~/.testeur_des_fous/debug/` : ta machine, chaque étape avec sa durée, les erreurs.

```bash
bash testeur_des_fous/run.sh debug          # la dernière session, erreurs en rouge
bash testeur_des_fous/run.sh debug liste    # les 15 dernières sessions
bash testeur_des_fous/run.sh debug suivre   # en direct, depuis un 2e terminal
bash testeur_des_fous/run.sh debug paquet   # un fichier à envoyer (sans ton code ni ton login)
```

Pendant l'analyse, Échap (ou Ctrl+C) arrête les tests et ouvre le menu.

## Discord

Entraide, bugs, idées : [discord.gg/SwAmfRcXCH](https://discord.gg/SwAmfRcXCH)

## Crédits et licence

- Créé et vibe-codé par Splatch (`romasant`). Licence MIT, voir [LICENSE](LICENSE).
- Exam : la liste des 57 exercices vient de [42exam.net](https://42exam.net/) (rang 02) et de [Don't Panic](https://code-dontpanic.lovable.app/). Les énoncés sont réécrits avec nos mots. Idée de départ et première version de l'examshell : [exam_shell](https://github.com/Sam-Sepi00l/exam_shell) de Sam-Sepi00l, avec son accord.
