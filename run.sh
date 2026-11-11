#!/usr/bin/env bash
set -u

TDF_DIR="$(cd "$(dirname "$0")" && pwd)"
VERSION_H="$TDF_DIR/moteur/version.h"

lire_version() {
	sed -n "s/^# *define $1 \"\(.*\)\"/\1/p" "$VERSION_H"
}

if [ ! -f "$VERSION_H" ]; then
	echo "testeur des fous : moteur/version.h introuvable, le dossier est incomplet." >&2
	exit 1
fi

TDF_VERSION="$(lire_version TDF_VERSION)"
TDF_DATE="$(lire_version TDF_DATE)"

usage() {
	cat <<EOF
Le testeur des fous $TDF_VERSION

  bash run.sh              lancer le testeur
  bash run.sh --no-lore    les tests seuls, sans histoire
  bash run.sh -a           sans animations pour ce lancement
  bash run.sh --anim on    animations activées pour de bon (off pour les couper)
  bash run.sh --debug      tous les rapports détaillés
  bash run.sh --version    le numéro de version
  bash run.sh --help       cette aide
EOF
}

case "${1:-}" in
	--version|-v)
		echo "Le testeur des fous $TDF_VERSION ($TDF_DATE)"
		exit 0
		;;
	--help|-h)
		usage
		exit 0
		;;
esac

echo "Le testeur des fous $TDF_VERSION : le socle est en place, les modules arrivent."
exit 0
