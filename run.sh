#!/usr/bin/env bash
set -u

TDF_RACINE="$(cd "$(dirname "$0")" && pwd)"
TDF_NOM="$(basename "$TDF_RACINE")"
VERSION_H="$TDF_RACINE/moteur/version.h"
DOSSIER="$HOME/.testeur_des_fous"

if [ ! -f "$VERSION_H" ]; then
	echo "testeur des fous : moteur/version.h introuvable, le dossier est incomplet." >&2
	exit 1
fi

MANQUE_OUTILS=""
for outil in cc make awk sed grep nm tar; do
	command -v "$outil" >/dev/null 2>&1 || MANQUE_OUTILS="$MANQUE_OUTILS $outil"
done
if [ -n "$MANQUE_OUTILS" ]; then
	echo "testeur des fous : il manque sur cette machine :$MANQUE_OUTILS"
	echo "Ces outils sont présents sur les postes 42. Ailleurs, installe-les avec le gestionnaire de paquets."
	exit 1
fi

lire_version() {
	sed -n "s/^# *define $1 \"\(.*\)\"/\1/p" "$VERSION_H" | head -1
}

TDF_VERSION="$(lire_version TDF_VERSION)"
TDF_DATE="$(lire_version TDF_DATE)"
. "$TDF_RACINE/moteur/chargement.sh"
[ -x "$DOSSIER/norminette/bin/norminette" ] && export PATH="$DOSSIER/norminette/bin:$PATH"

resume_partage() {
	local m d="$DOSSIER/derniere" out="$DOSSIER/derniere/resume_partage.txt" os ok tot
	m="$(head -1 "$d/module" 2>/dev/null)"
	os="$( (. /etc/os-release 2>/dev/null; printf '%s' "${PRETTY_NAME:-}") )"
	[ -n "$os" ] || os="$(uname -s)"
	{
		printf '%s %s · %s · %s · %s\n' "$(tt "Le testeur des fous" "The testeur des fous")" "$TDF_VERSION" "$(nom_module "$m")" "$os" "$(date +%d/%m/%Y)"
		case "$m" in
			libft)
				ok="$(grep -cP '^T\t[^\t]+\t(ok|warn)\t' "$d/libft.res")"
				tot="$(grep -cP '^T\t[^\t]+\t(ok|warn|ko|crash|leak|ub|time|build)\t' "$d/libft.res")"
				printf "$(tt "Résultat : %s/%s fonctions OK" "Result: %s/%s functions OK")\n" "$ok" "$tot"
				awk -F'\t' '$1 == "T" && $3 != "ok" && $3 != "warn" && $3 != "skip" { printf "- ft_%s : %s%s\n", $2, toupper($3), ($6 != "" ? " · " $6 : "") }' "$d/libft.res" | head -12
				awk -F'\t' '$1 == "P" && $3 != "ok" && $3 != "warn" { printf "- prototype ft_%s : %s\n", $2, $4 }' "$d/libft.res" | head -5
				awk -F'\t' '$1 == "A" && $2 == "ko" { printf "- audit : %s\n", $3 }' "$d/libft.res" | head -5
				;;
			printf)
				awk -F'\t' '$1 == "S" && $2 == "KO" { printf "- KO : %s\n", $3 }' "$d/printf.res" | head -8
				awk -F'\t' '$1 == "C" { t++; if ($5 > 0) { k++; printf "- %s : %s KO\n", $3, $5 } } END { printf "%d/%d catégories OK\n", t - k, t }' "$d/printf.res"
				;;
			gnl)
				awk -F'\t' '$1 == "F" { printf "%s OK · %s KO · BUFFER_SIZE %s\n", $2, $3, $5 }' "$d/gnl.res"
				awk -F'\t' '$1 == "L" && $2 != "OK" && $2 != "NOTE" && $2 != "INFO" { printf "- %s : %s\n", $2, $3 }' "$d/gnl.res" | head -12
				;;
			*)
				printf "%s\n" "$(tt "Aucune tentative Libft, ft_printf ou get_next_line à résumer pour l'instant." "No Libft, ft_printf or get_next_line attempt to summarise yet.")"
				;;
		esac
		printf "\n%s\n" "$(tt "(sans ton code ni tes chemins : tu peux le coller sur Discord pour demander de l'aide)" "(without your code or your paths: you can paste it on Discord to ask for help)")"
	} | sed -e "s#$HOME#~#g" -e "s#/home/[^/ ]*#~#g" > "$out"
	voir_rapport "$out" "$(tt "Résumé à partager" "Summary to share")"
}

norminette_etat() {
	if tdf_norminette_ok; then unset TDF_NORME_ABSENTE; else export TDF_NORME_ABSENTE=1; fi
}

norminette_installer() {
	local v="$DOSSIER/norminette"
	command -v python3 >/dev/null 2>&1 || { tt "python3 introuvable : impossible d'installer la norminette sans lui." "python3 not found: the norminette cannot be installed without it."; echo; return 1; }
	echo "$(tt "Environnement Python du testeur" "Tester's Python environment") : $v"
	if python3 -m venv "$v" && "$v/bin/python" -m pip install -q --upgrade norminette; then
		export PATH="$v/bin:$PATH"
		return 0
	fi
	rm -rf "$v"
	echo "$(tt "Environnement impossible (python3-venv manquant ?), essai avec pip --user." "Environment impossible (python3-venv missing?), trying pip --user.")"
	python3 -m pip install -q --user --upgrade norminette
}
. "$TDF_RACINE/moteur/langue.sh"
. "$TDF_RACINE/moteur/debug.sh"
. "$TDF_RACINE/moteur/cli.sh"
ARGS_ORIG=("$@")

usage() {
	local c="bash $TDF_NOM/run.sh"
	[ "${TDF_CLI:-0}" = 1 ] && c="$TDF_CMD_NOM"
	printf 'Le testeur des fous %s\n' "$TDF_VERSION"
	tl "Créé et vibe-codé par Splatch (romasant)" "Created and vibe-coded by Splatch (romasant)"
	printf '\n'
	u() { printf '  %s %-17s %s\n' "$c" "$1" "$2"; }
	u "" "$(tt "ouvrir le menu (projets détectés, exam, banque...) aux flèches" "open the menu (detected projects, exam, bank...) with the arrows")"
	u "libft" "$(tt "tester ta Libft" "test your Libft")"
	u "printf" "$(tt "tester ton ft_printf" "test your ft_printf")"
	u "gnl" "$(tt "tester ton get_next_line" "test your get_next_line")"
	u "exam" "$(tt "passer un exam en C (4 niveaux, grademe)" "take a C exam (4 levels, grademe)")"
	u "exam --serie 3 --difficulte croissante" "$(tt "une série de 1 à 5 exos (croissante ou 1 à 4)" "a series of 1 to 5 exercises (croissante or 1 to 4)")"
	u "banque" "$(tt "la banque d'exercices (exam, Libft, printf)" "the exercise bank (exam, Libft, printf)")"
	u "correction [CHEMIN]" "$(tt "le mode correction, sur le dossier du corrigé" "grading mode, on the graded folder")"
	u "--menu" "$(tt "ouvrir le menu sans lancer de tests (stats, chevalier...)" "open the menu without running tests (stats, knight...)")"
	u "debug" "$(tt "le journal de la dernière session (debug aide : tout)" "the log of the last session (debug aide: everything)")"
	u "--projet CHEMIN" "$(tt "tester le projet de ce dossier" "test the project in this folder")"
	u "--no-lore" "$(tt "les tests seuls, sans histoire" "tests only, no story")"
	u "-a" "$(tt "sans animations pour ce lancement" "no animations for this run")"
	u "--anim off" "$(tt "couper les animations pour de bon (on pour les remettre)" "turn animations off for good (on to bring them back)")"
	u "--version" "$(tt "le numéro de version" "the version number")"
	printf '\n'
	tl "La commande « %s », utilisable depuis n'importe quel dossier, sans sudo :" "The \"%s\" command, usable from any folder, no sudo:" "$TDF_CMD_NOM"
	printf '  %-36s %s\n' "bash $TDF_NOM/run.sh installer" "$(tt "installe ~/.local/bin/tdf" "installs ~/.local/bin/tdf")"
	printf '  %-36s %s\n' "$TDF_CMD_NOM maj" "$(tt "met le testeur à jour (git pull)" "updates the tester (git pull)")"
	printf '  %-36s %s\n' "$TDF_CMD_NOM desinstaller" "$(tt "retire la commande (le testeur reste)" "removes the command (the tester stays)")"
	printf '\n'
	tl "Les options d'un module : %s libft --help   ou   %s printf --help" "A module's options: %s libft --help   or   %s printf --help" "$c" "$c"
}

est_printf() {
	[ -f "$1/Makefile" ] || return 1
	[ -f "$1/ft_printf.h" ] && return 0
	grep -q 'libftprintf' "$1/Makefile" 2>/dev/null
}

est_libft() {
	[ -f "$1/libft.h" ] || return 1
	est_printf "$1" && return 1
	return 0
}

est_gnl() {
	[ -f "$1/get_next_line.c" ] && [ -f "$1/get_next_line.h" ]
}

LIBFTS=()
PRINTFS=()
GNLS=()
VUS="|"

examiner() {
	local d
	[ -d "$1" ] || return 0
	d="$(cd "$1" 2>/dev/null && pwd -P)" || return 0
	[ "$d" = "$(cd "$TDF_RACINE" && pwd -P)" ] && return 0
	case "$VUS" in *"|$d|"*) return 0 ;; esac
	VUS="$VUS$d|"
	if est_printf "$d"; then
		PRINTFS+=("$d")
	elif est_libft "$d"; then
		LIBFTS+=("$d")
	elif est_gnl "$d"; then
		GNLS+=("$d")
	fi
}

detecter() {
	local base d
	examiner "$PWD"
	examiner "$TDF_RACINE/.."
	for base in "$PWD" "$TDF_RACINE/.."; do
		for d in "$base"/*/; do
			[ -d "$d" ] && examiner "$d"
		done
	done
}

court() {
	case "$1" in
		"$PWD") printf '.' ;;
		"$PWD"/*) printf './%s' "${1#"$PWD"/}" ;;
		"$HOME"/*) printf '~/%s' "${1#"$HOME"/}" ;;
		*) printf '%s' "$1" ;;
	esac
}

LANCEUR="$DOSSIER/lanceur.txt"
INTERFACE="$(sed -n 's/^interface=//p' "$LANCEUR" 2>/dev/null | head -1)"
[ "$INTERFACE" = texte ] && INTERFACE=graphique

ecrire_interface() {
	mkdir -p "$DOSSIER" 2>/dev/null
	printf 'interface=%s\n' "$1" > "$LANCEUR"
	INTERFACE="$1"
}

tuto() {
	local i n=6 titres=() textes=() items=() prec suiv passer
	titres=(
		"$(tt "Bienvenue" "Welcome")"
		"$(tt "Tester un projet" "Test a project")"
		"$(tt "Exam en C" "C exam")"
		"$(tt "Banque d'exercices" "Exercise bank")"
		"$(tt "Chevalier, Lore et Stats" "Knight, Lore and Stats")"
		"$(tt "Paramètres" "Settings")"
	)
	textes=(
		"$(tt "Le testeur des fous teste ta Libft, ton ft_printf et ton get_next_line comme la moulinette, et t'entraîne à l'exam. Tout se fait aux flèches : ↑ ↓ pour choisir, Entrée pour valider, Échap ou q pour revenir." "The testeur des fous tests your Libft, ft_printf and get_next_line like the moulinette, and trains you for the exam. Everything works with the arrows: ↑ ↓ to pick, Enter to confirm, Esc or q to go back.")"
		"$(tt "Le menu liste les projets trouvés dans ce dossier ou à côté du testeur. Choisis-en un : tests des fonctions, fuites mémoire, Norme, fichiers parasites et audit de la moulinette. À la fin du rapport, Entrée te ramène au menu." "The menu lists the projects found in this folder or next to the tester. Pick one: function tests, memory leaks, the Norm, garbage files and the moulinette audit. At the end of the report, Enter brings you back to the menu.")"
		"$(tt "Examen blanc de 3 h, série d'exercices ou entraînement libre. Tu écris ton code dans un autre terminal, puis tu tapes grademe dans l'examshell. Une commande mal tapée ? Le testeur te propose la bonne." "3-hour mock exam, exercise series or free training. You write your code in another terminal, then type grademe in the examshell. Mistyped a command? The tester suggests the right one.")"
		"$(tt "Tous les exercices du testeur, dans l'ordre que tu veux. ◀ ▶ change d'onglet (Exam, Libft, printf), Entrée ouvre le sujet, puis « Corriger mon code ». Chaque sujet existe en 8 langues." "Every exercise of the tester, in any order. ◀ ▶ switches tabs (Exam, Libft, printf), Enter opens the subject, then 'Grade my code'. Every subject exists in 8 languages.")"
		"$(tt "Chaque fonction validée rapporte de l'expérience et des Octets. Habille ton chevalier, débloque l'histoire de la Cité qui compile et suis ta progression dans Stats." "Every function you pass earns experience and Octets. Dress your knight, unlock the story of the City that compiles and follow your progress in Stats.")"
		"$(tt "Langue, interface graphique ou texte, et la commande tdf pour lancer le testeur depuis n'importe quel dossier. Ce tuto et les nouveautés se relancent depuis Paramètres." "Language, graphical or text interface, and the tdf command to start the tester from any folder. This tutorial and the news can be replayed from Settings.")"
	)
	prec="$(tt "Précédent|" "Previous|")"
	passer="$(tt "Passer le tuto|" "Skip the tutorial|")"
	i=0
	while [ "$i" -lt "$n" ]; do
		items=()
		if [ "$i" -lt $((n - 1)) ]; then
			suiv="$(tt "Suivant|" "Next|")"
		else
			suiv="$(tt "C'est parti|" "Let's go|")"
		fi
		items+=("$suiv")
		[ "$i" -gt 0 ] && items+=("$prec")
		[ "$i" -lt $((n - 1)) ] && items+=("$passer")
		tdf_page "$(tt "Tuto" "Tutorial") · $((i + 1))/$n · ${titres[$i]}" "${textes[$i]}" "${items[@]}"
		[ "$CHOIX_MENU" -ge 0 ] || return 0
		case "${items[$CHOIX_MENU]}" in
			"$prec") i=$((i - 1)) ;;
			"$passer") return 0 ;;
			*) i=$((i + 1)) ;;
		esac
	done
}

nouveautes() {
	local t
	t="$(tt "• Un seul menu : bash run.sh ou tdf ouvre directement le menu, et tout s'y passe." "• One single menu: bash run.sh or tdf opens the menu directly, and everything happens there.")"
	t+=$'\n'"$(tt "• Banque d'exercices aux flèches : onglets, sujet, correction en un clic." "• Exercise bank with the arrows: tabs, subject, grading in one click.")"
	t+=$'\n'"$(tt "• Sujets de l'exam et de la banque en 8 langues." "• Exam and bank subjects in 8 languages.")"
	t+=$'\n'"$(tt "• Une commande mal tapée dans l'examshell ? Le testeur propose la bonne." "• Mistyped a command in the examshell? The tester suggests the right one.")"
	t+=$'\n'"$(tt "• Un tuto, à revoir depuis Paramètres." "• A tutorial, to replay from Settings.")"
	tdf_page "$(printf "$(tt "Nouveautés de la version %s" "What's new in version %s")" "$TDF_VERSION")" "$t" \
		"$(tt "Continuer|" "Continue|")" "$(tt "Voir le tuto|" "See the tutorial|")"
	[ "$CHOIX_MENU" = 1 ] && tuto
	return 0
}

premiers_pas() {
	[ -n "$INTERFACE" ] || ecrire_interface graphique
}

preparer_suite() {
	if [ "${INTERFACE:-}" = texte ] && { [ "$MODULE" = libft ] || [ "$MODULE" = printf ]; }; then
		case " ${SUITE[*]:-} " in *" --no-lore "*) ;; *) SUITE+=(--no-lore) ;; esac
	fi
}

attente_ligne() {
	printf '\033[%d;1H\033[2K%*s%s' "$1" "$2" '' "$3"
}

attente_module() {
	local pid="$1" f="$2" cle="$3" titre="$4" t0 e est h w y x pct plein i barre vide etape info code tour=0 lb=30
	mkdir -p "$DOSSIER/durees"
	est="$(cat "$DOSSIER/durees/$cle" 2>/dev/null)"
	case "$est" in ''|*[!0-9]*) est=0 ;; esac
	if [ "$est" = 0 ]; then
		case "$cle" in
			libft*debug*) est=45 ;;
			printf*debug*) est=35 ;;
			gnl*debug*) est=15 ;;
		esac
	fi
	t0=$(tdf_ms)
	printf '\033[2J'
	while kill -0 "$pid" 2>/dev/null; do
		h=24; w=80
		read -r h w < <(stty size 2>/dev/null </dev/tty) || true
		case "$h$w" in *[!0-9]*|'') h=24; w=80 ;; esac
		e=$(( ($(tdf_ms) - t0) / 1000 ))
		[ "$w" -lt 48 ] && lb=$(( w - 18 ))
		[ "$lb" -lt 8 ] && lb=8
		barre=""; vide=""
		if [ "$est" -gt 0 ]; then
			pct=$(( e * 100 / est ))
			[ "$pct" -gt 99 ] && pct=99
			plein=$(( pct * lb / 100 ))
			for ((i = 0; i < plein; i++)); do barre+="█"; done
			for ((i = plein; i < lb; i++)); do vide+="░"; done
			if [ "$e" -lt "$est" ]; then
				info="$(printf "$(tt "%d s écoulées · environ %d s restantes" "%d s elapsed · about %d s left")" "$e" "$((est - e))")"
			else
				info="$(printf "$(tt "%d s écoulées · presque fini" "%d s elapsed · almost done")" "$e")"
			fi
			pct="$(printf '%3d %%' "$pct")"
		else
			for ((i = 0; i < lb; i++)); do
				if [ $(( (i - tour + lb) % lb )) -lt 6 ]; then barre+="█"; else barre+="░"; fi
			done
			tour=$(( (tour + 1) % lb ))
			pct="     "
			info="$(printf "$(tt "%d s écoulées · première fois : pas encore d'estimation" "%d s elapsed · first time: no estimate yet")" "$e")"
		fi
		etape="$(tail -c 4000 "$f" 2>/dev/null | sed -e $'s/\x1b\\[[0-9;?]*[A-Za-z]//g' -e 's/\r/\n/g' | grep -v '^[[:space:]]*$' | tail -1 | sed -e 's/^[[:space:]]*//')"
		etape="${etape:0:$(( w - 8 ))}"
		y=$(( h / 2 - 3 )); [ "$y" -lt 1 ] && y=1
		x=$(( (w - lb - 8) / 2 )); [ "$x" -lt 2 ] && x=2
		attente_ligne "$y" "$x" "$(printf '\033[1;38;5;213m%s\033[0m' "$titre")"
		attente_ligne $((y + 2)) "$x" "$(printf '\033[38;5;117m%s\033[38;5;239m%s\033[0m %s' "$barre" "$vide" "$pct")"
		attente_ligne $((y + 3)) "$x" "$(printf '\033[2m%s\033[0m' "$info")"
		attente_ligne $((y + 5)) "$x" "$(printf '\033[38;5;250m%s\033[0m' "$etape")"
		attente_ligne $((y + 7)) "$x" "$(printf '\033[2m%s\033[0m' "$(tt "Ctrl+C pour arrêter" "Ctrl+C to stop")")"
		sleep 0.5
	done
	wait "$pid"
	code=$?
	e=$(( ($(tdf_ms) - t0) / 1000 ))
	{ [ "$code" = 0 ] || [ "$code" = 1 ]; } && [ "$e" -gt 0 ] && printf '%s\n' "$e" > "$DOSSIER/durees/$cle"
	printf '\033[2J'
	return "$code"
}

cle_duree() {
	local c="$MODULE ${SUITE[*]:-}"
	c="${c// /_}"
	printf '%s' "${c//[^A-Za-z0-9_]/}"
}

nom_module() {
	case "$1" in
		libft) printf 'Libft' ;;
		printf) printf 'ft_printf' ;;
		gnl) printf 'get_next_line' ;;
		*) printf '%s' "$1" ;;
	esac
}

titre_attente() {
	[ -n "${CORR_TITRE:-}" ] && { printf '%s' "$CORR_TITRE"; return; }
	case " ${SUITE[*]:-} " in
		*" --debug "*) printf "$(tt "Mode debug · %s : tous les rapports détaillés" "Debug mode · %s: every detailed report")" "$1" ;;
		*) printf "$(tt "Tests de %s en cours" "Testing %s")" "$1" ;;
	esac
}

voir_rapport() {
	TDF_ECRAN=rapport TDF_RAPPORT="$1" TDF_RAPPORT_TITRE="$2" TDF_PROJET="" bash "$TDF_RACINE/modules/libft/run.sh" --menu
}

voir_gnl() {
	TDF_ECRAN=gnl TDF_PROJET="" bash "$TDF_RACINE/modules/libft/run.sh" --menu
}

revoir_derniere() {
	local m
	m="$(head -1 "$DOSSIER/derniere/module" 2>/dev/null)"
	case "$m" in
		gnl) voir_gnl ;;
		libft|printf)
			TDF_ECRAN=revoir TDF_REVOIR="$DOSSIER/derniere/$m.res" TDF_PROJET="" bash "$TDF_RACINE/modules/$m/run.sh" --menu
			;;
		banque) voir_rapport "$DOSSIER/derniere/banque.txt" "$(tt "Banque d'exercices · dernière correction" "Exercise bank · last grading")" ;;
		*) voir_rapport "$DOSSIER/derniere/rapport.txt" "$(tt "Dernière tentative" "Last attempt")" ;;
	esac
}

lore_scene() {
	tdf_scene_bin "$TDF_RACINE/moteur" >/dev/null 2>&1 || return 0
	case "$1" in
		lore_gnl) "$TDF_SCENE" "$TDF_RACINE/modules/gnl" puits "$(head -1 "$DOSSIER/puits.txt" 2>/dev/null)" ;;
		lore_exam)
			"$TDF_SCENE" "$TDF_RACINE/modules/exam" avant
			"$TDF_SCENE" "$TDF_RACINE/modules/exam" apres
			;;
		lore_fin) "$TDF_SCENE" "$TDF_RACINE/modules/exam" fin ;;
	esac
}

login_corrige() {
	local l
	l="$(head -1 "$1/README.md" 2>/dev/null | sed -n 's/.* by \([^*_]*\).*/\1/p' | sed 's/[[:space:].]*$//')"
	[ -n "$l" ] || l="$(basename "$1")"
	printf '%s' "${l:0:40}"
}

correction_preparer() {
	local a="$1" p="$2" t="" quoi
	case "$a" in corr_test) t="${p#*$'\t'}"; p="${p%%$'\t'*}" ;; esac
	PROJET="$p"
	if est_printf "$PROJET"; then MODULE=printf
	elif est_libft "$PROJET"; then MODULE=libft
	elif est_gnl "$PROJET"; then MODULE=gnl
	else
		mkdir -p "$DOSSIER/derniere"
		printf "$(tt "Je ne reconnais ni une Libft, ni un ft_printf, ni un get_next_line dans %s" "I recognise neither a Libft, nor a ft_printf, nor a get_next_line in %s")\n" "$PROJET" > "$DOSSIER/derniere/message.txt"
		voir_rapport "$DOSSIER/derniere/message.txt" "$(tt "testeur des fous | mode : correction" "testeur des fous | mode: grading")"
		return 1
	fi
	export TDF_SANS_HISTOIRE=1 TDF_SANS_STATS=1
	TDF_BANDEAU="$(tt "testeur des fous | mode : correction" "testeur des fous | mode: grading") · $(login_corrige "$PROJET")"
	export TDF_BANDEAU
	if [ ! -d "$DOSSIER/derniere_hors_correction" ]; then
		mkdir -p "$DOSSIER/derniere"
		cp -a "$DOSSIER/derniere" "$DOSSIER/derniere_hors_correction"
	fi
	case " ${SUITE[*]:-} " in *" --fast "*) ;; *) SUITE+=(--fast) ;; esac
	case "$a" in
		corr_tout) return 0 ;;
		corr_rendu)
			SUITE+=(--rendu)
			quoi="$(tt "Vérifs de rendu" "Submission checks")"
			;;
		corr_test)
			case "$MODULE" in
				libft) SUITE+=(-v "$t"); quoi="ft_$t"; export TDF_CAS=1 ;;
				printf) SUITE+=(-v --no-lore --only "$t"); quoi="ft_printf %$t" ;;
				gnl) SUITE+=(-v --tailles "$t"); quoi="get_next_line BUFFER_SIZE=$t" ;;
			esac
			;;
	esac
	CORR_TITRE="$TDF_BANDEAU · $quoi · $(nom_module "$MODULE")"
	return 0
}

pousse_preparer() {
	local top rel t d msg="$DOSSIER/derniere/message.txt"
	PROJET_VRAI="$PROJET"
	mkdir -p "$DOSSIER/derniere"
	if ! git -C "$PROJET" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
		printf "$(tt "%s n'est pas un dépôt git : le testeur teste le dossier tel quel." "%s is not a git repository: the tester tests the folder as it is.")\n" "$PROJET" > "$msg"
		voir_rapport "$msg" "$(tt "Ce qui est poussé" "What is pushed")"
		return 0
	fi
	top="$(git -C "$PROJET" rev-parse --show-toplevel)"
	rel="$(git -C "$PROJET" rev-parse --show-prefix)"
	t="$(mktemp -d "${TMPDIR:-/tmp}/tdf_pousse.XXXXXX")" || return 0
	if ! git -C "$top" archive HEAD 2>/dev/null | tar -x -C "$t" 2>/dev/null; then
		rm -rf "$t"
		printf "$(tt "Aucun commit dans %s : le testeur teste le dossier tel quel." "No commit in %s: the tester tests the folder as it is.")\n" "$top" > "$msg"
		voir_rapport "$msg" "$(tt "Ce qui est poussé" "What is pushed")"
		return 0
	fi
	POUSSE_TMP="$t"
	PROJET="$t/${rel%/}"
	PROJET="${PROJET%/}"
	d="$(cd "$PROJET_VRAI" && { git ls-files --others --exclude-standard . ; git diff --name-only --relative HEAD ; } 2>/dev/null | sort -u | grep -Ev '^(testeur_des_fous|tdf)/' | head -25)"
	if [ -n "$d" ]; then
		{
			printf "$(tt "Le testeur teste ton dernier commit (%s). Ces fichiers ne sont pas dedans, ou ont changé depuis : la moulinette ne les verra pas tels qu'ils sont dans ton dossier." "The tester tests your last commit (%s). These files are not in it, or changed since: the moulinette will not see them as they are in your folder.")\n\n" "$(git -C "$top" log -1 --format='%h %s' | cut -c1-60)"
			printf '%s\n' "$d" | sed 's/^/  /'
			printf "\n$(tt "Pour les envoyer : git add, git commit, puis git push." "To send them: git add, git commit, then git push.")\n"
		} > "$msg"
		voir_rapport "$msg" "$(tt "Ce qui est poussé" "What is pushed")"
	fi
	return 0
}

options_test() {
	local f="$DOSSIER/options_test.txt"
	[ -n "${CORR_TITRE:-}" ] && return 0
	[ -f "$DOSSIER/correction.txt" ] && return 0
	unset TDF_SANS_HISTOIRE
	case "$MODULE" in libft|printf|gnl) ;; *) return 0 ;; esac
	[ -f "$f" ] || return 0
	grep -q '^debug=1' "$f" && SUITE+=(--debug)
	grep -q '^audit=1' "$f" && [ "$MODULE" = libft ] && SUITE+=(--audit)
	grep -q '^valgrind=1' "$f" && { [ "$MODULE" = libft ] || [ "$MODULE" = printf ]; } && SUITE+=(--valgrind)
	grep -q '^sans_anim=1' "$f" && SUITE+=(--fast)
	grep -q '^sans_histoire=1' "$f" && export TDF_SANS_HISTOIRE=1
	grep -q '^pousse=1' "$f" && pousse_preparer
	return 0
}

rapport_texte() {
	[ -n "${CORR_TITRE:-}" ] && return 0
	case " ${SUITE[*]:-} " in *" --debug "*) return 0 ;; esac
	[ "$MODULE" = gnl ] && return 0
	{ [ "$MODULE" = libft ] || [ "$MODULE" = printf ]; } && case " ${SUITE[*]:-} " in *" --no-lore "*) return 0 ;; esac
	return 1
}

noter_historique() {
	local m="$1" p="$2" l ok=0 tot=0 h="$DOSSIER/historique_projets.txt"
	[ -n "$p" ] || return 0
	[ -z "${TDF_SANS_STATS:-}" ] || return 0
	case "$m" in libft|printf|gnl) ;; *) return 0 ;; esac
	l="$(grep "^R [0-9]* $m " "$DOSSIER/stats.txt" 2>/dev/null | tail -1)"
	[ -n "$l" ] && ok="$(echo "$l" | cut -d' ' -f4)" && tot="$(echo "$l" | cut -d' ' -f5)"
	mkdir -p "$DOSSIER"
	printf '%s\t%s\t%s\t%s\t%s\n' "$(date +%s)" "$m" "$p" "$ok" "$tot" >> "$h"
	awk -F'\t' '{ d[$3] = $0; o[$3] = NR } END { for (k in d) print o[k] "\t" d[k] }' "$h" | sort -n | cut -f2- | tail -10 > "$h.tmp" && mv "$h.tmp" "$h"
}

choisir_au_menu() {
	local code l a p projets=""
	norminette_etat
	if [ "$(head -c1 "$DOSSIER/couleurs.txt" 2>/dev/null)" = 1 ]; then export TDF_DALTONIEN=1; else unset TDF_DALTONIEN; fi
	unset TDF_SANS_STATS TDF_CAS TDF_BANDEAU
	PROJET_VRAI=""
	if [ ! -f "$DOSSIER/correction.txt" ] && [ -d "$DOSSIER/derniere_hors_correction" ]; then
		rm -rf "$DOSSIER/derniere"
		mv "$DOSSIER/derniere_hors_correction" "$DOSSIER/derniere"
	fi
	for p in ${LIBFTS[@]+"${LIBFTS[@]}"}; do projets+="libft"$'\t'"$p"$'\n'; done
	for p in ${PRINTFS[@]+"${PRINTFS[@]}"}; do projets+="printf"$'\t'"$p"$'\n'; done
	for p in ${GNLS[@]+"${GNLS[@]}"}; do projets+="gnl"$'\t'"$p"$'\n'; done
	rm -f "$DOSSIER/menu_choix"
	a=""
	[ "$SANS_ANIM" = 1 ] && a="--fast"
	TDF_PROJETS="$projets" TDF_DEPART="$DEPART" TDF_PROJET="" bash "$TDF_RACINE/modules/libft/run.sh" --menu $a
	code=$?
	unset TDF_CORR_ACCUEIL
	export TDF_NO_UPDATE=1
	tdf_langue_lire
	INTERFACE=graphique
	export TDF_INTERFACE=graphique
	case "$code" in
		46) ;;
		0|130|143) debug_fermer "code 0"; exit 0 ;;
		*)
			menu_depart
			preparer_suite
			return 0
			;;
	esac
	l="$(head -1 "$DOSSIER/menu_choix" 2>/dev/null)"
	a="${l%%|*}"
	p="${l#*|}"
	SUITE=()
	[ "$SANS_ANIM" = 1 ] && SUITE+=(--fast)
	unset TDF_ECRAN
	CORR_TITRE=""
	unset TDF_CAS
	case "$a" in
		corr_tout|corr_rendu|corr_test)
			correction_preparer "$a" "$p" || return 1
			return 0
			;;
		libft|printf|gnl) MODULE="$a"; PROJET="$p" ;;
		revoir) revoir_derniere; return 1 ;;
		lore_printf) TDF_ECRAN=lore TDF_PROJET="" bash "$TDF_RACINE/modules/printf/run.sh" --menu; return 1 ;;
		lore_gnl|lore_exam|lore_fin) lore_scene "$a"; return 1 ;;
		exam) MODULE=exam; PROJET="" ;;
		banque) MODULE=exam; PROJET=""; SUITE+=(--banque) ;;
		autre)
			PROJET="$p"
			if est_printf "$PROJET"; then MODULE=printf
			elif est_libft "$PROJET"; then MODULE=libft
			elif est_gnl "$PROJET"; then MODULE=gnl
			else
				mkdir -p "$DOSSIER/derniere"
				printf "$(tt "Je ne reconnais ni une Libft, ni un ft_printf, ni un get_next_line dans %s" "I recognise neither a Libft, nor a ft_printf, nor a get_next_line in %s")\n" "$PROJET" > "$DOSSIER/derniere/message.txt"
				voir_rapport "$DOSSIER/derniere/message.txt" "$(tt "Tester mon projet" "Test my project")"
				return 1
			fi
			;;
		installer)
			mkdir -p "$DOSSIER/derniere"
			tdf_cli_installer > "$DOSSIER/derniere/message.txt" 2>&1
			voir_rapport "$DOSSIER/derniere/message.txt" "$(tt "Commande tdf" "tdf command")"
			return 1
			;;
		resume)
			resume_partage
			return 1
			;;
		norminette)
			mkdir -p "$DOSSIER/derniere"
			: > "$DOSSIER/derniere/message.txt"
			CORR_TITRE="$(tt "Installation de la norminette" "Installing the norminette")"
			norminette_installer > "$DOSSIER/derniere/message.txt" 2>&1 < /dev/null &
			attente_module $! "$DOSSIER/derniere/message.txt" "norminette_installation" "$CORR_TITRE"
			CORR_TITRE=""
			[ -x "$DOSSIER/norminette/bin/norminette" ] && export PATH="$DOSSIER/norminette/bin:$PATH"
			if tdf_norminette_ok; then
				printf "\n$(tt "La norminette est installée (%s), sans sudo. Le testeur l'utilise maintenant pour Libft, ft_printf et get_next_line." "The norminette is installed (%s), without sudo. The tester now uses it for Libft, ft_printf and get_next_line.")\n" "$(norminette --version 2>/dev/null | head -1)" >> "$DOSSIER/derniere/message.txt"
			else
				printf "\n$(tt "L'installation n'a pas marché. Sur un poste 42, le réseau ou pip peuvent être bloqués : demande au staff, ou essaie chez toi." "The install did not work. On a 42 computer, the network or pip may be blocked: ask the staff, or try at home.")\n" >> "$DOSSIER/derniere/message.txt"
			fi
			norminette_etat
			voir_rapport "$DOSSIER/derniere/message.txt" "$(tt "Installation de la norminette" "Installing the norminette")"
			return 1
			;;
		*) return 1 ;;
	esac
	preparer_suite
	options_test
	return 0
}

menu_params() {
	local items=() acts=()
	items+=("$(tt "Réglages du testeur|Animations, histoire, niveau d'aide, sauvegarde, crédits" "Tester settings|Animations, story, help level, save, credits")")
	acts+=("params")
	items+=("$(tt "Langue|" "Language|")${TDF_NOMS[${TDF_LANGUE_N:-0}]}")
	acts+=("langue")
	if [ "$INTERFACE" = texte ]; then
		items+=("$(tt "Interface : mode texte|Entrée pour passer à l'interface graphique (lore, animations)" "Interface: text mode|Enter to switch to the graphical interface (lore, animations)")")
	else
		items+=("$(tt "Interface : graphique|Entrée pour passer au mode texte (les tests seuls, sans animation)" "Interface: graphical|Enter to switch to text mode (tests only, no animation)")")
	fi
	acts+=("interface")
	if ! tdf_cli_installee; then
		items+=("$(tt "Installer la commande tdf|Ensuite tu tapes seulement « tdf » dans n'importe quel dossier (sans sudo)" "Install the tdf command|Then you just type \"tdf\" in any folder (no sudo)")")
		acts+=("installer")
	fi
	items+=("$(tt "Revoir le tuto|Les 6 écrans qui présentent le testeur" "Replay the tutorial|The 6 screens that present the tester")")
	acts+=("tuto")
	items+=("$(printf "$(tt "Nouveautés de la version %s|" "What's new in version %s|")" "$TDF_VERSION")")
	acts+=("nouveautes")
	items+=("$(tt "Retour|" "Back|")")
	acts+=("retour")
	tdf_choisir "TESTEUR" "DES FOUS" "$(tt "Paramètres" "Settings")" "${items[@]}"
	[ "$CHOIX_MENU" -ge 0 ] || return 1
	case "${acts[$CHOIX_MENU]}" in
		params) return 0 ;;
		tuto) tuto ;;
		nouveautes) nouveautes ;;
		langue)
			local i noms=()
			for i in 0 1 2 3 4 5 6 7; do noms+=("${TDF_NOMS[$i]}|"); done
			tdf_choisir "TESTEUR" "DES FOUS" "Langue · Language · Idioma · Sprache · Lingua · Dil · Язык" "${noms[@]}"
			[ "$CHOIX_MENU" -ge 0 ] && tdf_langue_ecrire "$CHOIX_MENU"
			;;
		interface)
			if [ "$INTERFACE" = texte ]; then ecrire_interface graphique; else ecrire_interface texte; fi
			;;
		installer)
			printf '\n'
			tdf_cli_installer
			printf '\n  %s ' "$(tt "Entrée pour revenir au menu" "Enter to go back to the menu")"
			read -r r
			;;
	esac
	return 1
}

menu_depart() {
	local items=() acts=() p a sous n
	while :; do
		items=()
		acts=()
		for p in ${LIBFTS[@]+"${LIBFTS[@]}"}; do
			items+=("$(tt "Tester ma Libft         " "Test my Libft           ") $(court "$p")|$(tt "Les 43 fonctions, la mémoire, la Norme, les fichiers parasites, l'audit moulinette" "The 43 functions, memory, the Norm, garbage files, the moulinette audit")")
			acts+=("libft|$p")
		done
		for p in ${PRINTFS[@]+"${PRINTFS[@]}"}; do
			items+=("$(tt "Tester mon ft_printf    " "Test my ft_printf       ") $(court "$p")|$(tt "Chaque conversion comparée au vrai printf, puis la trace moulinette" "Every conversion compared to the real printf, then the moulinette trace")")
			acts+=("printf|$p")
		done
		for p in ${GNLS[@]+"${GNLS[@]}"}; do
			items+=("$(tt "Tester mon get_next_line" "Test my get_next_line   ") $(court "$p")|$(tt "Toutes les tailles de BUFFER_SIZE, les fuites, la Norme, les fichiers parasites" "Every BUFFER_SIZE, leaks, the Norm, garbage files")")
			acts+=("gnl|$p")
		done
		items+=("$(tt "Exam en C|Examen blanc de 3 h, série d'exercices ou entraînement : tout se règle aux flèches" "C exam|3-hour mock exam, exercise series or training: all set with the arrows")")
		acts+=("exam|")
		items+=("$(tt "Banque d'exercices|Les exercices du testeur au choix : exam, Libft, printf" "Exercise bank|The tester's exercises, your pick: exam, Libft, printf")")
		acts+=("banque|")
		items+=("$(tt "Chevalier|Ton avatar, la boutique, ton profil et tes succès" "Knight|Your avatar, the shop, your profile and achievements")")
		acts+=("chevalier|")
		items+=("$(tt "Lore|Les boss, les cinématiques et la carte de la Cité" "Lore|The bosses, the cutscenes and the map of the City")")
		acts+=("lore|")
		items+=("$(tt "Stats|Ta progression, tes badges et le holygraph 42 Next" "Stats|Your progress, your badges and the 42 Next holygraph")")
		acts+=("stats|")
		items+=("$(tt "Paramètres|Réglages, langue, interface, commande tdf" "Settings|Settings, language, interface, tdf command")")
		acts+=("params|")
		items+=("$(tt "Tester un autre dossier|Le testeur te demande le chemin d'un projet qui n'est pas ici" "Test another folder|The tester asks for the path of a project that is not here")")
		acts+=("autre|")
		items+=("$(tt "Quitter|" "Quit|")")
		acts+=("quitter|")
		n=$(( ${#LIBFTS[@]} + ${#PRINTFS[@]} + ${#GNLS[@]} ))
		if [ "$n" = 0 ]; then
			sous="$(tt "Aucun projet trouvé ici ni à côté du testeur. L'exam et la banque marchent sans projet." "No project found here or next to the tester. The exam and the bank work without a project.")"
		else
			sous="$(printf "$(tt "%s projet(s) trouvé(s) dans ce dossier ou à côté du testeur." "%s project(s) found in this folder or next to the tester.")" "$n")"
		fi
		tdf_choisir "TESTEUR" "DES FOUS" "$sous" "${items[@]}"
		[ "$CHOIX_MENU" -ge 0 ] || exit 0
		a="${acts[$CHOIX_MENU]%%|*}"
		p="${acts[$CHOIX_MENU]#*|}"
		SUITE=()
		[ "$SANS_ANIM" = 1 ] && SUITE+=(--fast)
		unset TDF_ECRAN
		case "$a" in
			libft|printf|gnl) MODULE="$a"; PROJET="$p"; return 0 ;;
			exam) MODULE=exam; PROJET=""; return 0 ;;
			banque) MODULE=exam; PROJET=""; SUITE+=(--banque); return 0 ;;
			chevalier|lore|stats)
				MODULE=libft
				PROJET="${LIBFTS[0]:-}"
				SUITE+=(--menu)
				export TDF_ECRAN="$a"
				return 0
				;;
			params)
				if menu_params; then
					MODULE=libft
					PROJET="${LIBFTS[0]:-}"
					SUITE+=(--menu)
					export TDF_ECRAN=params
					return 0
				fi
				;;
			autre)
				printf '\n  %s ' "$(tt "Chemin du projet (Tab complète) :" "Project path (Tab completes):")"
				read -e -r p || exit 0
				case "$p" in "~"*) p="$HOME${p#\~}" ;; esac
				if [ -z "$p" ] || [ ! -d "$p" ]; then
					printf '  %s\n' "$(printf "$(tt "Dossier introuvable : %s" "Folder not found: %s")" "$p")"
					printf '  %s ' "$(tt "Entrée pour revenir au menu" "Enter to go back to the menu")"
					read -r p
					continue
				fi
				PROJET="$(cd "$p" && pwd -P)"
				if est_printf "$PROJET"; then MODULE=printf
				elif est_libft "$PROJET"; then MODULE=libft
				elif est_gnl "$PROJET"; then MODULE=gnl
				else
					printf '  %s\n' "$(printf "$(tt "Je ne reconnais ni une Libft, ni un ft_printf, ni un get_next_line dans %s" "I recognise neither a Libft, nor a ft_printf, nor a get_next_line in %s")" "$PROJET")"
					printf '  %s ' "$(tt "Entrée pour revenir au menu" "Enter to go back to the menu")"
					read -r p
					continue
				fi
				return 0
				;;
			*) exit 0 ;;
		esac
	done
}

proteger_sauvegarde() {
	local jour copie n
	[ -d "$DOSSIER" ] || return 0
	chmod 700 "$DOSSIER" 2>/dev/null
	[ -f "$DOSSIER/save.txt" ] || return 0
	chmod 600 "$DOSSIER/save.txt" 2>/dev/null
	mkdir -p "$DOSSIER/copies" 2>/dev/null || return 0
	chmod 700 "$DOSSIER/copies" 2>/dev/null
	jour="$(date +%Y%m%d)"
	copie="$DOSSIER/copies/save_$jour.txt"
	if [ ! -f "$copie" ]; then
		cp "$DOSSIER/save.txt" "$copie" 2>/dev/null && chmod 600 "$copie" 2>/dev/null
	fi
	n=0
	for copie in $(ls -1 "$DOSSIER/copies" 2>/dev/null | grep '^save_[0-9]*\.txt$' | sort -r); do
		n=$((n + 1))
		[ "$n" -gt 7 ] && rm -f "$DOSSIER/copies/$copie"
	done
	return 0
}

ecrire_anim() {
	local save="$DOSSIER/save.txt"
	[ -f "$save" ] || return 1
	if grep -q '^cfg_anim=' "$save"; then
		sed "s/^cfg_anim=.*/cfg_anim=$1/" "$save" > "$save.anim" && cat "$save.anim" > "$save"
		rm -f "$save.anim"
	else
		printf 'cfg_anim=%s\n' "$1" >> "$save"
	fi
	return 0
}

appliquer_anim() {
	[ -f "$DOSSIER/anim_off" ] || return 0
	if ecrire_anim 2; then
		rm -f "$DOSSIER/anim_off"
	else
		SUITE+=(--fast)
	fi
}

regler_anim() {
	mkdir -p "$DOSSIER" 2>/dev/null
	case "$1" in
		off|non|0)
			ecrire_anim 2 || : > "$DOSSIER/anim_off"
			tl "animations : coupées (--anim on pour les remettre)" "animations: off (--anim on to bring them back)"
			;;
		on|oui|1)
			rm -f "$DOSSIER/anim_off"
			ecrire_anim 0
			tl "animations : activées" "animations: on"
			;;
		*)
			tl "--anim : on ou off" "--anim: on or off"
			exit 2
			;;
	esac
	exit 0
}

MODULE=""
PROJET=""
SUITE=()
SANS_ANIM=0
MENU_UNIQUE=0
MENU_A_FAIRE=0
DEPART="$PWD"
MENU=0

if [ "${1:-}" = debug ]; then
	shift
	debug_commande "$@"
	exit $?
fi
case "${1:-}" in
	installer|install) tdf_cli_installer; exit $? ;;
	desinstaller|uninstall) tdf_cli_desinstaller; exit $? ;;
	maj|update) tdf_cli_maj; exit $? ;;
esac

while [ $# -gt 0 ]; do
	a="$1"
	shift
	case "$a" in
		--menu) MENU=1 ;;
		banque)
			if [ -z "$MODULE" ]; then MODULE=exam; SUITE+=(--banque); else SUITE+=("$a"); fi
			;;
		libft|printf|exam|gnl)
			if [ -z "$MODULE" ]; then MODULE="$a"; else SUITE+=("$a"); fi
			;;
		--version|-V)
			echo "Le testeur des fous $TDF_VERSION ($TDF_DATE)"
			exit 0
			;;
		--help|-h)
			if [ -z "$MODULE" ]; then usage; exit 0; fi
			SUITE+=("$a")
			;;
		correction)
			CORR_ARG=1
			c=""
			if [ $# -gt 0 ] && [ "${1#-}" = "$1" ]; then
				[ -d "$1" ] || { tl "%s : dossier introuvable : %s" "%s: folder not found: %s" "$a" "$1"; exit 2; }
				c="$(cd "$1" && pwd -P)"
				shift
			elif [ -f "$DOSSIER/correction.txt" ]; then
				c="$(head -1 "$DOSSIER/correction.txt")"
			fi
			mkdir -p "$DOSSIER" && printf '%s\n' "$c" > "$DOSSIER/correction.txt"
			;;
		--projet|--dir|projet)
			[ $# -gt 0 ] || { tl "%s : il manque le chemin du projet" "%s: the project path is missing" "$a"; exit 2; }
			[ -d "$1" ] || { tl "%s : dossier introuvable : %s" "%s: folder not found: %s" "$a" "$1"; exit 2; }
			PROJET="$(cd "$1" && pwd -P)"
			shift
			;;
		--anim)
			[ $# -gt 0 ] || { tl "--anim : on ou off" "--anim: on or off"; exit 2; }
			regler_anim "$1"
			;;
		-a|--fast|--no-anim) SUITE+=(--fast); SANS_ANIM=1 ;;
		--no-lord) SUITE+=(--no-lore) ;;
		--debug)
			tl "Le mode debug se règle maintenant dans le menu : Tester mon projet › Options du test › Mode debug." "Debug mode is now set in the menu: Test my project › Test options › Debug mode."
			exit 2
			;;
		*)
			if [ -z "$PROJET" ] && [ -d "$a" ] && case "$a" in */*|.|..) true ;; *) false ;; esac; then
				PROJET="$(cd "$a" && pwd -P)"
			else
				SUITE+=("$a")
			fi
			;;
	esac
done

if [ "$MENU" = 1 ] && [ "$MODULE" != exam ]; then
	detecter
	if [ -z "$MODULE" ]; then
		MODULE="libft"
		[ ${#PRINTFS[@]} -gt 0 ] && [ ${#LIBFTS[@]} -eq 0 ] && MODULE="printf"
	fi
	if [ -z "$PROJET" ]; then
		[ "$MODULE" = libft ] && [ ${#LIBFTS[@]} -gt 0 ] && PROJET="${LIBFTS[0]}"
		[ "$MODULE" = printf ] && [ ${#PRINTFS[@]} -gt 0 ] && PROJET="${PRINTFS[0]}"
	fi
	SUITE+=(--menu)
elif [ "$MODULE" = exam ]; then
	PROJET=""
	detecter
elif [ -n "$PROJET" ]; then
	if est_printf "$PROJET"; then
		[ -n "$MODULE" ] || MODULE="printf"
	elif est_libft "$PROJET"; then
		[ -n "$MODULE" ] || MODULE="libft"
	elif est_gnl "$PROJET"; then
		[ -n "$MODULE" ] || MODULE="gnl"
	elif [ -z "$MODULE" ]; then
		tl "Je ne reconnais ni une Libft ni un ft_printf dans : %s" "I recognise neither a Libft nor a ft_printf in: %s" "$PROJET"
		tl "Précise le module : %s" "Name the module: %s" "bash $TDF_NOM/run.sh libft --projet $PROJET"
		exit 1
	fi
elif [ -z "$MODULE" ] && [ -t 0 ] && [ -t 1 ]; then
	detecter
	premiers_pas
	MENU_UNIQUE=1
	export TDF_MENU_UNIQUE=1
	[ "${CORR_ARG:-0}" = 1 ] || export TDF_CORR_ACCUEIL=1
	MODULE=libft
	PROJET=""
	MENU_A_FAIRE=1
else
	detecter
	if [ -z "$MODULE" ]; then
		if [ ${#LIBFTS[@]} -gt 0 ] && [ ${#PRINTFS[@]} -eq 0 ] && [ ${#GNLS[@]} -eq 0 ]; then
			MODULE="libft"
		elif [ ${#PRINTFS[@]} -gt 0 ] && [ ${#LIBFTS[@]} -eq 0 ] && [ ${#GNLS[@]} -eq 0 ]; then
			MODULE="printf"
		elif [ ${#GNLS[@]} -gt 0 ] && [ ${#LIBFTS[@]} -eq 0 ] && [ ${#PRINTFS[@]} -eq 0 ]; then
			MODULE="gnl"
		fi
	fi
	CHOIX=()
	NOMS=()
	if [ -z "$MODULE" ] || [ "$MODULE" = libft ]; then
		for p in ${LIBFTS[@]+"${LIBFTS[@]}"}; do CHOIX+=("$p"); NOMS+=("libft"); done
	fi
	if [ -z "$MODULE" ] || [ "$MODULE" = printf ]; then
		for p in ${PRINTFS[@]+"${PRINTFS[@]}"}; do CHOIX+=("$p"); NOMS+=("printf"); done
	fi
	if [ -z "$MODULE" ] || [ "$MODULE" = gnl ]; then
		for p in ${GNLS[@]+"${GNLS[@]}"}; do CHOIX+=("$p"); NOMS+=("gnl"); done
	fi
	if [ -z "$MODULE" ] && [ ${#CHOIX[@]} -ne 1 ]; then
		CHOIX+=("" "" "")
		NOMS+=("exam" "banque" "menu")
	fi
	if [ ${#CHOIX[@]} -eq 0 ] || { [ ${#CHOIX[@]} -eq 3 ] && [ "${NOMS[0]}" = exam ] && ! { [ -t 0 ] && [ -t 1 ]; }; }; then
		printf 'Le testeur des fous %s\n\n' "$TDF_VERSION"
		tl "Aucun projet trouvé. Le testeur se clone à côté de tes projets :" "No project found. Clone the tester next to your projects:"
		printf '\n  %s/\n  ├── %s/\n  ├── libft/\n  └── ft_printf/\n\n' "$(tt "ton_dossier" "your_folder")" "$TDF_NOM"
		tl "Ou donne le chemin : %s" "Or give the path: %s" "bash $TDF_NOM/run.sh --projet CHEMIN"
		printf '\n'
		tl "L'exam en C n'a besoin d'aucun projet : %s" "The C exam needs no project: %s" "bash $TDF_NOM/run.sh exam"
		tl "Le menu (stats, chevalier...) non plus : %s" "Neither does the menu (stats, knight...): %s" "bash $TDF_NOM/run.sh --menu"
		exit 1
	elif [ ${#CHOIX[@]} -eq 1 ] && [ "${NOMS[0]}" != exam ]; then
		PROJET="${CHOIX[0]}"
		MODULE="${NOMS[0]}"
	elif [ -t 0 ] && [ -t 1 ]; then
		printf '\n  Le testeur des fous %s\n\n' "$TDF_VERSION"
		[ "${NOMS[0]}" = exam ] && printf '  %s\n\n' "$(tt "Aucun projet trouvé à côté du testeur (libft/, ft_printf/...)." "No project found next to the tester (libft/, ft_printf/...).")"
		i=0
		while [ $i -lt ${#CHOIX[@]} ]; do
			case "${NOMS[$i]}" in
				libft) etiquette="Libft" ;;
				exam) etiquette="$(tt "Exam en C" "C exam")" ;;
				banque) etiquette="$(tt "Banque d'exercices : exam, Libft, printf" "Exercise bank: exam, Libft, printf")" ;;
				gnl) etiquette="GNL" ;;
				menu) etiquette="$(tt "Menu seul : stats, chevalier, lore (sans tester)" "Menu only: stats, knight, lore (no tests)")" ;;
				*) etiquette="ft_printf" ;;
			esac
			if [ -n "${CHOIX[$i]}" ]; then
				printf '  %d  %-10s %s\n' $((i + 1)) "$etiquette" "$(court "${CHOIX[$i]}")"
			else
				printf '  %d  %s\n' $((i + 1)) "$etiquette"
			fi
			i=$((i + 1))
		done
		printf '  q  %s\n\n  > ' "$(tt "quitter" "quit")"
		read -r rep || exit 0
		case "$rep" in
			q|Q|'') exit 0 ;;
			*[!0-9]*) tl "choix inconnu : %s" "unknown choice: %s" "$rep"; exit 2 ;;
		esac
		if [ "$rep" -lt 1 ] || [ "$rep" -gt ${#CHOIX[@]} ]; then
			tl "choix inconnu : %s" "unknown choice: %s" "$rep"
			exit 2
		fi
		PROJET="${CHOIX[$((rep - 1))]}"
		MODULE="${NOMS[$((rep - 1))]}"
		if [ "$MODULE" = banque ]; then
			MODULE="exam"
			SUITE+=(--banque)
		fi
		if [ "$MODULE" = menu ]; then
			MODULE="libft"
			PROJET="${LIBFTS[0]:-}"
			if [ -z "$PROJET" ] && [ ${#PRINTFS[@]} -gt 0 ]; then
				MODULE="printf"
				PROJET="${PRINTFS[0]}"
			fi
			SUITE+=(--menu)
		fi
	else
		tl "Plusieurs projets trouvés, précise lequel :" "Several projects found, say which one:"
		i=0
		while [ $i -lt ${#CHOIX[@]} ]; do
			if [ -n "${CHOIX[$i]}" ]; then
				printf '  bash %s/run.sh %s --projet %s\n' "$TDF_NOM" "${NOMS[$i]}" "${CHOIX[$i]}"
			elif [ "${NOMS[$i]}" = exam ] || [ "${NOMS[$i]}" = banque ]; then
				printf '  bash %s/run.sh %s\n' "$TDF_NOM" "${NOMS[$i]}"
			else
				printf '  bash %s/run.sh --menu\n' "$TDF_NOM"
			fi
			i=$((i + 1))
		done
		exit 2
	fi
fi

if [ ! -f "$TDF_RACINE/modules/$MODULE/run.sh" ]; then
	echo "testeur des fous : module $MODULE introuvable, le dossier est incomplet." >&2
	exit 1
fi

proteger_sauvegarde
appliquer_anim
export TDF_INTERFACE="${INTERFACE:-}"
if [ "${INTERFACE:-}" = texte ] && { [ "$MODULE" = libft ] || [ "$MODULE" = printf ]; }; then
	case " ${SUITE[*]:-} " in *" --menu "*|*" --no-lore "*) ;; *) SUITE+=(--no-lore) ;; esac
fi

if [ "$MENU_UNIQUE" = 0 ] && [ "$MENU" = 0 ] && [ -n "$PROJET" ] && [ -t 0 ] && [ -t 1 ]; then
	case "$MODULE" in
		libft|printf|gnl)
			case " ${SUITE[*]:-} " in
				*" --no-lore "*|*" -v "*) ;;
				*)
					MENU_UNIQUE=1
					export TDF_MENU_UNIQUE=1
					MENU_A_FAIRE=0
					INTERFACE=graphique
					export TDF_INTERFACE=graphique
					;;
			esac
			;;
	esac
fi

[ ${#LIBFTS[@]} -eq 0 ] && [ ${#PRINTFS[@]} -eq 0 ] && [ ${#GNLS[@]} -eq 0 ] && detecter
TDF_DISPO="$MODULE exam"
RETOUR_MODULE=""
RETOUR_PROJET=""
[ ${#LIBFTS[@]} -gt 0 ] && TDF_DISPO="$TDF_DISPO libft"
[ ${#PRINTFS[@]} -gt 0 ] && TDF_DISPO="$TDF_DISPO printf"
[ ${#GNLS[@]} -gt 0 ] && TDF_DISPO="$TDF_DISPO gnl"
export TDF_DISPO
export TDF_NOM
export TDF_RACINE
export TDF_LIBFT="${LIBFTS[0]:-}"
[ -n "$TDF_LIBFT" ] || unset TDF_LIBFT

debug_ouvrir ${ARGS_ORIG[@]+"${ARGS_ORIG[@]}"}
TDF_JOURNAL_MOD="lanceur"
tdf_journal "projets trouvés : libft ${#LIBFTS[@]}, ft_printf ${#PRINTFS[@]}, gnl ${#GNLS[@]} · module $MODULE · projet ${PROJET:-aucun}"
trap 'tdf_journal "interrompu (Ctrl+C)"; debug_fermer "interrompu"; [ -n "${POUSSE_TMP:-}" ] && rm -rf "$POUSSE_TMP"; exit 130' INT
trap 'debug_fermer "arrêté"; [ -n "${POUSSE_TMP:-}" ] && rm -rf "$POUSSE_TMP"; exit 143' TERM HUP

garde_reprendre() {
	[ "$MENU_UNIQUE" = 1 ] && [ -t 1 ] || return 0
	export TDF_GARDE_ECRAN=1
	printf '\033[?1049h\033[?25l'
}

if [ "$MENU_UNIQUE" = 1 ] && [ -t 1 ]; then
	garde_reprendre
	trap 'printf "\033[0m\033[?25h\033[?1049l"' EXIT
fi

while :; do
	if [ "$MENU_A_FAIRE" = 1 ]; then
		choisir_au_menu || continue
		MENU_A_FAIRE=0
	fi
	rm -f "$DOSSIER/suivant"
	mkdir -p "$DOSSIER" && : > "$DOSSIER/.module_lance"
	export TDF_PROJET="$PROJET"
	tdf_journal "lancement du module $MODULE ${SUITE[*]:-}"
	tdf_chrono_debut mod "module $MODULE"
	if [ "$MENU_UNIQUE" = 1 ] && rapport_texte; then
		mkdir -p "$DOSSIER/derniere"
		if [ "${TDF_GARDE_ECRAN:-0}" = 1 ]; then
			: > "$DOSSIER/derniere/rapport.txt"
			bash "$TDF_RACINE/modules/$MODULE/run.sh" ${SUITE[@]+"${SUITE[@]}"} > "$DOSSIER/derniere/rapport.txt" 2>&1 < /dev/null &
			attente_module $! "$DOSSIER/derniere/rapport.txt" "$(cle_duree)" "$(titre_attente "$(nom_module "$MODULE")")"
			code=$?
		else
			bash "$TDF_RACINE/modules/$MODULE/run.sh" ${SUITE[@]+"${SUITE[@]}"} 2>&1 | tee "$DOSSIER/derniere/rapport.txt"
			code=${PIPESTATUS[0]}
		fi
		printf 'rapport\n' > "$DOSSIER/derniere/module"
	else
		bash "$TDF_RACINE/modules/$MODULE/run.sh" ${SUITE[@]+"${SUITE[@]}"}
		code=$?
	fi
	tdf_chrono_fin mod "module $MODULE" "code $code"
	[ "$MENU_UNIQUE" = 1 ] && [ "$code" != 130 ] && noter_historique "$MODULE" "${PROJET_VRAI:-$PROJET}"
	if [ -n "${POUSSE_TMP:-}" ]; then
		rm -rf "$POUSSE_TMP"
		POUSSE_TMP=""
		PROJET="${PROJET_VRAI:-$PROJET}"
	fi
	if [ "$code" != 0 ] && [ "$code" != 1 ] && [ "$code" != 45 ] && [ "$code" != 130 ]; then
		tdf_journal "ERREUR : le module $MODULE s'est arrêté avec le code $code"
	fi
	if { [ "$MODULE" = exam ] || [ "$MODULE" = gnl ]; } && [ -n "$RETOUR_MODULE" ]; then
		MODULE="$RETOUR_MODULE"
		PROJET="$RETOUR_PROJET"
		RETOUR_MODULE=""
		unset TDF_RETOUR
		SUITE=()
		[ "$SANS_ANIM" = 1 ] && SUITE+=(--fast)
		export TDF_NO_UPDATE=1
		continue
	fi
	if [ "$code" != 45 ] && [ "$MENU_UNIQUE" = 1 ] && [ "$code" != 130 ] && [ "$code" != 143 ]; then
		[ "$MODULE" = gnl ] && [ -z "${TDF_SANS_HISTOIRE:-}" ] && [ "$DOSSIER/puits.txt" -nt "$DOSSIER/.module_lance" ] && lore_scene lore_gnl
		debug_suite=0
		case " ${SUITE[*]:-} " in *" --debug "*) debug_suite=1 ;; esac
		if [ "$MODULE" = gnl ] && [ "$debug_suite" = 0 ] && [ -z "${CORR_TITRE:-}" ] && [ -s "$DOSSIER/derniere/gnl.res" ] && [ "$DOSSIER/derniere/gnl.res" -nt "$DOSSIER/.module_lance" ]; then
			printf 'gnl\n' > "$DOSSIER/derniere/module"
			voir_gnl
		elif rapport_texte; then
			voir_rapport "$DOSSIER/derniere/rapport.txt" "${CORR_TITRE:-$(tt "Rapport des tests" "Test report")}"
		fi
		MENU_A_FAIRE=1
		export TDF_NO_UPDATE=1
		continue
	fi
	if [ "$code" != 45 ]; then
		debug_fermer "code $code"
		exit "$code"
	fi
	suivant="$(head -1 "$DOSSIER/suivant" 2>/dev/null)"
	rm -f "$DOSSIER/suivant"
	tdf_journal "changement de module : $MODULE vers ${suivant:-?}"
	case "$suivant" in
		libft) [ ${#LIBFTS[@]} -gt 0 ] || { debug_fermer "code 0"; exit 0; }; PROJET="${LIBFTS[0]}" ;;
		printf) [ ${#PRINTFS[@]} -gt 0 ] || { debug_fermer "code 0"; exit 0; }; PROJET="${PRINTFS[0]}" ;;
		exam|banque) RETOUR_MODULE="$MODULE"; RETOUR_PROJET="$PROJET"; PROJET="" ;;
		gnl) [ ${#GNLS[@]} -gt 0 ] || { debug_fermer "code 0"; exit 0; }
			RETOUR_MODULE="$MODULE"; RETOUR_PROJET="$PROJET"; PROJET="${GNLS[0]}"; export TDF_RETOUR=1 ;;
		*) debug_fermer "code 0"; exit 0 ;;
	esac
	MODULE="$suivant"
	SUITE=()
	if [ "$MODULE" = banque ]; then
		MODULE="exam"
		SUITE+=(--banque)
	fi
	[ "$SANS_ANIM" = 1 ] && SUITE+=(--fast)
	export TDF_NO_UPDATE=1
done
