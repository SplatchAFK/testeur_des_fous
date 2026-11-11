#!/usr/bin/env bash
set -u

TDF_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$TDF_DIR" || exit 1

ECHECS=0

signaler() {
	printf 'KO  %s\n' "$1"
	ECHECS=$((ECHECS + 1))
}

fichiers() {
	find . -type f -name "$1" -not -path './.git/*' -not -path './.banc/*' -not -path './.build/*'
}

while IFS= read -r f; do
	bash -n "$f" 2>/dev/null || signaler "syntaxe shell : $f"
	if sed '1d' "$f" | grep -nE '^[[:space:]]*#' >/dev/null; then
		signaler "commentaire dans : $f"
	fi
done < <(fichiers '*.sh')

while IFS= read -r f; do
	if sed -e 's/\\"//g' -e 's/"[^"]*"//g' "$f" | grep -nE '//|/\*' >/dev/null; then
		signaler "commentaire dans : $f"
	fi
done < <(fichiers '*.[ch]')

INTERDITS='jeu vid|(^|[^a-z])jouer|Co-Authored'
while IFS= read -r f; do
	case "$f" in
		./outils/verif.sh) continue ;;
	esac
	if grep -nIiE "$INTERDITS" "$f" >/dev/null 2>&1; then
		signaler "mot interdit dans : $f"
	fi
done < <(find . -type f -not -path './.git/*' -not -path './.banc/*' -not -path './.build/*')

[ -f LICENSE ] || signaler "LICENSE absent"
[ -f moteur/version.h ] || signaler "moteur/version.h absent"

if [ "$ECHECS" -eq 0 ]; then
	echo "OK  vérifications passées"
	exit 0
fi
printf '%d problème(s)\n' "$ECHECS"
exit 1
