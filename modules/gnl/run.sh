#!/usr/bin/env bash
set -u

ICI="$(cd "$(dirname "$0")" && pwd)"
export TDF_RACINE="${TDF_RACINE:-$(cd "$ICI/../.." && pwd)}"
RACINE="$(cd "$ICI/../.." && pwd)"
MOT="$RACINE/moteur"
TDF_JOURNAL_MOD="gnl"
. "$MOT/chargement.sh"
. "$MOT/conformite.sh"
CC="${CC:-cc}"
OS="$(uname -s 2>/dev/null)"
PROJET="${TDF_PROJET:-}"
NOM="${TDF_NOM:-testeur_des_fous}"
VERSION="$(sed -n 's/^# *define TDF_VERSION "\(.*\)"/\1/p' "$MOT/version.h" | head -1)"
TAILLES="1 2 7 42 1024 9999 10000000"
VERBEUX=0
GARDER=0

usage() {
	cat <<EOF
Le testeur des fous · get_next_line

  bash $NOM/run.sh gnl                      tester ton get_next_line
  bash $NOM/run.sh gnl --projet CHEMIN      tester le get_next_line de ce dossier
  bash $NOM/run.sh gnl --tailles "1 42"     seulement ces BUFFER_SIZE
  bash $NOM/run.sh gnl -v                   tous les détails
EOF
}

while [ $# -gt 0 ]; do
	case "$1" in
		-h|--help) usage; exit 0 ;;
		-v|--debug) VERBEUX=1 ;;
		-k) GARDER=1 ;;
		--tailles) shift; TAILLES="${1:-$TAILLES}" ;;
		--projet|--dir) shift; PROJET="$(cd "${1:-.}" && pwd)" ;;
		--no-lore|--lore-off|--ci) SANS_LORE=1 ;;
		--fast|--no-anim|--no-update|--menu) ;;
		*) echo "option inconnue : $1 (voir --help)"; exit 2 ;;
	esac
	shift
done
[ -n "$PROJET" ] || PROJET="$(cd "$RACINE/.." && pwd)"

if [ -t 1 ] && [ -z "${NO_COLOR:-}" ]; then
	R=$'\033[0m'; B=$'\033[1m'; D=$'\033[2m'; AC=$'\033[38;5;213m'
	P_OK=$'\033[30;48;5;78m'; P_KO=$'\033[30;48;5;203m'; P_CR=$'\033[97;48;5;125m'; P_WN=$'\033[30;48;5;221m'
	P_TM=$'\033[30;48;5;141m'; P_LK=$'\033[30;48;5;215m'; F_OK=$'\033[38;5;78m'; F_KO=$'\033[38;5;203m'; F_WN=$'\033[38;5;221m'
else
	R=; B=; D=; AC=; P_OK=; P_KO=; P_CR=; P_WN=; P_TM=; P_LK=; F_OK=; F_KO=; F_WN=
fi

BUILD="$(mktemp -d "${TMPDIR:-/tmp}/tdf_gnl.XXXXXX")"
[ -n "$BUILD" ] && [ -d "$BUILD" ] || { echo "testeur des fous : impossible de créer un dossier temporaire" >&2; exit 1; }
trap '[ "$GARDER" = 1 ] || rm -rf "$BUILD"' EXIT
mkdir -p "$BUILD/fichiers"
N_OK=0
N_KO=0
N_INFO=0
ECHECS=()

pill() {
	case "$1" in
		OK) printf '%s %-6s %s' "$P_OK" OK "$R" ;;
		KO) printf '%s %-6s %s' "$P_KO" KO "$R" ;;
		CRASH) printf '%s %-6s %s' "$P_CR" CRASH "$R" ;;
		BOUCLE) printf '%s %-6s %s' "$P_TM" BOUCLE "$R" ;;
		FUITE) printf '%s %-6s %s' "$P_LK" FUITE "$R" ;;
		*) printf '%s %-6s %s' "$P_WN" "$1" "$R" ;;
	esac
}

ligne() {
	local st="$1" titre="$2" detail="${3:-}"
	printf '  '
	pill "$st"
	printf '  %s\n' "$titre"
	[ -n "$detail" ] && printf '            %s%s%s\n' "$D" "$detail" "$R"
	case "$st" in
		OK) N_OK=$((N_OK + 1)) ;;
		INFO|NOTE) N_INFO=$((N_INFO + 1)) ;;
		*) N_KO=$((N_KO + 1)); ECHECS+=("$titre|$st") ;;
	esac
}

section() {
	printf '\n  %s%s%s %s' "$AC" "$B" "$1" "$D"
	printf '%.0s─' $(seq 1 $((56 - ${#1})))
	printf '%s\n' "$R"
}

printf '\n  %s╭──────────────────────────────────────────────────────────╮%s\n' "$AC" "$R"
printf '  %s│%s  %sLE TESTEUR DES FOUS%s · get_next_line · v%-17s%s│%s\n' "$AC" "$R" "$B" "$R" "$VERSION" "$AC" "$R"
printf '  %s╰──────────────────────────────────────────────────────────╯%s\n' "$AC" "$R"
printf '  %sprojet : %s%s\n' "$D" "$PROJET" "$R"
tdf_journal "projet $PROJET · tailles $TAILLES"

section "Rendu · ce que le sujet demande"
MANQUE=0
for f in get_next_line.c get_next_line_utils.c get_next_line.h; do
	if [ -f "$PROJET/$f" ]; then
		ligne OK "$f à la racine"
	else
		ligne KO "$f à la racine" "fichier introuvable : le sujet demande les trois fichiers à la racine du dépôt"
		MANQUE=1
	fi
done
if [ "$MANQUE" = 1 ]; then
	printf '\n  %s%sIl manque des fichiers : les tests ne peuvent pas compiler.%s\n\n' "$F_KO" "$B" "$R"
	tdf_journal "fichiers manquants"
	exit 1
fi
if grep -Eq 'char[[:space:]]*\*[[:space:]]*get_next_line[[:space:]]*\([[:space:]]*int' "$PROJET/get_next_line.h"; then
	ligne OK "prototype de get_next_line dans get_next_line.h"
else
	ligne KO "prototype de get_next_line dans get_next_line.h" "attendu : char *get_next_line(int fd);"
fi

SRC=("$PROJET/get_next_line.c" "$PROJET/get_next_line_utils.c")
if "$CC" -Wall -Wextra -Werror -I "$PROJET" -c "${SRC[0]}" -o "$BUILD/a.o" 2> "$BUILD/sans_d.err" \
	&& "$CC" -Wall -Wextra -Werror -I "$PROJET" -c "${SRC[1]}" -o "$BUILD/b.o" 2>> "$BUILD/sans_d.err"; then
	ligne OK "compile sans -D BUFFER_SIZE (cc -Wall -Wextra -Werror)"
else
	ligne KO "compile sans -D BUFFER_SIZE (cc -Wall -Wextra -Werror)" "$(grep -m1 -E 'error' "$BUILD/sans_d.err" | sed "s#$PROJET/##")"
	printf '\n  %s%sLe projet ne compile pas : corrige d'"'"'abord ça.%s\n\n' "$F_KO" "$B" "$R"
	exit 1
fi

if [ "$OS" = Darwin ]; then
	UND="$(nm -u "$BUILD/a.o" "$BUILD/b.o" 2>/dev/null | awk 'NF && $NF !~ /:$/ {print $NF}' | sed 's/^_//' | sort -u)"
	DEF="$(nm "$BUILD/a.o" "$BUILD/b.o" 2>/dev/null | awk 'NF == 3 {print $3}' | sed 's/^_//' | sort -u)"
else
	UND="$(nm -u "$BUILD/a.o" "$BUILD/b.o" 2>/dev/null | awk 'NF && $NF !~ /:$/ {print $NF}' | sort -u)"
	DEF="$(nm --defined-only "$BUILD/a.o" "$BUILD/b.o" 2>/dev/null | awk 'NF == 3 {print $3}' | sort -u)"
fi
INTERDIT=""
for s in $UND; do
	case "$s" in
		read|malloc|free|__stack_chk_fail|__stack_chk_guard|_GLOBAL_OFFSET_TABLE_|dyld_stub_binder) ;;
		*) echo "$DEF" | grep -qx "$s" || INTERDIT="$INTERDIT $s" ;;
	esac
done
if [ -z "$INTERDIT" ]; then
	ligne OK "fonctions externes : seulement read, malloc et free"
else
	case " $INTERDIT " in
		*" lseek "*) ligne KO "fonctions externes : seulement read, malloc et free" "interdit :$INTERDIT (lseek est interdit par le sujet)" ;;
		*) ligne KO "fonctions externes : seulement read, malloc et free" "interdit :$INTERDIT" ;;
	esac
fi
GLOB="$(nm "$BUILD/a.o" "$BUILD/b.o" 2>/dev/null | awk 'NF == 3 && $2 ~ /^[BDCGS]$/ {print $3}' | sort -u | tr '\n' ' ')"
if [ -z "$GLOB" ]; then
	ligne OK "aucune variable globale (les static sont permises)"
else
	ligne KO "aucune variable globale (les static sont permises)" "trouvé : $GLOB"
fi

NORM=""
norminette --version 2>/dev/null | grep -qi norminette && NORM="norminette"
[ -z "$NORM" ] && python3 -m norminette --version 2>/dev/null | grep -qi norminette && NORM="python3 -m norminette"
if [ -z "$NORM" ]; then
	ligne NOTE "norminette introuvable : la Norme n'est pas vérifiée" "python3 -m pip install --user norminette"
else
	nout="$(cd "$PROJET" && $NORM get_next_line.c get_next_line_utils.c get_next_line.h 2>&1 | sed $'s/\x1b\\[[0-9;]*m//g')"
	if echo "$nout" | grep -q 'Error'; then
		ligne KO "norminette" "$(echo "$nout" | grep -m2 -E 'Error' | tr '\t' ' ' | tr '\n' ' ' | cut -c1-120)"
	else
		ligne OK "norminette"
	fi
fi

RM="$PROJET/README.md"
if [ ! -f "$RM" ]; then
	ligne KO "README.md à la racine" "le sujet (chapitre 5) le demande"
else
	pb=""
	head -1 "$RM" | grep -q "This activity has been created as part of the 42 curriculum by" || pb="1re ligne en italique imposée"
	head -1 "$RM" | grep -Eq '^[*_].*[*_][[:space:]]*$' || pb="${pb:+$pb, }1re ligne pas en italique"
	for t in Description Instructions Resources; do
		grep -qiE "^#+[[:space:]]*$t" "$RM" || pb="${pb:+$pb, }section $t absente"
	done
	grep -qiE 'algorithm|algorithme' "$RM" || pb="${pb:+$pb, }explication de l'algorithme introuvable"
	if [ -z "$pb" ]; then
		ligne OK "README.md : 1re ligne, Description, Instructions, Resources, algorithme"
	else
		ligne KO "README.md : 1re ligne, Description, Instructions, Resources, algorithme" "$pb"
	fi
fi
PARAS="$(tdf_parasites gnl "$PROJET")"
if [ -z "$PARAS" ]; then
	ligne OK "aucun fichier parasite dans le rendu"
else
	while IFS=$'\t' read -r f m; do
		ligne KO "fichier parasite : $f (erreur moulinette)" "$m · à retirer : git rm $f"
	done <<< "$PARAS"
fi

SAN=""
probe='int main(void){return 0;}'
if echo "$probe" | "$CC" -x c - -fsanitize=address,undefined -o "$BUILD/probe" 2>/dev/null && "$BUILD/probe" 2>/dev/null; then
	SAN="-fsanitize=address,undefined -fno-omit-frame-pointer"
fi
WRAP=""
WFLAGS=""
if [ "$OS" != Darwin ]; then
	WRAP="-DTDF_WRAP"
	WFLAGS="-Wl,--wrap=malloc,--wrap=free,--wrap=read"
fi
[ -n "$SAN" ] || tdf_journal "sanitizers indisponibles : fuites non vérifiées"

LISTE=()
for t in $TAILLES; do LISTE+=("$t"); done
LISTE+=("sans")
tdf_ch_debut "compilation" $(( ${#LISTE[@]} ))
pids=()
for t in "${LISTE[@]}"; do
	(
		if [ "$t" = sans ]; then dflag=""; else dflag="-D BUFFER_SIZE=$t"; fi
		"$CC" -Wall -Wextra -Werror -g $SAN $dflag -I "$PROJET" -c "${SRC[0]}" -o "$BUILD/a_$t.o" 2> "$BUILD/err_$t" &&
		"$CC" -Wall -Wextra -Werror -g $SAN $dflag -I "$PROJET" -c "${SRC[1]}" -o "$BUILD/b_$t.o" 2>> "$BUILD/err_$t" &&
		"$CC" -w -g $SAN $dflag $WRAP "$ICI/src/tdf_gnl.c" "$BUILD/a_$t.o" "$BUILD/b_$t.o" $WFLAGS -o "$BUILD/t_$t" 2>> "$BUILD/err_$t"
	) &
	pids+=($!)
done
n=0
for p in "${pids[@]}"; do
	wait "$p"
	n=$((n + 1))
	tdf_ch_pas "BUFFER_SIZE $n/${#LISTE[@]}"
done
tdf_ch_fin

mkdir -p "$BUILD/san"
export TDF_SANLOG="$BUILD/san/log"
export ASAN_OPTIONS="detect_leaks=1:exitcode=77:abort_on_error=0:allocator_may_return_null=1:log_path=$TDF_SANLOG"
export LSAN_OPTIONS="exitcode=23:log_path=$TDF_SANLOG"
export UBSAN_OPTIONS="halt_on_error=1:exitcode=77:print_stacktrace=1:log_path=$TDF_SANLOG"
: > "$BUILD/res.tsv"
tdf_ch_debut "tests" "${#LISTE[@]}"
for t in "${LISTE[@]}"; do
	tdf_ch_pas "BUFFER_SIZE=$t"
	if [ ! -x "$BUILD/t_$t" ]; then
		printf 'C\t%s\t%s\n' "$t" "$(grep -m1 -E 'error' "$BUILD/err_$t" | sed "s#$PROJET/##" | tr '\t' ' ')" >> "$BUILD/res.tsv"
		continue
	fi
	rm -rf "$BUILD/fichiers"/*
	"$BUILD/t_$t" "$BUILD/fichiers" 2>/dev/null | sed "s/^/$t	/" >> "$BUILD/res.tsv"
done
tdf_ch_fin

nocomp="$(awk -F'\t' '$1 == "C" {print $2}' "$BUILD/res.tsv" | tr '\n' ' ')"
ANNEAUX=""
if [ -n "$nocomp" ]; then
	section "Compilation avec -D BUFFER_SIZE"
	awk -F'\t' '$1 == "C"' "$BUILD/res.tsv" | while IFS=$'\t' read -r _ t msg; do
		printf '  '; pill KO; printf '  BUFFER_SIZE=%s ne compile pas\n            %s%s%s\n' "$t" "$D" "$msg" "$R"
	done
	N_KO=$((N_KO + 1))
	ECHECS+=("compilation avec -D BUFFER_SIZE|KO")
fi

for cat in base taille fin fd lecture malloc info; do
	case "$cat" in
		base) titre="Lecture de base" ;;
		taille) titre="Tailles de lignes et de fichiers" ;;
		fin) titre="Fin de fichier" ;;
		fd) titre="Descripteurs : stdin, tube, erreurs" ;;
		lecture) titre="Lecture : read en échec, lire le moins possible" ;;
		malloc) titre="malloc qui échoue" ;;
		info) titre="Pour info (hors sujet v1.3)" ;;
	esac
	noms="$(awk -F'\t' -v c="$cat" '$2 == "T" && $3 == c {print $4}' "$BUILD/res.tsv" | awk '!vu[$0]++')"
	[ -n "$noms" ] || continue
	section "$titre"
	while IFS= read -r nom; do
		[ -n "$nom" ] || continue
		pire="OK"
		tailles_ko=""
		msg=""
		while IFS=$'\t' read -r t st m; do
			[ "$st" = OK ] && continue
			tailles_ko="${tailles_ko:+$tailles_ko, }$t"
			[ -z "$msg" ] && msg="$m"
			case "$pire:$st" in
				OK:*|INFO:*) pire="$st" ;;
				KO:CRASH|KO:BOUCLE|KO:FUITE) pire="$st" ;;
			esac
		done < <(awk -F'\t' -v c="$cat" -v n="$nom" '$2 == "T" && $3 == c && $4 == n {print $1 "\t" $5 "\t" $6}' "$BUILD/res.tsv")
		if [ "$pire" = OK ]; then
			ligne OK "$nom"
			[ "$cat" = info ] || ANNEAUX="${ANNEAUX}O"
		elif [ "$pire" = INFO ]; then
			ligne INFO "$nom" "$msg"
		else
			ligne "$pire" "$nom" "BUFFER_SIZE en échec : $tailles_ko · $msg"
			[ "$cat" = info ] || ANNEAUX="${ANNEAUX}K"
		fi
	done <<< "$noms"
done

section "BUFFER_SIZE=0 et négatif (le sujet ne dit rien : pour info)"
for t in 0 -5; do
	if "$CC" -w -g $SAN -D BUFFER_SIZE="$t" $WRAP -I "$PROJET" "$ICI/src/tdf_gnl.c" "${SRC[@]}" $WFLAGS -o "$BUILD/z" 2>/dev/null; then
		out="$("$BUILD/z" "$BUILD/fichiers" "fd" 2>/dev/null | cut -f4 | grep -cE 'CRASH|BOUCLE')"
		if [ "$out" = 0 ]; then
			ligne OK "BUFFER_SIZE=$t : ni plantage ni boucle"
		else
			ligne INFO "BUFFER_SIZE=$t : plantage ou boucle" "un correcteur peut le tester : renvoyer NULL est le plus sûr"
		fi
	else
		ligne INFO "BUFFER_SIZE=$t : ne compile pas" "un correcteur peut le tester"
	fi
done
[ -n "$SAN" ] || printf '\n  %s! AddressSanitizer indisponible ici : fuites et erreurs mémoire non vérifiées (valgrind conseillé).%s\n' "$F_WN" "$R"

printf '\n  %s' "$D"; printf '%.0s─' $(seq 1 58); printf '%s\n\n' "$R"
printf '  %s%d OK%s' "$F_OK$B" "$N_OK" "$R"
[ "$N_KO" -gt 0 ] && printf '   %s%d en échec%s' "$F_KO$B" "$N_KO" "$R"
[ "$N_INFO" -gt 0 ] && printf '   %s%d pour info%s' "$F_WN" "$N_INFO" "$R"
printf '   %s(BUFFER_SIZE testés : %s, et sans -D)%s\n' "$D" "$TAILLES" "$R"
if [ "$N_KO" = 0 ]; then
	printf '\n  %s%sTout est propre.%s\n\n' "$F_OK" "$B" "$R"
else
	printf '\n  %s%sCorrige les rouges d'"'"'abord.%s %s-v pour tout le détail.%s\n\n' "$F_KO" "$B" "$R" "$D" "$R"
fi

STATS="$HOME/.testeur_des_fous/stats.txt"
if mkdir -p "$HOME/.testeur_des_fous" 2>/dev/null; then
	now="$(date +%s)"
	for e in ${ECHECS[@]+"${ECHECS[@]}"}; do
		nom="${e%%|*}"
		printf 'K %s gnl %s %s\n' "$now" "$(printf '%s' "$nom" | tr ' \t' '__' | cut -c1-39)" "${e##*|}" >> "$STATS"
	done
	printf 'R %s gnl %d %d\n' "$now" "$N_OK" $((N_OK + N_KO)) >> "$STATS"
	chmod 600 "$STATS" 2>/dev/null
fi
tdf_journal "résultat : $N_OK OK, $N_KO en échec, $N_INFO pour info"
[ -n "${TDF_JOURNAL:-}" ] && cp "$BUILD/res.tsv" "$(dirname "$TDF_JOURNAL")/gnl_resultats.tsv" 2>/dev/null

VU_PUITS=0
if [ -t 0 ] && [ -t 1 ] && [ "${TDF_INTERFACE:-graphique}" != texte ] && [ "${SANS_LORE:-0}" = 0 ] && [ -n "$ANNEAUX" ]; then
	if tdf_scene_bin "$MOT"; then
		printf '  %s[ Entrée : descendre dans le Puits ]%s ' "$D" "$R"
		read -r _
		"$TDF_SCENE" "$ICI" puits "$ANNEAUX" && VU_PUITS=1
	fi
fi
if [ "$VU_PUITS" = 0 ] && [ -n "${TDF_RETOUR:-}" ] && [ -t 0 ] && [ -t 1 ]; then
	printf '  %s[ Entrée pour revenir au menu ]%s ' "$D" "$R"
	read -r _
fi
[ "$N_KO" = 0 ]
