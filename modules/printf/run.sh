#!/usr/bin/env bash
set -u

TDF_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT="${TDF_PROJET:-$(dirname "$TDF_DIR")}"
MOT="$(cd "$TDF_DIR/../../moteur" && pwd)"
TDF_AFF="${TDF_NOM:-$(basename "$TDF_DIR")}"
TDF_JOURNAL_MOD="printf"
. "$MOT/chargement.sh"
MENU_SEUL=0
MODE="asan+ubsan"
ONLY=""
SEED=""
BONUS_OPT=""
STRESS=""
TIMEOUT=""
VERBOSE=""
DO_NORM=1
AIDE=2
MACHINE=""
DEBUG=0
NOLORE=0
FAST=0
NO_UPDATE=0
UPDATE_SET=""
if [ -f "$HOME/.testeur_des_fous/reglages.txt" ]; then
	while IFS='=' read -r k v; do
		case "$k=$v" in AIDE=[012]) AIDE="$v" ;; esac
	done < "$HOME/.testeur_des_fous/reglages.txt"
fi

R=$'\033[1;31m'; G=$'\033[1;32m'; Y=$'\033[1;33m'; B=$'\033[1;34m'; C=$'\033[0;36m'
D=$'\033[2m'; N=$'\033[0m'

usage() {
	cat <<EOF
Le testeur des fous · version printf

usage : ./run.sh [options]
  --valgrind        valgrind (--leak-check=full --show-leak-kinds=all --track-origins=yes)
  --asan-only       ASan seul, sans UBSan (défaut : ASan + UBSan)
  --no-sanitize     aucun sanitizer (traceur malloc + gardes maison seulement)
                    (défaut : ASan + UBSan)
  --bonus           DLC : teste aussi les flags - 0 . # + espace et la largeur
                    (défaut : non, le sujet v1.1 n'a pas de bonus)
  --only CAT        une ou plusieurs catégories séparées par des virgules :
                    c s p d i u x X % mix ret limites write malloc stress bonus
                    makefile nm norme      ex : --only %d   --only s,p,malloc
  --seed N          graine du stress (pour rejouer un échec)
  --stress N        nombre d'appels aléatoires (défaut 10000)
  --timeout S       timeout par test en secondes (défaut 3, 10 sous valgrind)
  --dir CHEMIN      dossier du projet ft_printf (défaut : dossier parent)
  --no-norm         ne lance pas norminette
  -v                détaille tous les KO / avertissements
  --aide N          0 : le cas fautif seulement, 1 : + la ligne de ton code,
                    2 : + un conseil général (défaut : réglage du menu, sinon 2)
  --no-lore         les tests seuls, sans boss, sans animation, sans lore
                    (synonyme : --lore-off)
  --debug           debug avancé : tout le détail (fuites, mémoire, UB), puis un
                    second passage sous valgrind s'il est installé
  --fast            sans animation ni cinématique (synonyme : --no-anim)
  --no-update       ne pas chercher de mise à jour à ce lancement
  --update off|on   couper ou remettre la recherche de mise à jour

Dans le menu : Q, Échap ou Ctrl+C arrêtent un run ; Ctrl+C deux fois quitte tout.
EOF
	exit "${1:-0}"
}

SAVED_ARGS=("$@")
while [ $# -gt 0 ]; do
	case "$1" in
		--valgrind) MODE="valgrind" ;;
		--asan-only) MODE="asan" ;;
		--ubsan) MODE="asan+ubsan" ;;
		--no-sanitize) MODE="none" ;;
		--bonus) BONUS_OPT="--bonus" ;;
		--no-bonus) BONUS_OPT="--no-bonus" ;;
		--only) shift; ONLY="${1:-}" ;;
		--only=*) ONLY="${1#--only=}" ;;
		--seed) shift; SEED="${1:-}" ;;
		--stress) shift; STRESS="${1:-}" ;;
		--timeout) shift; TIMEOUT="${1:-}" ;;
		--dir) shift; PROJECT="$(cd "${1:-.}" && pwd)" ;;
		--no-norm) DO_NORM=0 ;;
		-v) VERBOSE="-v" ;;
		--aide) shift; AIDE="${1:-2}" ;;
		--machine) shift; MACHINE="${1:-}" ;;
		--no-lore|--lore-off) NOLORE=1 ;;
		--fast|--no-anim) FAST=1 ;;
		--no-update) NO_UPDATE=1 ;;
		--update) shift; UPDATE_SET="${1:-}" ;;
		--debug) DEBUG=1 ;;
		--menu) MENU_SEUL=1 ;;
		--exo) shift; EXO="${1:-}" ;;
		--out) shift; EXO_OUT="${1:-}" ;;
		--reco) shift; RECO="${1:-}" ;;
		--version|-V) sed -n 's/^# *define TDF_VERSION "\(.*\)"/Le testeur des fous printf v\1/p' "$MOT/version.h"; exit 0 ;;
		-h|--help) usage 0 ;;
		*) echo "option inconnue : $1"; usage 2 ;;
	esac
	shift
done

case "$AIDE" in 0|1|2) ;; *) echo "--aide : 0, 1 ou 2"; exit 2 ;; esac
[ -z "$SEED" ] && SEED=$(( (RANDOM << 15 | RANDOM) ))
JEU="$TDF_DIR/.build_jeu"

if [ -n "${EXO:-}" ]; then
	out="${EXO_OUT:-/dev/stdout}"
	[ "$out" = /dev/stdout ] || : > "$out"
	n=$((10#$EXO))
	fn=$(sed -n "/^const t_pexo/,/^};/p" "$TDF_DIR/game/pf_data.c" | grep -o '{"ft_[a-z_]*"' | sed -n "${n}p" | tr -d '{"')
	file="$TDF_DIR/entrainement/niveau_$EXO/$fn.c"
	mkdir -p "$JEU"
	if [ -z "$fn" ] || [ ! -f "$file" ]; then
		printf 'X\tabsent\t0\nD\tfichier introuvable : %s/entrainement/niveau_%s/%s.c\n' "$TDF_AFF" "$EXO" "$fn" >> "$out"
		exit 1
	fi
	if command -v norminette >/dev/null 2>&1; then
		if ! norminette "$file" 2>&1 | grep -q ': OK!'; then
			printf 'X\tnorme\t0\n' >> "$out"
			norminette "$file" 2>&1 | grep -E 'Error' | head -4 | sed 's/\x1b\[[0-9;]*m//g' | sed 's/^/D\t/' >> "$out"
			exit 1
		fi
	fi
	if ! cc -Wall -Wextra -Werror -g -fsanitize=address,undefined -DLEVEL="$n" -I "$TDF_DIR/game" -I "$MOT" \
		"$TDF_DIR/game/exos/exos_pf.c" "$file" -o "$JEU/exo_$EXO" 2> "$JEU/exo_$EXO.err"; then
		printf 'X\tbuild\t0\n' >> "$out"
		grep -E 'error|warning' "$JEU/exo_$EXO.err" | head -4 | sed "s#$TDF_DIR/##" | sed 's/^/D\t/' >> "$out"
		exit 1
	fi
	ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 timeout 10 "$JEU/exo_$EXO" >> "$out" 2> "$JEU/exo_$EXO.san"
	rc=$?
	if [ $rc -ne 0 ] && ! grep -q '^X' "$out"; then
		printf 'X\tcrash\t0\n' >> "$out"
		grep -m2 -E 'ERROR|runtime error|SUMMARY' "$JEU/exo_$EXO.san" | sed "s#$TDF_DIR/##" | sed 's/^/D\t/' >> "$out"
	elif [ $rc -ne 0 ] && grep -q '^X	ok' "$out"; then
		sed -i 's/^X\tok/X\tfuite/' "$out"
		grep -m2 -E 'ERROR|SUMMARY' "$JEU/exo_$EXO.san" | sed "s#$TDF_DIR/##" | sed 's/^/D\t/' >> "$out"
	fi
	grep -q '^X	ok' "$out"
	exit $?
fi

kill_tree() {
	local p="$1" k
	[ -n "$p" ] || return 0
	kill -STOP "$p" 2>/dev/null || return 0
	if command -v pgrep >/dev/null 2>&1; then
		for k in $(pgrep -P "$p" 2>/dev/null); do kill_tree "$k"; done
	fi
	kill -TERM "$p" 2>/dev/null
	kill -CONT "$p" 2>/dev/null
}
build_jeu() {
	local empreinte
	mkdir -p "$JEU"
	empreinte=$(cat "$TDF_DIR"/game/*.c "$TDF_DIR"/game/*.h "$MOT"/*.c "$MOT"/*.h 2>/dev/null | cksum | cut -d' ' -f1)
	tdf_compiler "$JEU/fous" "$JEU/jeu.err" "$empreinte" -O2 -w -DTDF_PRINTF -I "$TDF_DIR/game" -I "$MOT" -- \
		"$TDF_DIR"/game/*.c "$MOT"/*.c -- -lm -ldl
}
ver_gt() {
	[ "$1" != "$2" ] && [ "$(printf '%s\n%s\n' "$1" "$2" | sort -t. -k1,1n -k2,2n -k3,3n | tail -1)" = "$1" ]
}
update_remote() {
	local v="" w="" t="" raw="https://raw.githubusercontent.com/$UPD_REPO/HEAD/$UPD_VH"
	command -v timeout >/dev/null 2>&1 && t="timeout 2"
	[ -z "$t" ] && command -v gtimeout >/dev/null 2>&1 && t="gtimeout 2"
	if [ -n "$t" ] && git -C "$UPD_DIR" remote get-url origin >/dev/null 2>&1; then
		v=$(GIT_TERMINAL_PROMPT=0 GIT_SSH_COMMAND='ssh -oBatchMode=yes -oConnectTimeout=2' \
			$t git -C "$UPD_DIR" ls-remote --tags origin 2>/dev/null </dev/null \
			| sed -n 's#.*refs/tags/v\{0,1\}\([0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\)$#\1#p' \
			| sort -t. -k1,1n -k2,2n -k3,3n | tail -1)
	fi
	if [ -z "$v" ] || ! ver_gt "$v" "$UPD_VER"; then
		if command -v curl >/dev/null 2>&1; then w=$(curl -fsS --max-time 2 "$raw" 2>/dev/null)
		elif command -v wget >/dev/null 2>&1; then w=$(wget -q -T 2 -O - "$raw" 2>/dev/null); fi
		w=$(printf '%s\n' "$w" | sed -n 's/^# *define TDF_VERSION "\([0-9.]*\)".*/\1/p' | head -1)
		if [ -n "$w" ] && { [ -z "$v" ] || ver_gt "$w" "$v"; }; then v="$w"; fi
	fi
	printf '%s' "$v"
}
update_check() {
	local dir="$HOME/.testeur_des_fous" st now last what remote p i ans old out
	local A=$'\033[38;5;213m' Y=$'\033[38;5;220m' V=$'\033[38;5;84m' E=$'\033[38;5;203m' G=$'\033[2m' Z=$'\033[0m'
	if [ "${NO_UPDATE:-0}" = 1 ] || [ -n "${TDF_NO_UPDATE:-}" ] || [ -f "$dir/no_update" ]; then return 0; fi
	{ [ -t 0 ] && [ -t 1 ]; } || return 0
	mkdir -p "$dir" 2>/dev/null
	st="$dir/maj_$UPD_GAME.txt"
	now=$(date +%s)
	if [ -f "$st" ]; then
		last=0; what=""
		read -r last what < "$st"
		case "$last" in ''|*[!0-9]*) last=0 ;; esac
		if [ "$what" = "-" ]; then
			[ $((now - last)) -lt 3600 ] && return 0
		else
			[ $((now - last)) -lt 21600 ] && return 0
		fi
	fi
	update_remote > "$st.tmp" 2>/dev/null &
	p=$!
	i=0
	while kill -0 "$p" 2>/dev/null && [ $i -lt 25 ]; do sleep 0.1; i=$((i + 1)); done
	kill "$p" 2>/dev/null
	wait "$p" 2>/dev/null
	remote=$(cat "$st.tmp" 2>/dev/null)
	rm -f "$st.tmp"
	printf '%s %s\n' "$now" "${remote:--}" > "$st"
	{ [ -n "$remote" ] && ver_gt "$remote" "$UPD_VER"; } || return 0
	printf '\n  %s╭──────────── NOUVELLE VERSION DISPONIBLE ────────────%s\n' "$A" "$Z"
	printf '  %s│%s  Le testeur des fous · %s   %sv%s%s  →  %sv%s%s\n' "$A" "$Z" "$UPD_NAME" "$G" "$UPD_VER" "$Z" "$V" "$remote" "$Z"
	printf '  %s│%s  Mettre à jour maintenant ? %s[O/n]%s   %s(sans réponse dans 20 s : plus tard)%s\n' "$A" "$Z" "$Y" "$Z" "$G" "$Z"
	printf '  %s╰─▶%s ' "$A" "$Z"
	ans="n"
	if read -r -t 20 -n 1 ans; then [ -n "$ans" ] || ans="o"; else ans="n"; fi
	printf '\n'
	case "$ans" in
		o|O|y|Y) ;;
		*) printf '  %s   plus tard : on continue en v%s (pas de nouvelle question avant 6 h)%s\n' "$G" "$UPD_VER" "$Z"; return 0 ;;
	esac
	if ! old=$(git -C "$UPD_DIR" rev-parse HEAD 2>/dev/null); then
		printf '  %s   ce dossier n'"'"'est pas un clone git : retélécharge le testeur depuis GitHub.%s\n' "$E" "$Z"
		sleep 2
		return 0
	fi
	printf '  %s   git pull...%s\n' "$G" "$Z"
	if ! out=$(GIT_TERMINAL_PROMPT=0 GIT_SSH_COMMAND='ssh -oBatchMode=yes -oConnectTimeout=5' \
		git -C "$UPD_DIR" pull --ff-only 2>&1 </dev/null); then
		printf '  %s   échec du git pull : rien n'"'"'a changé, on continue en v%s%s\n' "$E" "$UPD_VER" "$Z"
		printf '%s\n' "$out" | tail -3 | sed 's/^/      /'
		sleep 2
		return 0
	fi
	if ! out=$(upd_build_ok 2>&1); then
		git -C "$UPD_DIR" reset -q --keep "$old" 2>/dev/null
		printf '  %s   la nouvelle version ne compile pas ici : retour à la v%s, qui continue%s\n' "$E" "$UPD_VER" "$Z"
		printf '%s\n' "$out" | grep -m3 -E 'error|erreur' | cut -c1-150 | sed 's/^/      /'
		sleep 2
		return 0
	fi
	rm -f "$st"
	printf '  %s   mise à jour réussie : v%s. Relance du testeur...%s\n' "$V" "$remote" "$Z"
	sleep 1
	TDF_NO_UPDATE=1 exec bash "$UPD_DIR/run.sh" printf ${UPD_ARGS[@]+"${UPD_ARGS[@]}"}
}
update_setting() {
	mkdir -p "$HOME/.testeur_des_fous" 2>/dev/null
	case "$1" in
		off|non|0) : > "$HOME/.testeur_des_fous/no_update"; echo "recherche de mise à jour : coupée (--update on pour la remettre)" ;;
		on|oui|1) rm -f "$HOME/.testeur_des_fous/no_update"; echo "recherche de mise à jour : activée" ;;
		*) echo "--update : on ou off"; exit 2 ;;
	esac
	exit 0
}
UPD_DIR="$(cd "$TDF_DIR/../.." && pwd)"; UPD_GAME="testeur"; UPD_NAME="printf"
UPD_VER="$(sed -n 's/^# *define TDF_VERSION "\(.*\)"/\1/p' "$MOT/version.h" 2>/dev/null | head -1)"
UPD_REPO="SplatchAFK/testeur_des_fous"; UPD_VH="moteur/version.h"
UPD_ARGS=(${SAVED_ARGS[@]+"${SAVED_ARGS[@]}"})
upd_build_ok() {
	bash -n "$TDF_DIR/run.sh" || return 1
	cc -fsyntax-only -w -DTDF_PRINTF -I "$TDF_DIR/game" -I "$MOT" "$TDF_DIR"/game/*.c "$MOT"/*.c
}
[ -n "$UPDATE_SET" ] && update_setting "$UPDATE_SET"

grand_terminal() {
	local c l t term lanceur a
	c=$(tput cols 2>/dev/null || echo 80)
	l=$(tput lines 2>/dev/null || echo 24)
	{ [ "$c" -ge 120 ] && [ "$l" -ge 36 ]; } && return 0
	[ -z "${TDF_FENETRE:-}" ] || return 0
	printf '\e[8;42;140t'
	sleep 0.3
	c=$(tput cols 2>/dev/null || echo 80)
	l=$(tput lines 2>/dev/null || echo 24)
	{ [ "$c" -ge 120 ] && [ "$l" -ge 36 ]; } && return 0
	lanceur="${TMPDIR:-/tmp}/testeur_des_fous_fenetre_$$.sh"
	{
		printf 'rm -f -- "$0"\n'
		printf 'cd %q || exit 1\n' "$PWD"
		printf 'export TDF_PROJET=%q TDF_NOM=%q\n' "$PROJECT" "${TDF_NOM:-}"
		printf 'TDF_FENETRE=1 bash %q printf --projet %q' "$UPD_DIR/run.sh" "$PROJECT"
		for a in ${SAVED_ARGS[@]+"${SAVED_ARGS[@]}"}; do printf ' %q' "$a"; done
		printf '\nprintf "\\n  (Entrée pour fermer la fenêtre) "; read -r _\n'
	} > "$lanceur" || return 0
	if grep -qi microsoft /proc/version 2>/dev/null && command -v cmd.exe >/dev/null 2>&1; then
		( cd /mnt/c 2>/dev/null; cmd.exe /c start "Le testeur des fous" cmd /c \
			"mode con: cols=140 lines=42 & wsl.exe ${WSL_DISTRO_NAME:+-d $WSL_DISTRO_NAME} -- bash $lanceur" ) >/dev/null 2>&1 \
			|| return 0
	elif [ -n "${DISPLAY:-}${WAYLAND_DISPLAY:-}" ]; then
		term=""
		for t in gnome-terminal konsole xfce4-terminal xterm; do
			command -v "$t" >/dev/null 2>&1 && { term="$t"; break; }
		done
		case "$term" in
			gnome-terminal) gnome-terminal --geometry=140x42 -- bash "$lanceur" >/dev/null 2>&1 || return 0 ;;
			konsole) konsole -p TerminalColumns=140 -p TerminalRows=42 -e bash "$lanceur" >/dev/null 2>&1 & ;;
			xfce4-terminal) xfce4-terminal --geometry=140x42 -x bash "$lanceur" >/dev/null 2>&1 & ;;
			xterm) xterm -geometry 140x42 -e bash "$lanceur" >/dev/null 2>&1 & ;;
			*) rm -f -- "$lanceur"; return 0 ;;
		esac
	elif [ "$(uname -s)" = Darwin ] && command -v osascript >/dev/null 2>&1; then
		osascript -e "tell application \"Terminal\" to do script \"bash '$lanceur'\"" \
			-e 'tell application "Terminal" to set bounds of front window to {40, 40, 1180, 900}' >/dev/null 2>&1 || return 0
	else
		rm -f -- "$lanceur"
		return 0
	fi
	printf '\n  Ton terminal fait %s x %s : le testeur s'"'"'ouvre dans une nouvelle fenêtre de 140 x 42.\n' "$c" "$l"
	printf '  (pour rester ici : TDF_FENETRE=1 bash %s/run.sh)\n\n' "$TDF_AFF"
	exit 0
}
if [ -z "${TDF_ENGINE:-}" ] && [ "$NOLORE" = 0 ] && [ "$DEBUG" = 0 ] && [ -z "$VERBOSE" ] && [ -z "$ONLY" ] \
	&& [ -z "$MACHINE" ] && [ -t 0 ] && [ -t 1 ]; then
	grand_terminal
	tdf_chrono_debut maj "recherche de mise à jour"
	update_check
	tdf_chrono_fin maj "recherche de mise à jour"
	tdf_chrono_debut jeu "compilation de l'interface"
	if build_jeu; then
		tdf_chrono_fin jeu "compilation de l'interface"
		if [ "$MENU_SEUL" = 1 ]; then
			GARGS=(--dir "$TDF_DIR")
			[ "$FAST" = 1 ] && GARGS+=(--fast)
			tdf_journal "menu seul, sans tests"
			"$JEU/fous" "${GARGS[@]}" 2>> "${TDF_JOURNAL:-/dev/null}"
			code=$?
			tdf_journal "fin de l'interface (code $code)"
			exit $code
		fi
		first=1
		dlc=0
		while :; do
			RES="$JEU/resultats.tsv"
			: > "$RES"
			EARGS=(--seed "$SEED" --dir "$PROJECT" --aide "$AIDE" --machine "$RES")
			case "$MODE" in valgrind) EARGS+=(--valgrind) ;; asan) EARGS+=(--asan-only) ;; none) EARGS+=(--no-sanitize) ;; esac
			[ "$DO_NORM" = 0 ] && EARGS+=(--no-norm)
			[ -n "$STRESS" ] && EARGS+=(--stress "$STRESS")
			[ -n "$TIMEOUT" ] && EARGS+=(--timeout "$TIMEOUT")
			{ [ "$dlc" = 1 ] || [ "$BONUS_OPT" = "--bonus" ]; } && EARGS+=(--bonus)
			tdf_journal "tests lancés en arrière-plan (graine $SEED, mode $MODE)"
			(
				MPID=
				trap 'kill_tree "$MPID"; exit 143' TERM INT HUP
				t0=$(tdf_ms)
				TDF_ENGINE=1 bash "$TDF_DIR/run.sh" "${EARGS[@]}" > "$JEU/moteur.log" 2>&1 < /dev/null &
				MPID=$!
				wait "$MPID"
				rc=$?
				printf 'E\n' >> "$RES"
				tdf_journal "tests terminés (code $rc, $(( ($(tdf_ms) - t0) / 1000 )) s)"
			) &
			EPID=$!
			trap 'kill "$EPID" 2>/dev/null' EXIT
			GARGS=(--results "$RES" --pid "$EPID" --dir "$TDF_DIR")
			[ "$first" = 0 ] && GARGS+=(--relance)
			[ "$FAST" = 1 ] && GARGS+=(--fast)
			[ "$dlc" = 1 ] && GARGS+=(--dlc)
			[ "$first" = 1 ] && [ -n "${RECO:-}" ] && GARGS+=(--reco "$RECO")
			tdf_journal "interface ouverte"
			"$JEU/fous" "${GARGS[@]}" 2>> "${TDF_JOURNAL:-/dev/null}"
			code=$?
			tdf_journal "interface fermée (code $code)"
			kill "$EPID" 2>/dev/null
			wait "$EPID" 2>/dev/null
			[ -n "${TDF_JOURNAL:-}" ] && cp "$JEU/moteur.log" "$(dirname "$TDF_JOURNAL")/printf_moteur.log" 2>/dev/null
			first=0
			dlc=0
			[ "$code" = 43 ] && { dlc=1; continue; }
			[ "$code" = 42 ] || break
		done
		exit $code
	fi
	printf '\r\033[K  le menu ne compile pas, passage en mode texte :\n'
	sed 's/^/    /' "$JEU/jeu.err" | head -5
	tdf_journal "ERREUR : l'interface ne compile pas, passage en mode texte"
	[ -n "${TDF_JOURNAL:-}" ] && cp "$JEU/jeu.err" "$(dirname "$TDF_JOURNAL")/printf_compilation.err" 2>/dev/null
	[ "$MENU_SEUL" = 1 ] && exit 1
fi
if [ "$NOLORE" = 1 ] && [ -z "${TDF_ENGINE:-}" ] && [ -z "$MACHINE" ]; then
	mkdir -p "$JEU"
	MACHINE="$JEU/resultats.tsv"
	NOLORE_RECORD=1
fi
if [ "$DEBUG" = 1 ]; then
	PASS=(--seed "$SEED" --aide 2 -v --dir "$PROJECT")
	[ -n "$ONLY" ] && PASS+=(--only "$ONLY")
	[ "$BONUS_OPT" = "--bonus" ] && PASS+=(--bonus)
	[ "$DO_NORM" = 0 ] && PASS+=(--no-norm)
	printf "\n%s  DEBUG AVANCÉ · passage 1/2 : ASan + UBSan, tout le détail%s\n" "$B" "$N"
	bash "$TDF_DIR/run.sh" "${PASS[@]}"
	rc=$?
	if command -v valgrind >/dev/null 2>&1; then
		printf "\n%s  DEBUG AVANCÉ · passage 2/2 : valgrind (fuites de toutes sortes, lectures non initialisées)%s\n" "$B" "$N"
		bash "$TDF_DIR/run.sh" "${PASS[@]}" --valgrind --stress 300 --no-norm || rc=1
	else
		printf "\n  %svalgrind n'est pas installé : passage 2/2 sauté%s\n" "$Y" "$N"
	fi
	exit $rc
fi
[ -n "$MACHINE" ] && : > "$MACHINE"
OS="$(uname -s)"
BUILD="$TDF_DIR/.build"
COPY="$BUILD/project"
STATIC_KO=0
STATIC_WARN=0

want() {
	[ -z "$ONLY" ] && return 0
	local c
	IFS=',' read -ra cs <<< "$ONLY"
	for c in "${cs[@]}"; do [ "$c" = "$1" ] && return 0; done
	return 1
}
line() {
	local col="$G"
	[ "$1" = "KO" ] && col="$R" && STATIC_KO=$((STATIC_KO + 1))
	[ "$1" = "~~" ] && col="$Y" && STATIC_WARN=$((STATIC_WARN + 1))
	[ "$1" = ".." ] && col="$D"
	printf "  %s%-3s%s %s %s\n" "$col" "$1" "$N" "$2" "${3:-}"
	[ -n "$MACHINE" ] && printf 'S\t%s\t%s\t%s\n' "$1" "$2" "${3:-}" >> "$MACHINE"
	return 0
}
detail() {
	printf "      %s\n" "$@"
	[ -n "$MACHINE" ] && printf 'D\t%s\n' "$*" | sed 's/\x1b\[[0-9;]*m//g' >> "$MACHINE"
	return 0
}

CACHE="$TDF_DIR/.build_jeu/cache"
mkdir -p "$CACHE" 2>/dev/null
fingerprint() {
	{ printf '%s|%s|%s|%s\n' "${TDF_V:-}" "$OS" "$ONLY" "$BONUS_OPT"; cat "$@" 2>/dev/null; } | cksum | cut -d' ' -f1
}
cache_run() {
	local tdf_cf="$CACHE/$1.$2" tdf_cm="$MACHINE"
	if [ "${TDF_CACHE:-1}" != 1 ] || [ ! -d "$CACHE" ] || [ -z "$2" ]; then
		"$3"
		return 0
	fi
	if [ -f "$tdf_cf.out" ] && [ -f "$tdf_cf.tsv" ]; then
		cat "$tdf_cf.out"
		[ -n "$tdf_cm" ] && cat "$tdf_cf.tsv" >> "$tdf_cm"
		STATIC_KO=$((STATIC_KO + $(awk -F'\t' '$1=="S" && $2=="KO"' "$tdf_cf.tsv" | wc -l)))
		STATIC_WARN=$((STATIC_WARN + $(awk -F'\t' '$1=="S" && $2=="~~"' "$tdf_cf.tsv" | wc -l)))
		return 0
	fi
	MACHINE="$CACHE/.rec.tsv"
	: > "$MACHINE"
	"$3" > "$CACHE/.rec.out"
	MACHINE="$tdf_cm"
	cat "$CACHE/.rec.out"
	[ -n "$tdf_cm" ] && cat "$CACHE/.rec.tsv" >> "$tdf_cm"
	rm -f "$CACHE/$1".*
	mv "$CACHE/.rec.out" "$tdf_cf.out" && mv "$CACHE/.rec.tsv" "$tdf_cf.tsv"
	return 0
}

printf "\n%s ═══════════════════════════════════════════════════════════════%s\n" "$B" "$N"
TDF_V=$(sed -n 's/^# *define TDF_VERSION "\(.*\)"/\1/p' "$MOT/version.h" 2>/dev/null)
printf "%s   Le testeur des fous · version printf v%s   %smode : %s%s%s\n" "$B" "${TDF_V:-?}" "$N" "$C" "$MODE" "$N"
printf "%s ═══════════════════════════════════════════════════════════════%s\n" "$B" "$N"
printf "  projet : %s\n  OS : %s   graine du stress : %s   aide : %s/2\n" "$PROJECT" "$OS" "$SEED" "$AIDE"

if [ ! -f "$PROJECT/Makefile" ]; then
	printf "\n  %sKO%s pas de Makefile dans %s\n" "$R" "$N" "$PROJECT"
	printf "     Clone le testeur à côté de ton dossier ft_printf, ou utilise --projet.\n\n"
	exit 1
fi

rm -rf "$BUILD"
mkdir -p "$COPY" "$BUILD/obj" "$BUILD/obj_bonus" "$BUILD/vg"
EXCL=()
case "$TDF_DIR/" in "$PROJECT/"*) EXCL+=(--exclude="./${TDF_DIR#$PROJECT/}");; esac
tar -C "$PROJECT" "${EXCL[@]}" --exclude=./.git --exclude='testeur_des_fous*' --exclude='francinette' \
	--exclude='printfTester*' --exclude='ft_printf_tester*' --exclude='printf-tester*' --exclude='.build*' \
	-cf - . 2>/dev/null | tar -C "$COPY" -xf -
( cd "$COPY" && make fclean >/dev/null 2>&1 )

TESTER_DIRS='printfTester|testeur_des_fous|francinette|ft_printf_tester|printf-tester|tester'
student_find() {
	find "$COPY" "$@" -not -path '*/.git/*' 2>/dev/null | sed "s#^$COPY/##" \
		| grep -Ev "(^|/)($TESTER_DIRS)[^/]*/" | sed "s#^#$COPY/#" | sort
}

printf "\n%s  ── Contrôles statiques ──%s\n" "$B" "$N"
MK_LOG="$BUILD/make.log"
if ! ( cd "$COPY" && make ) >"$MK_LOG" 2>&1; then
	line KO "Makefile : 'make' échoue"
	tail -n 15 "$MK_LOG" | sed 's/^/      /'
	exit 1
fi
if [ ! -f "$COPY/libftprintf.a" ]; then
	line KO "Makefile : libftprintf.a absent à la racine du dépôt"
	exit 1
fi

mk_audit() {
	MK_PB=()
	MK_WARN=()
	for r in all clean fclean re libftprintf.a; do
		out=$(cd "$COPY" && make -n "$r" 2>&1)
		if [ $? -ne 0 ] || echo "$out" | grep -q "No rule to make target"; then
			MK_PB+=("règle '$r' absente")
		fi
	done
	grep -Eq '^[[:space:]]*NAME[[:space:]]*[:+?]?=[[:space:]]*libftprintf\.a' "$COPY/Makefile" \
		|| MK_PB+=("variable NAME = libftprintf.a introuvable")
	grep -qi 'libtool' "$COPY/Makefile" && MK_PB+=("libtool utilisé (interdit, il faut ar)")
	CMDS=$(cd "$COPY" && make -n -B all 2>/dev/null)
	COMPILES=$(echo "$CMDS" | grep -E '(^|[[:space:]/])(cc|gcc|clang)[[:space:]].*-c' )
	if [ -z "$COMPILES" ]; then
		MK_WARN+=("impossible de voir les lignes de compilation (make -n -B)")
	else
		NOFLAGS=$(echo "$COMPILES" | grep -v -- '-Wall' ; echo "$COMPILES" | grep -v -- '-Wextra'; echo "$COMPILES" | grep -v -- '-Werror')
		[ -n "$(echo "$NOFLAGS" | tr -d '[:space:]')" ] && MK_PB+=("compilation sans -Wall -Wextra -Werror : $(echo "$NOFLAGS" | head -1 | cut -c1-90)")
		echo "$COMPILES" | grep -Eqv '(^|[[:space:]/])cc[[:space:]]' && MK_WARN+=("le sujet demande 'cc' comme compilateur")
	fi
	echo "$CMDS" | grep -Eq '(^|[[:space:]])ar[[:space:]]' || MK_WARN+=("'ar' introuvable dans make -n (lib créée autrement ?)")
	before=$(stat -c '%Y.%y' "$COPY/libftprintf.a" 2>/dev/null || stat -f '%m' "$COPY/libftprintf.a")
	sleep 1
	again=$(cd "$COPY" && make 2>&1)
	after=$(stat -c '%Y.%y' "$COPY/libftprintf.a" 2>/dev/null || stat -f '%m' "$COPY/libftprintf.a")
	if [ "$before" != "$after" ] || echo "$again" | grep -Eq '(^|[[:space:]/])(cc|gcc|clang|ar)[[:space:]]'; then
		MK_PB+=("relink : un 2e 'make' sans modification reconstruit")
	fi
	( cd "$COPY" && make clean >/dev/null 2>&1 )
	[ -n "$(student_find -name '*.o')" ] && MK_PB+=("'make clean' laisse des .o")
	( cd "$COPY" && make fclean >/dev/null 2>&1 )
	[ -f "$COPY/libftprintf.a" ] && MK_PB+=("'make fclean' ne supprime pas libftprintf.a")
	( cd "$COPY" && make re >/dev/null 2>&1 )
	[ -f "$COPY/libftprintf.a" ] || MK_PB+=("'make re' ne reconstruit pas libftprintf.a")
	( cd "$COPY" && make >/dev/null 2>&1 )
	if [ ${#MK_PB[@]} -eq 0 ] && [ ${#MK_WARN[@]} -eq 0 ]; then
		line OK "Makefile (règles, NAME, flags, cc, ar, relink, clean/fclean/re)"
	elif [ ${#MK_PB[@]} -eq 0 ]; then
		line "~~" "Makefile"
		for m in "${MK_WARN[@]}"; do detail "${Y}⚠${N} $m"; done
	else
		line KO "Makefile"
		for m in "${MK_PB[@]}"; do detail "${R}✗${N} $m"; done
		for m in "${MK_WARN[@]}"; do detail "${Y}⚠${N} $m"; done
	fi
	if [ -d "$COPY/libft" ]; then
		if echo "$CMDS" | grep -Eq '(make|MAKE)[^;&|]*-C[[:space:]]*(\./)?libft|cd[[:space:]]+(\./)?libft[^;&|]*(&&|;)[[:space:]]*\$?\(?(make|MAKE)'; then
			line OK "Makefile : la libft est compilée par son propre Makefile (make -C libft)"
		else
			line KO "Makefile : la libft doit être compilée par son propre Makefile"
			detail "sujet, instructions communes : « Your activity's Makefile must compile the library by using its Makefile »"
		fi
	fi
	HDR=$(student_find -name 'ft_printf.h' | head -1)
	if [ -z "$HDR" ]; then
		line KO "ft_printf.h introuvable"
	elif ! grep -Eq 'int[[:space:]]+ft_printf[[:space:]]*\([[:space:]]*const[[:space:]]+char[[:space:]]*\*' "$HDR"; then
		line KO "ft_printf.h : prototype 'int ft_printf(const char *, ...);' introuvable"
	else
		line OK "ft_printf.h et prototype"
	fi
}
if want makefile; then
	MK_KEY=$(fingerprint "$COPY/Makefile" "$COPY/libft/Makefile" $(student_find -name '*.h') \
		<(student_find -type f | sed "s#^$COPY/##"))
	cache_run makefile "$MK_KEY" mk_audit
fi

if want readme || want makefile; then
	RD="$COPY/README.md"
	if [ ! -f "$RD" ]; then
		line KO "README.md absent à la racine (sujet, chapitre 5)"
	else
		RD_PB=()
		first=$(grep -m1 -v '^[[:space:]]*$' "$RD")
		case "$first" in
			[*_]*"This activity has been created as part of the 42 curriculum by "*[*_]) ;;
			*) RD_PB+=("1re ligne : en italique, « This activity has been created as part of the 42 curriculum by <login> »") ;;
		esac
		for sec in Description Instructions Resources; do
			grep -Eqi "^#+[[:space:]]*$sec" "$RD" || RD_PB+=("section « $sec » absente")
		done
		RES=$(sed -n '/^#\{1,\}[[:space:]]*[Rr]esources/,$p' "$RD")
		{ echo "$RES" | grep -Eqiw 'ai|ia|llm|chatgpt|gpt|claude|copilot|mistral|gemini' \
			|| echo "$RES" | grep -qi 'intelligence'; } \
			|| RD_PB+=("Resources : décris comment l'IA a été utilisée (ou qu'elle ne l'a pas été)")
		if [ ${#RD_PB[@]} -eq 0 ]; then
			line OK "README.md (1re ligne, Description, Instructions, Resources)"
			line "~~" "README.md : explication de l'algorithme et de la structure de données" \
				"(exigée par le sujet, à vérifier par toi : le testeur ne juge pas le fond)"
		else
			line KO "README.md"
			for m in "${RD_PB[@]}"; do detail "${R}✗${N} $m"; done
		fi
	fi
fi

lr_check() {
	LR="$BUILD/libft_rapide"
	mkdir -p "$LR"
	cp -r "$COPY/libft/." "$LR/" 2>/dev/null
	( cd "$LR" && make fclean >/dev/null 2>&1; make >/dev/null 2>&1 )
	LRA=$(find "$LR" -maxdepth 1 -name '*.a' | head -1)
	if [ -z "$LRA" ]; then
		line KO "libft/ : contrôle rapide impossible, la libft ne compile pas avec son Makefile"
	elif ! { WA=(-Wl,--whole-archive "$LRA" -Wl,--no-whole-archive); [ "$OS" = Darwin ] && WA=("-Wl,-force_load,$LRA")
		cc -g -w -fsanitize=address,undefined "$TDF_DIR/game/exos/libft_rapide.c" "${WA[@]}" -o "$LR/rapide" 2>"$LR/build.log"; }; then
		line "~~" "libft/ : contrôle rapide impossible (édition de liens)" "$(head -1 "$LR/build.log" | cut -c1-90)"
	else
		ASAN_OPTIONS=detect_leaks=0 timeout 60 "$LR/rapide" > "$LR/res.txt" 2>/dev/null
		LOK=$(awk -F'\t' '$1=="T"{print $2}' "$LR/res.txt")
		LTOT=$(awk -F'\t' '$1=="T"{print $3}' "$LR/res.txt")
		if [ -n "$LOK" ] && [ "$LOK" = "$LTOT" ]; then
			line OK "libft/ : contrôle rapide de ta libft ($LOK/$LTOT fonctions)"
		else
			line KO "libft/ : contrôle rapide de ta libft (${LOK:-0}/${LTOT:-43} fonctions)"
			awk -F'\t' '$1=="F" && $3!="OK"{print $2 " : " $3 " " $4}' "$LR/res.txt" | head -8 | while IFS= read -r l; do detail "$l"; done
		fi
	fi
}
if { want makefile || want libft; } && [ -f "$COPY/libft/Makefile" ]; then
	LR_KEY=$(fingerprint "$TDF_DIR/game/exos/libft_rapide.c" \
		$(find "$COPY/libft" -type f \( -name '*.c' -o -name '*.h' -o -name 'Makefile' \) | sort) \
		<(cd "$COPY/libft" && find . -type f \( -name '*.c' -o -name '*.h' \) | sort))
	cache_run libft "$LR_KEY" lr_check
fi

if want nm; then
	UNDERSCORE='s/^//'
	[ "$OS" = "Darwin" ] && UNDERSCORE='s/^_//'
	NMU=$(nm -u "$COPY/libftprintf.a" 2>/dev/null | awk 'NF==2 && $1=="U"{print $2} NF==1 && $1!~/:$/{print $1}' | sed "$UNDERSCORE" | sort -u)
	NMD=$(nm -g "$COPY/libftprintf.a" 2>/dev/null | awk 'NF==3 && $2!="U"{print $3}' | sed "$UNDERSCORE" | sort -u)
	BAD=(); SUSP=()
	for s in $NMU; do
		echo "$NMD" | grep -qx "$s" && continue
		case "$s" in
			write|malloc|free) ;;
			__stack_chk_fail|__stack_chk_guard|_GLOBAL_OFFSET_TABLE_|__gmon_start__|_ITM_*|__cxa_*|stack_chk_fail|stack_chk_guard|_DefaultRuneLocale|dyld_stub_binder) ;;
			memset|memcpy|memmove|bzero) SUSP+=("$s") ;;
			*) BAD+=("$s") ;;
		esac
	done
	if [ ${#BAD[@]} -gt 0 ]; then
		LIBFT_A=$(find "$COPY" -name 'libft*.a' -not -name 'libftprintf.a' 2>/dev/null | head -1)
		LFB=(); [ -n "$LIBFT_A" ] && for s in "${BAD[@]}"; do
			nm -g "$LIBFT_A" 2>/dev/null | awk 'NF==3 && $2!="U"{print $3}' | sed "$UNDERSCORE" | grep -qx "$s" && LFB+=("$s")
		done
		if [ ${#LFB[@]} -gt 0 ]; then
			line KO "libft utilisée mais NON incluse dans libftprintf.a (nm)" "${R}${LFB[*]}${N}"
			detail "la libft est autorisée, mais ton libftprintf.a doit la contenir (make -C libft, puis ar des .o de libft)"
			detail "sinon le correcteur ne pourra pas lier : undefined reference"
		else
			line KO "fonctions externes interdites (nm)" "${R}${BAD[*]}${N}"
			detail "autorisées : write malloc free + ta libft (si elle est dans libftprintf.a)"
		fi
	elif [ ${#SUSP[@]} -gt 0 ]; then
		line "~~" "fonctions externes (nm)" "${Y}${SUSP[*]}${N}"
		detail "pas autorisées par le sujet, mais parfois générées par le compilateur : vérifie ton code"
	else
		line OK "fonctions externes (nm) : seulement write/malloc/free"
	fi
fi

objs_to_srcs() {
	local o base cands miss=0
	for o in $(ar t "$1"); do
		case "$o" in *.o) ;; *) continue ;; esac
		base="${o%.o}"
		cands=$(student_find -name "$base.c")
		if [ -z "$cands" ]; then
			echo "MISSING:$o"; miss=1
		elif [ "$(echo "$cands" | wc -l)" -gt 1 ]; then
			local od; od=$(dirname "$(student_find -name "$o" | head -1)" 2>/dev/null)
			echo "$cands" | grep -m1 "^$od/" || echo "$cands" | head -1
		else
			echo "$cands"
		fi
	done
	return $miss
}
SRCS_M=$(objs_to_srcs "$COPY/libftprintf.a")
HAS_BONUS=0
if [ "$BONUS_OPT" = "--bonus" ] && (cd "$COPY" && make -n bonus >/dev/null 2>&1); then
	HAS_BONUS=1
	( cd "$COPY" && make fclean >/dev/null 2>&1 && make bonus >"$BUILD/make_bonus.log" 2>&1 )
	if [ -f "$COPY/libftprintf.a" ]; then
		SRCS_B=$(objs_to_srcs "$COPY/libftprintf.a")
		cp "$COPY/libftprintf.a" "$BUILD/student_bonus.a"
	else
		HAS_BONUS=0
	fi
	( cd "$COPY" && make fclean >/dev/null 2>&1 && make >/dev/null 2>&1 )
fi
cp "$COPY/libftprintf.a" "$BUILD/student.a"

if want norme && [ $DO_NORM -eq 1 ]; then
	if command -v norminette >/dev/null 2>&1; then
		NORM_LATEST="3.3.60"
		NV=$(norminette --version 2>/dev/null | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)
		if [ -n "$NV" ] && [ "$(printf '%s\n%s\n' "$NV" "$NORM_LATEST" | sort -V | head -1)" != "$NORM_LATEST" ]; then
			line "~~" "norminette $NV : la dernière connue est $NORM_LATEST" "(python3 -m pip install --user -U norminette)"
		fi
		NFILES=$( { echo "$SRCS_M"; [ $HAS_BONUS -eq 1 ] && echo "$SRCS_B"; student_find -name '*.h'; } | grep -v '^MISSING:' | sort -u)
		norm_run() {
			NOUT=$(cd "$COPY" && norminette $NFILES 2>&1)
			NERR=$(echo "$NOUT" | grep -c ': Error!')
			NTOT=$(echo "$NFILES" | grep -c .)
			if [ "$NERR" -eq 0 ]; then
				line OK "norminette ($NTOT fichiers)"
			else
				line KO "norminette : $NERR fichier(s) sur $NTOT en erreur"
				echo "$NOUT" | grep -E 'Error!|Error:' | head -n "$( [ -n "$VERBOSE" ] && echo 100000 || echo 12 )" | sed "s#$COPY/##" | sed 's/^/      /'
			fi
		}
		cache_run norme "$(fingerprint <(echo "$NV $VERBOSE"; echo "$NFILES" | sed "s#^$COPY/##") $NFILES)" norm_run
	else
		line ".." "norminette non installée : vérification de la norme ignorée"
	fi
fi

case "$MODE" in
	asan+ubsan) SAN="-fsanitize=address,undefined -fno-sanitize-recover=all" ;;
	asan) SAN="-fsanitize=address" ;;
	none) SAN="-DTDF_GUARD=1" ;;
	valgrind)
		SAN="-DTDF_VALGRIND=1"
		command -v valgrind >/dev/null || { printf "  %sKO%s valgrind n'est pas installé\n" "$R" "$N"; exit 1; } ;;
esac
CFLAGS_COMMON="-g -O0 -fno-omit-frame-pointer"
san_ok() { echo 'int main(void){return 0;}' | LC_ALL=C cc $1 -x c - -o "$BUILD/.san_probe" >/dev/null 2>&1; }
if [ "$MODE" = "asan+ubsan" ] && ! san_ok "$SAN"; then
	if san_ok "-fsanitize=address"; then
		line "~~" "UBSan indisponible sur cette machine (libubsan absente) : repli sur ASan seul"
		MODE="asan"; SAN="-fsanitize=address"
	else
		line "~~" "sanitizers indisponibles sur cette machine : repli sur le mode sans sanitizer"
		MODE="none"; SAN="-DTDF_GUARD=1"
	fi
elif [ "$MODE" = "asan" ] && ! san_ok "$SAN"; then
	line "~~" "ASan indisponible sur cette machine : repli sur le mode sans sanitizer"
	MODE="none"; SAN="-DTDF_GUARD=1"
fi
INCS=$(student_find -name '*.h' -exec dirname {} \; | sort -u | sed 's/^/-I/')
LINK_WRAP="-Wl,--wrap=malloc,--wrap=free,--wrap=calloc,--wrap=realloc"
TRACK="-DTDF_TRACK=1"
if [ "$OS" = "Darwin" ]; then LINK_WRAP=""; TRACK="-DTDF_TRACK=0"; fi
CC_T=cc
command -v cc >/dev/null || CC_T=gcc

build_lib() {
	local srcs="$1" od="$2" out="$3" fallback="$4" i=0 s
	if echo "$srcs" | grep -q '^MISSING:'; then
		cp "$fallback" "$out"; return 1
	fi
	: > "$od.list"
	rm -f "$od"/*.fail
	for s in $srcs; do
		i=$((i + 1))
		{ $CC_T $CFLAGS_COMMON $SAN -w $INCS -c "$s" -o "$od/$i.o" 2>>"$BUILD/build.log" || : > "$od/$i.fail"; } &
		echo "$s" >> "$od.list"
		[ $((i % 8)) -eq 0 ] && wait
	done
	wait
	if ls "$od"/*.fail >/dev/null 2>&1; then
		cp "$fallback" "$out"; return 1
	fi
	ar rcs "$out" "$od"/*.o
	return 0
}
if ! build_lib "$SRCS_M" "$BUILD/obj" "$BUILD/lib_m.a" "$BUILD/student.a"; then
	line "~~" "lib non instrumentée" "(sources introuvables pour : $(echo "$SRCS_M" | grep MISSING | tr '\n' ' '))"
	detail "les erreurs mémoire seront moins bien localisées"
fi
echo "$SRCS_M" | grep -v '^MISSING:' > "$BUILD/srcs.txt"
RUNNER_SRC="$TDF_DIR/src"
RUNNER_OBJ=""
runner_objs() {
	local key f o ok=1
	[ "${TDF_CACHE:-1}" = 1 ] && [ -d "$CACHE" ] || return 1
	key=$(fingerprint "$RUNNER_SRC"/*.c "$RUNNER_SRC"/*.h <(echo "$CC_T $CFLAGS_COMMON $SAN $TRACK $MODE"))
	RUNNER_OBJ="$CACHE/runner.$key"
	[ -f "$RUNNER_OBJ/ok" ] && return 0
	rm -rf "$CACHE"/runner.*
	mkdir -p "$RUNNER_OBJ"
	for f in "$RUNNER_SRC"/*.c; do
		o="$RUNNER_OBJ/$(basename "$f" .c).o"
		{ LC_ALL=C $CC_T $CFLAGS_COMMON $SAN $TRACK -DTDF_MODE="\"$MODE\"" -fno-builtin -Wall -Wextra \
			-I"$RUNNER_SRC" -c "$f" -o "$o" 2>>"$BUILD/runner_build.log" || : > "$o.fail"; } &
	done
	wait
	ls "$RUNNER_OBJ"/*.fail >/dev/null 2>&1 && { rm -rf "$RUNNER_OBJ"; RUNNER_OBJ=""; return 1; }
	: > "$RUNNER_OBJ/ok"
}
build_runner() {
	if [ -n "$RUNNER_OBJ" ] || runner_objs; then
		LC_ALL=C $CC_T $CFLAGS_COMMON $SAN "$RUNNER_OBJ"/*.o "$1" $LINK_WRAP -ldl -o "$2" 2>>"$BUILD/runner_build.log" \
			&& return 0
	fi
	LC_ALL=C $CC_T $CFLAGS_COMMON $SAN $TRACK -DTDF_MODE="\"$MODE\"" -fno-builtin -Wall -Wextra \
		-I"$RUNNER_SRC" "$RUNNER_SRC"/*.c "$1" $LINK_WRAP -ldl -o "$2" 2>"$BUILD/runner_build.log"
}
if ! build_runner "$BUILD/lib_m.a" "$BUILD/tdf_runner"; then
	printf "  %sKO%s impossible de lier le testeur avec ta lib :\n" "$R" "$N"
	grep -v '^$' "$BUILD/runner_build.log" | cut -c1-200 | head -n 14 | sed 's/^/      /'
	exit 1
fi
if [ $HAS_BONUS -eq 1 ]; then
	build_lib "$SRCS_B" "$BUILD/obj_bonus" "$BUILD/lib_b.a" "$BUILD/student_bonus.a" || true
	echo "$SRCS_B" | grep -v '^MISSING:' >> "$BUILD/srcs.txt"
	build_runner "$BUILD/lib_b.a" "$BUILD/tdf_runner_bonus" || HAS_BONUS=0
fi
sort -u -o "$BUILD/srcs.txt" "$BUILD/srcs.txt"

export ASAN_OPTIONS="check_printf=0:detect_leaks=1:halt_on_error=1:symbolize=1:allocator_may_return_null=1:detect_odr_violation=0:print_summary=1"
export UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1"
export LSAN_OPTIONS="print_suppressions=0"
ARGS=(--seed "$SEED" --project "$COPY" --srclist "$BUILD/srcs.txt" --vglogdir "$BUILD/vg" --aide "$AIDE")
[ -n "$MACHINE" ] && ARGS+=(--machine "$MACHINE")
[ -n "$STRESS" ] && ARGS+=(--stress "$STRESS")
[ -n "$TIMEOUT" ] && ARGS+=(--timeout "$TIMEOUT")
[ -n "$VERBOSE" ] && ARGS+=("$VERBOSE")
DYN_ONLY=""
if [ -n "$ONLY" ]; then
	DYN_ONLY=$(echo "$ONLY" | tr ',' '\n' | grep -Ev '^(makefile|nm|norme)$' | paste -sd, -)
fi

run_runner() {
	local exe="$1"; shift
	if [ "$MODE" = "valgrind" ]; then
		valgrind -q --leak-check=full --show-leak-kinds=all --track-origins=yes \
			--errors-for-leak-kinds=definite,indirect,possible --error-exitcode=42 \
			--log-file="$BUILD/vg/vg.%p" "$exe" --exe "$exe" "$@"
	else
		"$exe" "$@"
	fi
}

RC=0
if [ -z "$ONLY" ] || [ -n "$DYN_ONLY" ]; then
	printf "\n%s  ── Tests dynamiques ──%s  %s(chaque test dans un processus fils, comparé au vrai printf)%s\n" "$B" "$N" "$D" "$N"
	M_ARGS=("${ARGS[@]}")
	[ -n "$DYN_ONLY" ] && M_ARGS+=(--only "$DYN_ONLY")
	if [ $HAS_BONUS -eq 1 ]; then
		M_ARGS+=(--no-bonus)
	elif [ "$BONUS_OPT" = "--bonus" ]; then
		M_ARGS+=(--bonus)
	else
		M_ARGS+=(--no-bonus)
	fi
	run_runner "$BUILD/tdf_runner" "${M_ARGS[@]}" || RC=1
	if [ $HAS_BONUS -eq 1 ] && [ "$BONUS_OPT" != "--no-bonus" ] && { [ -z "$DYN_ONLY" ] || echo ",$DYN_ONLY," | grep -q ',bonus,'; }; then
		printf "  %s(bonus : lib construite par 'make bonus')%s\n" "$D" "$N"
		B_ARGS=("${ARGS[@]}" --only bonus)
		[ -n "$BONUS_OPT" ] && B_ARGS+=("$BONUS_OPT")
		run_runner "$BUILD/tdf_runner_bonus" "${B_ARGS[@]}" || RC=1
	fi
fi

printf "\n"
if [ $STATIC_KO -gt 0 ]; then
	printf "  %sContrôles statiques : %d KO%s" "$R" "$STATIC_KO" "$N"
else
	printf "  %sContrôles statiques : OK%s" "$G" "$N"
fi
[ $STATIC_WARN -gt 0 ] && printf "  %s(%d avertissement(s))%s" "$Y" "$STATIC_WARN" "$N"
printf "\n"
if want stress; then
	printf "  %sPour rejouer le stress : bash %s/run.sh printf --only stress --seed %s%s\n" "$D" "$TDF_AFF" "$SEED" "$N"
fi
printf "  %stestable aussi avec : --valgrind | --asan-only | --no-sanitize%s\n\n" "$D" "$N"
[ $STATIC_KO -gt 0 ] && RC=1
if [ "${NOLORE_RECORD:-0}" = 1 ]; then
	printf 'E\n' >> "$MACHINE"
	if build_jeu; then
		"$JEU/fous" --results "$MACHINE" --record --dir "$TDF_DIR" || true
	else
		printf '  (progression non enregistrée : le menu ne compile pas)\n'
	fi
fi
exit $RC
