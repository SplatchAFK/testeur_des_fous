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

lire_version() {
	sed -n "s/^# *define $1 \"\(.*\)\"/\1/p" "$VERSION_H" | head -1
}

TDF_VERSION="$(lire_version TDF_VERSION)"
TDF_DATE="$(lire_version TDF_DATE)"

usage() {
	cat <<EOF
Le testeur des fous $TDF_VERSION
Créé et vibe-codé par Splatch (romasant)

  bash $TDF_NOM/run.sh                  détecter tes projets et lancer le testeur
  bash $TDF_NOM/run.sh libft            tester ta Libft
  bash $TDF_NOM/run.sh printf           tester ton ft_printf
  bash $TDF_NOM/run.sh --projet CHEMIN  tester le projet de ce dossier
  bash $TDF_NOM/run.sh --no-lore        les tests seuls, sans histoire
  bash $TDF_NOM/run.sh -a               sans animations pour ce lancement
  bash $TDF_NOM/run.sh --anim off       couper les animations pour de bon (on pour les remettre)
  bash $TDF_NOM/run.sh --debug          tous les rapports détaillés
  bash $TDF_NOM/run.sh --version        le numéro de version

Les options d'un module : bash $TDF_NOM/run.sh libft --help   ou   printf --help
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

LIBFTS=()
PRINTFS=()
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

regler_anim() {
	mkdir -p "$DOSSIER" 2>/dev/null
	case "$1" in
		off|non|0)
			: > "$DOSSIER/anim_off"
			echo "animations : coupées (--anim on pour les remettre)"
			;;
		on|oui|1)
			rm -f "$DOSSIER/anim_off"
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

while [ $# -gt 0 ]; do
	a="$1"
	shift
	case "$a" in
		libft|printf)
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
		-a) SUITE+=(--fast) ;;
		--no-lord) SUITE+=(--no-lore) ;;
		*) SUITE+=("$a") ;;
	esac
done

if [ -n "$PROJET" ]; then
	if est_printf "$PROJET"; then
		[ -n "$MODULE" ] || MODULE="printf"
	elif est_libft "$PROJET"; then
		[ -n "$MODULE" ] || MODULE="libft"
	elif [ -z "$MODULE" ]; then
		echo "Je ne reconnais ni une Libft ni un ft_printf dans : $PROJET"
		echo "Précise le module : bash $TDF_NOM/run.sh libft --projet $PROJET"
		exit 1
	fi
else
	detecter
	if [ -z "$MODULE" ]; then
		if [ ${#LIBFTS[@]} -gt 0 ] && [ ${#PRINTFS[@]} -eq 0 ]; then
			MODULE="libft"
		elif [ ${#PRINTFS[@]} -gt 0 ] && [ ${#LIBFTS[@]} -eq 0 ]; then
			MODULE="printf"
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
	if [ ${#CHOIX[@]} -eq 0 ]; then
		cat <<EOF
Le testeur des fous $TDF_VERSION

Aucun projet trouvé. Le testeur se clone à côté de tes projets :

  ton_dossier/
  ├── $TDF_NOM/
  ├── libft/
  └── ft_printf/

Ou donne le chemin : bash $TDF_NOM/run.sh --projet CHEMIN
EOF
		exit 1
	elif [ ${#CHOIX[@]} -eq 1 ]; then
		PROJET="${CHOIX[0]}"
		MODULE="${NOMS[0]}"
	elif [ -t 0 ] && [ -t 1 ]; then
		printf '\n  Le testeur des fous %s\n\n' "$TDF_VERSION"
		i=0
		while [ $i -lt ${#CHOIX[@]} ]; do
			case "${NOMS[$i]}" in
				libft) etiquette="Libft" ;;
				*) etiquette="ft_printf" ;;
			esac
			printf '  %d  %-10s %s\n' $((i + 1)) "$etiquette" "$(court "${CHOIX[$i]}")"
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
	else
		echo "Plusieurs projets trouvés, précise lequel :"
		i=0
		while [ $i -lt ${#CHOIX[@]} ]; do
			printf '  bash %s/run.sh %s --projet %s\n' "$TDF_NOM" "${NOMS[$i]}" "${CHOIX[$i]}"
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

[ -f "$DOSSIER/anim_off" ] && SUITE+=(--fast)

export TDF_PROJET="$PROJET"
export TDF_NOM
exec bash "$TDF_RACINE/modules/$MODULE/run.sh" ${SUITE[@]+"${SUITE[@]}"}
