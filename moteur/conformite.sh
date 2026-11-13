tdf_valgrind_essai() {
	local d="$1"
	valgrind -q --error-exitcode=42 "$d/p" > "$d/sortie" 2>&1
}

tdf_valgrind_pret() {
	local d vg base c
	[ -n "${TDF_VG_ETAT:-}" ] && { [ "$TDF_VG_ETAT" = ok ]; return; }
	TDF_VG_ETAT=ko
	if ! command -v valgrind >/dev/null 2>&1; then
		TDF_VG_RAISON="valgrind n'est pas installé"
		return 1
	fi
	d=$(mktemp -d "${TMPDIR:-/tmp}/tdf_vg.XXXXXX") || { TDF_VG_RAISON="dossier temporaire impossible"; return 1; }
	printf '#include <stdlib.h>\nint main(void)\n{\n\tfree(malloc(8));\n\treturn (0);\n}\n' > "$d/p.c"
	if ! cc -g "$d/p.c" -o "$d/p" 2>/dev/null; then
		rm -rf "$d"
		TDF_VG_RAISON="impossible de compiler le programme d'essai"
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
	TDF_VG_RAISON="valgrind ne démarre pas ici : $(grep -v '^[[:space:]]*$' "$d/sortie" | sed 's/^==[0-9]*== //' | head -2 | tr '\n' ' ' | cut -c1-200)"
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
				printf '%s\tun testeur rangé dans ton projet : pas encore dans git, mais « git add . » l'"'"'enverrait à la moulinette (range le testeur à côté du projet)\n' "$f" ;;
			*)
				printf '%s\tpas encore dans git : « git add . » l'"'"'enverrait ; ajoute seulement les fichiers à rendre\n' "$f" ;;
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
			printf '%s\tREADME en trop : le seul attendu est à la racine, la norminette de la moulinette le refuse\n' "$f"
			continue
		fi
		if [ "$b" = .gitignore ]; then
			printf '%s\tpas dans les fichiers à rendre (le sujet interdit les fichiers inutiles)\n' "$f"
			continue
		fi
		if echo "$dups" | grep -qxF "$b" && [ "$b" != README.md ]; then
			printf '%s\ten double : un autre %s existe dans le dépôt\n' "$f" "$b"
			continue
		fi
		case "$projet" in
			libft)
				if [ "$n" -gt 0 ]; then
					printf '%s\tsous-dossier : le sujet veut tous les fichiers à la racine\n' "$f"
					continue
				fi
				case "$b" in
					Makefile|README.md|libft.h) ;;
					ft_*.c) tdf_utilise_c "$d" "$f" || printf '%s\tjamais compilé par le Makefile : fichier inutilisé\n' "$f" ;;
					*.h) printf '%s\ten-tête en trop : le sujet n'"'"'attend que libft.h\n' "$f" ;;
					*.c) printf '%s\tne fait pas partie de la libft (main de test ?)\n' "$f" ;;
					*) printf '%s\tpas dans les fichiers à rendre (ft_*.c, libft.h, Makefile, README.md)\n' "$f" ;;
				esac
				;;
			printf)
				if [ "$n" -gt 1 ]; then
					printf '%s\ttrop profond : le sujet accepte *.c, */*.c, *.h, */*.h et Makefile\n' "$f"
					continue
				fi
				case "$b" in
					Makefile)
						[ "$n" = 0 ] || [ "$dir" = libft ] || printf '%s\tMakefile en trop : seuls celui de la racine et celui de libft/ sont attendus\n' "$f" ;;
					README.md) ;;
					*.c) tdf_utilise_c "$d" "$f" || printf '%s\tjamais compilé par un Makefile : fichier inutilisé\n' "$f" ;;
					*.h) tdf_utilise_h "$d" "$f" || printf '%s\tjamais inclus : fichier inutilisé\n' "$f" ;;
					*) printf '%s\tpas dans les fichiers à rendre (*.c, */*.c, *.h, */*.h, Makefile)\n' "$f" ;;
				esac
				;;
			gnl)
				case "$f" in
					get_next_line.c|get_next_line_utils.c|get_next_line.h|README.md) ;;
					*) printf '%s\tpas dans les fichiers à rendre (get_next_line.c, get_next_line_utils.c, get_next_line.h)\n' "$f" ;;
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
