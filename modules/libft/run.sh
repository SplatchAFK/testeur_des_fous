#!/bin/bash

HERE="$(cd "$(dirname "$0")" && pwd)"
ORIG_ARGS=("$@")
ROOT="${TDF_PROJET:-$(cd "$HERE/.." && pwd)}"
TDF_DIR="${TDF_NOM:-$(basename "$HERE")}"
TDF_ICI="$TDF_DIR"
[ -n "${TDF_NOM:-}" ] && TDF_ICI="$TDF_NOM/modules/libft"
SRC="$HERE/src"
MOT="$(cd "$HERE/../../moteur" && pwd)"
TDF_JOURNAL_MOD="libft"
. "$MOT/chargement.sh"
MENU_SEUL=0
DEDANS=0
case "$HERE/" in "$ROOT"/*) DEDANS=1 ;; esac
VERSION="$(sed -n 's/^# *define TDF_VERSION "\(.*\)"/\1/p' "$MOT/version.h" 2>/dev/null | head -1)"
[ -n "$VERSION" ] || VERSION="?"
CC="${CC:-cc}"
OS="$(uname -s 2>/dev/null)"
NORM_LATEST="3.3.60"
VERBOSE=0
KEEP=0
FORCEAUDIT=0
CI=0
FAST=0
TMO_CLI=""
NO_UPDATE=0
UPDATE_SET=""
VALG=0
SELECT=()
EXO=""
EXO_OUT=""
RECO=""
CFG_TIMEOUT=30
CFG_ASAN=1
CFG_UBSAN=1
CFG_VALGRIND=0
CFG_NORM=1
CFG_AUDIT=0
CFG_AIDE=2
NOLORE=0
DEBUG=0
if [ -f "$HOME/.testeur_des_fous/reglages.txt" ]; then
	while IFS='=' read -r k v; do
		case "$v" in ''|*[!0-9]*) continue ;; esac
		case "$k" in
			TIMEOUT|ASAN|UBSAN|VALGRIND|NORM|AUDIT|AIDE) printf -v "CFG_$k" '%s' "$v" ;;
		esac
	done < "$HOME/.testeur_des_fous/reglages.txt"
fi
TMO="${LIBFT_TIMEOUT:-$CFG_TIMEOUT}"
[ "$CFG_VALGRIND" = 1 ] && VALG=1
[ "$CFG_AUDIT" = 1 ] && FORCEAUDIT=1

usage() {
	cat <<EOF
Le testeur des fous - Version Libft v$VERSION
Créé et vibe-codé par Splatch (romasant)

  bash $TDF_DIR/run.sh libft                tout tester, combattre les boss, menu
  bash $TDF_DIR/run.sh libft split itoa     seulement ces fonctions
  bash $TDF_DIR/run.sh libft --ci           texte simple, sans interface
  bash $TDF_DIR/run.sh libft --exo 07       tester l'exercice d'entraînement 07
  bash $TDF_DIR/run.sh libft --audit        forcer l'audit moulinette
  bash $TDF_DIR/run.sh libft --valgrind     repasser les fonctions OK sous valgrind
  bash $TDF_DIR/run.sh libft -v             rapports complets des sanitizers (avec --ci)
  bash $TDF_DIR/run.sh libft -k             garder le dossier de build
  bash $TDF_DIR/run.sh libft --version      afficher le numéro de version
  bash $TDF_DIR/run.sh libft --reco FICHIER  se connecter avec une sauvegarde copiée (save.txt)
  bash $TDF_DIR/run.sh libft --no-lore       les tests seuls : ni boss, ni animation, ni lore
                                       (synonyme --lore-off ; la progression est enregistrée)
  bash $TDF_DIR/run.sh libft --debug         debug avancé : tous les rapports (fuites, mémoire, UB),
                                       valgrind et audit complet
  bash $TDF_DIR/run.sh libft --aide 0|1|2    0 : le cas seul, 1 : + la ligne, 2 : + un conseil
  bash $TDF_DIR/run.sh libft --fast          sans animation ni cinématique (synonyme --no-anim)
  bash $TDF_DIR/run.sh libft --timeout S     temps max par fonction, en secondes (défaut 30)
  bash $TDF_DIR/run.sh libft --no-update     ne pas chercher de mise à jour à ce lancement
  bash $TDF_DIR/run.sh libft --update off|on couper ou remettre la recherche de mise à jour

Dans le menu : Q, Échap ou Ctrl+C arrêtent un run ; Ctrl+C deux fois quitte tout.
EOF
}

while [ $# -gt 0 ]; do
	a="$1"
	shift
	case "$a" in
		-v) VERBOSE=1 ;;
		-k) KEEP=1 ;;
		--audit) FORCEAUDIT=1 ;;
		--ci) CI=1 ;;
		--fast|--no-anim) FAST=1 ;;
		--timeout) TMO_CLI="$1"; shift ;;
		--no-update) NO_UPDATE=1 ;;
		--update) UPDATE_SET="$1"; shift ;;
		--valgrind) VALG=1 ;;
		--exo) EXO="$1"; shift ;;
		--out) EXO_OUT="$1"; shift ;;
		--reco) RECO="$1"; shift ;;
		--no-lore|--lore-off) NOLORE=1; CI=1 ;;
		--debug) DEBUG=1; CI=1; VERBOSE=1; VALG=1; FORCEAUDIT=1 ;;
		--menu) MENU_SEUL=1 ;;
		--aide) CFG_AIDE="$1"; shift ;;
		-h|--help) usage; exit 0 ;;
		-V|--version) echo "Le testeur des fous v$VERSION"; exit 0 ;;
		-*) echo "option inconnue : $a (voir --help)"; exit 2 ;;
		*) a="${a%.c}"; SELECT+=("${a#ft_}") ;;
	esac
done
case "$TMO_CLI" in '') ;; *[!0-9]*|0) echo "--timeout : un nombre de secondes"; exit 2 ;; *) TMO="$TMO_CLI" ;; esac
if [ -n "$RECO" ]; then
	case "$RECO" in "~"|"~/"*) RECO="$HOME${RECO#"~"}" ;; esac
	[ -f "$RECO" ] || { echo "sauvegarde introuvable : $RECO"; exit 2; }
	RECO="$(cd "$(dirname "$RECO")" && pwd)/$(basename "$RECO")"
fi
[ -n "$EXO" ] || { [ -t 0 ] && [ -t 1 ]; } || CI=1
case "$CFG_AIDE" in 0|1|2) ;; *) echo "--aide : 0, 1 ou 2"; exit 2 ;; esac
[ "$DEBUG" = 1 ] && CFG_AIDE=2
if [ "$CI" = 1 ] && [ "$CFG_AIDE" = 0 ] && [ -z "${TDF_FILTRE:-}" ] && [ -z "$EXO" ]; then
	TDF_FILTRE=1 bash "$0" ${ORIG_ARGS[@]+"${ORIG_ARGS[@]}"} | sed -E 's/(\.c):[0-9]+(:[0-9]+)?/\1/g'
	exit "${PIPESTATUS[0]}"
fi
if [ -n "$RECO" ] && [ "$CI" = 1 ]; then
	echo "--reco a besoin d'un vrai terminal (sans --ci)"
	exit 2
fi

PART1="isalpha isdigit isalnum isascii isprint strlen memset bzero memcpy
memmove strlcpy strlcat toupper tolower strchr strrchr strncmp memchr memcmp
strnstr atoi calloc strdup"
PART2="substr strjoin strtrim split itoa strmapi striteri putchar_fd putstr_fd
putendl_fd putnbr_fd"
PART3="lstnew lstadd_front lstsize lstlast lstadd_back lstdelone lstclear
lstiter lstmap"
ALL=" $(echo $PART1 $PART2 $PART3) "
for s in ${SELECT[@]+"${SELECT[@]}"}; do
	case "$ALL" in *" $s "*) ;; *) echo "fonction inconnue : ft_$s"; exit 2 ;; esac
done

proto_of() {
	case "$1" in
		isalpha|isdigit|isalnum|isascii|isprint|toupper|tolower) echo "int ft_$1(int c);" ;;
		strlen) echo "size_t ft_strlen(const char *s);" ;;
		memset) echo "void *ft_memset(void *s, int c, size_t n);" ;;
		bzero) echo "void ft_bzero(void *s, size_t n);" ;;
		memcpy) echo "void *ft_memcpy(void *dest, const void *src, size_t n);" ;;
		memmove) echo "void *ft_memmove(void *dest, const void *src, size_t n);" ;;
		strlcpy) echo "size_t ft_strlcpy(char *dst, const char *src, size_t size);" ;;
		strlcat) echo "size_t ft_strlcat(char *dst, const char *src, size_t size);" ;;
		strchr) echo "char *ft_strchr(const char *s, int c);" ;;
		strrchr) echo "char *ft_strrchr(const char *s, int c);" ;;
		strncmp) echo "int ft_strncmp(const char *s1, const char *s2, size_t n);" ;;
		memchr) echo "void *ft_memchr(const void *s, int c, size_t n);" ;;
		memcmp) echo "int ft_memcmp(const void *s1, const void *s2, size_t n);" ;;
		strnstr) echo "char *ft_strnstr(const char *big, const char *little, size_t len);" ;;
		atoi) echo "int ft_atoi(const char *nptr);" ;;
		calloc) echo "void *ft_calloc(size_t nmemb, size_t size);" ;;
		strdup) echo "char *ft_strdup(const char *s);" ;;
		substr) echo "char *ft_substr(char const *s, unsigned int start, size_t len);" ;;
		strjoin) echo "char *ft_strjoin(char const *s1, char const *s2);" ;;
		strtrim) echo "char *ft_strtrim(char const *s1, char const *set);" ;;
		split) echo "char **ft_split(char const *s, char c);" ;;
		itoa) echo "char *ft_itoa(int n);" ;;
		strmapi) echo "char *ft_strmapi(char const *s, char (*f)(unsigned int, char));" ;;
		striteri) echo "void ft_striteri(char *s, void (*f)(unsigned int, char*));" ;;
		putchar_fd) echo "void ft_putchar_fd(char c, int fd);" ;;
		putstr_fd) echo "void ft_putstr_fd(char *s, int fd);" ;;
		putendl_fd) echo "void ft_putendl_fd(char *s, int fd);" ;;
		putnbr_fd) echo "void ft_putnbr_fd(int n, int fd);" ;;
		lstnew) echo "t_list *ft_lstnew(void *content);" ;;
		lstadd_front) echo "void ft_lstadd_front(t_list **lst, t_list *new);" ;;
		lstsize) echo "int ft_lstsize(t_list *lst); (ou unsigned int)" ;;
		lstlast) echo "t_list *ft_lstlast(t_list *lst);" ;;
		lstadd_back) echo "void ft_lstadd_back(t_list **lst, t_list *new);" ;;
		lstdelone) echo "void ft_lstdelone(t_list *lst, void (*del)(void *));" ;;
		lstclear) echo "void ft_lstclear(t_list **lst, void (*del)(void *));" ;;
		lstiter) echo "void ft_lstiter(t_list *lst, void (*f)(void *));" ;;
		lstmap) echo "t_list *ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *));" ;;
	esac
}

set_colors() {
	if [ -z "$NO_COLOR" ] && [ -t 1 ]; then
		R=$'\e[0m'; B=$'\e[1m'; D=$'\e[38;5;245m'; ACC=$'\e[38;5;213m'
		TXT=$'\e[38;5;252m'
		P_OK=$'\e[30;48;5;78m'; P_KO=$'\e[30;48;5;203m'; P_CR=$'\e[97;48;5;125m'
		P_LK=$'\e[30;48;5;221m'; P_UB=$'\e[30;48;5;215m'; P_TM=$'\e[30;48;5;141m'
		P_BD=$'\e[30;48;5;117m'; P_SK=$'\e[38;5;245;48;5;236m'; P_WN=$'\e[30;48;5;221m'
		F_OK=$'\e[38;5;78m'; F_KO=$'\e[38;5;203m'; F_LK=$'\e[38;5;221m'
	else
		R=; B=; D=; ACC=; TXT=; P_OK=; P_KO=; P_CR=; P_LK=; P_UB=; P_TM=; P_BD=
		P_WN=; P_SK=; F_OK=; F_KO=; F_LK=
	fi
}

say() { [ "$CI" = 1 ] && printf "$@"; }

rule() { local i; for ((i = 0; i < $1; i++)); do printf '─'; done; }

header() {
	[ "$CI" = 1 ] || return
	local w=58
	printf '\n  %s╭' "$ACC"; rule $w; printf '╮%s\n' "$R"
	printf '  %s│%s  %sLE TESTEUR DES FOUS%s %s· Version Libft v%s%s%*s%s│%s\n' "$ACC" "$R" "$B" "$R" "$TXT" "$VERSION" "$R" $((w - 39 - ${#VERSION})) '' "$ACC" "$R"
	printf '  %s│%s  %scréé et vibe-codé par Splatch (romasant)%s%*s%s│%s\n' "$ACC" "$R" "$D" "$R" $((w - 42)) '' "$ACC" "$R"
	printf '  %s╰' "$ACC"; rule $w; printf '╯%s\n' "$R"
}

section() {
	[ "$CI" = 1 ] || return
	printf '\n  %s%s%s %s' "$ACC" "$B" "$1" "$D"; rule $((56 - $(printf '%s' "$1" | wc -m))); printf '%s\n' "$R"
}

pill() { printf '%s %-6s %s' "$1" "$2" "$R"; }

TMPROOT="${TMPDIR:-/tmp}"
BUILD="$(mktemp -d "${TMPROOT%/}/testeur_des_fous.XXXXXX")"
RES="$BUILD/results"
: > "$RES"
mkdir -p "$BUILD/obj" "$BUILD/bin" "$BUILD/log" "$BUILD/plain" "$BUILD/proto" "$BUILD/vg"

CACHE="$HERE/.build/cache"
{ [ "${TDF_CACHE:-1}" = 1 ] && mkdir -p "$CACHE" 2>/dev/null; } || CACHE=""
fingerprint() {
	{ printf '%s|%s\n' "$OS" "$CC"; cat "$@" 2>/dev/null; } | cksum | cut -d' ' -f1
}

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

cleanup() {
	[ -n "$TPID" ] && kill "$TPID" 2>/dev/null && wait "$TPID" 2>/dev/null
	[ -n "$NPID" ] && kill_tree "$NPID"
	[ "$KEEP" = 1 ] || rm -rf "$BUILD"
}
trap cleanup EXIT

emit() { printf '%s\n' "$*" | tr -d '\r' >> "$RES"; }

HAVE_WAITN=0
if [ "${BASH_VERSINFO[0]}" -gt 4 ] || { [ "${BASH_VERSINFO[0]}" = 4 ] && [ "${BASH_VERSINFO[1]}" -ge 3 ]; }; then
	HAVE_WAITN=1
fi
JOBS="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"
case "$JOBS" in ''|*[!0-9]*) JOBS=4 ;; esac
wait_slot() {
	while [ "$(jobs -rp | wc -l)" -ge "$JOBS" ]; do
		if [ "$HAVE_WAITN" = 1 ]; then wait -n; else sleep 0.05; fi
	done
}

TOCMD=""
command -v timeout >/dev/null 2>&1 && TOCMD="timeout"
[ -z "$TOCMD" ] && command -v gtimeout >/dev/null 2>&1 && TOCMD="gtimeout"
with_timeout() {
	local s="$1"
	shift
	if [ -n "$TOCMD" ]; then "$TOCMD" "$s" "$@"; else "$@"; fi
}

SANS=""
[ "$CFG_ASAN" = 1 ] && SANS="address"
[ "$CFG_UBSAN" = 1 ] && SANS="${SANS:+$SANS,}undefined"
san_ok() { echo 'int main(void){return 0;}' | $CC -x c - -fsanitize="$1" -o "$BUILD/san_probe" 2>/dev/null && "$BUILD/san_probe" 2>/dev/null; }
if [ -n "$SANS" ] && ! san_ok "$SANS"; then
	SANS_OLD="$SANS"
	if [ "$SANS" != address ] && san_ok address; then SANS=address; else SANS=""; fi
	emit "G	sanitizers indisponibles ($SANS_OLD) : repli sur ${SANS:-aucun} (le traqueur mémoire du testeur reste actif)"
fi
SAN=""
[ -n "$SANS" ] && SAN="-fsanitize=$SANS -fno-sanitize-recover=all -fno-omit-frame-pointer"
LDX=""
[ "$OS" = Darwin ] && LDX="-Wl,-U,___sanitizer_set_death_callback"
HOOK=(-include "$SRC/tdf_hook.h")

wanted() {
	[ ${#SELECT[@]} -eq 0 ] && return 0
	local s; for s in "${SELECT[@]}"; do [ "$s" = "$1" ] && return 0; done
	return 1
}

compile_obj() {
	local f="$1" b
	b="$(basename "$f" .c)"
	$CC -Wall -Wextra -Werror -g $SAN "${HOOK[@]}" -I "$ROOT" -c "$f" -o "$BUILD/obj/$b.o" \
		2> "$BUILD/obj/$b.err" || rm -f "$BUILD/obj/$b.o"
}

compile_plain() {
	local f="$1" b
	b="$(basename "$f" .c)"
	$CC -Wall -Wextra -Werror -g -I "$ROOT" -c "$f" -o "$BUILD/plain/$b.o" \
		2> /dev/null || rm -f "$BUILD/plain/$b.o"
}

compile_vg() {
	local f="$1" b
	b="$(basename "$f" .c)"
	$CC -Wall -Wextra -Werror -g "${HOOK[@]}" -I "$ROOT" -c "$f" -o "$BUILD/vg/$b.o" \
		2> /dev/null || rm -f "$BUILD/vg/$b.o"
}

make_archive() {
	local dir="$1" out="$2" objs=() o
	for o in "$dir"/*.o; do [ -f "$o" ] && objs+=("$o"); done
	rm -f "$out"
	[ ${#objs[@]} -gt 0 ] && ar rcs "$out" "${objs[@]}" 2>/dev/null
}

TESTS_OBJ=""
tests_objs_prepare() {
	local key
	TESTS_OBJ=""
	[ -n "$CACHE" ] || return 0
	key=$(fingerprint "$SRC/tests.c" "$SRC"/*.h "$MOT"/*.h "$ROOT/libft.h" <(echo "$SAN $LDX"))
	TESTS_OBJ="$CACHE/tests.$key"
	if [ ! -d "$TESTS_OBJ" ]; then
		rm -rf "$CACHE"/tests.*
		mkdir -p "$TESTS_OBJ" || TESTS_OBJ=""
	fi
}

build_bin() {
	local n="$1"
	if [ -n "$TESTS_OBJ" ]; then
		[ -f "$TESTS_OBJ/$n.o" ] || { $CC -g -Wno-unused-function -Wno-unused-variable -Wno-format-security \
			-Werror=implicit-function-declaration $SAN -DT_"$n" -I "$ROOT" -c "$SRC/tests.c" \
			-o "$TESTS_OBJ/$n.o.part" 2> "$BUILD/bin/$n.err" && mv "$TESTS_OBJ/$n.o.part" "$TESTS_OBJ/$n.o"; }
		[ -f "$TESTS_OBJ/$n.o" ] && $CC -g $SAN "$TESTS_OBJ/$n.o" "$BUILD/libtest.a" $LDX -o "$BUILD/bin/$n" \
			2> "$BUILD/bin/$n.err" && return 0
	fi
	$CC -g -Wno-unused-function -Wno-unused-variable -Wno-format-security \
		-Werror=implicit-function-declaration $SAN -DT_"$n" -I "$ROOT" \
		"$SRC/tests.c" "$BUILD/libtest.a" $LDX -o "$BUILD/bin/$n" \
		2> "$BUILD/bin/$n.err" || rm -f "$BUILD/bin/$n"
}

build_game() {
	local objs=() o f pids=() n=0 nft=0
	local GSRC=("$SRC"/main.c "$SRC"/screens.c "$MOT"/ui.c "$SRC"/train.c "$SRC"/scene.c
		"$MOT"/save.c "$MOT"/data.c "$MOT"/avatar.c "$MOT"/art.c "$MOT"/term.c
		"$MOT"/font.c "$MOT"/menu.c "$MOT"/stats.c "$SRC"/lib.c "$MOT"/hub.c "$MOT"/images.c "$MOT"/cine.c "$SRC"/story.c)
	local GX="" GOBJ="" key g ok=1
	for f in "$ROOT"/ft_*.c; do [ -f "$f" ] && nft=$((nft + 1)); done
	tdf_ch_debut "préparation" $((nft + ${#GSRC[@]} + 1))
	for f in "$ROOT"/ft_*.c; do
		[ -f "$f" ] || continue
		compile_plain "$f" &
		pids+=($!)
	done
	for o in ${pids[@]+"${pids[@]}"}; do
		wait "$o"
		n=$((n + 1))
		tdf_ch_pas "tes fonctions $n/$nft"
	done
	for o in "$BUILD"/plain/*.o; do [ -f "$o" ] && objs+=("$o"); done
	[ "$OS" = Darwin ] && GX="-Wl,-undefined,dynamic_lookup"
	if [ -n "$CACHE" ]; then
		key=$(fingerprint "${GSRC[@]}" "$SRC"/*.h "$MOT"/*.h)
		GOBJ="$CACHE/jeu.$key"
		if [ ! -f "$GOBJ/ok" ]; then
			rm -rf "$CACHE"/jeu.*
			mkdir -p "$GOBJ"
			pids=()
			for g in "${GSRC[@]}"; do
				$CC -O2 -w -I "$SRC" -I "$MOT" -c "$g" -o "$GOBJ/$(basename "$g" .c).o" 2>> "$BUILD/game.err" &
				pids+=($!)
			done
			n=0
			for o in "${pids[@]}"; do
				wait "$o"
				n=$((n + 1))
				tdf_ch_pas "interface $n/${#GSRC[@]}"
			done
			for g in "${GSRC[@]}"; do [ -f "$GOBJ/$(basename "$g" .c).o" ] || ok=0; done
			if [ "$ok" = 1 ]; then : > "$GOBJ/ok"; else GOBJ=""; fi
		else
			tdf_ch_pas "interface en cache" "${#GSRC[@]}"
		fi
	fi
	if [ -n "$GOBJ" ]; then
		$CC "$GOBJ"/*.o ${objs[@]+"${objs[@]}"} -lm $GX -o "$BUILD/fous" 2> "$BUILD/game.err" && { tdf_ch_fin; return 0; }
		$CC "$GOBJ"/*.o ${objs[@]+"${objs[@]}"} -lm $GX -Wl,--allow-multiple-definition -o "$BUILD/fous" \
			2>> "$BUILD/game.err" && { tdf_ch_fin; return 0; }
	fi
	tdf_ch_fin "secours"
	$CC -O2 -w -I "$SRC" -I "$MOT" "${GSRC[@]}" ${objs[@]+"${objs[@]}"} -lm $GX -o "$BUILD/fous" \
		2> "$BUILD/game.err" && return 0
	$CC -O2 -w -I "$SRC" -I "$MOT" "${GSRC[@]}" ${objs[@]+"${objs[@]}"} -lm $GX \
		-Wl,--allow-multiple-definition -o "$BUILD/fous" 2>> "$BUILD/game.err"
}

check_proto() {
	local n="$1" out decl want
	if [ ! -f "$ROOT/libft.h" ]; then
		echo "missing	libft.h introuvable" > "$BUILD/proto/$n"; return
	fi
	want="$(proto_of "$n")"
	out="$($CC -fsyntax-only -DF_"$n" -I "$ROOT" "$SRC/protos.c" 2>&1)"
	if [ -z "$out" ]; then
		echo "ok" > "$BUILD/proto/$n"
	elif echo "$out" | grep -qE "undeclared|implicit"; then
		echo "missing	absent de libft.h, attendu : $want" > "$BUILD/proto/$n"
	else
		decl="$(grep -m1 -E "ft_$n[[:space:]]*\(" "$ROOT/libft.h" | tr -s '\t ' ' ' | sed 's/^ //')"
		echo "ko	trouvé : $decl   attendu : $want" > "$BUILD/proto/$n"
	fi
}

DET=()
det() { DET+=("$(printf '%s' "$1" | tr '\t' ' ')"); }

report() {
	local st="$1" n="$2" c="$3" ms="$4" t l
	local line="T	$n	$st	${c:-0}	$ms"
	for l in ${DET[@]+"${DET[@]}"}; do line+="	$l"; done
	emit "$line"
	[ "$CI" = 1 ] || return
	t="$(printf '%d.%02ds' $((ms / 1000)) $((ms % 1000 / 10)))"
	n_seen=$((n_seen + 1))
	printf '  %s%2d/%d%s ' "$D" "$n_seen" "$N_TOTAL" "$R"
	case "$st" in
		ok)    pill "$P_OK" OK ;;
		warn)  pill "$P_WN" NOTE ;;
		ko)    pill "$P_KO" FAUX ;;
		crash) pill "$P_CR" CRASH ;;
		leak)  pill "$P_LK" FUITE ;;
		ub)    pill "$P_UB" UB ;;
		time)  pill "$P_TM" BOUCLE ;;
		build) pill "$P_BD" COMPIL ;;
		skip)  pill "$P_SK" '--' ;;
	esac
	if [ "$st" = skip ]; then
		printf '  %s%-18s pas encore écrite%s\n' "$D" "ft_$n" "$R"
	else
		printf '  %s%-18s%s %s%6s contrôles%s   %s%s%s\n' "$TXT" "ft_$n" "$R" "$D" "${c:--}" "$R" "$D" "$t" "$R"
	fi
	for l in ${DET[@]+"${DET[@]}"}; do printf '            %s%s%s\n' "$D" "$l" "$R"; done
}

where() {
	awk '
	/ERROR: (Address|Leak)Sanitizer/ { sec = "à"; got = 0 }
	/^(READ|WRITE) of size/ { sec = "à"; got = 0 }
	/allocated by thread/ { sec = "alloué à"; got = 0 }
	/freed by thread/ { sec = "libéré à"; got = 0 }
	/^ *#[0-9]+ 0x[0-9a-f]+ in _?ft_[a-z_0-9]+ / {
		if (!got) {
			path = $5; sub(/.*\//, "", path); fn = $4; sub(/^_/, "", fn)
			print sec " " path " (" fn ")"; got = 1
		}
	}' "$1" | awk '!seen[$0]++' | head -4
}

fullreport() { [ "$VERBOSE" = 1 ] && [ "$CI" = 1 ] && sed 's/^/             /' "$1" | head -45; }

n_ok=0; n_bad=0; n_skip=0; n_warn=0; n_seen=0
N_TOTAL=43
[ ${#SELECT[@]} -gt 0 ] && N_TOTAL=${#SELECT[@]}

valgrind_one() {
	local n="$1" out
	command -v valgrind >/dev/null 2>&1 || return 0
	[ -f "$BUILD/libvg.a" ] || return 0
	$CC -g -w -DT_"$n" -I "$ROOT" "$SRC/tests.c" "$BUILD/libvg.a" \
		-o "$BUILD/vg/$n" 2>/dev/null || return 0
	TDF_TIMEOUT=$((TMO * 4)) with_timeout $((TMO * 4 + 5)) valgrind -q --leak-check=full \
		--errors-for-leak-kinds=definite,indirect --error-exitcode=42 \
		--log-file="$BUILD/vg/$n.log" "$BUILD/vg/$n" > /dev/null 2>&1
	[ $? = 42 ] || return 0
	out="$(grep -m1 -E 'definitely|indirectly|Invalid|uninitialised' "$BUILD/vg/$n.log" | sed 's/^==[0-9]*== //')"
	det "valgrind : ${out:-erreur mémoire}"
	return 1
}

run_one() {
	local n="$1" bin="$BUILD/bin/$n" code marker checks fails warns ms l
	local san="$BUILD/san_$n" out="$BUILD/out_$n" casec caset leakl dep
	DET=()

	if [ ! -f "$ROOT/ft_$n.c" ]; then
		report skip "$n" "" 0; n_skip=$((n_skip + 1)); return
	fi
	if [ ! -f "$BUILD/obj/ft_$n.o" ]; then
		while IFS= read -r l; do det "$l"; done < <(grep -E 'error|warning' "$BUILD/obj/ft_$n.err" | sed -E 's/^.*\/(ft_|libft\.h)/\1/' | head -4)
		report build "$n" "" 0; n_bad=$((n_bad + 1)); return
	fi
	if [ ! -f "$bin" ]; then
		dep="$(grep -oE "undefined (reference|symbol)[^\`']*[\`']_?ft_[a-z_0-9]+" "$BUILD/bin/$n.err" | grep -oE 'ft_[a-z_0-9]+' | sort -u | tr '\n' ' ')"
		[ -n "$dep" ] && det "ta fonction appelle ${dep}qui manque ou ne compile pas"
		while IFS= read -r l; do det "$l"; done < <(grep -E 'error|multiple definition|first defined|cannot find' "$BUILD/bin/$n.err" | grep -v 'ld returned' | sed -E 's/^.*\/(ft_|libft\.h)/\1/' | head -3)
		grep -q 'implicit' "$BUILD/bin/$n.err" && det "piste : prototype absent de libft.h ?"
		report build "$n" "" 0; n_bad=$((n_bad + 1)); return
	fi

	rm -f "$BUILD"/log/*
	TDF_TIMEOUT="$TMO" with_timeout $((TMO + 5)) "$bin" > "$out" 2> "$BUILD/err_$n"
	code=$?
	cat "$BUILD"/log/san.* "$BUILD/err_$n" > "$san" 2>/dev/null
	marker="$(grep '^@@ ' "$out" | tail -1)"
	checks="$(echo "$marker" | awk '{print $2}')"
	fails="$(echo "$marker" | awk '{print $3}')"
	warns="$(echo "$marker" | awk '{print $4}')"
	ms="$(echo "$marker" | awk '{print $5}')"
	case "$ms" in ''|*[!0-9]*) ms=0 ;; esac
	casec="$(grep -m1 '^@@C ' "$out" | cut -c5-)"
	caset="$(grep -m1 '^@@T ' "$out" | cut -c5-)"
	leakl="$(grep -m1 '^@@L ' "$out")"

	if [ "$code" = 124 ] || [ -n "$caset" ]; then
		det "aucune réponse après $TMO s : boucle infinie ?"
		[ -n "$caset" ] && det "pendant : $caset"
		report time "$n" "" "$((TMO * 1000))"; n_bad=$((n_bad + 1))
	elif grep -q 'runtime error' "$san"; then
		while IFS= read -r l; do det "$l"; done < <(grep -m1 'runtime error' "$san" | sed -E 's/^.*\/(ft_)/\1/')
		[ -n "$casec" ] && det "pendant : $casec"
		report ub "$n" "$checks" "$ms"; n_bad=$((n_bad + 1)); fullreport "$san"
	elif grep -q 'ERROR: AddressSanitizer' "$san"; then
		det "$(grep -m1 'ERROR: AddressSanitizer' "$san" | sed -E 's/.*AddressSanitizer: ([^ ]+).*/\1/')"
		[ -n "$casec" ] && det "pendant : $casec"
		while IFS= read -r l; do det "$l"; done < <(where "$san")
		while IFS= read -r l; do det "$l"; done < <(grep -m1 -E 'is located .* (after|before|inside)' "$san" | sed -E 's/^ *//')
		report crash "$n" "$checks" "$ms"; n_bad=$((n_bad + 1)); fullreport "$san"
	elif [ -n "$casec" ]; then
		det "plantage (code $code) pendant : $casec"
		report crash "$n" "$checks" "$ms"; n_bad=$((n_bad + 1))
	elif [ -n "$fails" ] && [ "$fails" -gt 0 ]; then
		while IFS= read -r l; do det "$l"; done < <(grep '^  ko| ' "$out" | sed 's/^  ko| //' | head -5)
		[ "$fails" -gt 5 ] && det "... et $((fails - 5)) autres contrôles ratés sur $fails"
		[ -n "$leakl" ] && det "(fuite mémoire aussi : corrige d'abord la logique)"
		report ko "$n" "$checks" "$ms"; n_bad=$((n_bad + 1))
	elif [ -n "$leakl" ]; then
		det "$(echo "$leakl" | awk '{print $2 " bloc(s), " $3 " octets jamais libérés"}')"
		while IFS= read -r l; do det "$l"; done < <(grep '^  lk| ' "$out" | sed 's/^  lk| //' | head -3)
		report leak "$n" "$checks" "$ms"; n_bad=$((n_bad + 1))
	elif [ "$code" = 0 ] && [ -n "$marker" ]; then
		if [ "$VALG" = 1 ] && ! valgrind_one "$n"; then
			report leak "$n" "$checks" "$ms"; n_bad=$((n_bad + 1))
		elif [ -n "$warns" ] && [ "$warns" -gt 0 ]; then
			while IFS= read -r l; do det "$l"; done < <(grep '^  wn| ' "$out" | sed 's/^  wn| //' | head -3)
			[ "$warns" -gt 3 ] && det "... $warns avertissements en tout"
			report warn "$n" "$checks" "$ms"; n_ok=$((n_ok + 1)); n_warn=$((n_warn + 1))
		else
			report ok "$n" "$checks" "$ms"; n_ok=$((n_ok + 1))
		fi
	else
		det "sortie avec le code $code avant la fin du test"
		while IFS= read -r l; do det "$l"; done < <(head -3 "$BUILD/err_$n")
		report crash "$n" "$checks" "$ms"; n_bad=$((n_bad + 1))
	fi
}

n_audit_ko=0
audit_line() {
	local st="$1" label="$2" detail="$3"
	emit "A	$st	$label	$detail"
	[ "$st" = ko ] && n_audit_ko=$((n_audit_ko + 1))
	[ "$CI" = 1 ] || return
	printf '  '
	case "$st" in
		ok)   pill "$P_OK" OK ;;
		ko)   pill "$P_KO" FAUX ;;
		warn) pill "$P_WN" WARN ;;
	esac
	printf '  %s%s%s\n' "$TXT" "$label" "$R"
	[ -n "$detail" ] && printf '            %s%s%s\n' "$D" "$detail" "$R"
}

undef_syms() {
	nm -u "$@" 2>/dev/null | awk 'NF >= 1 && $NF !~ /:$/ {print $NF}' \
		| sed -e 's/^_\(.*\)/\1/' | sort -u
}
def_syms() {
	nm "$@" 2>/dev/null | awk 'NF == 3 && $2 ~ /^[TDBRSCtdbrsc]$/ {print $3}' \
		| sed -e 's/^_\(.*\)/\1/' | sort -u
}
[ "$OS" = Darwin ] || {
	undef_syms() {
		nm -u "$@" 2>/dev/null | awk 'NF >= 1 && $NF !~ /:$/ {print $NF}' | sort -u
	}
	def_syms() {
		nm --defined-only "$@" 2>/dev/null | awk 'NF == 3 {print $3}' | sort -u
	}
}
LIBC_OK='^(malloc|free|write|_GLOBAL_OFFSET_TABLE_|__stack_chk_fail|__stack_chk_guard|dyld_stub_binder)$'

NORM_CMD=""
if command -v norminette >/dev/null 2>&1; then NORM_CMD="norminette"
elif python3 -m norminette --version >/dev/null 2>&1; then NORM_CMD="python3 -m norminette"
fi

ver_lt() {
	awk -v a="$1" -v b="$2" 'BEGIN {
		na = split(a, x, "."); nb = split(b, y, ".")
		for (i = 1; i <= 3; i++) { if (x[i] + 0 < y[i] + 0) exit 0; if (x[i] + 0 > y[i] + 0) exit 1 }
		exit 1 }'
}

start_norm() {
	[ "$CFG_NORM" = 1 ] && [ -n "$NORM_CMD" ] || return
	( cd "$ROOT" && $NORM_CMD ft_*.c libft.h 2>&1 | sed $'s/\x1b\\[[0-9;]*m//g' > "$BUILD/norm.out"; echo done > "$BUILD/norm.done" ) &
	NPID=$!
}

n_norm_ko=0
finish_norm() {
	local v nerr nfile files l
	section "Norme · norminette"
	if [ "$CFG_NORM" != 1 ]; then
		audit_line warn "norminette désactivée dans les paramètres" "la moulinette, elle, la lance toujours"
		return
	fi
	if [ -z "$NORM_CMD" ]; then
		audit_line warn "norminette introuvable : la Norme n'est PAS vérifiée" "installe-la : python3 -m pip install --user norminette"
		emit "G	norminette introuvable : la Norme n'est pas vérifiée (python3 -m pip install --user norminette)"
		return
	fi
	[ -n "$NPID" ] && wait "$NPID" 2>/dev/null
	NPID=""
	v="$($NORM_CMD --version 2>/dev/null | grep -oE 'norminette [0-9]+\.[0-9]+\.[0-9]+' | head -1 | grep -oE '[0-9.]+$')"
	if [ -n "$v" ] && ver_lt "$v" "$NORM_LATEST"; then
		audit_line warn "norminette $v : version plus ancienne que $NORM_LATEST" "mets-la à jour : python3 -m pip install --user -U norminette"
		emit "G	norminette $v installée, la dernière connue est $NORM_LATEST : mets-la à jour"
	fi
	[ -f "$BUILD/norm.out" ] || { audit_line warn "norminette n'a rien renvoyé"; return; }
	nerr="$(grep -c '^Error' "$BUILD/norm.out")"
	files="$(grep -E ': Error!$' "$BUILD/norm.out" | sed 's/: Error!$//' | tr '\n' ' ')"
	nfile="$(grep -cE ': Error!$' "$BUILD/norm.out")"
	if [ "$nerr" = 0 ] && [ "$nfile" = 0 ]; then
		audit_line ok "norminette ${v:-?} : aucune erreur (ft_*.c et libft.h)"
	else
		n_norm_ko=1
		audit_line ko "norminette ${v:-?} : $nerr erreur(s) dans $nfile fichier(s)" "$(echo "$files" | cut -c1-90)"
		if [ "$CI" = 1 ]; then
			grep -E ': Error!$|^Error' "$BUILD/norm.out" | head -12 | while IFS= read -r l; do
				printf '            %s%s%s\n' "$D" "$(printf '%s' "$l" | tr '\t' ' ' | cut -c1-100)" "$R"
			done
		fi
	fi
}

run_audit() {
	local A="$BUILD/audit" out nsrc nobj bad extra key mst mlabel mdetail
	rm -rf "$A"; mkdir -p "$A"
	cp "$ROOT"/ft_*.c "$ROOT"/libft.h "$ROOT"/Makefile "$A"/ 2>/dev/null
	emit "S	audit moulinette"
	section "Audit · ce que la moulinette regarde"

	if [ ! -f "$ROOT/Makefile" ]; then
		audit_line ko "Makefile à la racine"; emit "AD"; return
	fi
	audit_line ok "Makefile à la racine"

	bad=""
	for r in all clean fclean re; do
		grep -qE "^$r[[:space:]]*:" "$ROOT/Makefile" || bad+="$r "
	done
	grep -qE '^(\$\(NAME\)|\$\{NAME\}|libft\.a)[[:space:]]*:' "$ROOT/Makefile" || bad+='$(NAME) '
	if [ -z "$bad" ]; then audit_line ok "règles \$(NAME), all, clean, fclean, re"
	else audit_line ko "règles \$(NAME), all, clean, fclean, re" "manquantes : $bad"; fi

	if grep -q -- '-Wall' "$ROOT/Makefile" && grep -q -- '-Wextra' "$ROOT/Makefile" \
		&& grep -q -- '-Werror' "$ROOT/Makefile"; then
		audit_line ok "flags -Wall -Wextra -Werror"
	else audit_line ko "flags -Wall -Wextra -Werror"; fi

	if grep -q -- '-std=c99' "$ROOT/Makefile"; then
		audit_line ko "pas de -std=c99 (interdit par le sujet)"
	else audit_line ok "pas de -std=c99 (interdit par le sujet)"; fi

	if grep -qE 'wildcard|\*\.c|\*\.o' "$ROOT/Makefile"; then
		audit_line warn "sources nommées une par une (Norme v4.1 III.11)" "wildcard ou *.c trouvé"
	else audit_line ok "sources nommées une par une (Norme v4.1 III.11)"; fi

	if grep -q 'libtool' "$ROOT/Makefile"; then
		audit_line ko "ar et pas libtool (sujet 4.1)"
	else audit_line ok "ar et pas libtool (sujet 4.1)"; fi

	out="$(cd "$A" && make 2>&1)"
	if [ ! -f "$A/libft.a" ]; then
		audit_line ko "make crée libft.a à la racine" "$(echo "$out" | grep -m2 -E '\*\*\*|No rule|error:' | cut -c1-90 | tr '\n' ' ')"
		emit "AD"
		return
	fi
	audit_line ok "make crée libft.a à la racine"

	key=$(fingerprint "$ROOT/Makefile" "$ROOT/libft.h" <(cd "$ROOT" && ls ft_*.c 2>/dev/null))
	if [ -n "$CACHE" ] && [ -f "$CACHE/makefile.$key" ]; then
		while IFS=$'\t' read -r mst mlabel mdetail; do
			audit_line "$mst" "$mlabel" "$mdetail"
		done < "$CACHE/makefile.$key"
	else
		: > "$BUILD/makefile.rec"
		mk_line() {
			audit_line "$@"
			printf '%s\t%s\t%s\n' "$1" "$2" "${3:-}" >> "$BUILD/makefile.rec"
		}
		out="$(cd "$A" && make 2>&1)"
		if echo "$out" | grep -qE '(^|[[:space:]])(cc|gcc|clang|ar)[[:space:]]'; then
			mk_line ko "un 2e make ne relink pas" "$(echo "$out" | head -1 | cut -c1-90)"
		else mk_line ok "un 2e make ne relink pas"; fi

		sleep 1
		touch "$A/libft.h"
		out="$(cd "$A" && make 2>&1)"
		if echo "$out" | grep -qE '(^|[[:space:]])(cc|gcc|clang)[[:space:]]'; then
			mk_line ok "modifier libft.h recompile les .o"
		else
			mk_line warn "modifier libft.h recompile les .o" "les .o ne dépendent pas de libft.h"
		fi

		(cd "$A" && make fclean >/dev/null 2>&1)
		if ls "$A"/*.o "$A"/libft.a >/dev/null 2>&1; then
			mk_line ko "fclean supprime les .o et libft.a"
		else mk_line ok "fclean supprime les .o et libft.a"; fi
		(cd "$A" && make re >/dev/null 2>&1)
		if [ -f "$A/libft.a" ]; then mk_line ok "re reconstruit tout"
		else mk_line ko "re reconstruit tout"; fi
		if [ -n "$CACHE" ]; then
			rm -f "$CACHE"/makefile.*
			cp "$BUILD/makefile.rec" "$CACHE/makefile.$key"
		fi
	fi
	[ -f "$A/libft.a" ] || (cd "$A" && make >/dev/null 2>&1)

	nsrc=$(ls "$A"/ft_*.c 2>/dev/null | wc -l | tr -d ' ')
	nobj=$(ar t "$A/libft.a" 2>/dev/null | grep -c '\.o$')
	if [ "$nsrc" = "$nobj" ]; then
		audit_line ok "libft.a contient chaque ft_*.c ($nobj objets)"
	else audit_line ko "libft.a contient chaque ft_*.c" "$nsrc sources, $nobj objets"; fi

	def_syms "$A/libft.a" > "$A/defined.txt"
	extra="$(undef_syms "$A/libft.a" | comm -23 - "$A/defined.txt" | grep -vE "$LIBC_OK")"
	if [ -z "$extra" ]; then
		audit_line ok "seuls malloc, free et write viennent de la libc"
	else
		audit_line ko "seuls malloc, free et write viennent de la libc" "interdit : $(echo $extra | tr '\n' ' ')"
	fi

	if ! grep -qE '^#[[:space:]]*ifndef' "$ROOT/libft.h"; then
		audit_line ko "libft.h : garde d'inclusion"
	else audit_line ok "libft.h : garde d'inclusion"; fi

	(cd "$A" && make >/dev/null 2>&1)
	extra="$(nm "$A/libft.a" 2>/dev/null | awk '$2 ~ /^[BDCG]$/ {print $3}' | sort -u | tr '\n' ' ')"
	if [ -n "$extra" ]; then
		audit_line ko "aucune variable globale (sujet 4.1)" "trouvé : $extra"
	else audit_line ok "aucune variable globale (sujet 4.1)"; fi

	bad="$(list_extra_files)"
	if [ -z "$bad" ]; then
		audit_line ok "aucun fichier en trop dans le dépôt"
	else
		audit_line ko "aucun fichier en trop dans le dépôt (sujet : interdit)" "$(echo $bad | cut -c1-100)"
	fi

	if [ ! -f "$ROOT/README.md" ]; then
		audit_line ko "README.md à la racine (sujet chapitre 5)"
	elif head -1 "$ROOT/README.md" | grep -q "This activity has been created as part of the 42 curriculum by"; then
		audit_line ok "README.md : 1re ligne imposée par le sujet"
	else
		audit_line warn "README.md : 1re ligne imposée par le sujet" "attendu : *This activity has been created as part of the 42 curriculum by <login>.*"
	fi
	emit "AD"
}

list_extra_files() {
	local f
	if (cd "$ROOT" && git rev-parse --is-inside-work-tree >/dev/null 2>&1); then
		(cd "$ROOT" && git ls-files) | sed 's#/.*##' | sort -u \
			| grep -vE '^(ft_[a-z_0-9]+\.c|libft\.h|Makefile|README\.md|\.gitignore)$'
	else
		while IFS= read -r f; do
			case "$f" in
				ft_*.c|libft.h|Makefile|README.md|.gitignore|"$TDF_DIR"|*.o|libft.a) ;;
				*) echo "$f" ;;
			esac
		done < <(cd "$ROOT" && ls -A)
	fi
}

run_all() {
	local n f p st msg part list

	start_norm
	if [ "$DEDANS" = 1 ] && (cd "$ROOT" && [ -n "$(git ls-files "$TDF_DIR" 2>/dev/null | head -1)" ]); then
		emit "G	$TDF_DIR/ est suivi par git : retire-le avant de rendre (git rm -r --cached $TDF_DIR)"
	fi
	if [ "$DEDANS" = 1 ] && [ -d "$ROOT/.git" ] && ! grep -qs "$TDF_DIR" "$ROOT/.gitignore"; then
		emit "G	ajoute $TDF_DIR/ dans ton .gitignore pour ne jamais le pousser"
	fi
	while IFS= read -r f; do
		case "$f" in
			ft_*.c|libft.h|Makefile|README.md|.gitignore|.git|"$TDF_DIR"|*.o|libft.a) ;;
			*) (cd "$ROOT" && git check-ignore -q "$f" 2>/dev/null) \
				|| emit "G	fichier en trop à la racine : $f (interdit dans le rendu)" ;;
		esac
	done < <(cd "$ROOT" && ls -A)

	emit "S	vérification des 43 prototypes"
	header
	for n in $PART1 $PART2 $PART3; do
		wait_slot
		check_proto "$n" &
	done
	wait
	section "Prototypes · sujet"
	p=0
	for n in $PART1 $PART2 $PART3; do
		IFS=$'\t' read -r st msg < "$BUILD/proto/$n"
		emit "P	$n	$st	$msg"
		if [ "$st" != ok ]; then
			p=$((p + 1))
			say '  %s  %s%-18s%s %s%s%s\n' "$(pill "$P_KO" PROTO)" "$TXT" "ft_$n" "$R" "$D" "$msg" "$R"
		fi
	done
	[ "$p" = 0 ] && say '  %s  %s43 prototypes conformes au sujet%s\n' "$(pill "$P_OK" OK)" "$TXT" "$R"
	if [ -f "$ROOT/libft.h" ] && ! $CC -fsyntax-only -DF_t_list -I "$ROOT" "$SRC/protos.c" >/dev/null 2>&1; then
		emit "G	t_list ne correspond pas au sujet : typedef struct s_list { void *content; struct s_list *next; } t_list;"
		say '  %s  %st_list différent du sujet%s\n' "$(pill "$P_KO" PROTO)" "$TXT" "$R"
	fi
	emit "PD"

	emit "S	compilation de tes fonctions ${SANS:+avec $SANS}"
	for f in "$ROOT"/ft_*.c; do
		[ -f "$f" ] || continue
		wait_slot
		compile_obj "$f" &
		if [ "$VALG" = 1 ]; then wait_slot; compile_vg "$f" & fi
	done
	wait
	make_archive "$BUILD/obj" "$BUILD/libtest.a"
	[ "$VALG" = 1 ] && make_archive "$BUILD/vg" "$BUILD/libvg.a"
	tests_objs_prepare
	for n in $PART1 $PART2 $PART3; do
		wanted "$n" || continue
		[ -f "$BUILD/obj/ft_$n.o" ] || continue
		wait_slot
		build_bin "$n" &
	done
	wait

	export ASAN_OPTIONS="exitcode=77:detect_leaks=0:allocator_may_return_null=1:abort_on_error=0:strict_string_checks=1:detect_stack_use_after_return=1:log_path=$BUILD/log/san"
	export UBSAN_OPTIONS="exitcode=77:print_stacktrace=1:abort_on_error=1:log_path=$BUILD/log/san"

	for part in 1 2 3; do
		case $part in
			1) section "Partie 1 · libc"; list="$PART1" ;;
			2) section "Partie 2 · additionnelles"; list="$PART2" ;;
			3) section "Partie 3 · liste chaînée"; list="$PART3" ;;
		esac
		for n in $list; do
			if wanted "$n"; then
				emit "S	test de ft_$n"
				run_one "$n"
			else
				emit "T	$n	skip	0	0	non sélectionnée"
			fi
		done
	done

	emit "S	norminette"
	finish_norm

	if [ "$CFG_AUDIT" = 2 ]; then
		section "Audit · ce que la moulinette regarde"
		say '  %sdésactivé dans les paramètres%s\n' "$D" "$R"
	elif [ "$FORCEAUDIT" = 1 ]; then
		run_audit
	elif [ ${#SELECT[@]} -eq 0 ] && [ "$n_skip" -eq 0 ] && [ "$n_bad" -eq 0 ]; then
		run_audit
	else
		section "Audit · ce que la moulinette regarde"
		say '  %sse lance quand les 43 fonctions sont écrites et propres (ou : --audit)%s\n' "$D" "$R"
	fi
	emit "S	terminé"
	emit "END"

	if [ "$CI" = 1 ]; then
		printf '\n  %s' "$D"; rule 58; printf '%s\n\n' "$R"
		printf '  %s%d OK%s' "$F_OK$B" "$n_ok" "$R"
		[ "$n_warn" -gt 0 ] && printf '   %s(dont %d avec une remarque)%s' "$F_LK" "$n_warn" "$R"
		[ "$n_bad" -gt 0 ] && printf '   %s%d en échec%s' "$F_KO$B" "$n_bad" "$R"
		[ "$n_skip" -gt 0 ] && printf '   %s%d pas encore écrites%s' "$D" "$n_skip" "$R"
		[ "$p" -gt 0 ] && printf '   %s%d prototype(s) différent(s) du sujet%s' "$F_KO$B" "$p" "$R"
		[ "$n_audit_ko" -gt 0 ] && printf '   %s%d contrôles d'"'"'audit en échec%s' "$F_KO$B" "$n_audit_ko" "$R"
		printf '\n'
		grep '^G	' "$RES" | cut -f2 | while IFS= read -r l; do printf '  %s! %s%s\n' "$F_LK" "$l" "$R"; done
		if [ $((n_bad + n_audit_ko + p)) -eq 0 ]; then
			printf '\n  %s%sTout est propre. Lance sans --ci pour ouvrir le menu et dépenser tes Octets.%s\n\n' "$F_OK" "$B" "$R"
		else
			printf '\n  %s%sCorrige les rouges d'"'"'abord.%s %s-v montre le rapport complet.%s\n\n' "$F_KO" "$B" "$R" "$D" "$R"
		fi
	fi
	[ $((n_bad + n_audit_ko + p)) -eq 0 ]
}

EXOS=(strrev strspn memrchr strcasecmp memswap bitcount atoi_base strndup itoa_base
	tabdup split_set str_replace putnbr_base_fd sort_strs lstget lstreverse lstfind
	lstremove_if lstsort lst_to_array lstrotate)

run_exo() {
	local idx fn dir file out nout c b xobjs=() extra code marker checks fails l st
	case "$EXO" in ''|*[!0-9]*) echo "niveau inconnu : $EXO"; return 2 ;; esac
	idx=$((10#$EXO))
	[ "$idx" -ge 1 ] && [ "$idx" -le 21 ] || { echo "niveau inconnu : $EXO"; return 2; }
	EXO="$(printf '%02d' "$idx")"
	fn="ft_${EXOS[$((idx - 1))]}"
	out="$EXO_OUT"
	[ -z "$out" ] || : > "$out"
	xemit() { if [ -n "$out" ]; then printf '%s\n' "$*" >> "$out"; else printf '%s\n' "$*"; fi; }
	xemit_all() { while IFS= read -r l; do xemit "$l"; done; }
	dir="$HERE/entrainement/niveau_$EXO"
	file="$dir/$fn.c"
	if [ ! -f "$file" ]; then
		xemit "X	absent	0"
		xemit "D	fichier introuvable : $TDF_ICI/entrainement/niveau_$EXO/$fn.c"
		return 1
	fi
	if [ -z "$NORM_CMD" ]; then
		xemit "X	norme	0"
		xemit "D	norminette introuvable : la Norme est obligatoire à l'entraînement"
		xemit "D	installe-la (python3 -m pip install --user norminette) ou lance le testeur sur un poste 42"
		return 1
	fi
	nout="$(cd "$dir" && $NORM_CMD *.c 2>&1)"
	if echo "$nout" | grep -q 'Error'; then
		xemit "X	norme	0"
		echo "$nout" | grep 'Error' | head -7 | tr '\t' ' ' | sed -e $'s/\x1b\\[[0-9;]*m//g' -e 's/^ */D	/' | xemit_all
		return 1
	fi
	mkdir -p "$BUILD/x"
	for c in "$dir"/*.c; do
		b="$(basename "$c" .c)"
		if ! $CC -Wall -Wextra -Werror -I "$ROOT" -c "$c" -o "$BUILD/x/plain_$b.o" 2> "$BUILD/x/$b.err"; then
			xemit "X	build	0"
			grep -E 'error|warning' "$BUILD/x/$b.err" | sed -E 's/^.*\/(ft_)/\1/' | head -5 | sed 's/^/D	/' | xemit_all
			return 1
		fi
		$CC -Wall -Wextra -Werror -g $SAN "${HOOK[@]}" -I "$ROOT" -c "$c" -o "$BUILD/x/san_$b.o" 2>/dev/null
		xobjs+=("$BUILD/x/san_$b.o")
	done
	def_syms "$BUILD"/x/plain_*.o > "$BUILD/x/def"
	extra="$(undef_syms "$BUILD"/x/plain_*.o | comm -23 - "$BUILD/x/def" \
		| grep -vE "$LIBC_OK" | grep -vE '^ft_[a-z_0-9]+$')"
	if [ -n "$extra" ]; then
		xemit "X	interdit	0"
		xemit "D	fonctions interdites : $(echo $extra | tr '\n' ' ')"
		xemit "D	autorisé : malloc, free, write et ta libft"
		return 1
	fi
	for c in "$ROOT"/ft_*.c; do
		[ -f "$c" ] || continue
		[ "$(basename "$c" .c)" = "$fn" ] && continue
		wait_slot
		compile_obj "$c" &
	done
	wait
	make_archive "$BUILD/obj" "$BUILD/libtest.a"
	if ! $CC -g -w $SAN -DX_"$EXO" -I "$ROOT" "$SRC/exos.c" "${xobjs[@]}" "$BUILD/libtest.a" \
		$LDX -o "$BUILD/x/bin" 2> "$BUILD/x/link.err"; then
		xemit "X	build	0"
		grep -E 'undefined|error' "$BUILD/x/link.err" | sed -E 's/^.*\/(ft_|exos)/\1/' | head -4 | sed 's/^/D	/' | xemit_all
		return 1
	fi
	mkdir -p "$BUILD/log"
	rm -f "$BUILD"/log/*
	export ASAN_OPTIONS="exitcode=77:detect_leaks=1:allocator_may_return_null=1:abort_on_error=0:log_path=$BUILD/log/san"
	export UBSAN_OPTIONS="exitcode=77:print_stacktrace=1:log_path=$BUILD/log/san"
	export LSAN_OPTIONS="exitcode=77:log_path=$BUILD/log/san"
	with_timeout "$TMO" "$BUILD/x/bin" > "$BUILD/x/out" 2> "$BUILD/x/err"
	code=$?
	cat "$BUILD"/log/san.* "$BUILD/x/err" > "$BUILD/x/san" 2>/dev/null
	marker="$(grep '^@@ ' "$BUILD/x/out" | tail -1)"
	checks="$(echo "$marker" | awk '{print $2}')"
	fails="$(echo "$marker" | awk '{print $3}')"
	if [ "$code" = 124 ]; then
		st=time; DET=("aucune réponse après $TMO s : boucle infinie ?")
	elif grep -q 'runtime error' "$BUILD/x/san"; then
		st=ub; DET=("$(grep -m1 'runtime error' "$BUILD/x/san" | sed -E 's/^.*\/(ft_)/\1/')")
	elif grep -q 'ERROR: AddressSanitizer' "$BUILD/x/san"; then
		st=crash; DET=("$(grep -m1 'ERROR: AddressSanitizer' "$BUILD/x/san" | sed -E 's/.*AddressSanitizer: ([^ ]+).*/\1/')")
		while IFS= read -r l; do DET+=("$l"); done < <(where "$BUILD/x/san")
	elif [ -n "$fails" ] && [ "$fails" -gt 0 ]; then
		st=ko; DET=()
		while IFS= read -r l; do DET+=("$l"); done < <(grep '^  ko| ' "$BUILD/x/out" | sed 's/^  ko| //' | head -6)
	elif grep -q 'LeakSanitizer' "$BUILD/x/san"; then
		st=leak; DET=("$(grep -m1 'SUMMARY' "$BUILD/x/san" | sed -E 's/SUMMARY: [A-Za-z]+: //')")
		while IFS= read -r l; do DET+=("$l"); done < <(where "$BUILD/x/san")
	elif [ "$code" = 0 ] && [ -n "$marker" ]; then
		st=ok; DET=()
	else
		st=crash; DET=("le test s'est arrêté avec le code $code")
		while IFS= read -r l; do DET+=("$l"); done < <(head -3 "$BUILD/x/err")
	fi
	xemit "X	$st	${checks:-0}"
	for l in ${DET[@]+"${DET[@]}"}; do xemit "D	$(printf '%s' "$l" | tr '\t' ' ')"; done
	[ "$st" = ok ]
}

if [ -n "$EXO" ]; then
	run_exo
	exit $?
fi

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
	TDF_NO_UPDATE=1 exec bash "$UPD_DIR/run.sh" libft ${UPD_ARGS[@]+"${UPD_ARGS[@]}"}
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
UPD_DIR="$(cd "$HERE/../.." && pwd)"; UPD_VER="$VERSION"; UPD_GAME="testeur"; UPD_NAME="Libft"
UPD_REPO="SplatchAFK/testeur_des_fous"; UPD_VH="moteur/version.h"
UPD_ARGS=(${ORIG_ARGS[@]+"${ORIG_ARGS[@]}"})
upd_build_ok() {
	local f
	bash -n "$HERE/run.sh" || return 1
	for f in "$SRC"/*.c "$MOT"/*.c; do
		case "$(basename "$f")" in tests.c|exos.c|protos.c) continue ;; esac
		$CC -fsyntax-only -w -I "$SRC" -I "$MOT" "$f" || return 1
	done
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
		printf 'export TDF_PROJET=%q TDF_NOM=%q\n' "$ROOT" "${TDF_NOM:-}"
		printf 'TDF_FENETRE=1 bash %q libft --projet %q' "$UPD_DIR/run.sh" "$ROOT"
		for a in ${ORIG_ARGS[@]+"${ORIG_ARGS[@]}"}; do printf ' %q' "$a"; done
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
	printf '  (pour rester ici : TDF_FENETRE=1 bash %s/run.sh)\n\n' "$TDF_DIR"
	exit 0
}

if [ "$MENU_SEUL" = 1 ] && [ "$CI" = 1 ]; then
	echo "le menu a besoin d'un vrai terminal"
	exit 2
fi

if [ "$CI" = 0 ]; then
	grand_terminal
	tdf_chrono_debut maj "recherche de mise à jour"
	update_check
	tdf_chrono_fin maj "recherche de mise à jour"
	tdf_chrono_debut jeu "compilation de l'interface"
	if ! build_game; then
		printf '\r\e[K  le menu ne compile pas, passage en mode texte\n'
		sed 's/^/    /' "$BUILD/game.err" | head -5
		tdf_journal "ERREUR : l'interface ne compile pas, passage en mode texte"
		[ -n "${TDF_JOURNAL:-}" ] && cp "$BUILD/game.err" "$(dirname "$TDF_JOURNAL")/libft_compilation.err" 2>/dev/null
		[ "$MENU_SEUL" = 1 ] && exit 1
		CI=1
	else
		tdf_chrono_fin jeu "compilation de l'interface"
	fi
	printf '\r\e[K'
	if [ "$MENU_SEUL" = 1 ]; then
		ARGS=(--dir "$HERE")
		[ "$FAST" = 1 ] && ARGS+=(--fast)
		tdf_journal "menu seul, sans tests"
		"$BUILD/fous" "${ARGS[@]}" 2>> "${TDF_JOURNAL:-/dev/null}"
		code=$?
		tdf_journal "fin de l'interface (code $code)"
		exit $code
	fi
fi

if [ "$CI" = 1 ]; then
	set_colors
	run_all
	code=$?
	if [ "$NOLORE" = 1 ]; then
		if build_game; then
			"$BUILD/fous" --results "$RES" --record --dir "$HERE" || true
		else
			printf '  (progression non enregistrée : le menu ne compile pas)\n'
		fi
	fi
	[ "$KEEP" = 1 ] && printf '  %sbuild gardé dans %s%s\n\n' "$D" "$BUILD" "$R"
	exit $code
fi

first=1
while :; do
	if [ "$first" = 0 ]; then
		rm -rf "$BUILD"/obj "$BUILD"/bin "$BUILD"/log "$BUILD"/plain "$BUILD"/proto \
			"$BUILD"/vg "$BUILD"/audit "$BUILD"/san_* "$BUILD"/out_* "$BUILD"/err_* \
			"$BUILD"/norm.* "$BUILD"/lib*.a
		mkdir -p "$BUILD/obj" "$BUILD/bin" "$BUILD/log" "$BUILD/plain" "$BUILD/proto" "$BUILD/vg"
		: > "$RES"
		n_ok=0; n_bad=0; n_skip=0; n_warn=0; n_seen=0; n_audit_ko=0; n_norm_ko=0
		printf '\e[0m\e[2J\e[H\n  \e[38;5;213mLe testeur des fous\e[0m · recompilation...'
		if ! build_game; then
			printf '\r\e[K  le menu ne compile plus :\n'
			sed 's/^/    /' "$BUILD/game.err" | head -5
			exit 1
		fi
	fi
	tdf_journal "tests lancés en arrière-plan"
	(
		RPID=
		trap 'kill_tree "$RPID"; exit 143' TERM INT HUP
		t0=$(tdf_ms)
		run_all > "$BUILD/run.log" 2>&1 &
		RPID=$!
		wait "$RPID"
		rc=$?
		tdf_journal "tests terminés (code $rc, $(( ($(tdf_ms) - t0) / 1000 )) s)"
	) &
	TPID=$!
	ARGS=(--results "$RES" --pid "$TPID" --dir "$HERE")
	[ "$first" = 0 ] && ARGS+=(--relance)
	[ "$FAST" = 1 ] && ARGS+=(--fast)
	[ "$first" = 1 ] && [ -n "$RECO" ] && ARGS+=(--reco "$RECO")
	tdf_journal "interface ouverte"
	"$BUILD/fous" "${ARGS[@]}" 2>> "${TDF_JOURNAL:-/dev/null}"
	code=$?
	tdf_journal "interface fermée (code $code)"
	kill "$TPID" 2>/dev/null
	wait "$TPID" 2>/dev/null
	[ -n "${TDF_JOURNAL:-}" ] && { cp "$BUILD/run.log" "$(dirname "$TDF_JOURNAL")/libft_tests.log"; cp "$RES" "$(dirname "$TDF_JOURNAL")/libft_resultats.tsv"; } 2>/dev/null
	TPID=
	[ "$code" = 42 ] || break
	first=0
done
[ "$KEEP" = 1 ] && printf '  build gardé dans %s\n\n' "$BUILD"
exit $code
