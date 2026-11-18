# Le testeur des fous

> **Version 3.3.0** · [historique](CHANGELOG.md) · créé et vibe-codé par **Splatch** (`romasant`) pour les étudiants de 42 Next.

Un seul testeur pour **Libft**, **ft_printf** et **get_next_line**, plus un **examshell en C** et une **banque d'exercices**. Il teste comme la moulinette, il ne contient aucune solution et n'écrit rien à ta place : il casse ton code et te dit où regarder.

## Démarrer en 30 secondes

1. Clone le testeur **à côté** de tes projets, pas dedans :

   ```bash
   git clone https://github.com/SplatchAFK/testeur_des_fous.git
   ```

   ```text
   ton_dossier/
   ├── testeur_des_fous/
   ├── libft/
   ├── ft_printf/
   └── get_next_line/
   ```

2. Lance-le :

   ```bash
   bash testeur_des_fous/run.sh
   ```

3. Mets ton terminal en plein écran, choisis ta langue, suis le tuto (6 écrans, tu peux le passer), puis tout se fait dans le menu aux flèches.

Il te faut seulement `bash`, `cc` et `make`. Aucun `sudo`. `norminette` et `valgrind` sont utilisés s'ils sont installés.

## Le menu

| Touche | Action |
|---|---|
| ↑ ↓ | choisir |
| Entrée | valider |
| ◀ ▶ | changer d'onglet ou de réglage |
| Échap ou q | revenir |

| Entrée du menu | Ce que ça fait |
|---|---|
| Tester mon projet | choisis le dossier de ta Libft, de ton ft_printf ou de ton get_next_line : « . » pour le dossier où tu es, Tab pour parcourir comme dans zsh, ou un des projets trouvés |
| Exam en C | l'examshell des fous : examen blanc de 3 h, série d'exercices ou entraînement |
| Banque d'exercices | 94 exercices au choix, aux flèches |
| Revoir la dernière tentative | tous les tests de ton dernier passage, avec pourquoi ça passe ou non |
| Chevalier | ton avatar, la boutique, ton profil et tes succès |
| Lore | toutes les cinématiques (Libft, ft_printf, get_next_line, exam) et la carte de la Cité |
| Stats | ta progression, tes badges et le holygraph 42 Next |
| Paramètres | réglages, commande `tdf`, revoir le tuto et le changelog |

Après un test, l'écran des résultats s'affiche directement ; en le quittant, tu reviens au menu.

Astuce : dans **Paramètres**, installe la commande `tdf` (sans sudo) pour ouvrir le menu depuis n'importe quel dossier.

## Ce qu'il teste

| Projet | Ce qui est vérifié |
|---|---|
| Libft | 43 fonctions, fuites et erreurs mémoire, Norme, Makefile (règles, relink, flags), fichiers parasites, audit moulinette |
| ft_printf | chaque conversion comparée au vrai `printf`, valeur de retour, `malloc` qui échoue, trace au format de la moulinette |
| get_next_line | 7 BUFFER_SIZE et sans `-D`, entrée standard, tubes, fd invalides, `read` et `malloc` qui échouent, fuites, Norme |
| push_swap | plus tard |

Pour ft_printf, la trace est enregistrée dans `~/.testeur_des_fous/traces/`. Le testeur compile ton projet comme la moulinette, `CFLAGS` imposés compris, et signale les fichiers qu'un `git add .` enverrait en trop.

## L'examshell des fous

1. Règle ta session aux flèches : examen ou entraînement, niveau, chronomètre, langue du sujet, ou « conditions réelles ».
2. Lis le sujet dans `testeur_des_fous/exam/sujet/<exercice>/`.
3. Écris ton code dans `testeur_des_fous/exam/rendu/<exercice>/`, dans un autre terminal.
4. Dans l'examshell des fous, tape `grademe`.

| Commande | Rôle |
|---|---|
| `grademe` | corriger ton rendu |
| `sujet` | relire le sujet (`sujet en`, `sujet es`… pour une autre langue) |
| `status` | temps restant et progression |
| `help` | la liste des commandes |
| `finish` | terminer la session |

Une commande mal tapée ? Le testeur propose la bonne : Entrée pour accepter.

La correction compile avec `cc -Wall -Wextra -Werror`, refuse les fonctions non autorisées et compare ta sortie octet par octet, comme la vraie. Elle signale en plus les erreurs mémoire et les fuites.

## La banque d'exercices

94 exercices : les 57 de l'exam, 21 d'entraînement Libft, 16 d'entraînement printf.

1. ◀ ▶ choisit l'onglet (Tout, Exam, Libft, printf), ↑ ↓ l'exercice, Entrée l'ouvre.
2. Écris ton code dans `testeur_des_fous/banque_travail/<source>/<exercice>/`.
3. Choisis **Corriger mon code**. ◀ ▶ sur « Langue du sujet » change la langue.

Chaque sujet existe en 8 langues : français, anglais, espagnol, allemand, italien, portugais, turc, russe.

## Commandes

Tout passe par le menu. Deux commandes seulement :

```bash
bash testeur_des_fous/run.sh exam      # l'examshell des fous, directement
```

## Mode correction

Pour corriger un autre étudiant : menu › Mode correction, puis choisis le dossier de son projet. Le haut de l'écran affiche « testeur des fous | mode : correction » et le menu ne propose plus que ce qui sert pendant une correction :

- Tester tout le projet : tous les tests, sans cinématiques ni animations ;
- Vérifs de rendu : fichiers en trop, norminette, Makefile, fonctions interdites, en un seul écran ;
- Lancer un test précis : une fonction Libft, une conversion ft_printf ou un BUFFER_SIZE get_next_line, relancé seul avec tout le détail (pour la Libft, la liste des cas testés).

« Quitter le mode correction » ramène au testeur normal.

## Ta progression

Elle est dans `~/.testeur_des_fous/save.txt`, hors du dossier du testeur : tu peux supprimer ou recloner le testeur sans rien perdre. Une sauvegarde des anciens testeurs Libft et printf est reprise telle quelle.

## Un problème ?

Active le mode debug dans le menu (Tester mon projet › Options du test › Mode debug) pour voir tous les rapports détaillés. Il est plus long (ASan, valgrind, audit) : une barre d'avancement montre l'étape en cours et le temps restant estimé. Chaque lancement est aussi noté dans `~/.testeur_des_fous/debug/` : envoie ce dossier sur Discord si tu bloques.

Pendant l'analyse, Échap (ou Ctrl+C) arrête les tests.

## Discord

Serveur 42 Next Global (non officiel) · entraide, bugs, idées : [discord.gg/SwAmfRcXCH](https://discord.gg/SwAmfRcXCH) · si le lien ne marche plus, contacte **@splatch_afk** sur Discord.

## Crédits et licence

- Créé et vibe-codé par Splatch (`romasant`). Licence MIT, voir [LICENSE](LICENSE).
- Exam : la liste des 57 exercices vient de [42exam.net](https://42exam.net/) (rang 02) et de [Don't Panic](https://code-dontpanic.lovable.app/). Les énoncés sont réécrits avec nos mots. Idée de départ et première version de l'examshell : [exam_shell](https://github.com/Sam-Sepi00l/exam_shell) de Sam-Sepi00l, avec son accord.
- Mode série de l'exam et commande `tdf` : inspirés de [42-cli](https://github.com/brunogo25/42-cli) de Bruno Gomez.
- Moulinette, la vraie, est une chatte grise et libre.
