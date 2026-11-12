TDF_BIN="${TDF_BIN:-$HOME/.local/bin}"
TDF_CMD_NOM="${TDF_CMD_NOM:-tdf}"

tdf_cli_dans_path() {
	case ":$PATH:" in *":$TDF_BIN:"*) return 0 ;; esac
	return 1
}

tdf_cli_installer() {
	local cible="$TDF_BIN/$TDF_CMD_NOM" rc
	mkdir -p "$TDF_BIN" || { echo "impossible de créer $TDF_BIN"; return 1; }
	if [ -e "$cible" ] && ! grep -q 'TDF_CLI=1' "$cible" 2>/dev/null; then
		echo "$cible existe déjà et n'appartient pas au testeur des fous : rien n'est modifié."
		return 1
	fi
	printf '\043!/usr/bin/env bash\nexport TDF_CLI=1\nexec bash "%s/run.sh" "$@"\n' "$TDF_RACINE" > "$cible" || return 1
	chmod 755 "$cible" || return 1
	echo "Commande installée : $cible (sans sudo)"
	echo "Elle lance le testeur rangé dans : $TDF_RACINE"
	if tdf_cli_dans_path; then
		echo "Tape « $TDF_CMD_NOM » dans n'importe quel dossier de projet."
	else
		case "${SHELL:-}" in
			*zsh) rc="~/.zshrc" ;;
			*) rc="~/.bashrc" ;;
		esac
		echo "$TDF_BIN n'est pas dans ton PATH. Ajoute cette ligne à la fin de $rc, puis ouvre un nouveau terminal :"
		echo "  export PATH=\"\$HOME/.local/bin:\$PATH\""
	fi
}

tdf_cli_desinstaller() {
	local cible="$TDF_BIN/$TDF_CMD_NOM"
	if [ ! -e "$cible" ]; then
		echo "Aucune commande $TDF_CMD_NOM installée dans $TDF_BIN."
		return 0
	fi
	if ! grep -q 'TDF_CLI=1' "$cible" 2>/dev/null; then
		echo "$cible n'appartient pas au testeur des fous : rien n'est supprimé."
		return 1
	fi
	rm -f "$cible" && echo "Commande $TDF_CMD_NOM retirée. Le testeur et ta sauvegarde restent en place."
}

tdf_cli_maj() {
	if ! git -C "$TDF_RACINE" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
		echo "Le testeur n'est pas un clone git : mets-le à jour en le clonant de nouveau."
		return 1
	fi
	if [ -n "$(git -C "$TDF_RACINE" status --porcelain --untracked-files=no 2>/dev/null)" ]; then
		echo "Le testeur a des modifications locales : mise à jour annulée pour ne rien écraser."
		return 1
	fi
	echo "Mise à jour du testeur ($TDF_RACINE)..."
	git -C "$TDF_RACINE" pull --ff-only && echo "Le testeur est à jour."
}

tdf_cli_installee() {
	[ -f "$TDF_BIN/$TDF_CMD_NOM" ] && grep -q 'TDF_CLI=1' "$TDF_BIN/$TDF_CMD_NOM" 2>/dev/null
}

tdf_touche() {
	local k s
	IFS= read -rsn1 k || { TOUCHE=q; return; }
	case "$k" in
		$'\033')
			if [ "${BASH_VERSINFO[0]}" -ge 4 ]; then
				IFS= read -rsn2 -t 0.05 s
			else
				IFS= read -rsn2 -t 1 s
			fi
			case "$s" in
				'[A'|'OA') TOUCHE=haut ;;
				'[B'|'OB') TOUCHE=bas ;;
				'[C'|'OC') TOUCHE=droite ;;
				'[D'|'OD') TOUCHE=gauche ;;
				'') TOUCHE=echap ;;
				*) TOUCHE=autre ;;
			esac
			;;
		'') TOUCHE=entree ;;
		k|K|z|Z|w|W) TOUCHE=haut ;;
		j|J|s|S) TOUCHE=bas ;;
		q|Q) TOUCHE=q ;;
		*) TOUCHE=autre ;;
	esac
}

tdf_menu_fleches() {
	local titre="$1" sous="$2" i n sel=0 lab desc
	shift 2
	n=$#
	local items=("$@")
	CHOIX_MENU=-1
	printf '\033[?25l'
	while :; do
		printf '\033[H\033[2J'
		printf '\n  \033[1;36m%s\033[0m\n' "$titre"
		[ -n "$sous" ] && printf '  \033[2m%s\033[0m\n' "$sous"
		printf '\n'
		for ((i = 0; i < n; i++)); do
			lab="${items[$i]%%|*}"
			if [ "$i" = "$sel" ]; then
				printf '  \033[1;36m❯ %s\033[0m\n' "$lab"
			else
				printf '    %s\n' "$lab"
			fi
		done
		desc="${items[$sel]#*|}"
		[ "$desc" = "${items[$sel]}" ] && desc=""
		printf '\n  \033[2m%s\033[0m\n' "$desc"
		printf '\n  \033[2m↑ ↓ choisir   Entrée valider   Échap ou q : retour\033[0m\n'
		tdf_touche
		case "$TOUCHE" in
			haut) sel=$(( (sel + n - 1) % n )) ;;
			bas) sel=$(( (sel + 1) % n )) ;;
			entree|droite) CHOIX_MENU=$sel; break ;;
			echap|q|gauche) break ;;
		esac
	done
	printf '\033[?25h\033[H\033[2J'
}

tdf_accueil_bin() {
	local d="$TDF_RACINE/.build_accueil" cle
	ACCUEIL="$d/accueil"
	cle=$(cat "$TDF_RACINE/moteur/lanceur/accueil.c" "$TDF_RACINE/moteur/entete.c" "$TDF_RACINE/moteur/fou_ascii.c" \
		"$TDF_RACINE/moteur/fous.h" 2>/dev/null | cksum | cut -d' ' -f1)
	[ -x "$ACCUEIL" ] && [ -f "$d/cle.$cle" ] && return 0
	mkdir -p "$d" 2>/dev/null || { ACCUEIL=""; return 1; }
	rm -f "$d"/cle.* 2>/dev/null
	if "${CC:-cc}" -O2 -w -I "$TDF_RACINE/moteur" "$TDF_RACINE/moteur/lanceur/accueil.c" "$TDF_RACINE/moteur/entete.c" \
		"$TDF_RACINE/moteur/fou_ascii.c" -lm -o "$ACCUEIL" 2>/dev/null; then
		: > "$d/cle.$cle"
		return 0
	fi
	ACCUEIL=""
	return 1
}

tdf_choisir() {
	local l1="$1" l2="$2" sous="$3" rc
	shift 3
	if [ -n "${ACCUEIL:-}" ] || tdf_accueil_bin; then
		"$ACCUEIL" --titre "$l1" "$l2" --sous "$sous" "$@"
		rc=$?
		if [ "$rc" != 254 ]; then
			if [ "$rc" = 255 ]; then CHOIX_MENU=-1; else CHOIX_MENU=$rc; fi
			return 0
		fi
	fi
	tdf_menu_fleches "$l1 $l2" "$sous" "$@"
}
