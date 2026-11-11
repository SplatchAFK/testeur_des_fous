# Historique du testeur des fous

Le numéro de version est dans `moteur/version.h`.

## 3.0.0 · en construction

Un seul testeur réunit le testeur des fous Libft (2.2.1) et le testeur des fous printf (1.1.1), et ajoute un examshell en C.

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
