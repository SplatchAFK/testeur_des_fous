# Historique du testeur des fous

Le numéro de version est dans `moteur/version.h`.

## 3.0.0 · en construction

Un seul testeur réunit le testeur des fous Libft (2.2.1) et le testeur des fous printf (1.1.1), et ajoute un examshell en C.

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
