#!/bin/bash
# install.sh - installs the ardupic platform (PIC18F25K80/2550/4550, PIC16F877A)
# into the Arduino hardware directory.
# Usage: ./install.sh [arduino_user_dir]
# Default: ~/Arduino  (arduino-cli/IDE 2.x scan hardware/ there)
set -u
cd "$(dirname "$0")"

USER_DIR="${1:-$HOME/Arduino}"
DEST="$USER_DIR/hardware/marcinfilipiak/ardupic"

mkdir -p "$(dirname "$DEST")"
rm -rf "$DEST"
mkdir -p "$DEST"

for item in boards.txt platform.txt programmers.txt cores variants tools toolchain systems; do
    [ -e "$item" ] || continue
    cp -r "$item" "$DEST/"
done

echo "Installed the platform at: $DEST"
echo "Verify with:  arduino-cli board listall  | grep -i ardupic"
