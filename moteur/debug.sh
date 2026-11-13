TDF_DEBUG_DIR="$HOME/.testeur_des_fous/debug"
TDF_DEBUG_GARDE=15

debug_couleurs() {
	if [ -t 1 ] && [ -z "${NO_COLOR:-}" ]; then
		DB_T=$'\033[1;38;5;213m'; DB_B=$'\033[1m'; DB_D=$'\033[2m'; DB_OK=$'\033[1;32m'
		DB_KO=$'\033[1;31m'; DB_WN=$'\033[1;33m'; DB_AC=$'\033[38;5;117m'; DB_R=$'\033[0m'
	else
		DB_T=; DB_B=; DB_D=; DB_OK=; DB_KO=; DB_WN=; DB_AC=; DB_R=
	fi
}

debug_env() {
	local f="$1" distro=""
	shift
	[ -f /etc/os-release ] && distro="$(. /etc/os-release 2>/dev/null; echo "${PRETTY_NAME:-}")"
	{
		echo "date        : $(date '+%Y-%m-%d %H:%M:%S')"
		echo "testeur     : $TDF_VERSION ($TDF_DATE) dans $TDF_RACINE"
		echo "commande    : run.sh $*"
		echo "dossier     : $PWD"
		echo "système     : $(uname -srm 2>/dev/null)"
		echo "distribution: ${distro:-inconnue}"
		grep -qi microsoft /proc/version 2>/dev/null && echo "wsl         : oui (${WSL_DISTRO_NAME:-?})"
		case "$TDF_RACINE" in /mnt/*) echo "disque      : disque Windows (/mnt) : plus lent que le disque Linux" ;; esac
		echo "bash        : ${BASH_VERSION:-?}"
		echo "cc          : $(cc --version 2>/dev/null | head -1)"
		echo "make        : $(make --version 2>/dev/null | head -1)"
		echo "norminette  : $( (norminette --version 2>/dev/null || python3 -m norminette --version 2>/dev/null) | head -1)"
		echo "valgrind    : $(command -v valgrind >/dev/null 2>&1 && valgrind --version 2>/dev/null || echo absent)"
		san_d=$(mktemp -d "${TMPDIR:-/tmp}/tdf_san.XXXXXX") || san_d=""
		[ -n "$san_d" ] && echo 'int main(void){return 0;}' > "$san_d/s.c"
		if [ -n "$san_d" ] && cc -fsanitize=address,undefined "$san_d/s.c" -o "$san_d/s" 2>/dev/null && "$san_d/s" 2>/dev/null; then
			echo "sanitizers  : ASan et UBSan disponibles"
		else
			echo "sanitizers  : indisponibles (repli sur le traqueur mémoire du testeur)"
		fi
		[ -n "$san_d" ] && rm -rf "$san_d"
		echo "terminal    : TERM=${TERM:-?} TERM_PROGRAM=${TERM_PROGRAM:-?} COLORTERM=${COLORTERM:-?}"
		echo "taille      : $(tput cols 2>/dev/null || echo ?) x $(tput lines 2>/dev/null || echo ?)"
		echo "langue      : ${LANG:-?}"
		echo "processeurs : $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo ?)"
		echo "place libre : $(df -h "$HOME" 2>/dev/null | awk 'NR==2 {print $4}')"
	} > "$f" 2>/dev/null
}

debug_ouvrir() {
	local nom n s
	mkdir -p "$TDF_DEBUG_DIR/sessions" 2>/dev/null || return 0
	chmod 700 "$TDF_DEBUG_DIR" 2>/dev/null
	nom="$(date +%Y%m%d_%H%M%S)"
	TDF_SESSION="$TDF_DEBUG_DIR/sessions/$nom"
	mkdir -p "$TDF_SESSION" 2>/dev/null || return 0
	export TDF_JOURNAL="$TDF_SESSION/journal.txt"
	: > "$TDF_JOURNAL"
	printf '%s\n' "$nom" > "$TDF_DEBUG_DIR/derniere"
	debug_env "$TDF_SESSION/environnement.txt" "$@"
	n=0
	for s in $(ls -1 "$TDF_DEBUG_DIR/sessions" 2>/dev/null | sort -r); do
		n=$((n + 1))
		[ "$n" -gt "$TDF_DEBUG_GARDE" ] && rm -rf "${TDF_DEBUG_DIR:?}/sessions/$s"
	done
	TDF_JOURNAL_MOD="lanceur" tdf_journal "session ouverte : run.sh $*"
}

debug_fermer() {
	[ -n "${TDF_JOURNAL:-}" ] || return 0
	TDF_JOURNAL_MOD="lanceur" tdf_journal "session fermée ($1)"
	printf '%s\n' "$1" > "$TDF_SESSION/fin"
}

debug_etat() {
	local d="$1" fin
	fin="$(cat "$d/fin" 2>/dev/null)"
	if [ -z "$fin" ]; then
		echo "en cours ou coupée"
	elif grep -q "ERREUR" "$d/journal.txt" 2>/dev/null; then
		echo "erreur"
	else
		case "$fin" in
			"code 0"|"code 1") echo "terminée normalement" ;;
			"code 130"|interrompu) echo "interrompue (Ctrl+C)" ;;
			*) echo "$fin" ;;
		esac
	fi
}

debug_sessions() {
	ls -1 "$TDF_DEBUG_DIR/sessions" 2>/dev/null | sort -r
}

debug_resoudre() {
	local n="${1:-1}" s
	case "$n" in *[!0-9]*|'') n=1 ;; esac
	s="$(debug_sessions | sed -n "${n}p")"
	[ -n "$s" ] && printf '%s\n' "$TDF_DEBUG_DIR/sessions/$s"
}

debug_liste() {
	local i=0 s d mod etat c
	debug_couleurs
	printf '\n  %sJOURNAL DU TESTEUR%s · %s\n\n' "$DB_T" "$DB_R" "$TDF_DEBUG_DIR/sessions"
	for s in $(debug_sessions); do
		i=$((i + 1))
		d="$TDF_DEBUG_DIR/sessions/$s"
		mod="$(grep -m1 "session ouverte" "$d/journal.txt" 2>/dev/null | sed 's/.*run.sh *//')"
		etat="$(debug_etat "$d")"
		case "$etat" in erreur) c="$DB_KO" ;; "terminée normalement") c="$DB_OK" ;; *) c="$DB_WN" ;; esac
		printf '  %s%2d%s  %s-%s-%s %s:%s  %s%-22s%s %s\n' "$DB_AC" "$i" "$DB_R" "${s:0:4}" "${s:4:2}" "${s:6:2}" "${s:9:2}" "${s:11:2}" \
			"$c" "$etat" "$DB_R" "${mod:-(sans argument)}"
	done
	[ "$i" = 0 ] && printf '  aucune session enregistrée pour l'"'"'instant\n'
	printf '\n  %sbash %s/run.sh debug N%s pour en voir une\n\n' "$DB_D" "$TDF_NOM" "$DB_R"
}

debug_voir() {
	local d f ligne
	debug_couleurs
	d="$(debug_resoudre "${1:-1}")"
	if [ -z "$d" ]; then
		echo "aucune session enregistrée (lance d'abord le testeur)"
		return 1
	fi
	printf '\n  %sSESSION %s%s · %s\n\n' "$DB_T" "$(basename "$d")" "$DB_R" "$(debug_etat "$d")"
	printf '  %sMACHINE%s\n' "$DB_B" "$DB_R"
	sed 's/^/    /' "$d/environnement.txt" 2>/dev/null
	printf '\n  %sDÉROULÉ%s\n' "$DB_B" "$DB_R"
	while IFS= read -r ligne; do
		case "$ligne" in
			*ERREUR*|*interromp*|*"code 1"*|*"code 2"*|*"code 13"*) printf '    %s%s%s\n' "$DB_KO" "$ligne" "$DB_R" ;;
			*"fin   :"*|*terminé*) printf '    %s%s%s\n' "$DB_OK" "$ligne" "$DB_R" ;;
			*) printf '    %s\n' "$ligne" ;;
		esac
	done < "$d/journal.txt"
	printf '\n  %sFICHIERS%s\n' "$DB_B" "$DB_R"
	for f in "$d"/*; do
		[ -f "$f" ] || continue
		printf '    %-28s %6s octets\n' "$(basename "$f")" "$(wc -c < "$f" | tr -d ' ')"
	done
	printf '\n  %scat %s/FICHIER pour le lire · bash %s/run.sh debug paquet pour l'"'"'envoyer%s\n\n' "$DB_D" "$d" "$TDF_NOM" "$DB_R"
}

debug_paquet() {
	local d out f login
	d="$(debug_resoudre "${1:-1}")"
	[ -n "$d" ] || { echo "aucune session enregistrée"; return 1; }
	out="$HOME/testeur_des_fous_debug_$(basename "$d").txt"
	login="$(sed -n 's/^login=//p' "$HOME/.testeur_des_fous/save.txt" 2>/dev/null | head -1)"
	{
		echo "Le testeur des fous · paquet de debug · session $(basename "$d")"
		for f in "$d"/*; do
			[ -f "$f" ] || continue
			echo
			echo "===== $(basename "$f")"
			head -c 200000 "$f"
		done
	} | sed -e "s#$HOME#~#g" -e "s#${USER:-@@@}#<utilisateur>#g" ${login:+-e "s#$login#<login>#g"} > "$out"
	chmod 600 "$out" 2>/dev/null
	echo "paquet créé : $out"
	echo "ni ton code, ni ton login dedans. Envoie-le à splatch_afk sur Discord avec ce qui s'est passé."
}

debug_suivre() {
	local d
	d="$(debug_resoudre 1)"
	[ -n "$d" ] || { echo "aucune session en cours"; return 1; }
	echo "journal de $(basename "$d") en direct (Ctrl+C pour arrêter) :"
	tail -n 30 -f "$d/journal.txt"
}

debug_commande() {
	case "${1:-}" in
		''|voir) debug_voir "${2:-1}" ;;
		liste|ls) debug_liste ;;
		paquet|envoyer) debug_paquet "${2:-1}" ;;
		suivre|direct) debug_suivre ;;
		effacer)
			printf 'effacer tout le journal (%s) ? (o/n) ' "$TDF_DEBUG_DIR/sessions"
			read -r r
			case "$r" in o|O|y|Y) rm -rf "${TDF_DEBUG_DIR:?}/sessions" "$TDF_DEBUG_DIR/derniere"; echo "journal effacé" ;; *) echo "rien n'a été effacé" ;; esac
			;;
		aide|help|-h|--help)
			cat <<EOF
Journal du testeur ($TDF_DEBUG_DIR)

  bash $TDF_NOM/run.sh debug            la dernière session : machine, déroulé, erreurs
  bash $TDF_NOM/run.sh debug liste      toutes les sessions gardées ($TDF_DEBUG_GARDE au plus)
  bash $TDF_NOM/run.sh debug N          la session N de la liste
  bash $TDF_NOM/run.sh debug suivre     le journal en direct, depuis un 2e terminal
  bash $TDF_NOM/run.sh debug paquet     un fichier à envoyer (sans code ni login)
  bash $TDF_NOM/run.sh debug effacer    tout effacer
EOF
			;;
		*[!0-9]*) echo "debug : commande inconnue (bash $TDF_NOM/run.sh debug aide)"; return 2 ;;
		*) debug_voir "$1" ;;
	esac
}
