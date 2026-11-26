# Historique du testeur des fous

Le numéro de version est dans `moteur/version.h`.

## 3.5.0 · en préparation

### Boutique

- Édition du voyageur galactique : 11 pièces (ciel, serviette, chausses, veste, bulle, regard, antenne, lame, guide, sonde, étoiles), 2 500 ◈ chacune.
- Le casque moulinette : un seul objet, 10 000 ◈.
- Rechercher : « / » (ou la ligne « Rechercher » en haut de la liste) filtre tous les rayons pendant que tu tapes, par nom d'objet ou de collection.
- Nouveau rayon « Collections » : chaque tenue complète en aperçu sur ton chevalier, puis « Porter toute la tenue » ou « Acheter les pièces manquantes ».
- Sous la liste, l'emplacement et la collection de l'objet choisi.
- La sauvegarde peut maintenant garder plus de 1 000 objets.
- 3 nouvelles collections de 11 pièces : Astronaute, Néon, Cyberpunk.
- 6 collections folles de 11 pièces : le Fou, Carnaval, Cirque, Savant fou, Chapelier, Clown triste.
- 7 collections drôles de 11 pièces : Cuisinier, Pyjama, Canard en caoutchouc, Fruits, Carton et scotch, Mauvaise idée, Déguisement raté.
- Correction : le chevalier ne disparaît plus de l'aperçu après avoir parcouru beaucoup d'objets (les images étaient limitées à 96 en mémoire).
- La recherche ignore les accents : « neon » trouve « Néon ».
- Les auras des collections scintillent aux couleurs de leur thème.
- Chaque objet a son histoire, affichée sous la liste dans la boutique et la garde-robe : toutes se passent pendant le Long Silence, avant l'arrivée de ton chevalier.
- Images de la garde-robe allégées (palette réduite, aucune différence à l'écran) : 2,3 Mo deviennent 0,5 Mo.

### Personnalisation

- Nouveau rayon « Personnalisation » dans la boutique : d'autres façons de dépenser tes Octets que les tenues.
- Vague de lumière du menu : or, rose, verte, violette (150 ◈) ou arc-en-ciel (300 ◈).
- Grand titre : penché, ombré ou tramé (250 ◈).
- Costume du fou : or, nuit, néon ou givre (200 ◈).
- Cadre des bulles « Le savais-tu ? » : doré, cyan ou rose (120 ◈).
- Curseur de sélection : ◆ ★ ➤ ● ♦ (80 ◈), partout dans les menus.
- Teintures : rouge, bleue, verte, or, violette, noire (200 ◈). Touche C sur un objet porté pour le teindre.
- Emblème devant ton login (★ ♛ ☾ ✿ ♞ ♥, 100 ◈) et cadre du profil or, cyan ou rose (150 ◈).
- Emplacements de tenues 6 à 10 (300 ◈ chacun), à acheter dans l'ordre.

### Animations

- Ton chevalier est animé, dans le menu, la garde-robe et la boutique : sa tête respire, sa cape ondule, ses yeux clignent, sa plume bouge, sa lanterne scintille, un reflet glisse sur son arme et son familier flotte. Le fou du menu se balance.
- Léger : 5 images par seconde au plus, et seules les cases qui changent sont redessinées. Réglages › Animations « aucune » les coupe.

### Commande tdf

- Si tu supprimes le testeur et le reclones ailleurs, Paramètres propose « Réinstaller la commande tdf » : l'ancienne commande pointait vers un dossier disparu. Marche aussi dans le menu texte.

### Plus simple à utiliser

- Résultats Libft et ft_printf : Entrée ouvre un petit menu près de la fonction (fiche, norminette, audit, relancer les tests, continuer). Le pied de page tient maintenant sur un terminal de 80 colonnes. Les touches f, n, m et r marchent toujours.
- Les listes (Paramètres par exemple) s'adaptent à la hauteur du terminal : en 80x24, tout est visible.
- La touche q ne sert plus à quitter (avec le réglage ZQSD, q veut dire gauche) : Échap revient en arrière partout, Ctrl+C quitte l'écran « Agrandis ton terminal ».
- Le tuto annonce bien 7 écrans, et le menu texte affiche les nouveautés de la bonne version.

### Le savais-tu ?

- Un nouvel easter egg.

### Maintenance

- `outils/verif.sh` vérifie aussi que le code C compile.

## 3.4.0 · 2026-11-20

### Prototypes

- Libft : les prototypes sont comparés au sujet actuel dans libft.h, et maintenant aussi dans chaque ft_*.c : un .c qui définit la fonction autrement (même sans inclure libft.h) est signalé ;
- ft_lstsize doit renvoyer unsigned int : le sujet écrit int, mais une vraie trace de la moulinette montre que son main déclare unsigned int ft_lstsize(t_list *begin), et une Libft en int n'y compile pas ;
- restrict est refusé dans les prototypes (le sujet l'interdit) ;
- parties 2 et 3 : une remarque si les noms des paramètres diffèrent de ceux du sujet ;
- ft_printf et get_next_line : le prototype est vérifié par le compilateur (type exact), plus seulement par une recherche de texte, et le .c qui définit la fonction est comparé au .h.

### Ce qui est poussé

- nouvelle option du test « Ce qui est poussé » : le testeur teste ton dernier commit git au lieu du dossier, et liste d'abord les fichiers qui n'y sont pas ou ont changé depuis (ce que la moulinette ne verra pas).

### Échéances

- Stats › Résumé : les deux prochaines échéances avec le nombre de jours restants (Milestone 1 · Libft le 12/12/2026, Milestone 2 · ft_printf et get_next_line le 27/01/2027 par défaut), en orange à 3 semaines et en rouge à une semaine ; le vendredi, un rappel des inscriptions aux exams ;
- Paramètres › Échéances : changer une date, renommer, ajouter ou supprimer (les dates dépendent de ta promo et de ton campus).

### Comparer deux tentatives

- résultats Libft et get_next_line : « depuis la dernière fois : 2 réparée(s) (split, itoa) · 1 cassée (strlcat) », aussi dans « Revoir la dernière tentative ».

### Lenteur

- Libft : une remarque si strtrim, strjoin, strmapi, striteri, strnstr, strlcat, substr ou strdup devient beaucoup plus lente sur une grande chaîne (16 fois plus de caractères, plus de 100 fois plus de temps : signe d'une boucle qui rappelle ft_strlen) ; mesure relative, donc indépendante de la vitesse de la machine ; ft_split n'est pas mesurée (le suivi mémoire du testeur fausserait la mesure).

### Norminette sans sudo

- Paramètres : si la norminette est introuvable, « Installer la norminette » l'installe dans un environnement Python du testeur (sans sudo, pip --user en secours) ; le testeur l'utilise ensuite tout seul.

### Résumé à partager

- Paramètres : « Résumé à partager » écrit ta dernière tentative en quelques lignes (version, système, résultat, ce qui est faux), sans ton code ni tes chemins, à coller sur Discord pour demander de l'aide.

### Couleurs adaptées

- Paramètres : « Couleurs adaptées » remplace le vert et le rouge par du bleu (OK) et de l'orange (FAUX) dans tous les écrans et les résultats en texte, pour les daltoniens.

### Questions pour la soutenance

- Libft : chaque fiche propose 3 questions pour te préparer à la soutenance (cas limites, choix de types, mémoire), sans les réponses. Ce sont nos questions, pas celles d'une grille de correction.

### Relance automatique

- Options du test : « Relance automatique » (Libft) relance les tests dès que tu sauvegardes un .c, un .h ou le Makefile, tant que l'écran des résultats est ouvert à côté de ton éditeur.

### Includes

- nouvelle section dans les trois projets : chaque #include doit exister, aucun .c inclus, aucune fonction utilisée sans déclaration (avec l'en-tête à ajouter, par exemple <stdlib.h> pour malloc) ;
- chaque .h doit compiler seul, même inclus deux fois, et avoir une garde d'inclusion ;
- remarques : un en-tête système inclus pour rien, un .c qui n'inclut aucun .h du projet.
- les remarques identiques sont regroupées : « 7 fichiers : n'inclut aucun .h du projet (ft_isdigit, ft_isalnum, …) » au lieu de 7 lignes.

### Corrections

- ft_lstmap : nouveau test repris d'une vraie trace de la moulinette, un maillon au content NULL au milieu de la liste ; f renvoie NULL pour ce maillon, qui doit rester dans la nouvelle liste (la liste ne doit pas être libérée) ;
- fiche d'une fonction Libft : la commande de terminal héritée de l'ancien testeur (« bash libft/run.sh --ci -v … », qui ne marchait plus) est remplacée par l'action « r : relancer les tests sans quitter le testeur » ;
- écran d'accueil de ft_printf : plus de commande de terminal, il indique le chemin dans le menu ;
- 21 messages des tests (fichiers en trop, hors git, norminette, nm, sanitizers, plantages, boucle infinie…) étaient seulement en français : ils sont traduits dans les 8 langues ;
- note estimée Libft : quand la norminette est introuvable ou désactivée, c'est maintenant reconnu dans toutes les langues (avant, seulement en français).

## 3.3.0 · 2026-11-19

### Mode correction

- nouveau « Mode correction » dans le menu, pour corriger un autre étudiant : tu choisis le dossier du corrigé, le haut de l'écran affiche « testeur des fous | mode : correction » et le menu ne garde que ce qui sert pendant une correction ;
- Tester tout le projet : tous les tests du corrigé, sans cinématiques ni animations ;
- Vérifs de rendu : fichiers en trop, norminette, Makefile (règles, flags, relink), fonctions interdites, en un seul écran, sans lancer les tests ; marche pour Libft, ft_printf et get_next_line ;
- Lancer un test précis : une grille aux flèches pour choisir une fonction Libft, une catégorie ft_printf (%c, %d, malloc, stress…) ou un BUFFER_SIZE get_next_line ; il se relance seul avec tout le détail ; pour la Libft, la liste des cas testés s'affiche (par exemple chaque split(…) essayé) ;
- les tests du corrigé ne comptent pas dans ta progression : ni stats, ni Octets, ni niveau, ni historique de « Tester mon projet » ;
- « Quitter le mode correction » ramène au testeur normal ;
- choix du dossier du corrigé : « Choisir ce dossier », tes propres projets récents ne sont plus mélangés, et un dossier qui n'est pas un projet est refusé tout de suite ;
- si tu avais quitté le testeur en mode correction, il te demande au lancement si tu continues la correction ou si tu reviens au testeur normal ;
- Échap dans le menu de correction ramène au testeur normal ;
- le login du corrigé (lu dans la 1re ligne de son README, sinon le nom du dossier) s'affiche dans le titre ;
- un bandeau « testeur des fous | mode : correction · login » reste en haut de tous ses écrans de résultats, pour ne jamais les confondre avec les tiens ;
- test précis : une ligne explique la catégorie ft_printf ou le BUFFER_SIZE sélectionné ;
- les anneaux du Puits ne s'affichent plus dans les résultats get_next_line du corrigé ;
- « Revoir la dernière tentative » montre de nouveau TA dernière tentative après une correction, pas celle du corrigé ;
- test précis Libft : plus de sections vides ni d'avertissements sans rapport avec le test choisi ;
- « Mode correction » est un bouton bleu à part, juste sous « Tester mon projet », avec le badge NEW ;
- commande directe : tdf correction [chemin] (ou bash run.sh correction [chemin]) ouvre le testeur en mode correction ;
- une page du tuto et une astuce « Le savais-tu ? » présentent le mode correction ;
- les badges NEW restent maintenant le temps de 3 mises à jour (ceux de la 3.2 reviennent).

### Le savais-tu ?

- chaque bulle reste deux fois plus longtemps à l'écran (15 s au lieu de 7,5 s) ;
- 50 nouvelles phrases, sans contexte, pour garder le moral ou faire réfléchir, dans les 8 langues ; elles apparaissent plus souvent.
- les phrases-clins d'œil entre amis sont devenues rares ;
- nouveau réglage dans Paramètres : « Bulles « Le savais-tu ? » : oui / non ».

### Menu et README

- la vague de lumière sur le titre et le fou ne se met plus en pause quand tu bouges les flèches ;
- README refait : tout le testeur en un coup d'œil, avec 3 captures d'écran (docs/captures).

### Mode debug

- les rapports affichés dans le menu n'ont plus de conseils de ligne de commande (--ci, -v, --valgrind…) ;
- le mode debug ne laisse plus un écran vide pendant toute l'analyse : une barre d'avancement affiche l'étape en cours, le temps écoulé et le temps restant estimé d'après ton dernier passage ; Ctrl+C arrête ;
- dès le premier passage, une estimation approximative du temps restant s'affiche.

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
