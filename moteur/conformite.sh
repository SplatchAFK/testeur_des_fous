tdf_valgrind_essai() {
	local d="$1"
	valgrind -q --error-exitcode=42 "$d/p" > "$d/sortie" 2>&1
}

tdf_valgrind_pret() {
	local d vg base c
	[ -n "${TDF_VG_ETAT:-}" ] && { [ "$TDF_VG_ETAT" = ok ]; return; }
	TDF_VG_ETAT=ko
	if ! command -v valgrind >/dev/null 2>&1; then
		TDF_VG_RAISON="$(tt "valgrind n'est pas installé" "valgrind is not installed")"
		return 1
	fi
	d=$(mktemp -d "${TMPDIR:-/tmp}/tdf_vg.XXXXXX") || { TDF_VG_RAISON="$(tt "dossier temporaire impossible" "cannot create a temporary folder")"; return 1; }
	printf '#include <stdlib.h>\nint main(void)\n{\n\tfree(malloc(8));\n\treturn (0);\n}\n' > "$d/p.c"
	if ! cc -g "$d/p.c" -o "$d/p" 2>/dev/null; then
		rm -rf "$d"
		TDF_VG_RAISON="$(tt "impossible de compiler le programme d'essai" "cannot compile the test program")"
		return 1
	fi
	if tdf_valgrind_essai "$d"; then
		rm -rf "$d"
		TDF_VG_ETAT=ok
		export TDF_VG_ETAT
		return 0
	fi
	vg="$(command -v valgrind)"
	base="$(cd "$(dirname "$(readlink -f "$vg" 2>/dev/null || echo "$vg")")/.." 2>/dev/null && pwd)"
	for c in "${VALGRIND_LIB:-}" "$base/libexec/valgrind" "$base/lib64/valgrind" "$base/lib/valgrind" \
		/usr/libexec/valgrind /usr/lib64/valgrind /usr/lib/valgrind /usr/lib/x86_64-linux-gnu/valgrind \
		"$HOME/.local/libexec/valgrind" "$HOME/.local/lib64/valgrind" "$HOME/.local/lib/valgrind"; do
		[ -n "$c" ] && ls "$c"/memcheck-* >/dev/null 2>&1 || continue
		if VALGRIND_LIB="$c" tdf_valgrind_essai "$d"; then
			export VALGRIND_LIB="$c"
			rm -rf "$d"
			TDF_VG_ETAT=ok
			export TDF_VG_ETAT
			return 0
		fi
	done
	if [ -z "${DEBUGINFOD_URLS:-}" ] && grep -qiE 'ld-linux|mandatory|redirection' "$d/sortie"; then
		if DEBUGINFOD_URLS="https://debuginfod.fedoraproject.org/" tdf_valgrind_essai "$d"; then
			export DEBUGINFOD_URLS="https://debuginfod.fedoraproject.org/"
			rm -rf "$d"
			TDF_VG_ETAT=ok
			export TDF_VG_ETAT
			return 0
		fi
	fi
	TDF_VG_RAISON="$(tt "valgrind ne démarre pas ici :" "valgrind does not start here:") $(grep -v '^[[:space:]]*$' "$d/sortie" | sed 's/^==[0-9]*== //' | head -2 | tr '\n' ' ' | cut -c1-200)"
	rm -rf "$d"
	export TDF_VG_ETAT TDF_VG_RAISON
	return 1
}

tdf_rendus() {
	local d="$1"
	if (cd "$d" && git rev-parse --is-inside-work-tree >/dev/null 2>&1); then
		(cd "$d" && git ls-files)
	else
		(cd "$d" && find . -type f -not -path './.git/*' -not -path '*/testeur_des_fous*' \
			-not -path './.build*' -not -path './.banc*' -not -name '*.o' -not -name '*.a' \
			-not -name '*.d' | sed 's#^\./##')
	fi | grep -Ev '(^|/)(testeur_des_fous|francinette|printfTester|ft_printf_tester|printf-tester|gnlTester|libftTester|Tripouille)[^/]*/' | sort
}

tdf_non_suivis() {
	local d="$1"
	tdf_suivi_git "$d" || return 0
	(cd "$d" && git ls-files --others --exclude-standard --directory) | while IFS= read -r f; do
		[ -n "$f" ] || continue
		case "$f" in
			*.o|*.a|*.d) continue ;;
		esac
		case "$f" in
			testeur_des_fous*/|*Tester*/|*tester*/|francinette*/|Tripouille*/)
				printf '%s\t%s\n' "$f" "$(tt "un testeur rangé dans ton projet : pas encore dans git, mais « git add . » l'enverrait à la moulinette (range le testeur à côté du projet)" "a tester stored in your project: not in git yet, but \"git add .\" would send it to the moulinette (keep the tester next to the project)")" ;;
			*)
				printf '%s\t%s\n' "$f" "$(tt "pas encore dans git : « git add . » l'enverrait ; ajoute seulement les fichiers à rendre" "not in git yet: \"git add .\" would send it; add only the files to hand in")" ;;
		esac
	done
}

tdf_suivi_git() {
	(cd "$1" && git rev-parse --is-inside-work-tree >/dev/null 2>&1)
}

tdf_utilise_c() {
	local d="$1" f="$2" b dir
	b="$(basename "$f" .c)"
	dir="$(dirname "$f")"
	[ -f "$d/$dir/Makefile" ] && grep -q -- "$b" "$d/$dir/Makefile" && return 0
	[ -f "$d/Makefile" ] && grep -q -- "$b" "$d/Makefile" && return 0
	return 1
}

tdf_utilise_h() {
	local d="$1" f="$2" b
	b="$(basename "$f")"
	grep -rqsE "#[[:space:]]*include[[:space:]]*[\"<]([^\">]*/)?$b[\">]" --include='*.c' --include='*.h' "$d" && return 0
	return 1
}

tdf_parasites() {
	local projet="$1" d="$2" f b dir n dups
	dups="$(tdf_rendus "$d" | awk -F/ '{print $NF}' | grep -E '\.h$|^README\.md$' | sort | uniq -d)"
	while IFS= read -r f; do
		[ -n "$f" ] || continue
		b="$(basename "$f")"
		dir="$(dirname "$f")"
		n="$(echo "$f" | tr -cd '/' | wc -c)"
		if [ "$b" = README.md ] && [ "$f" != README.md ]; then
			printf '%s\t%s\n' "$f" "$(tt "README en trop : le seul attendu est à la racine, la norminette de la moulinette le refuse" "extra README: the only one expected is at the root, the moulinette's norminette rejects it")"
			continue
		fi
		if [ "$b" = .gitignore ]; then
			printf '%s\t%s\n' "$f" "$(tt "pas dans les fichiers à rendre (le sujet interdit les fichiers inutiles)" "not among the files to hand in (the subject forbids useless files)")"
			continue
		fi
		if echo "$dups" | grep -qxF "$b" && [ "$b" != README.md ]; then
			printf '%s\t%s\n' "$f" "$(printf "$(tt "en double : un autre %s existe dans le dépôt" "duplicate: another %s exists in the repository")" "$b")"
			continue
		fi
		case "$projet" in
			libft)
				if [ "$n" -gt 0 ]; then
					printf '%s\t%s\n' "$f" "$(tt "sous-dossier : le sujet veut tous les fichiers à la racine" "subfolder: the subject wants every file at the root")"
					continue
				fi
				case "$b" in
					Makefile|README.md|libft.h) ;;
					ft_*.c) tdf_utilise_c "$d" "$f" || printf '%s\t%s\n' "$f" "$(tt "jamais compilé par le Makefile : fichier inutilisé" "never compiled by the Makefile: unused file")" ;;
					*.h) printf '%s\t%s\n' "$f" "$(tt "en-tête en trop : le sujet n'attend que libft.h" "extra header: the subject only expects libft.h")" ;;
					*.c) printf '%s\t%s\n' "$f" "$(tt "ne fait pas partie de la libft (main de test ?)" "not part of the libft (test main?)")" ;;
					*) printf '%s\t%s\n' "$f" "$(tt "pas dans les fichiers à rendre (ft_*.c, libft.h, Makefile, README.md)" "not among the files to hand in (ft_*.c, libft.h, Makefile, README.md)")" ;;
				esac
				;;
			printf)
				if [ "$n" -gt 1 ]; then
					printf '%s\t%s\n' "$f" "$(tt "trop profond : le sujet accepte *.c, */*.c, *.h, */*.h et Makefile" "too deep: the subject accepts *.c, */*.c, *.h, */*.h and Makefile")"
					continue
				fi
				case "$b" in
					Makefile)
						[ "$n" = 0 ] || [ "$dir" = libft ] || printf '%s\t%s\n' "$f" "$(tt "Makefile en trop : seuls celui de la racine et celui de libft/ sont attendus" "extra Makefile: only the root one and the libft/ one are expected")" ;;
					README.md) ;;
					*.c) tdf_utilise_c "$d" "$f" || printf '%s\t%s\n' "$f" "$(tt "jamais compilé par un Makefile : fichier inutilisé" "never compiled by a Makefile: unused file")" ;;
					*.h) tdf_utilise_h "$d" "$f" || printf '%s\t%s\n' "$f" "$(tt "jamais inclus : fichier inutilisé" "never included: unused file")" ;;
					*) printf '%s\t%s\n' "$f" "$(tt "pas dans les fichiers à rendre (*.c, */*.c, *.h, */*.h, Makefile)" "not among the files to hand in (*.c, */*.c, *.h, */*.h, Makefile)")" ;;
				esac
				;;
			gnl)
				case "$f" in
					get_next_line.c|get_next_line_utils.c|get_next_line.h|README.md) ;;
					*) printf '%s\t%s\n' "$f" "$(tt "pas dans les fichiers à rendre (get_next_line.c, get_next_line_utils.c, get_next_line.h)" "not among the files to hand in (get_next_line.c, get_next_line_utils.c, get_next_line.h)")" ;;
				esac
				;;
		esac
	done < <(tdf_rendus "$d")
}

tdf_portee() {
	local d="$1" autorises="$2" f
	for f in "$d"/ft_*.c "$d"/libft.h; do
		[ -f "$f" ] || continue
		awk -v fic="$(basename "$f")" -v ok=" $autorises " '
			function nom(l,   s) {
				s = l
				sub(/\(.*/, "", s)
				gsub(/[*\t ]+$/, "", s)
				n = split(s, t, /[\t *]+/)
				return t[n]
			}
			{
				l = $0
				o = ""
				while (length(l) > 0) {
					if (com) {
						e = index(l, "*/")
						if (!e)
							l = ""
						else {
							l = substr(l, e + 2)
							com = 0
						}
					} else {
						s = index(l, "/*")
						if (!s) {
							o = o l
							l = ""
						} else {
							o = o substr(l, 1, s - 1)
							l = substr(l, s + 2)
							com = 1
						}
					}
				}
				l = o
				sub(/\/\/.*/, "", l)
			}
			prof == 0 && l ~ /^[A-Za-z_]/ && l !~ /^(typedef|struct|union|enum|#)/ {
				if (l ~ /\(/ && l !~ /;[ \t]*$/) {
					c = nom(l)
					if (l !~ /^static[ \t]/ && index(ok, " " c " ") == 0)
						printf "%s:%d\tfonction %s non static : le sujet demande static pour les fonctions d'"'"'aide (erreur moulinette)\n", fic, NR, c
				} else if (l ~ /;[ \t]*$/ && (l !~ /\(/ || substr(l, index(l, "(") + 1, 1) == "*")) {
					if (l !~ /^extern[ \t]/ || fic ~ /\.h$/)
						printf "%s:%d\tvariable globale : interdite par le sujet (erreur moulinette)\n", fic, NR
				}
			}
			{
				o = gsub(/\{/, "{", l)
				f2 = gsub(/\}/, "}", l)
				prof += o - f2
			}
		' "$f"
	done
}

tdf_inc_symboles() {
	case "$1" in
		stdlib.h) printf '%s' 'malloc|free|calloc|realloc|exit|abort|atoi|atol|strtol|abs|labs|EXIT_SUCCESS|EXIT_FAILURE|NULL|size_t' ;;
		unistd.h) printf '%s' 'write|read|close|lseek|ssize_t|STDIN_FILENO|STDOUT_FILENO|STDERR_FILENO|NULL|size_t' ;;
		stddef.h) printf '%s' 'size_t|NULL|ptrdiff_t|offsetof|wchar_t' ;;
		stdarg.h) printf '%s' 'va_list|va_start|va_arg|va_end|va_copy' ;;
		limits.h) printf '%s' '[A-Z]+_(MAX|MIN|BIT)' ;;
		stdint.h) printf '%s' 'u?int[0-9]+_t|u?intptr_t|u?intmax_t|[A-Z0-9]+_MAX|[A-Z0-9]+_MIN' ;;
		fcntl.h) printf '%s' 'open|O_[A-Z]+' ;;
		stdio.h) printf '%s' 'printf|fprintf|dprintf|sprintf|snprintf|puts|perror|FILE|EOF|BUFSIZ|fopen|fclose|getchar|putchar' ;;
		string.h) printf '%s' '(mem|str)(cpy|ncpy|cmp|ncmp|len|chr|rchr|dup|ndup|cat|ncat|set|move|str|nstr|lcpy|lcat|tok|error|sep)' ;;
		ctype.h) printf '%s' 'is(alpha|digit|alnum|ascii|print|space|upper|lower|punct|xdigit)|toupper|tolower' ;;
		errno.h) printf '%s' 'errno|E[A-Z]+' ;;
		stdbool.h) printf '%s' 'bool|true|false' ;;
	esac
}

tdf_inc_conseil() {
	case "$1" in
		malloc|free|calloc|realloc|exit|abort|atoi|atol|strtol) printf '<stdlib.h>' ;;
		write|read|close|lseek) printf '<unistd.h>' ;;
		va_start|va_arg|va_end|va_copy) printf '<stdarg.h>' ;;
		open) printf '<fcntl.h>' ;;
		printf|fprintf|dprintf|perror|puts) printf '<stdio.h>' ;;
		mem*|str*) printf '<string.h>' ;;
	esac
}

tdf_inc_code() {
	awk '
	{
		l = $0; o = ""
		while (l != "") {
			if (dans) {
				i = index(l, "*/")
				if (i == 0) { l = ""; break }
				l = substr(l, i + 2); dans = 0
			} else {
				i = index(l, "/*")
				if (i == 0) { o = o l; l = ""; break }
				o = o substr(l, 1, i - 1); l = substr(l, i + 2); dans = 1
			}
		}
		sub(/\/\/.*/, "", o)
		if (o !~ /^[ \t]*#[ \t]*include/)
			print o
	}' "$@" 2>/dev/null
}

tdf_inc_garde() {
	tdf_inc_code "$1" | grep -v '^[[:space:]]*$' | head -2 | awk '
		NR == 1 && /^[ \t]*#[ \t]*pragma[ \t]+once/ { ok = 1 }
		NR == 1 && /^[ \t]*#[ \t]*ifndef[ \t]+/ { m = $NF; if ($1 == "#") m = $3 }
		NR == 2 && m != "" && $0 ~ ("^[ \t]*#[ \t]*define[ \t]+" m "([ \t]|$)") { ok = 1 }
		END { exit !ok }'
}

tdf_inclusions() {
	local d="$1" f rel inc out err dirs=() fn h sym tout n
	shift
	local fs=("$@")
	while IFS= read -r h; do dirs+=(-I "$h"); done < <(for f in "${fs[@]}"; do case "$f" in *.h) dirname "$f" ;; esac; done | sort -u)
	tout="$(tdf_inc_code "${fs[@]}")"
	for f in "${fs[@]}"; do
		[ -f "$f" ] || continue
		rel="${f#"$d"/}"
		while IFS= read -r inc; do
			[ -n "$inc" ] || continue
			case "$inc" in
				*.c) printf 'ko\t%s\t%s\n' "$rel" "$(printf "$(tt "inclut un fichier .c (%s) : on n'inclut que des .h" "includes a .c file (%s): only .h files are included")" "$inc")" ;;
			esac
		done < <(sed -n 's/^[[:space:]]*#[[:space:]]*include[[:space:]]*"\([^"]*\)".*/\1/p' "$f")
		if [ "${f##*.}" = c ]; then
			out="$(LC_ALL=C ${CC:-cc} -fsyntax-only -H -Werror=implicit-function-declaration "${dirs[@]}" "$f" 2>&1)"
		else
			out="$(printf '#include "%s"\n#include "%s"\n' "$f" "$f" | LC_ALL=C ${CC:-cc} -fsyntax-only -Wall -Wextra -Werror "${dirs[@]}" -x c - 2>&1)"
		fi
		err="$(printf '%s\n' "$out" | grep -m1 'fatal error: .*No such file' | sed -E 's/.*fatal error: ([^:]*):.*/\1/')"
		[ -n "$err" ] && printf 'ko\t%s\t%s\n' "$rel" "$(printf "$(tt "#include \"%s\" introuvable depuis ce fichier" "#include \"%s\" cannot be found from this file")" "$err")"
		if [ "${f##*.}" = c ]; then
			while IFS= read -r fn; do
				[ -n "$fn" ] || continue
				h="$(tdf_inc_conseil "$fn")"
				if [ -n "$h" ]; then
					printf 'ko\t%s\t%s\n' "$rel" "$(printf "$(tt "utilise %s sans l'avoir déclarée : il manque %s" "uses %s without declaring it: %s is missing")" "$fn" "$h")"
				else
					printf 'ko\t%s\t%s\n' "$rel" "$(printf "$(tt "utilise %s sans l'avoir déclarée : il manque l'include du .h qui la déclare" "uses %s without declaring it: the include of the .h that declares it is missing")" "$fn")"
				fi
			done < <(printf '%s\n' "$out" | grep -oE "implicit declaration of function '[^']+'" | sed -E "s/.*'([^']+)'/\1/" | sort -u)
			if [ -z "$err" ] && ! printf '%s\n' "$out" | grep -E '^\.+ ' | sed -E 's/^\.+ //' | grep -q "^$d/\|^\./\|^[^/]"; then
				printf 'warn\t%s\t%s\n' "$rel" "$(tt "n'inclut aucun .h du projet : si son prototype change, rien ne le compare au .h" "includes no .h of the project: if its prototype changes, nothing compares it with the .h")"
			fi
		else
			if [ -z "$err" ] && printf '%s\n' "$out" | grep -q 'error'; then
				printf 'ko\t%s\t%s\n' "$rel" "$(printf "$(tt "ne compile pas seul ou inclus deux fois : %s" "does not compile on its own or included twice: %s")" "$(printf '%s\n' "$out" | grep -m1 'error' | sed -E 's/^[^:]*:[0-9]+:[0-9]+: //' | cut -c1-90)")"
			fi
			tdf_inc_garde "$f" || printf 'warn\t%s\t%s\n' "$rel" "$(tt "pas de garde d'inclusion (#ifndef ... #define ... en tête du fichier)" "no include guard (#ifndef ... #define ... at the top of the file)")"
		fi
		while IFS= read -r inc; do
			[ -n "$inc" ] || continue
			sym="$(tdf_inc_symboles "$inc")"
			[ -n "$sym" ] || continue
			if [ "${f##*.}" = c ]; then
				tdf_inc_code "$f" | grep -Eqw "($sym)" && continue
			else
				printf '%s\n' "$tout" | grep -Eqw "($sym)" && continue
			fi
			printf 'warn\t%s\t%s\n' "$rel" "$(printf "$(tt "<%s> inclus pour rien : aucune de ses fonctions ni de ses types n'est utilisé" "<%s> included for nothing: none of its functions or types is used")" "$inc")"
		done < <(sed -n 's/^[[:space:]]*#[[:space:]]*include[[:space:]]*<\([^>]*\)>.*/\1/p' "$f")
		if grep -qw restrict "$f"; then
			printf 'ko\t%s\t%s\n' "$rel" "$(tt "utilise le mot-clé restrict : le sujet l'interdit dans tes prototypes" "uses the restrict keyword: the subject forbids it in your prototypes")"
		fi
	done
}

tdf_noms_params() {
	printf '%s\n' "$1" | awk '
	{
		s = $0; i = index(s, "("); if (i == 0) exit
		s = substr(s, i + 1); p = 1; cur = ""; out = ""
		for (k = 1; k <= length(s) && p > 0; k++) {
			c = substr(s, k, 1)
			if (c == "(") p++
			if (c == ")") p--
			if ((c == "," && p == 1) || p == 0) { out = out nom(cur) ","; cur = ""; continue }
			cur = cur c
		}
		sub(/,$/, "", out); print out
	}
	function nom(t,   m) {
		if (match(t, /\(\*[ \t]*[A-Za-z_][A-Za-z0-9_]*/)) { m = substr(t, RSTART + 2, RLENGTH - 2); gsub(/[ \t]/, "", m); return m }
		gsub(/[^A-Za-z0-9_]+/, " ", t); n = split(t, w, " "); return w[n]
	}'
}
