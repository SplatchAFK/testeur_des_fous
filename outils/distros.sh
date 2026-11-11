#!/usr/bin/env bash
set -u

TDF_DIR="$(cd "$(dirname "$0")/.." && pwd)"
LISTE="${TDF_DISTROS:-Ubuntu FedoraLinux-45 FedoraLinux-44 Debian archlinux}"

if ! command -v wsl.exe >/dev/null 2>&1; then
	echo "distros : wsl.exe introuvable, ce script se lance depuis Git Bash sous Windows." >&2
	exit 1
fi

case "$TDF_DIR" in
	/mnt/*) ;;
	/[a-zA-Z]/*) TDF_DIR="/mnt/$(printf '%s' "${TDF_DIR:1:1}" | tr 'A-Z' 'a-z')${TDF_DIR:2}" ;;
esac
export MSYS_NO_PATHCONV=1

if [ $# -eq 0 ]; then
	set -- bash outils/banc.sh
fi

COMMANDE="$*"
ECHECS=0
TOTAL=0

for distro in $LISTE; do
	TOTAL=$((TOTAL + 1))
	printf '=== %s\n' "$distro"
	if wsl.exe -d "$distro" -u root -- bash -lc "cd '$TDF_DIR' && $COMMANDE"; then
		printf 'OK    %s\n' "$distro"
	else
		printf 'ECHEC %s\n' "$distro"
		ECHECS=$((ECHECS + 1))
	fi
done

printf '%d distributions, %d en échec\n' "$TOTAL" "$ECHECS"
[ "$ECHECS" -eq 0 ]
