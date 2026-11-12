TDF_CH_TOTAL=1
TDF_CH_FAIT=0
TDF_CH_DEBUT=0
TDF_CH_ACTIF=0
TDF_CH_TITRE=""

tdf_ms() {
	local t
	t="$(date +%s%N 2>/dev/null)"
	case "$t" in
		*N|'') echo "$(( $(date +%s) * 1000 ))" ;;
		*) echo "$(( t / 1000000 ))" ;;
	esac
}

tdf_journal() {
	[ -n "${TDF_JOURNAL:-}" ] || return 0
	printf '%s  %-8s %s\n' "$(date +%H:%M:%S)" "${TDF_JOURNAL_MOD:-testeur}" "$*" >> "$TDF_JOURNAL" 2>/dev/null
}

tdf_chrono_debut() {
	eval "TDF_T_$1=\$(tdf_ms)"
	tdf_journal "début : $2"
}

tdf_chrono_fin() {
	local d0 d
	eval "d0=\${TDF_T_$1:-0}"
	d=$(( $(tdf_ms) - d0 ))
	tdf_journal "fin   : $2 ($((d / 1000)).$(printf '%03d' $((d % 1000))) s)${3:+ · $3}"
}

tdf_ch_dessiner() {
	local largeur=28 plein pct i barre="" vide="" ecoule
	[ "$TDF_CH_ACTIF" = 1 ] || return 0
	pct=$(( TDF_CH_FAIT * 100 / TDF_CH_TOTAL ))
	[ "$pct" -gt 100 ] && pct=100
	plein=$(( pct * largeur / 100 ))
	for ((i = 0; i < plein; i++)); do barre="$barre█"; done
	for ((i = plein; i < largeur; i++)); do vide="$vide░"; done
	ecoule=$(( ($(tdf_ms) - TDF_CH_DEBUT) / 1000 ))
	printf '\r\033[K  \033[38;5;213m%s\033[0m  \033[38;5;117m%s\033[38;5;239m%s\033[0m %3d %%  \033[2m%s · %d s\033[0m' \
		"$TDF_CH_TITRE" "$barre" "$vide" "$pct" "$1" "$ecoule"
}

tdf_ch_debut() {
	TDF_CH_TOTAL="${2:-1}"
	[ "$TDF_CH_TOTAL" -gt 0 ] || TDF_CH_TOTAL=1
	TDF_CH_FAIT=0
	TDF_CH_TITRE="$1"
	TDF_CH_DEBUT=$(tdf_ms)
	TDF_CH_ACTIF=0
	{ [ -t 1 ] && [ -z "${NO_COLOR:-}" ]; } && TDF_CH_ACTIF=1
	tdf_journal "chargement : $1 ($TDF_CH_TOTAL étapes)"
	tdf_ch_dessiner "démarrage"
}

tdf_ch_pas() {
	TDF_CH_FAIT=$(( TDF_CH_FAIT + ${2:-1} ))
	tdf_ch_dessiner "$1"
}

tdf_ch_fin() {
	local d
	d=$(( $(tdf_ms) - TDF_CH_DEBUT ))
	TDF_CH_FAIT=$TDF_CH_TOTAL
	tdf_ch_dessiner "${1:-prêt}"
	tdf_journal "chargement terminé : $TDF_CH_TITRE ($((d / 1000)).$(printf '%03d' $((d % 1000))) s)"
	[ "$TDF_CH_ACTIF" = 1 ] && printf '\r\033[K'
	TDF_CH_ACTIF=0
}

tdf_compiler() {
	local sortie="$1" journal_err="$2" cle="$3"
	shift 3
	local cc_opts=() sources=() lien=() etat=cc f o objdir pids=() n=0 ok=1 total
	for f in "$@"; do
		case "$etat:$f" in
			cc:--) etat=src ;;
			src:--) etat=lien ;;
			cc:*) cc_opts+=("$f") ;;
			src:*) sources+=("$f") ;;
			lien:*) lien+=("$f") ;;
		esac
	done
	objdir="$(dirname "$sortie")/obj.$cle"
	if [ -x "$sortie" ] && [ -f "$objdir/ok" ]; then
		tdf_journal "interface déjà compilée (cache $cle)"
		return 0
	fi
	rm -rf "$(dirname "$sortie")"/obj.* 2>/dev/null
	mkdir -p "$objdir" || return 1
	: > "$journal_err"
	total=$(( ${#sources[@]} + 1 ))
	tdf_ch_debut "préparation" "$total"
	for f in "${sources[@]}"; do
		o="$objdir/$(basename "$f" .c).o"
		( "${CC:-cc}" "${cc_opts[@]}" -c "$f" -o "$o" 2>> "$journal_err" ) &
		pids+=($!)
	done
	for o in "${pids[@]}"; do
		wait "$o" || ok=0
		n=$((n + 1))
		tdf_ch_pas "compilation $n/${#sources[@]}"
	done
	if [ "$ok" = 1 ] && "${CC:-cc}" "$objdir"/*.o "${lien[@]}" -o "$sortie" 2>> "$journal_err"; then
		: > "$objdir/ok"
		tdf_ch_fin "prêt"
		return 0
	fi
	tdf_ch_fin "échec"
	tdf_journal "ERREUR compilation de l'interface : $(head -3 "$journal_err" | tr '\n' ' ')"
	rm -rf "$objdir"
	return 1
}

tdf_scene_bin() {
	local mot="$1" d cle
	d="$(cd "$mot/.." && pwd)/.build_accueil"
	cle=$(cat "$mot/lanceur/scene.c" "$mot/term.c" "$mot/rendu.c" "$mot/images.c" "$mot/fous.h" 2>/dev/null | cksum | cut -d' ' -f1)
	TDF_SCENE="$d/scene"
	if [ ! -x "$TDF_SCENE" ] || [ ! -f "$d/scene.$cle" ]; then
		mkdir -p "$d" && rm -f "$d"/scene.* 2>/dev/null
		if "${CC:-cc}" -O2 -w -I "$mot" "$mot/lanceur/scene.c" "$mot/term.c" "$mot/rendu.c" "$mot/images.c" \
			-lm -ldl -o "$TDF_SCENE" 2>/dev/null; then
			: > "$d/scene.$cle"
		else
			TDF_SCENE=""
			return 1
		fi
	fi
	export TDF_SCENE
	return 0
}
