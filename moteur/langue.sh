TDF_CODES=(fr en es de it pt tr ru)
TDF_NOMS=("Français" "English" "Español" "Deutsch" "Italiano" "Português" "Türkçe" "Русский")

tdf_langue_lire() {
	local d="$HOME/.testeur_des_fous" n=""
	[ -f "$d/save.txt" ] && n="$(sed -n 's/^cfg_lang=//p' "$d/save.txt" 2>/dev/null | head -1)"
	[ -z "$n" ] && [ -f "$d/langue.txt" ] && n="$(head -1 "$d/langue.txt" 2>/dev/null)"
	case "$n" in [0-7]) ;; *) n="" ;; esac
	TDF_LANGUE_N="$n"
	export TDF_LANGUE_N
	TDF_LANGUE_FICHIER=""
	if [ -n "$n" ] && [ "$n" -ge 2 ]; then
		TDF_LANGUE_FICHIER="${TDF_RACINE:-.}/moteur/langues/${TDF_CODES[$n]}.txt"
	fi
}

tdf_langue_ecrire() {
	local d="$HOME/.testeur_des_fous"
	mkdir -p "$d" 2>/dev/null
	printf '%s\n' "$1" > "$d/langue.txt" 2>/dev/null
	if [ -f "$d/save.txt" ]; then
		if grep -q '^cfg_lang=' "$d/save.txt"; then
			sed "s/^cfg_lang=.*/cfg_lang=$1/" "$d/save.txt" > "$d/save.txt.lang" && cat "$d/save.txt.lang" > "$d/save.txt"
			rm -f "$d/save.txt.lang"
		else
			printf 'cfg_lang=%s\n' "$1" >> "$d/save.txt"
		fi
	fi
	tdf_langue_lire
}

tdf_langue_auto() {
	local v="${LC_ALL:-${LC_MESSAGES:-${LANG:-}}}" i
	for i in 0 1 2 3 4 5 6 7; do
		case "$v" in "${TDF_CODES[$i]}"*) printf '%s' "$i"; return ;; esac
	done
	printf '0'
}

tt() {
	local r=""
	case "${TDF_LANGUE_N:-0}" in
		0|"") printf '%s' "$1"; return ;;
		1) printf '%s' "$2"; return ;;
	esac
	if [ -n "${TDF_LANGUE_FICHIER:-}" ] && [ -f "$TDF_LANGUE_FICHIER" ]; then
		r="$(awk -v k="$2" 'BEGIN { n = length(k) } substr($0, 1, n + 1) == k "\t" { print substr($0, n + 2); exit }' "$TDF_LANGUE_FICHIER")"
	fi
	printf '%s' "${r:-$2}"
}

tl() {
	local f="$1" e="$2"
	shift 2
	printf "$(tt "$f" "$e")\n" "$@"
}

tdf_langue_lire
