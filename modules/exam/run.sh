#!/usr/bin/env bash
set -u

ICI="$(cd "$(dirname "$0")" && pwd)"
RACINE="$(cd "$ICI/../.." && pwd)"
MOT="$RACINE/moteur"
BUILD="$RACINE/exam/.build"
DOSSIER="$HOME/.testeur_des_fous"
CC="${CC:-cc}"
TDF_JOURNAL_MOD="exam"
. "$MOT/chargement.sh"

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

SOURCES=("$ICI/src/exam.c" "$ICI/src/code.c" "$MOT/save.c" "$MOT/data.c" "$MOT/art.c" "$MOT/stats.c" "$MOT/entete.c" "$MOT/fou_ascii.c")
EMPREINTE="$(cat "${SOURCES[@]}" "$MOT"/*.h "$ICI"/src/*.h 2>/dev/null | cksum | cut -d' ' -f1)"
if ! tdf_compiler "$BUILD/exam" "$BUILD/exam.err" "$EMPREINTE" -O2 -w -DTDF_EXAM -DTDF_STATS_SEUL -I "$MOT" -I "$ICI/src" -- \
	"${SOURCES[@]}" -- -lm; then
	echo "exam : le moteur ne compile pas sur cette machine."
	head -5 "$BUILD/exam.err"
	[ -n "${TDF_JOURNAL:-}" ] && cp "$BUILD/exam.err" "$(dirname "$TDF_JOURNAL")/exam_compilation.err" 2>/dev/null
	exit 1
fi

tdf_journal "examshell ouvert"
if [ -t 0 ] && [ -t 1 ] && [ "${TDF_INTERFACE:-graphique}" != texte ]; then
	tdf_scene_bin "$MOT" >/dev/null 2>&1 || true
	export TDF_SCENE_DIR="$ICI"
fi
"$BUILD/exam" --racine "$RACINE" --home "$DOSSIER" ${SUITE[@]+"${SUITE[@]}"}
code=$?
tdf_journal "examshell fermé (code $code)"
[ -n "${TDF_JOURNAL:-}" ] && [ -f "$RACINE/exam/etat" ] && cp "$RACINE/exam/etat" "$(dirname "$TDF_JOURNAL")/exam_etat.txt" 2>/dev/null
[ -n "${TDF_JOURNAL:-}" ] && ls -1 "$RACINE"/exam/traces/*.txt >/dev/null 2>&1 && cat "$RACINE"/exam/traces/*.txt > "$(dirname "$TDF_JOURNAL")/exam_traces.txt" 2>/dev/null
exit $code
