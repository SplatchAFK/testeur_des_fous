# Historique du testeur des fous

Le numéro de version est dans `moteur/version.h`.

## 3.0.0 · 2026-11-13

Un seul testeur réunit le testeur des fous Libft (2.2.1) et le testeur des fous printf (1.1.1), et ajoute get_next_line, un examshell en C et une banque d'exercices.

### Lancement

- `bash run.sh` ou `tdf` sans argument : menu aux flèches avec les projets trouvés, l'exam, la banque, le menu du testeur ; aucune commande à connaître ;
- premier lancement : langue, interface graphique ou texte, commande `tdf` à installer sans sudo (`~/.local/bin/tdf`, `tdf maj`, `tdf desinstaller`) ;
- en-tête « LE TESTEUR DES FOUS » avec le fou en ASCII couleur, qui s'adapte à la taille du terminal (grosses lettres et grand fou sur grand écran, texte simple sur tout petit) ;
- tout se passe dans un seul terminal : plus de fenêtre séparée, plus de nouveau terminal qui s'ouvre.

### Lore « La Cité qui compile »

- histoire sérieuse réécrite : la Tour des Fondations (Libft), la Fonderie des Formats (printf), le Puits (get_next_line), l'Épreuve du Silence (exam), les Deux Piles (push_swap, plus tard) ;
- décors peints : la tour aux 43 fenêtres qui s'allument selon tes fonctions, les trois gardiens, la Fonderie où Moulinette est en cage et sort quand les 9 chaînes sont brisées, le Puits aux 22 anneaux, l'Épreuve aux 4 portes, la fin et la fin secrète ;
- Lore → Carte de la Cité : chaque lieu s'allume selon ta progression, Entrée pour t'approcher ;
- Marvin et sa garde-robe repeints : 37 objets ; la sauvegarde passe au format 4 et rembourse en Octets les objets disparus ;
- images dans le terminal (kitty, sixel) ou en cellules fines ; mode texte toujours disponible (`--no-lore`).

### Langues

- français, anglais, espagnol, allemand, italien, portugais, turc et russe, au choix au premier lancement et dans Paramètres ;
- une traduction mal formée est ignorée au lieu de faire planter le testeur.

### Conformité et trace moulinette

- printf : une trace au même format que celle de la moulinette (mêmes blocs, mêmes intitulés), enregistrée dans `~/.testeur_des_fous/traces/` ;
- la compilation est faite deux fois, dont une avec des `CFLAGS` imposés comme le fait la moulinette : un `-c` rangé dans `CFLAGS` est détecté ;
- fichiers parasites : README en trop, fichiers hors sujet, `.c` jamais compilés, `.h` jamais inclus, doublons ; les fichiers pas encore dans git qu'un `git add .` enverrait sont signalés ;
- Libft : fonctions d'aide non static et variables globales signalées (sujet 4.1).

### Examshell

- complétion Tab des commandes et de leurs arguments, historique avec les flèches ;
- mode série façon 42-cli : 1 à 5 exercices, difficulté croissante ou fixe (`--serie`, `--difficulte`).

### Sécurité et fiabilité

- audit : appels au shell protégés (chemins avec apostrophes ou `$`), fichiers temporaires créés avec `mktemp`, commande `tdf` écrite sans risque ;
- les décodeurs PNG et JPG, la lecture d'une sauvegarde et les fichiers de langue ont été testés au fuzzing (ASan et UBSan) sans plantage ;
- valgrind est essayé avant usage : s'il ne démarre pas, le testeur le dit et continue avec ASan ;
- testé sur Ubuntu 24.04, Fedora 45, Fedora 44, Debian et Arch ; macOS codé mais non vérifié.

### Discord

- à côté du lien : « Lien mort ? Contacte splatch_afk sur Discord ».

### Banque d'exercices

- `run.sh banque`, ou Tester puis Banque d'exercices : les 94 exercices du testeur (57 de l'exam, 21 d'entraînement Libft, 16 d'entraînement printf), dans l'ordre que tu veux ;
- `liste [exam|libft|printf] [niveau]`, `cherche MOT` (nom ou notion), `ouvre N`, `grademe`, `sujet`, `fait`, `aide` ;
- tu codes dans `testeur_des_fous/banque_travail/<source>/<exercice>/` ; l'exam est corrigé comme à l'exam, Libft et printf par leur propre testeur (Norme, sanitizers, échecs de malloc) ;
- Octets et XP au premier succès, progression gardée dans la sauvegarde.

### get_next_line

- `run.sh gnl`, détection d'un dossier get_next_line à côté du testeur, entrée get_next_line du menu Tester ;
- d'après le sujet v1.3 : les 3 fichiers à la racine, prototype, compilation avec et sans `-D BUFFER_SIZE`, seulement `read`, `malloc` et `free` (`lseek` interdit), aucune variable globale, Norme, README ;
- 22 tests lancés avec 7 BUFFER_SIZE (1 à 10 000 000) et sans `-D` : fichiers vides, sans `\n` final, lignes autour de BUFFER_SIZE, très longues lignes, gros fichiers, NULL après la fin, entrée standard, tubes, fd invalides, dossier, read qui échoue, chaque malloc qui échoue, lecture minimale ;
- erreurs mémoire et fuites avec le fichier et la ligne (AddressSanitizer quand il est là) ; BUFFER_SIZE=0 et négatif, et plusieurs fd en alternance, pour info seulement ;
- résultats dans les Stats et le journal de debug.

### Printf

- cas limites et write en échec passent en bonus : affichés, sans faire échouer le run.

### Lancement, debug et stats

- barre de progression pendant la préparation ; l'interface se compile en parallèle et reste en cache (environ 3 fois plus rapide sur le disque Windows) ;
- l'analyse de ft_printf ne recopie plus les anciens testeurs rangés dans le projet ;
- `--menu`, ou « Menu seul » au lancement : stats, chevalier et lore sans lancer de tests, même sans aucun projet ;
- l'exam se lance sans projet à côté ;
- journal de chaque session dans `~/.testeur_des_fous/debug/` et commandes `run.sh debug` (`liste`, `N`, `suivre`, `paquet`, `effacer`) ;
- Stats en 5 pages (Résumé, Libft, ft_printf, Exam, Historique) : graphique des derniers lancements, plus longue série, fonctions pas encore validées, conversions et flags, exercices de l'exam réussis par niveau, historique détaillé.

### Exam

- `run.sh exam` ou Tester, puis Exam en C : un examshell en C, sans Python ;
- 57 exercices sur 4 niveaux (12, 20, 15, 10), énoncés en français et en anglais, 25 points par niveau, 100 pour valider ;
- session réglable : examen ou entraînement, chronomètre de 3 h, attente après un échec, trace, langue, conditions réelles ;
- `grademe` : fichiers attendus, compilation, fonctions autorisées, sortie comparée octet par octet, 5 s par test, puis contrôle mémoire à titre d'information ;
- `skip`, `reroll`, `trace` avec un code personnel, lié au login ; un exam passé avec un cheat ne rapporte rien ;
- Octets et XP selon la réussite ; l'exam alimente l'écran Stats.

### Menu et Stats

- le menu passe de 13 boutons à 6 : Tester, Chevalier, Lore, Stats, Paramètres, Quitter ; tout ce qui existait reste accessible dans les sous-menus ;
- Tester : relancer, résultats détaillés, entraînement, et passer de Libft à ft_printf sans quitter ; l'exam, la banque d'exercices, get_next_line et push_swap sont annoncés en gris ;
- Stats : progression de chaque module, avancement global, activité (jours actifs, série, réussite récente) et faiblesses (ce qui échoue le plus souvent, avec le type d'échec) ; l'historique est gardé dans `~/.testeur_des_fous/stats.txt` ;
- `--anim on|off` et le réglage « animations » du menu sont désormais le même réglage.

### Fusion

- Libft et printf sont deux modules du même testeur ; le moteur commun n'existe plus qu'en un exemplaire (`moteur/`) ;
- le testeur se clone à côté des projets et les détecte tout seul ; `--projet CHEMIN` pour en désigner un ;
- `-a` coupe les animations pour un lancement, `--anim on|off` pour de bon ; `--no-lore` pour les tests seuls ;
- sauvegarde au format 3 : l'ancienne est reprise sans perte et copiée dans `~/.testeur_des_fous/copies/save_avant_v3.txt` ; une copie par jour, 7 gardées ; dossier et fichiers lisibles par toi seul ;
- les résultats sont les mêmes que ceux des testeurs Libft 2.2.1 et printf 1.1.1 sur le même code ;
- plus aucun commentaire dans le code.

### Socle

- dépôt unique, licence MIT ;
- `run.sh` : `--version`, `--help` ;
- `outils/banc.sh` : lance le testeur avec un dossier personnel factice, sans toucher à la vraie sauvegarde ;
- `outils/distros.sh` : lance une commande sur plusieurs distributions ;
- `outils/verif.sh` : contrôles du dépôt.
