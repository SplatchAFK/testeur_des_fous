#include <stdio.h>
#include "fous.h"

const t_boss	g_bosses[3] = {
	{"Le Gardien de la Mémoire", "The Memory Warden",
		"Rien ne sort d'ici sans être rendu.",
		"{n} de tes pierres ne tiennent pas.\nReviens quand elles tiendront.",
		"La brume se retire dans les joints.\nLe Gardien s'immobilise et devient un pilier de la Tour.",
		0, 22, {120, 255, 160}},
	{"Le Tisseur", "The Weaver",
		"Tout ce qui entre ici est découpé.\nMal, le plus souvent.",
		"Ses fils se nouent autour de ton épée.\nIl reste {n} fonctions à reprendre.",
		"Ses fils se tendent droit, comme une harpe,\npuis se taisent.",
		23, 33, {255, 220, 90}},
	{"L'Hydre des Maillons", "The Link Hydra",
		"Aucun mot. Seulement le bruit des chaînes.",
		"Une tête reste suspendue dans le vide, jamais libérée.\nIl reste {n} fonctions qui la nourrissent.",
		"Les maillons se détachent un par un, proprement,\net tombent.",
		34, 42, {120, 220, 255}},
};

const t_exo	g_exos[NEXO] = {
	{"ft_strrev", "char\t*ft_strrev(char *s);", "échange en place, deux index",
		"Inverse la chaîne s sur place (sans malloc) et renvoie s.\n"
		"Exemple : \"abc\" devient \"cba\". La chaîne vide reste vide.",
		"Reverse s in place (no malloc) and return s.",
		"Un pigeon en peignoir",
		"J'ai écrit mon nom à l'envers sur ma boîte aux lettres.\n"
		"Personne ne m'écrit. Retourne-le, mais SUR PLACE :\n"
		"je n'ai pas de deuxième boîte.",
		"Le facteur est passé ! C'était une pub, mais quand même."},
	{"ft_strspn", "size_t\tft_strspn(const char *s, const char *accept);",
		"recherche imbriquée, comme la libc",
		"Renvoie la longueur du début de s fait uniquement de caractères présents dans accept.\n"
		"Même comportement que strspn(3) : ft_strspn(\"hello\", \"leh\") vaut 4.",
		"Length of the initial segment of s made only of characters from accept, like strspn(3).",
		"Une baguette de pain qui fait du yoga",
		"Je compte combien de mes premières lettres sont des lettres zen.\n"
		"Dès qu'une lettre n'est pas zen, j'arrête de compter et je râle.",
		"Ommmm. 4 lettres zen. Je me sens croustillante."},
	{"ft_memrchr", "void\t*ft_memrchr(const void *s, int c, size_t n);",
		"boucle à l'envers avec un size_t",
		"Cherche la DERNIÈRE occurrence de l'octet c (converti en unsigned char)\n"
		"dans les n premiers octets de s. Renvoie son adresse, ou NULL.\n"
		"Piège : n vaut parfois 0, et un size_t ne descend jamais sous 0.",
		"Find the LAST byte c (as unsigned char) in the first n bytes of s, or NULL.",
		"Un escargot archiviste",
		"Je lis mes archives à l'envers, c'est plus rapide pour moi.\n"
		"Trouve la DERNIÈRE fois où j'ai écrit ce caractère...\n"
		"et ne lis jamais avant le début, j'ai horreur des courants d'air.",
		"Trouvé ! Il était caché sous une feuille de salade."},
	{"ft_strcasecmp", "int\t\tft_strcasecmp(const char *s1, const char *s2);",
		"unsigned char et tolower",
		"Compare s1 et s2 sans tenir compte des majuscules.\n"
		"Renvoie un nombre < 0, 0 ou > 0, avec le même signe que strcasecmp(3).\n"
		"Les octets se comparent en unsigned char.",
		"Compare s1 and s2 ignoring case, same sign as strcasecmp(3).",
		"Deux jumeaux qui crient",
		"On dit exactement la même chose, mais lui CRIE et moi je chuchote.\n"
		"Compare-nous sans faire attention aux majuscules.",
		"ON EST D'ACCORD ! enfin... on est d'accord."},
	{"ft_memswap", "void\tft_memswap(void *a, void *b, size_t n);",
		"void * et octets",
		"Échange octet par octet les n premiers octets de a et de b.\n"
		"Les zones ne se chevauchent pas. Doit marcher pour n'importe quel type.",
		"Swap the first n bytes of a and b. Works for any type.",
		"Un brocanteur",
		"Échange ces deux coffres, octet par octet.\n"
		"Je ne sais pas ce qu'il y a dedans. Toi non plus. C'est du void *.",
		"Parfait. Je vais les revendre tous les deux plus cher."},
	{"ft_bitcount", "int\t\tft_bitcount(unsigned int n);", "opérateurs binaires",
		"Renvoie le nombre de bits à 1 dans n.\nExemple : ft_bitcount(7) vaut 3.",
		"Return the number of bits set to 1 in n.",
		"Une ampoule sentimentale",
		"Combien de mes interrupteurs sont allumés ?\n"
		"Compte les bits à 1, s'il te plaît. J'ai peur du noir.",
		"Ouf. Il y a de la lumière quelque part."},
	{"ft_atoi_base", "int\t\tft_atoi_base(const char *str, const char *base);",
		"conversion de base et validation",
		"Convertit str, écrit dans la base donnée, en int.\n"
		"Base invalide (moins de 2 caractères, doublon, '+', '-' ou espace) : renvoie 0.\n"
		"Saute les espaces du début (comme atoi), accepte UN signe, s'arrête au premier\n"
		"caractère qui n'est pas dans la base.",
		"Convert str written in base to int. Invalid base returns 0.",
		"La Grande Moulinette, à moitié réveillée",
		"Je ne lis plus le décimal, c'est trop mainstream.\n"
		"Lis ce nombre dans n'importe quelle base. Si la base est bizarre,\n"
		"réponds 0 et ne discute pas. Je retourne dormir.",
		"Zzz... 255 en hexadécimal... zzz... bien joué... zzz."},
	{"ft_strndup", "char\t*ft_strndup(const char *s, size_t n);",
		"taille d'allocation exacte",
		"Duplique au plus n caractères de s dans une nouvelle chaîne terminée par '\\0'.\n"
		"Alloue exactement ce qu'il faut. Renvoie NULL si malloc échoue.",
		"Duplicate at most n characters of s. NULL if malloc fails.",
		"Un photocopieur fatigué",
		"Copie ce texte, mais pas plus de n lettres.\n"
		"Au-delà, je fais grève et je clignote en orange.",
		"Bip. Copie terminée. Je vais faire une sieste."},
	{"ft_itoa_base", "char\t*ft_itoa_base(long n, const char *base);",
		"LONG_MIN, comptage de chiffres, malloc exact",
		"Écrit n dans la base donnée, dans une chaîne allouée.\n"
		"Base invalide (moins de 2 caractères, doublon, '+' ou '-') : renvoie NULL.\n"
		"Doit gérer LONG_MIN. Renvoie NULL si malloc échoue.",
		"Write n in base into a new string. Handle LONG_MIN. NULL on invalid base.",
		"Un pirate comptable",
		"Note mes trésors dans la base de mon choix.\n"
		"Même mes dettes. Même la plus grosse dette du monde : LONG_MIN.",
		"Arrr ! -9223372036854775808 pièces d'or. Je suis ruiné mais organisé."},
	{"ft_tabdup", "char\t**ft_tabdup(char **tab);",
		"tableau 2D et libération partielle",
		"Duplique un tableau de chaînes terminé par NULL (le tableau ET chaque chaîne).\n"
		"Si un malloc échoue en route, libère tout ce qui a déjà été alloué et renvoie NULL.\n"
		"ft_tabdup(NULL) renvoie NULL.",
		"Deep copy a NULL-terminated array of strings. Free everything if a malloc fails.",
		"Une poule qui collectionne les listes de courses",
		"Duplique ma liste de listes. Toutes les feuilles.\n"
		"Si tu n'as plus de papier en route, rends TOUT et ne garde rien.",
		"Cot cot ! Deux exemplaires de chaque. Je peux enfin perdre l'original."},
	{"ft_split_set", "char\t**ft_split_set(char const *s, char const *set);",
		"split généralisé",
		"Comme ft_split, mais coupe sur N'IMPORTE QUEL caractère présent dans set.\n"
		"Les morceaux vides ne sont pas gardés. Le tableau finit par NULL.\n"
		"Si un malloc échoue : tout libérer et renvoyer NULL.",
		"Like ft_split, but splits on any character of set.",
		"Un chef qui coupe tout",
		"Coupe cette phrase à chaque ingrédient interdit.\n"
		"Pas seulement les virgules : TOUS les ingrédients de ma liste noire.",
		"Magnifique julienne de chaînes. Trois étoiles."},
	{"ft_str_replace", "char\t*ft_str_replace(const char *s, const char *old, const char *new);",
		"deux passes : compter puis construire",
		"Renvoie une nouvelle chaîne où chaque occurrence de old est remplacée par new,\n"
		"de gauche à droite, sans chevauchement (\"aaaa\", \"aa\", \"b\" donne \"bb\").\n"
		"Si old est vide, renvoie une copie de s. NULL si malloc échoue.",
		"Replace every occurrence of old by new, left to right, in a new string.",
		"Un perroquet correcteur",
		"Remplace chaque mot par un autre. Partout.\n"
		"Et alloue la bonne taille, je ne veux pas de plumes qui dépassent.",
		"COCO CORRIGÉ ! COCO CORRIGÉ !"},
	{"ft_putnbr_base_fd", "void\tft_putnbr_base_fd(long n, const char *base, int fd);",
		"récursion et write",
		"Écrit n dans la base donnée sur le file descriptor fd.\n"
		"Base invalide (moins de 2 caractères, doublon, '+' ou '-') : n'écrit rien.\n"
		"Doit gérer LONG_MIN. Seule fonction d'écriture autorisée : write.",
		"Write n in base on fd. Invalid base writes nothing. Handle LONG_MIN.",
		"Une radio pirate",
		"Diffuse ce nombre sur ma fréquence fd, dans n'importe quelle base.\n"
		"Les auditeurs adorent l'hexadécimal le dimanche.",
		"Ici Radio Fous, 7FFFFFFFFFFFFFFF auditeurs nous écoutent."},
	{"ft_sort_strs", "void\tft_sort_strs(char **tab, int (*cmp)(const char *, const char *));",
		"pointeurs sur fonction et tri",
		"Trie sur place un tableau de chaînes terminé par NULL, dans l'ordre croissant\n"
		"donné par cmp (cmp(a, b) > 0 veut dire que a va après b).\n"
		"Échange les pointeurs, ne recopie jamais les chaînes.",
		"Sort a NULL-terminated array in place using cmp. Swap pointers only.",
		"Un bibliothécaire indécis",
		"Range mes livres. Mais c'est moi qui décide comment on compare :\n"
		"par titre, par taille, par couleur de couverture...\n"
		"Déplace les étiquettes, pas les livres.",
		"Rangés par longueur de titre. C'est absurde. J'adore."},
	{"ft_lstget", "t_list\t*ft_lstget(t_list *lst, size_t index);", "parcours et bornes",
		"Renvoie le maillon numéro index (le premier est 0), ou NULL s'il n'existe pas.",
		"Return the node at index (0 is the first), or NULL.",
		"Un train fantôme",
		"Dis-moi quel wagon est à la position i.\n"
		"S'il n'existe pas, ne monte surtout pas. Il y a des fantômes.",
		"Wagon trouvé. Le fantôme te fait un signe de la main."},
	{"ft_lstreverse", "void\tft_lstreverse(t_list **lst);", "jongler avec trois pointeurs",
		"Retourne la liste sur place : le dernier maillon devient le premier.\n"
		"Réutilise les maillons existants, aucun malloc. *lst pointe sur la nouvelle tête.",
		"Reverse the list in place, reusing the nodes.",
		"Une chenille qui a perdu le nord",
		"Je veux marcher à reculons.\n"
		"Retourne-moi, mais sans me fabriquer un nouveau corps, j'y tiens.",
		"Je recule ! Je recule ! Attention au mur."},
	{"ft_lstfind", "t_list\t*ft_lstfind(t_list *lst, const void *ref,\n\t\t\tint (*cmp)(const void *, const void *));",
		"callback avec contexte",
		"Renvoie le PREMIER maillon tel que cmp(maillon->content, ref) vaut 0, sinon NULL.",
		"Return the first node where cmp(content, ref) == 0, else NULL.",
		"Un chien renifleur",
		"Trouve le premier maillon qui sent comme ma référence.\n"
		"C'est toi qui renifles, avec la fonction cmp. Moi je remue la queue.",
		"Wouf ! (ça veut dire bravo en chien)"},
	{"ft_lstremove_if", "void\tft_lstremove_if(t_list **lst, const void *ref,\n\t\t\tint (*cmp)(const void *, const void *), void (*del)(void *));",
		"supprimer en parcourant, t_list **",
		"Retire de la liste tous les maillons tels que cmp(content, ref) vaut 0.\n"
		"Pour chacun : del(content) puis free(maillon). La tête peut changer.",
		"Remove every node where cmp(content, ref) == 0 (del then free).",
		"Un jardinier",
		"Arrache toutes les mauvaises herbes de la rangée.\n"
		"Même la première ! Et jette-les avec del, pas dans mes chaussons.",
		"Rangée impeccable. Les mauvaises herbes ont été libérées."},
	{"ft_lstsort", "void\tft_lstsort(t_list **lst, int (*cmp)(const void *, const void *));",
		"trier une liste chaînée",
		"Trie la liste par ordre croissant selon cmp (cmp(a, b) > 0 : a va après b).\n"
		"Tu peux déplacer les maillons ou échanger les content.",
		"Sort the list in ascending order according to cmp.",
		"Un sergent à moustache",
		"Mets-moi cette file en ordre ! Du plus petit au plus grand !\n"
		"Et plus vite que ça, ma moustache s'impatiente !",
		"REPOS ! Enfin une file présentable."},
	{"ft_lst_to_array", "void\t**ft_lst_to_array(t_list *lst);", "liste vers tableau",
		"Renvoie un tableau alloué contenant les pointeurs content de la liste, dans l'ordre,\n"
		"terminé par NULL (pour une liste vide : un tableau qui contient juste NULL).\n"
		"Ne copie pas les contenus. NULL si malloc échoue.",
		"Return a NULL-terminated array of the content pointers, in order.",
		"Un déménageur",
		"Mets tous les cartons de la liste dans le camion, dans l'ordre,\n"
		"et un NULL tout au fond pour fermer la porte.",
		"Camion fermé. On a perdu aucun carton, c'est une première."},
	{"ft_lstrotate", "void\tft_lstrotate(t_list **lst, int n);", "modulo, négatifs, relier la queue",
		"Fait tourner la liste de n crans : n > 0 envoie les n premiers maillons à la fin,\n"
		"n < 0 envoie les |n| derniers au début. n peut être plus grand que la liste\n"
		"(et peut valoir INT_MIN). Liste vide ou NULL : ne rien faire.",
		"Rotate the list by n (negative goes backwards, n may exceed the size or be INT_MIN).",
		"La Grande Moulinette, pour de bon",
		"Dernier test. Fais tourner ma file de n crans.\n"
		"Vers l'avant, vers l'arrière, même si n est ÉNORME.\n"
		"Après, je me rendors pour mille ans.",
		"La Moulinette sourit, se retourne et ronfle. Tu as tout compris."},
};

const char	*g_ach_fr[NACH][2] = {
	{"Premier sang", "Lancer le testeur une première fois."},
	{"Zéro fuite", "Aucune fuite sur les 43 fonctions."},
	{"43 sur 43", "Les 43 fonctions passent."},
	{"Prototypes parfaits", "Les 43 prototypes collent au sujet."},
	{"La moulinette sourit", "Audit complet sans erreur."},
	{"Survivant du segfault", "Faire planter une scène, puis relancer."},
	{"Maître du chevauchement", "ft_memmove propre."},
	{"Le Gardien est tombé", "Battre le Gardien de la Mémoire."},
	{"L'Alchimiste est dissous", "Battre l'Alchimiste des Chaînes."},
	{"L'Hydre est vidée", "Battre l'Hydre t_list."},
	{"Apprenti fou", "Réussir 7 niveaux d'entraînement."},
	{"Compagnon fou", "Réussir 14 niveaux d'entraînement."},
	{"Maître fou", "Réussir les 21 niveaux d'entraînement."},
	{"Fashion victim", "Posséder 10 objets."},
	{"Collectionneur de titres", "Posséder 25 titres."},
	{"Riche en Octets", "Avoir 1000 Octets en poche."},
	{"Assidu", "Lancer le testeur 10 fois."},
	{"Styliste complet", "Équiper un objet dans chaque emplacement."},
};

const char	*g_ach_en[NACH][2] = {
	{"First blood", "Run the tester once."},
	{"Zero leaks", "No leak in the 43 functions."},
	{"43 out of 43", "All 43 functions pass."},
	{"Perfect prototypes", "The 43 prototypes match the subject."},
	{"The moulinette smiles", "Full audit without errors."},
	{"Segfault survivor", "Crash a scene, then restart."},
	{"Overlap master", "Clean ft_memmove."},
	{"The Warden fell", "Beat the Memory Warden."},
	{"The Alchemist dissolved", "Beat the String Alchemist."},
	{"The Hydra drained", "Beat the t_list Hydra."},
	{"Crazy apprentice", "Clear 7 training levels."},
	{"Crazy companion", "Clear 14 training levels."},
	{"Crazy master", "Clear all 21 training levels."},
	{"Fashion victim", "Own 10 items."},
	{"Title collector", "Own 25 titles."},
	{"Rich in Octets", "Have 1000 Octets."},
	{"Dedicated", "Run the tester 10 times."},
	{"Full stylist", "Equip an item in every slot."},
};

const t_rgb	g_swatch[10] = {
	{0, 0, 0}, {232, 64, 74}, {247, 146, 44}, {252, 218, 66}, {110, 220, 100},
	{86, 222, 232}, {74, 124, 232}, {164, 96, 226}, {255, 140, 200}, {245, 245, 250},
};

const char	*g_slot_fr[NSLOT] = {"Fond", "Cape", "Jambes", "Armure", "Casque", "Yeux",
	"Déco de casque", "Arme", "Main gauche", "Familier", "Aura"};
const char	*g_slot_en[NSLOT] = {"Background", "Cape", "Legs", "Armor", "Helmet", "Eyes",
	"Helmet decoration", "Weapon", "Left hand", "Pet", "Aura"};

static const char	*g_tn_fr[40] = {
	"Pigeon", "Chevalier", "Grille-pain", "Pointeur", "Octet", "Canard", "Segfault",
	"Escargot", "Baguette", "Moulinette", "Chaussette", "Malloc", "Hamster", "Cornichon",
	"Fantôme", "Brocoli", "Pingouin", "Bug", "Compilateur", "Lama", "Tabouret", "Macro",
	"Spaghetti", "Hérisson", "Ver de terre", "Frigo", "Lutin", "Buffer", "Poulpe",
	"Raton laveur", "Tournevis", "Pixel", "Champignon", "Kebab", "Robot", "Yaourt",
	"Dragon", "Clavier", "Cactus", "Gnome"};
static const char	*g_tc_fr[25] = {
	"de la Pile", "du Tas", "sans Fuite", "de la Norme", "en Peignoir",
	"des Pointeurs Perdus", "du '\\0'", "de la Libc", "du Makefile", "en Promo",
	"qui Relink", "du Segfault Sacré", "de la Mémoire Vive", "à Moustache",
	"des Listes Chaînées", "du Dimanche", "Inoxydable", "en Carton", "de minuit",
	"Quantique", "des 43 Fonctions", "Libéré", "Légendaire", "Fou Furieux", "Absolu"};
static const char	*g_tn_en[40] = {
	"Pigeon", "Knight", "Toaster", "Pointer", "Byte", "Duck", "Segfault", "Snail",
	"Baguette", "Moulinette", "Sock", "Malloc", "Hamster", "Pickle", "Ghost", "Broccoli",
	"Penguin", "Bug", "Compiler", "Llama", "Stool", "Macro", "Spaghetti", "Hedgehog",
	"Earthworm", "Fridge", "Goblin", "Buffer", "Octopus", "Raccoon", "Screwdriver",
	"Pixel", "Mushroom", "Kebab", "Robot", "Yogurt", "Dragon", "Keyboard", "Cactus", "Gnome"};
static const char	*g_tc_en[25] = {
	"of the Stack", "of the Heap", "without Leaks", "of the Norm", "in a Bathrobe",
	"of Lost Pointers", "of the '\\0'", "of the Libc", "of the Makefile", "on Sale",
	"who Relinks", "of the Holy Segfault", "of the RAM", "with a Moustache",
	"of Linked Lists", "of Sunday", "Stainless", "made of Cardboard", "of Midnight",
	"Quantum", "of the 43 Functions", "Freed", "Legendary", "Furious", "Absolute"};

const char	*title_name(int i, char *buf, size_t sz)
{
	if (i < 0 || i >= NTITLE)
		i = 0;
	if (i == NTITLE - 1)
		return (snprintf(buf, sz, "%s", L("Vétéran de la Libft", "Libft Veteran")), buf);
	snprintf(buf, sz, "%s %s", L(g_tn_fr[i % 40], g_tn_en[i % 40]), L(g_tc_fr[i / 40], g_tc_en[i / 40]));
	return (buf);
}

int	title_price(int i)
{
	unsigned int	h;

	if (i == 1)
		return (0);
	if (i == NTITLE - 1)
		return (-1);
	h = (unsigned int)i * 2654435761u;
	return (15 + (int)((h >> 7) % 12) * 15);
}
