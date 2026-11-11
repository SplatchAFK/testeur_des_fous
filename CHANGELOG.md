# Historique du testeur des fous

Le numéro de version est dans `moteur/version.h`.

## 3.0.0 · en construction

Un seul testeur réunit le testeur des fous Libft (2.2.1) et le testeur des fous printf (1.1.1), et ajoute un examshell en C.

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
