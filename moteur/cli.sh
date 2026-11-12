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
