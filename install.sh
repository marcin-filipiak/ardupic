#!/bin/bash
# install.sh - instaluje platformę PIC18F25K80 w katalogu hardware Arduino.
# Użycie: ./install.sh [katalog_użytkownika_arduino]
# Domyślnie: ~/Arduino  (arduino-cli/IDE 2.x skanują tam hardware/)
set -u
cd "$(dirname "$0")"

USER_DIR="${1:-$HOME/Arduino}"
DEST="$USER_DIR/hardware/kolgreen/pic18f25k80"

mkdir -p "$(dirname "$DEST")"
rm -rf "$DEST"
mkdir -p "$DEST"

for item in boards.txt platform.txt programmers.txt cores variants tools toolchain systems; do
    [ -e "$item" ] || continue
    cp -r "$item" "$DEST/"
done

echo "Zainstalowano platformę w: $DEST"
echo "Weryfikacja:  arduino-cli board listall  | grep -i pic18f25k80"