#!/bin/bash
# release.sh - buduje paczkę platformy (zip z osadzonym toolchainem),
# liczy SHA-256 i generuje indeks dla Arduino Board Manager:
#   package_pic18f25k80_index.json
#
# Użycie: ./release.sh [wersja]   (domyślnie wersja z platform.txt)
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE"

VERSION="${1:-$(sed -n 's/^version=//p' platform.txt | head -1)}"
BRANCH="${BRANCH:-main}"
OWNER="${OWNER:-marcin-filipiak}"
REPO="${REPO:-arduino_pic18f25k80}"

ZIP_NAME="pic18f25k80-platform-$VERSION.zip"
ZIP="$HERE/assets/$ZIP_NAME"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

echo "==> Budowanie paczki $ZIP_NAME"

mkdir -p "$HERE/assets" "$STAGE/root"
rm -f "$ZIP"

for item in boards.txt platform.txt programmers.txt cores variants tools toolchain examples; do
    [ -e "$HERE/$item" ] && cp -r "$HERE/$item" "$STAGE/root/"
done
rm -f "$STAGE/root/tools/arduino-cli"

python3 - "$ZIP" "$STAGE/root" <<'PYEOF'
import sys, zipfile, os
zip, root = sys.argv[1], sys.argv[2]
with zipfile.ZipFile(zip, "w", zipfile.ZIP_DEFLATED) as z:
    for path, dirs, files in os.walk(root):
        for f in files:
            p = os.path.join(path, f)
            z.write(p, os.path.join("pic18f25k80", os.path.relpath(p, root)))
PYEOF

SIZE=$(stat -c%s "$ZIP")
SHA=$(sha256sum "$ZIP" | awk '{print $1}')

python3 - "$VERSION" "$BRANCH" "$OWNER" "$REPO" "$ZIP_NAME" "$SHA" "$SIZE" <<'PYEOF' > "$HERE/package_pic18f25k80_index.json"
import json, sys
version, branch, owner, repo, zipname, sha, size = sys.argv[1:8]
base = f"https://raw.githubusercontent.com/{owner}/{repo}/{branch}"
pkg = {
    "name": "kolgreen",
    "maintainer": "Marcin Filipiak",
    "websiteURL": f"https://github.com/{owner}/{repo}",
    "email": f"{owner}@users.noreply.github.com",
    "help": {"online": f"https://github.com/{owner}/{repo}"},
    "platforms": [{
        "name": "PIC18F25K80 (SDCC)",
        "architecture": "pic18f25k80",
        "version": version,
        "category": "Pic18",
        "url": f"{base}/assets/{zipname}",
        "archiveFileName": zipname,
        "checksum": f"SHA-256:{sha}",
        "size": int(size),
        "toolsDependencies": [],
    }],
    "tools": [],
}
json.dump({"packages": [pkg]}, sys.stdout, indent=2, ensure_ascii=False)
sys.stdout.write("\n")
PYEOF

echo "OK: assets/$ZIP_NAME  ($SIZE bajtów, SHA-256: $SHA)"
echo "Indeks: package_pic18f25k80_index.json"
echo "Adres URL dla Arduino IDE/CLI:"
echo "  https://raw.githubusercontent.com/$OWNER/$REPO/$BRANCH/package_pic18f25k80_index.json"