# Historique du testeur des fous

Le numéro de version est dans `moteur/version.h`.

## 3.3.0 · 2026-11-19

### Mode correction

- nouveau « Mode correction » dans le menu, pour corriger un autre étudiant : tu choisis le dossier du corrigé, le haut de l'écran affiche « testeur des fous | mode : correction » et le menu ne garde que ce qui sert pendant une correction ;
- Tester tout le projet : tous les tests du corrigé, sans cinématiques ni animations ;
- Vérifs de rendu : fichiers en trop, norminette, Makefile (règles, flags, relink), fonctions interdites, en un seul écran, sans lancer les tests ; marche pour Libft, ft_printf et get_next_line ;
- Lancer un test précis : une grille aux flèches pour choisir une fonction Libft, une catégorie ft_printf (%c, %d, malloc, stress…) ou un BUFFER_SIZE get_next_line ; il se relance seul avec tout le détail ; pour la Libft, la liste des cas testés s'affiche (par exemple chaque split(…) essayé) ;
- « Quitter le mode correction » ramène au testeur normal.

### Mode debug

- le mode debug ne laisse plus un écran vide pendant toute l'analyse : une barre d'avancement affiche l'étape en cours, le temps écoulé et le temps restant estimé d'après ton dernier passage ; Ctrl+C arrête.

## 3.2.0 · 2026-11-18

### Libft

- ft_lstclear : nouveau test repris d'une trace de la moulinette, un maillon au content NULL au milieu de la liste ; del doit être appelée sur chaque maillon, même quand son content vaut NULL.

### Norminette

- les erreurs de la norminette se voient clairement dans les trois projets : bannière rouge sur les résultats, touche n pour la liste complète (fichier, ligne, colonne, règle, explication) ;
- Libft : la fiche de chaque fonction (touche f) montre les erreurs de Norme de son fichier ;
- une norminette installée mais cassée (le module Python manque) n'est plus prise pour une norminette sans erreur : le testeur dit qu'elle est introuvable.

### get_next_line

- nouvel écran de résultats dans le style du testeur : bilan, BUFFER_SIZE testés, anneaux du Puits, norminette, vérifications par partie et détail de chacune avec ce qu'il faut corriger ;
- « Revoir la dernière tentative » rouvre cet écran ;
- un test qui échoue seulement avec BUFFER_SIZE=10000000 passe en avertissement (on ne sait pas si la moulinette teste une taille aussi grande), avec le conseil d'allouer le tampon avec malloc ;
- un échec dans la partie « Pour info (hors sujet) » ne compte plus comme un échec.

### Chevalier

- Tenues : un rayon « Tenues » directement dans la garde-robe (◀ ▶, ou Chevalier › Tenues) : « ＋ Enregistrer ce que je porte », puis jusqu'à 5 tenues ; survoler une tenue la montre sur ton chevalier, Entrée ouvre un petit menu (porter, remplacer par ce que je porte, renommer, supprimer) ; tout se fait aux flèches et à Entrée ; elles sont gardées dans `tenues.txt`, à côté de la sauvegarde ;
- garde-robe et boutique : les actions possibles s'affichent juste sous la liste, au lieu d'une ligne en bas de l'écran ;
- grand aperçu (touche v, dans la garde-robe, la boutique et les tenues) : ton chevalier en grand, avec l'objet survolé ;
- pseudo : vraie saisie (les lettres z, q, s, d ne sont plus prises pour des flèches), avec curseur (◀ ▶), Ctrl+U pour tout effacer, compteur de caractères, aperçu dans la couleur du pseudo, Échap pour annuler ; lettres, chiffres et - _ . seulement ; la même saisie sert pour la bio, les tenues et les chemins.

### Examshell des fous

- grademe refuse un rendu avec des fichiers en trop (garbage files) : seuls les fichiers demandés sont acceptés dans `rendu/<exercice>/`, la liste des fichiers en trop s'affiche ; la banque n'est pas concernée.

### Stats et credits

- Stats : ↓ sur les pages Libft, ft_printf, get_next_line et Exam ouvre les détails (chaque fonction ou test, validé ou non, nombre d'échecs, dernier échec, types d'échec) ;
- Stats : nouvel onglet get_next_line (passages, dernier et meilleur score, validé ou non, anneaux du dernier Puits, tes passages, ce qui échoue le plus) ;
- « Ce qui échoue le plus » : noms des tests get_next_line lisibles et colonnes alignées ;
- credits refaits pour tout le testeur : Libft, ft_printf, get_next_line, Examshell des fous et banque, moteur de tests, lore et langues.

### Confort

- Tester mon projet : options du test aux flèches, gardées d'un lancement à l'autre : mode debug (tous les rapports détaillés), audit complet, valgrind en plus, sans cinématiques, sans animations ;
- un badge NEW rose signale les nouveautés de la version, là où elles se trouvent (il disparaît à la version suivante) ;
- l'option `--debug` de la ligne de commande est retirée : le mode debug se règle dans Tester mon projet › Options du test ;
- `bash run.sh projet <chemin>` (ou `bash run.sh <chemin>`) teste le projet puis ouvre le vrai menu, avec le titre et le fou ;
- Tester mon projet : après ↑ (écrire un chemin), ↓ ramène à la liste ;
- « Le savais-tu ? » apparaît directement dans son style, en fondu, sans passer par du texte blanc ;
- « Le savais-tu ? » n'est plus coupé : la bulle de gauche ne descend plus sur la description du menu, et une astuce trop longue pour la place disponible est remplacée par une autre au lieu d'être tronquée ;
- get_next_line : la cinématique du Puits se joue de nouveau après le test, avant le rapport ;
- Banque d'exercices : chaque exercice affiche les Octets qu'il rapporte à la première réussite (+15 à +30 ◈) ;
- le changelog affiché dans le testeur montre de nouveau le lien du Discord en entier.

## 3.1.1 · 2026-11-16

- examshell : les flèches maintenues ou tapées vite ne figent plus les menus : seules les lignes qui changent sont réécrites (environ 200 octets par flèche au lieu de 13 Ko) et plus aucun appui n'est perdu ; plus de « [A » dans la ligne de commande ;
- examshell lancé depuis le menu : il reste dans l'écran du testeur, plus rien ne traîne dans le terminal après l'avoir quitté ;
- résultats Libft et ft_printf : l'écran ne se ferme plus tout seul après quelques secondes (la réponse de certains terminaux, kitty, WezTerm, Ghostty, iTerm2…, était lue comme la touche Échap) ; les touches tapées pendant les tests sont ignorées ;
- les dossiers de compilation font le ménage tout seuls : seule la version à jour de chaque cache est gardée, les anciennes versions et les caches des autres langues (inutilisés depuis plus d'une heure) sont supprimés.

## 3.1.0 · 2026-11-15

### Un seul menu, graphique

- `bash run.sh` ou `tdf` ouvre directement le menu graphique d'origine : titre puis fou centrés en haut, chevalier à gauche, choix centrés ;
- premier bouton « Tester mon projet » : « . » pour le dossier où tu es, les projets trouvés en raccourci, ou le parcours du PC comme dans zsh (Tab, Tab répété, ▶ ouvre, ◀ remonte) ;
- puis Exam en C, Banque d'exercices, Revoir la dernière tentative, Chevalier, Lore, Stats, Paramètres, Quitter ;
- conseils d'affichage au premier lancement, après chaque mise à jour, et dès qu'un problème est détecté (terminal plus petit que 120 × 38, pas d'UTF-8, couleurs limitées) ; ils rappellent aussi de mettre le testeur sur un côté et l'éditeur en grand à côté ;
- après une mise à jour (`git pull`), tout se passe comme à la première installation (conseils, tuto, changelog), sans perdre ta progression ;
- premier lancement : la langue, l'écran d'accueil, le login, puis le tuto ; ensuite plus rien avant le menu ;
- si l'interface graphique ne compile pas, l'ancien menu texte prend le relais.

### Le savais-tu ?

- les conseils apparaissent tour à tour à gauche et à droite du menu, avec un effet pop (le texte jaillit, puis se resserre), restent quelques secondes puis disparaissent en fondu ;
- 50 conseils pour mieux utiliser le testeur et 15 phrases bienveillantes ; de temps en temps, un easter egg.

### get_next_line

- BUFFER_SIZE=0 et BUFFER_SIZE=-1 : get_next_line doit renvoyer NULL, sans planter (des étudiants rapportent que la moulinette teste ce cas).

### Tester mon projet

- aux flèches : « Tester ce dossier » en haut, puis tes derniers tests (chemin, score, date) qu'Entrée relance, puis les projets trouvés à côté du testeur ;
- ↑ tout en haut pour écrire un chemin : Tab complète comme zsh, sans tenir compte des majuscules, Tab répété fait défiler, ▶ ouvre, ◀ remonte ;
- les 10 derniers projets testés sont gardés.

### Discord, crédits et Changelog

- le Discord s'appelle « 42 Next Global - Serveur non Officiel » ; contact : @splatch_afk ;
- crédits du testeur des fous (et non plus d'une édition Libft ou printf), avec le rappel que c'est un testeur et pas un jeu vidéo ;
- « Nouveautés » devient « Changelog », avec le lien du Discord, cliquable (Ctrl + clic) dans les terminaux qui le permettent ;
- la trace « Nous aider » (touche t) est créée dans `testeur_des_fous/trace/` ;
- le mode texte, qui ne changeait rien, est retiré.

### Progression

- Profil : les succès se parcourent aux flèches, avec pour chacun s'il est débloqué et comment le débloquer ;
- la cape et le casque du vétéran se débloquent dès qu'un passage Libft sans échec est enregistré ;
- holygraph : un projet réussi une fois sans échec reste validé pour toujours ; les modules après Programming Fundamentals sont grisés en attendant leurs testeurs ;
- banque : le nombre d'exercices et de réussites s'affiche pour chaque catégorie.

### Niveaux et rangs

- le niveau monte sans limite avec l'XP (tests Libft et ft_printf, banque, exam) ;
- un nouveau rang tous les 10 niveaux, avec des noms tirés de la Cité : Apprenti de la Tour, Gardien des Fondations, Forgeron des Formats, Porteur de Lanterne, Plongeur du Puits, Trieur des Deux Piles, Veilleur du Silence, Ouvreur de Portes, Horloger du Mécanisme, Architecte de la Cité, puis Légende de la Cité (II, III…) ;
- le rang s'affiche sous ton chevalier dans le menu, sur le profil et dans les Stats (avec le niveau du prochain rang) ; un écran « Nouveau rang » apparaît quand tu en changes ;
- Chevalier › Rangs : tous les rangs, ceux déjà passés en vert, le tien en surbrillance, ceux à venir grisés, et en bas ta progression vers le prochain.

### Confort

- le testeur reste à l'écran entre deux choix : plus de retour furtif au terminal ;
- les conseils d'affichage ne reviennent plus à chaque retour au menu : une fois par problème détecté (et toujours au premier lancement et après une mise à jour) ;
- Tester mon projet : un encadré explique où chercher ton projet (../ quand le testeur est à côté, . pour ici, ~/ et /, fichiers attendus) et rappelle le dossier où tu es ;
- « Le savais-tu ? » ne reste plus bloqué en blanc quand tu reviens au menu ;
- Nous aider : après « t », le chemin de la trace reste affiché sur la page ;
- Stats › Courbes : mention « en cours d'amélioration · BETA ».

### Stats

- les pages prennent toute la largeur du terminal ;
- nouvelle page Courbes, sur tes 50 dernières tentatives : réussite par tentative (couleur du projet), niveau et XP, activité par jour, exercices d'exam et de la banque réussis cumulés ;
- Historique : les 50 dernières tentatives se parcourent aux flèches, avec pour chacune la date, le projet, le score, l'écart avec la tentative d'avant et ce qui a échoué ;
- seules les 50 dernières tentatives sont gardées (c'est écrit sur la page) ; un projet validé une fois reste validé même quand sa tentative est supprimée.

### Après un test

- l'écran des résultats s'affiche directement avec tous les tests et leur détail ; en le quittant, retour au menu, sans « Entrée pour continuer » ;
- « Revoir la dernière tentative » rouvre ces résultats depuis le menu (Libft, ft_printf, rapport get_next_line, dernière correction de la banque) ;
- les rapports texte (get_next_line, mode texte) s'affichent dans une fenêtre graphique qu'on fait défiler ;
- plus d'images en pixels à droite des tests ;
- cinématiques seulement quand une étape est franchie : une partie de la Libft terminée, une nouvelle chaîne de ft_printf brisée ;
- toutes les cinématiques se revoient dans Lore : Libft, ft_printf (Fonderie, chaînes, big boss, fins), get_next_line (le Puits de ton dernier passage), l'Épreuve du Silence, la fin.

### Cinématiques

- le Puits (get_next_line), l'Épreuve du Silence (avant et après l'exam), le dernier mécanisme et la fin secrète sont redessinés en pixel art, dans le même style que la Tour de la Libft et la Fonderie de ft_printf, avec ton chevalier, Moulinette et les gardiens en peluche ;
- le Puits : la lanterne descend le long des 22 anneaux, bleus quand le test passe, rouges quand il échoue ;
- l'Épreuve : les portes s'ouvrent une à une selon ton résultat ;
- Lore : la Libft se rejoue scène par scène (intro, trois salles, trois combats après un test, finale), même sans test chargé ; get_next_line, l'exam et la fin se rejouent sans quitter le menu, même si le lore est désactivé pendant les tests.

### Banque d'exercices graphique

- la banque passe dans le menu graphique : onglets Tout, Exam, Libft, printf, sujet qui défile, boutons Corriger mon code, Langue du sujet, Retour ;
- la correction reste celle de l'examshell (mêmes tests, Norme, sanitizers) et s'affiche dans une fenêtre graphique.

### Tuto et nouveautés

- tuto graphique en 6 écrans au premier lancement : les touches, tester un projet, l'exam, la banque, Chevalier/Lore/Stats, Paramètres ;
- à chaque nouvelle version, un écran « Nouveautés de la version X » s'affiche une fois ;
- les deux se revoient depuis Paramètres.

### Le fou

- plus précis : rendu en demi-blocs à partir du dessin peint (deux pixels de couleur par case), dans le menu et l'en-tête de l'examshell des fous.

### Flèches

- une flèche maintenue ne fige plus l'écran : plus d'animations de couleur redessinées à chaque image, les appuis répétés sont regroupés ;
- ✓ à la place de ✔ dans la banque (il était coupé dans certains terminaux).

### Banque d'exercices

- aux flèches : onglets Tout, Exam, Libft, printf (◀ ▶), Entrée ouvre le sujet ;
- sous le sujet : Corriger mon code, Langue du sujet (◀ ▶), Retour à la liste ;
- plus besoin de taper `liste`, `ouvre` ou `grademe`.

### Examshell des fous

- en-tête plus petit, en haut à gauche : le fou et « EXAMSHELL DES FOUS » quand la place le permet, le titre seul sinon ;
- commandes en badges de couleurs claires (grademe, sujet, status, finish, help), comme le badge Discord ;
- SUCCESS et FAILURE en badges vert et rose ;
- une commande mal tapée propose la plus proche (« Tu voulais dire grademe ? »), Entrée pour accepter.

### Langues

- les 94 sujets de l'exam et de la banque existent en 8 langues ; `sujet <code>` (fr, en, es, de, it, pt, tr, ru), l'anglais sert de repli ;
- la langue du sujet suit celle de l'interface par défaut ;
- lanceur, aide, examshell, banque, répliques des boss, rapports Libft, ft_printf et get_next_line, fichiers parasites et valgrind traduits ;
- les sujets anglais Libft et printf n'ont plus de lignes en français ;
- corrigé : « Réglages » et « Paramètres » donnaient le même mot en anglais, les Octets de l'avatar, « niv. », « programme », « fonction » et les notions Libft et printf de la banque restaient en français, les notions en cyrillique étaient coupées au milieu d'une lettre.

### README

- réécrit : démarrage en 30 secondes, le menu et ses touches, l'examshell des fous et la banque pas à pas ;
- deux commandes seulement : `run.sh exam` et `--debug`.

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
