#!/usr/bin/env bash
set -u

TDF_RACINE="$(cd "$(dirname "$0")" && pwd)"
TDF_NOM="$(basename "$TDF_RACINE")"
VERSION_H="$TDF_RACINE/moteur/version.h"
DOSSIER="$HOME/.testeur_des_fous"

if [ ! -f "$VERSION_H" ]; then
	echo "testeur des fous : moteur/version.h introuvable, le dossier est incomplet." >&2
	exit 1
fi

MANQUE_OUTILS=""
for outil in cc make awk sed grep nm tar; do
	command -v "$outil" >/dev/null 2>&1 || MANQUE_OUTILS="$MANQUE_OUTILS $outil"
done
if [ -n "$MANQUE_OUTILS" ]; then
	echo "testeur des fous : il manque sur cette machine :$MANQUE_OUTILS"
	echo "Ces outils sont présents sur les postes 42. Ailleurs, installe-les avec le gestionnaire de paquets."
	exit 1
fi

lire_version() {
	sed -n "s/^# *define $1 \"\(.*\)\"/\1/p" "$VERSION_H" | head -1
}

TDF_VERSION="$(lire_version TDF_VERSION)"
TDF_DATE="$(lire_version TDF_DATE)"
. "$TDF_RACINE/moteur/chargement.sh"
. "$TDF_RACINE/moteur/debug.sh"
. "$TDF_RACINE/moteur/cli.sh"
ARGS_ORIG=("$@")

usage() {
	local c="bash $TDF_NOM/run.sh"
	[ "${TDF_CLI:-0}" = 1 ] && c="$TDF_CMD_NOM"
	cat <<EOF
Le testeur des fous $TDF_VERSION
Créé et vibe-codé par Splatch (romasant)

  $c                  ouvrir le menu (projets détectés, exam, banque...) aux flèches
  $c libft            tester ta Libft
  $c printf           tester ton ft_printf
  $c gnl              tester ton get_next_line
  $c exam             passer un exam en C (4 niveaux, grademe)
  $c exam --serie 3 --difficulte croissante   une série de 1 à 5 exos (croissante ou 1 à 4)
  $c banque           la banque d'exercices (exam, Libft, printf)
  $c --menu           ouvrir le menu sans lancer de tests (stats, chevalier...)
  $c debug            le journal de la dernière session (debug aide : tout)
  $c --projet CHEMIN  tester le projet de ce dossier
  $c --no-lore        les tests seuls, sans histoire
  $c -a               sans animations pour ce lancement
  $c --anim off       couper les animations pour de bon (on pour les remettre)
  $c --debug          tous les rapports détaillés
  $c --version        le numéro de version

La commande « $TDF_CMD_NOM », utilisable depuis n'importe quel dossier, sans sudo :
  bash $TDF_NOM/run.sh installer     installe ~/.local/bin/$TDF_CMD_NOM
  $TDF_CMD_NOM maj                         met le testeur à jour (git pull)
  $TDF_CMD_NOM desinstaller                retire la commande (le testeur reste)

Les options d'un module : $c libft --help   ou   $c printf --help
EOF
}

est_printf() {
	[ -f "$1/Makefile" ] || return 1
	[ -f "$1/ft_printf.h" ] && return 0
	grep -q 'libftprintf' "$1/Makefile" 2>/dev/null
}

est_libft() {
	[ -f "$1/libft.h" ] || return 1
	est_printf "$1" && return 1
	return 0
}

est_gnl() {
	[ -f "$1/get_next_line.c" ] && [ -f "$1/get_next_line.h" ]
}

LIBFTS=()
PRINTFS=()
GNLS=()
VUS="|"

examiner() {
	local d
	[ -d "$1" ] || return 0
	d="$(cd "$1" 2>/dev/null && pwd -P)" || return 0
	[ "$d" = "$(cd "$TDF_RACINE" && pwd -P)" ] && return 0
	case "$VUS" in *"|$d|"*) return 0 ;; esac
	VUS="$VUS$d|"
	if est_printf "$d"; then
		PRINTFS+=("$d")
	elif est_libft "$d"; then
		LIBFTS+=("$d")
	elif est_gnl "$d"; then
		GNLS+=("$d")
	fi
}

detecter() {
	local base d
	examiner "$PWD"
	examiner "$TDF_RACINE/.."
	for base in "$PWD" "$TDF_RACINE/.."; do
		for d in "$base"/*/; do
			[ -d "$d" ] && examiner "$d"
		done
	done
}

court() {
	case "$1" in
		"$PWD") printf '.' ;;
		"$PWD"/*) printf './%s' "${1#"$PWD"/}" ;;
		"$HOME"/*) printf '~/%s' "${1#"$HOME"/}" ;;
		*) printf '%s' "$1" ;;
	esac
}

menu_depart() {
	local items=() acts=() p a sous n
	for p in ${LIBFTS[@]+"${LIBFTS[@]}"}; do
		items+=("Tester ma Libft          $(court "$p")|Les 43 fonctions, la mémoire, la Norme, les fichiers parasites, l'audit moulinette")
		acts+=("libft|$p")
	done
	for p in ${PRINTFS[@]+"${PRINTFS[@]}"}; do
		items+=("Tester mon ft_printf     $(court "$p")|Chaque conversion comparée au vrai printf, puis la trace moulinette")
		acts+=("printf|$p")
	done
	for p in ${GNLS[@]+"${GNLS[@]}"}; do
		items+=("Tester mon get_next_line $(court "$p")|Toutes les tailles de BUFFER_SIZE, les fuites, la Norme, les fichiers parasites")
		acts+=("gnl|$p")
	done
	items+=("Exam en C|Examen blanc de 3 h, série d'exercices ou entraînement : tout se règle aux flèches")
	acts+=("exam|")
	items+=("Banque d'exercices|Les exercices du testeur au choix : exam, Libft, printf")
	acts+=("banque|")
	items+=("Menu du testeur|Stats, chevalier, lore et paramètres, sans lancer de tests")
	acts+=("menu|")
	items+=("Tester un autre dossier|Le testeur te demande le chemin d'un projet qui n'est pas ici")
	acts+=("autre|")
	if ! tdf_cli_installee; then
		items+=("Installer la commande tdf|Ensuite tu tapes seulement « tdf » dans n'importe quel dossier (sans sudo)")
		acts+=("installer|")
	fi
	items+=("Quitter|")
	acts+=("quitter|")
	n=$(( ${#LIBFTS[@]} + ${#PRINTFS[@]} + ${#GNLS[@]} ))
	if [ "$n" = 0 ]; then
		sous="Aucun projet trouvé ici ni à côté du testeur. L'exam et la banque marchent sans projet."
	else
		sous="$n projet(s) trouvé(s) dans ce dossier ou à côté du testeur."
	fi
	tdf_menu_fleches "LE TESTEUR DES FOUS $TDF_VERSION" "$sous" "${items[@]}"
	[ "$CHOIX_MENU" -ge 0 ] || exit 0
	a="${acts[$CHOIX_MENU]%%|*}"
	p="${acts[$CHOIX_MENU]#*|}"
	case "$a" in
		libft|printf|gnl) MODULE="$a"; PROJET="$p" ;;
		exam) MODULE=exam; PROJET="" ;;
		banque) MODULE=exam; PROJET=""; SUITE+=(--banque) ;;
		menu)
			MODULE=libft
			PROJET="${LIBFTS[0]:-}"
			if [ -z "$PROJET" ] && [ ${#PRINTFS[@]} -gt 0 ]; then
				MODULE=printf
				PROJET="${PRINTFS[0]}"
			fi
			SUITE+=(--menu)
			;;
		autre)
			printf '\n  Chemin du projet (Tab complète) : '
			read -e -r p || exit 0
			case "$p" in "~"*) p="$HOME${p#\~}" ;; esac
			if [ -z "$p" ] || [ ! -d "$p" ]; then
				printf '  Dossier introuvable : %s\n\n' "$p"
				exit 2
			fi
			PROJET="$(cd "$p" && pwd -P)"
			if est_printf "$PROJET"; then MODULE=printf
			elif est_libft "$PROJET"; then MODULE=libft
			elif est_gnl "$PROJET"; then MODULE=gnl
			else
				printf '  Je ne reconnais ni une Libft, ni un ft_printf, ni un get_next_line dans %s\n\n' "$PROJET"
				exit 2
			fi
			;;
		installer)
			printf '\n'
			tdf_cli_installer
			printf '\n  Entrée pour revenir au menu '
			read -r p
			menu_depart
			;;
		*) exit 0 ;;
	esac
}

proteger_sauvegarde() {
	local jour copie n
	[ -d "$DOSSIER" ] || return 0
	chmod 700 "$DOSSIER" 2>/dev/null
	[ -f "$DOSSIER/save.txt" ] || return 0
	chmod 600 "$DOSSIER/save.txt" 2>/dev/null
	mkdir -p "$DOSSIER/copies" 2>/dev/null || return 0
	chmod 700 "$DOSSIER/copies" 2>/dev/null
	jour="$(date +%Y%m%d)"
	copie="$DOSSIER/copies/save_$jour.txt"
	if [ ! -f "$copie" ]; then
		cp "$DOSSIER/save.txt" "$copie" 2>/dev/null && chmod 600 "$copie" 2>/dev/null
	fi
	n=0
	for copie in $(ls -1 "$DOSSIER/copies" 2>/dev/null | grep '^save_[0-9]*\.txt$' | sort -r); do
		n=$((n + 1))
		[ "$n" -gt 7 ] && rm -f "$DOSSIER/copies/$copie"
	done
	return 0
}

ecrire_anim() {
	local save="$DOSSIER/save.txt"
	[ -f "$save" ] || return 1
	if grep -q '^cfg_anim=' "$save"; then
		sed "s/^cfg_anim=.*/cfg_anim=$1/" "$save" > "$save.anim" && cat "$save.anim" > "$save"
		rm -f "$save.anim"
	else
		printf 'cfg_anim=%s\n' "$1" >> "$save"
	fi
	return 0
}

appliquer_anim() {
	[ -f "$DOSSIER/anim_off" ] || return 0
	if ecrire_anim 2; then
		rm -f "$DOSSIER/anim_off"
	else
		SUITE+=(--fast)
	fi
}

regler_anim() {
	mkdir -p "$DOSSIER" 2>/dev/null
	case "$1" in
		off|non|0)
			ecrire_anim 2 || : > "$DOSSIER/anim_off"
			echo "animations : coupées (--anim on pour les remettre)"
			;;
		on|oui|1)
			rm -f "$DOSSIER/anim_off"
			ecrire_anim 0
			echo "animations : activées"
			;;
		*)
			echo "--anim : on ou off"
			exit 2
			;;
	esac
	exit 0
}

MODULE=""
PROJET=""
SUITE=()
SANS_ANIM=0
MENU=0

if [ "${1:-}" = debug ]; then
	shift
	debug_commande "$@"
	exit $?
fi
case "${1:-}" in
	installer|install) tdf_cli_installer; exit $? ;;
	desinstaller|uninstall) tdf_cli_desinstaller; exit $? ;;
	maj|update) tdf_cli_maj; exit $? ;;
esac

while [ $# -gt 0 ]; do
	a="$1"
	shift
	case "$a" in
		--menu) MENU=1 ;;
		banque)
			if [ -z "$MODULE" ]; then MODULE=exam; SUITE+=(--banque); else SUITE+=("$a"); fi
			;;
		libft|printf|exam|gnl)
			if [ -z "$MODULE" ]; then MODULE="$a"; else SUITE+=("$a"); fi
			;;
		--version|-V)
			echo "Le testeur des fous $TDF_VERSION ($TDF_DATE)"
			exit 0
			;;
		--help|-h)
			if [ -z "$MODULE" ]; then usage; exit 0; fi
			SUITE+=("$a")
			;;
		--projet|--dir)
			[ $# -gt 0 ] || { echo "$a : il manque le chemin du projet"; exit 2; }
			[ -d "$1" ] || { echo "$a : dossier introuvable : $1"; exit 2; }
			PROJET="$(cd "$1" && pwd -P)"
			shift
			;;
		--anim)
			[ $# -gt 0 ] || { echo "--anim : on ou off"; exit 2; }
			regler_anim "$1"
			;;
		-a|--fast|--no-anim) SUITE+=(--fast); SANS_ANIM=1 ;;
		--no-lord) SUITE+=(--no-lore) ;;
		*) SUITE+=("$a") ;;
	esac
done

if [ "$MENU" = 1 ] && [ "$MODULE" != exam ]; then
	detecter
	if [ -z "$MODULE" ]; then
		MODULE="libft"
		[ ${#PRINTFS[@]} -gt 0 ] && [ ${#LIBFTS[@]} -eq 0 ] && MODULE="printf"
	fi
	if [ -z "$PROJET" ]; then
		[ "$MODULE" = libft ] && [ ${#LIBFTS[@]} -gt 0 ] && PROJET="${LIBFTS[0]}"
		[ "$MODULE" = printf ] && [ ${#PRINTFS[@]} -gt 0 ] && PROJET="${PRINTFS[0]}"
	fi
	SUITE+=(--menu)
elif [ "$MODULE" = exam ]; then
	PROJET=""
	detecter
elif [ -n "$PROJET" ]; then
	if est_printf "$PROJET"; then
		[ -n "$MODULE" ] || MODULE="printf"
	elif est_libft "$PROJET"; then
		[ -n "$MODULE" ] || MODULE="libft"
	elif est_gnl "$PROJET"; then
		[ -n "$MODULE" ] || MODULE="gnl"
	elif [ -z "$MODULE" ]; then
		echo "Je ne reconnais ni une Libft ni un ft_printf dans : $PROJET"
		echo "Précise le module : bash $TDF_NOM/run.sh libft --projet $PROJET"
		exit 1
	fi
elif [ -z "$MODULE" ] && [ -t 0 ] && [ -t 1 ]; then
	detecter
	menu_depart
else
	detecter
	if [ -z "$MODULE" ]; then
		if [ ${#LIBFTS[@]} -gt 0 ] && [ ${#PRINTFS[@]} -eq 0 ] && [ ${#GNLS[@]} -eq 0 ]; then
			MODULE="libft"
		elif [ ${#PRINTFS[@]} -gt 0 ] && [ ${#LIBFTS[@]} -eq 0 ] && [ ${#GNLS[@]} -eq 0 ]; then
			MODULE="printf"
		elif [ ${#GNLS[@]} -gt 0 ] && [ ${#LIBFTS[@]} -eq 0 ] && [ ${#PRINTFS[@]} -eq 0 ]; then
			MODULE="gnl"
		fi
	fi
	CHOIX=()
	NOMS=()
	if [ -z "$MODULE" ] || [ "$MODULE" = libft ]; then
		for p in ${LIBFTS[@]+"${LIBFTS[@]}"}; do CHOIX+=("$p"); NOMS+=("libft"); done
	fi
	if [ -z "$MODULE" ] || [ "$MODULE" = printf ]; then
		for p in ${PRINTFS[@]+"${PRINTFS[@]}"}; do CHOIX+=("$p"); NOMS+=("printf"); done
	fi
	if [ -z "$MODULE" ] || [ "$MODULE" = gnl ]; then
		for p in ${GNLS[@]+"${GNLS[@]}"}; do CHOIX+=("$p"); NOMS+=("gnl"); done
	fi
	if [ -z "$MODULE" ] && [ ${#CHOIX[@]} -ne 1 ]; then
		CHOIX+=("" "" "")
		NOMS+=("exam" "banque" "menu")
	fi
	if [ ${#CHOIX[@]} -eq 0 ] || { [ ${#CHOIX[@]} -eq 3 ] && [ "${NOMS[0]}" = exam ] && ! { [ -t 0 ] && [ -t 1 ]; }; }; then
		cat <<EOF
Le testeur des fous $TDF_VERSION

Aucun projet trouvé. Le testeur se clone à côté de tes projets :

  ton_dossier/
  ├── $TDF_NOM/
  ├── libft/
  └── ft_printf/

Ou donne le chemin : bash $TDF_NOM/run.sh --projet CHEMIN

L'exam en C, lui, n'a besoin d'aucun projet : bash $TDF_NOM/run.sh exam
Le menu (stats, chevalier...) non plus      : bash $TDF_NOM/run.sh --menu
EOF
		exit 1
	elif [ ${#CHOIX[@]} -eq 1 ] && [ "${NOMS[0]}" != exam ]; then
		PROJET="${CHOIX[0]}"
		MODULE="${NOMS[0]}"
	elif [ -t 0 ] && [ -t 1 ]; then
		printf '\n  Le testeur des fous %s\n\n' "$TDF_VERSION"
		[ "${NOMS[0]}" = exam ] && printf '  Aucun projet trouvé à côté du testeur (libft/, ft_printf/...).\n\n'
		i=0
		while [ $i -lt ${#CHOIX[@]} ]; do
			case "${NOMS[$i]}" in
				libft) etiquette="Libft" ;;
				exam) etiquette="Exam en C" ;;
				banque) etiquette="Banque d'exercices : exam, Libft, printf" ;;
				gnl) etiquette="GNL" ;;
				menu) etiquette="Menu seul : stats, chevalier, lore (sans tester)" ;;
				*) etiquette="ft_printf" ;;
			esac
			if [ -n "${CHOIX[$i]}" ]; then
				printf '  %d  %-10s %s\n' $((i + 1)) "$etiquette" "$(court "${CHOIX[$i]}")"
			else
				printf '  %d  %s\n' $((i + 1)) "$etiquette"
			fi
			i=$((i + 1))
		done
		printf '  q  quitter\n\n  > '
		read -r rep || exit 0
		case "$rep" in
			q|Q|'') exit 0 ;;
			*[!0-9]*) echo "choix inconnu : $rep"; exit 2 ;;
		esac
		if [ "$rep" -lt 1 ] || [ "$rep" -gt ${#CHOIX[@]} ]; then
			echo "choix inconnu : $rep"
			exit 2
		fi
		PROJET="${CHOIX[$((rep - 1))]}"
		MODULE="${NOMS[$((rep - 1))]}"
		if [ "$MODULE" = banque ]; then
			MODULE="exam"
			SUITE+=(--banque)
		fi
		if [ "$MODULE" = menu ]; then
			MODULE="libft"
			PROJET="${LIBFTS[0]:-}"
			if [ -z "$PROJET" ] && [ ${#PRINTFS[@]} -gt 0 ]; then
				MODULE="printf"
				PROJET="${PRINTFS[0]}"
			fi
			SUITE+=(--menu)
		fi
	else
		echo "Plusieurs projets trouvés, précise lequel :"
		i=0
		while [ $i -lt ${#CHOIX[@]} ]; do
			if [ -n "${CHOIX[$i]}" ]; then
				printf '  bash %s/run.sh %s --projet %s\n' "$TDF_NOM" "${NOMS[$i]}" "${CHOIX[$i]}"
			elif [ "${NOMS[$i]}" = exam ] || [ "${NOMS[$i]}" = banque ]; then
				printf '  bash %s/run.sh %s\n' "$TDF_NOM" "${NOMS[$i]}"
			else
				printf '  bash %s/run.sh --menu\n' "$TDF_NOM"
			fi
			i=$((i + 1))
		done
		exit 2
	fi
fi

if [ ! -f "$TDF_RACINE/modules/$MODULE/run.sh" ]; then
	echo "testeur des fous : module $MODULE introuvable, le dossier est incomplet." >&2
	exit 1
fi

proteger_sauvegarde
appliquer_anim

[ ${#LIBFTS[@]} -eq 0 ] && [ ${#PRINTFS[@]} -eq 0 ] && [ ${#GNLS[@]} -eq 0 ] && detecter
TDF_DISPO="$MODULE exam"
RETOUR_MODULE=""
RETOUR_PROJET=""
[ ${#LIBFTS[@]} -gt 0 ] && TDF_DISPO="$TDF_DISPO libft"
[ ${#PRINTFS[@]} -gt 0 ] && TDF_DISPO="$TDF_DISPO printf"
[ ${#GNLS[@]} -gt 0 ] && TDF_DISPO="$TDF_DISPO gnl"
export TDF_DISPO
export TDF_NOM
export TDF_RACINE
export TDF_LIBFT="${LIBFTS[0]:-}"
[ -n "$TDF_LIBFT" ] || unset TDF_LIBFT

debug_ouvrir ${ARGS_ORIG[@]+"${ARGS_ORIG[@]}"}
TDF_JOURNAL_MOD="lanceur"
tdf_journal "projets trouvés : libft ${#LIBFTS[@]}, ft_printf ${#PRINTFS[@]}, gnl ${#GNLS[@]} · module $MODULE · projet ${PROJET:-aucun}"
trap 'tdf_journal "interrompu (Ctrl+C)"; debug_fermer "interrompu"; exit 130' INT
trap 'debug_fermer "arrêté"; exit 143' TERM HUP

while :; do
	rm -f "$DOSSIER/suivant"
	export TDF_PROJET="$PROJET"
	tdf_journal "lancement du module $MODULE ${SUITE[*]:-}"
	tdf_chrono_debut mod "module $MODULE"
	bash "$TDF_RACINE/modules/$MODULE/run.sh" ${SUITE[@]+"${SUITE[@]}"}
	code=$?
	tdf_chrono_fin mod "module $MODULE" "code $code"
	if [ "$code" != 0 ] && [ "$code" != 1 ] && [ "$code" != 45 ] && [ "$code" != 130 ]; then
		tdf_journal "ERREUR : le module $MODULE s'est arrêté avec le code $code"
	fi
	if { [ "$MODULE" = exam ] || [ "$MODULE" = gnl ]; } && [ -n "$RETOUR_MODULE" ]; then
		MODULE="$RETOUR_MODULE"
		PROJET="$RETOUR_PROJET"
		RETOUR_MODULE=""
		unset TDF_RETOUR
		SUITE=()
		[ "$SANS_ANIM" = 1 ] && SUITE+=(--fast)
		export TDF_NO_UPDATE=1
		continue
	fi
	if [ "$code" != 45 ]; then
		debug_fermer "code $code"
		exit "$code"
	fi
	suivant="$(head -1 "$DOSSIER/suivant" 2>/dev/null)"
	rm -f "$DOSSIER/suivant"
	tdf_journal "changement de module : $MODULE vers ${suivant:-?}"
	case "$suivant" in
		libft) [ ${#LIBFTS[@]} -gt 0 ] || { debug_fermer "code 0"; exit 0; }; PROJET="${LIBFTS[0]}" ;;
		printf) [ ${#PRINTFS[@]} -gt 0 ] || { debug_fermer "code 0"; exit 0; }; PROJET="${PRINTFS[0]}" ;;
		exam|banque) RETOUR_MODULE="$MODULE"; RETOUR_PROJET="$PROJET"; PROJET="" ;;
		gnl) [ ${#GNLS[@]} -gt 0 ] || { debug_fermer "code 0"; exit 0; }
			RETOUR_MODULE="$MODULE"; RETOUR_PROJET="$PROJET"; PROJET="${GNLS[0]}"; export TDF_RETOUR=1 ;;
		*) debug_fermer "code 0"; exit 0 ;;
	esac
	MODULE="$suivant"
	SUITE=()
	if [ "$MODULE" = banque ]; then
		MODULE="exam"
		SUITE+=(--banque)
	fi
	[ "$SANS_ANIM" = 1 ] && SUITE+=(--fast)
	export TDF_NO_UPDATE=1
done
