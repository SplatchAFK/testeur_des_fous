# Historique du testeur des fous

Le numéro de version est dans `moteur/version.h`.

## 3.0.0 · en construction

Un seul testeur réunit le testeur des fous Libft (2.2.1) et le testeur des fous printf (1.1.1), et ajoute un examshell en C.

### Socle

- dépôt unique, licence MIT ;
- `run.sh` : `--version`, `--help` ;
- `outils/banc.sh` : lance le testeur avec un dossier personnel factice, sans toucher à la vraie sauvegarde ;
- `outils/distros.sh` : lance une commande sur plusieurs distributions ;
- `outils/verif.sh` : contrôles du dépôt.
