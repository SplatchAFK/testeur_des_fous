# Le testeur des fous

> Version 3.0.0 en construction · [historique](CHANGELOG.md) · créé par **Splatch** (`romasant`) pour les étudiants de 42 Next.

## Commandes

```bash
bash testeur_des_fous/run.sh              # lancer le testeur
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

Il te faut `cc`, `make` et `bash`. Rien à installer, aucun `sudo`.

## Ce qu'il teste

| Module | État |
|---|---|
| Libft | en cours d'intégration |
| ft_printf | en cours d'intégration |
| Exam (C) | à venir |
| Banque d'exercices | à venir |
| get_next_line | plus tard |
| push_swap | plus tard |

Le testeur ne contient aucune solution et n'écrit rien à ta place : il casse ton code et te dit où regarder.

## Discord

Entraide, bugs, idées : [discord.gg/SwAmfRcXCH](https://discord.gg/SwAmfRcXCH)

## Crédits et licence

Créé et vibe-codé par Splatch (`romasant`). Licence MIT, voir [LICENSE](LICENSE).
