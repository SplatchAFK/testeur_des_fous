#!/usr/bin/env bash
set -u

ICI="$(cd "$(dirname "$0")" && pwd)"
RACINE="$(cd "$ICI/../.." && pwd)"
MOT="$RACINE/moteur"
BUILD="$RACINE/exam/.build"
DOSSIER="$HOME/.testeur_des_fous"
CC="${CC:-cc}"

usage() {
	cat <<EOF
Le testeur des fous · Exam en C

  bash ${TDF_NOM:-testeur_des_fous}/run.sh exam            régler puis commencer une session
  bash ${TDF_NOM:-testeur_des_fous}/run.sh exam --liste    la liste des exercices

Pendant la session : grademe, status, sujet, finish, help.
Tu écris ton code dans ${TDF_NOM:-testeur_des_fous}/exam/rendu/<exercice>/.
EOF
}

SUITE=()
while [ $# -gt 0 ]; do
	case "$1" in
		-h|--help) usage; exit 0 ;;
		--fast|--no-anim|--no-lore|--lore-off|--no-update) ;;
		*) SUITE+=("$1") ;;
	esac
	shift
done

for outil in "$CC" nm; do
	if ! command -v "$outil" >/dev/null 2>&1; then
		echo "exam : il manque $outil sur cette machine."
		exit 1
	fi
done

mkdir -p "$BUILD" "$DOSSIER" 2>/dev/null
chmod 700 "$DOSSIER" 2>/dev/null

SOURCES=("$ICI/src/exam.c" "$MOT/save.c" "$MOT/data.c" "$MOT/art.c" "$MOT/stats.c")
EMPREINTE="$(cat "${SOURCES[@]}" "$MOT"/*.h 2>/dev/null | cksum | cut -d' ' -f1)"
if [ ! -x "$BUILD/exam" ] || [ "$(cat "$BUILD/empreinte" 2>/dev/null)" != "$EMPREINTE" ]; then
	if ! "$CC" -O2 -w -DTDF_EXAM -DTDF_STATS_SEUL -I "$MOT" "${SOURCES[@]}" -lm -o "$BUILD/exam" 2> "$BUILD/exam.err"; then
		echo "exam : le moteur ne compile pas sur cette machine."
		head -5 "$BUILD/exam.err"
		exit 1
	fi
	printf '%s\n' "$EMPREINTE" > "$BUILD/empreinte"
fi

exec "$BUILD/exam" --racine "$RACINE" --home "$DOSSIER" ${SUITE[@]+"${SUITE[@]}"}
