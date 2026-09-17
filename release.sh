#!/bin/bash
# release.sh - buduje paczkę platformy ardupic (zip z osadzonym toolchainem),
# liczy SHA-256 i generuje indeks dla Arduino Board Manager:
#   package_ardupic_index.json
#
# Użycie: ./release.sh [wersja]   (domyślnie wersja z platform.txt)
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE"

VERSION="${1:-$(sed -n 's/^version=//p' platform.txt | head -1)}"
BRANCH="${BRANCH:-main}"
OWNER="${OWNER:-marcin-filipiak}"
REPO="${REPO:-ardupic}"
ARCH="ardupic"
PKG_NAME="marcinfilipiak"

ZIP_NAME="$ARCH-platform-$VERSION.zip"
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

python3 - "$ZIP" "$STAGE/root" "$ARCH" <<'PYEOF'
import sys, zipfile, os
zip, root, arch = sys.argv[1], sys.argv[2], sys.argv[3]
with zipfile.ZipFile(zip, "w", zipfile.ZIP_DEFLATED) as z:
    for path, dirs, files in os.walk(root):
        for f in files:
            p = os.path.join(path, f)
            z.write(p, os.path.join(arch, os.path.relpath(p, root)))
PYEOF

SIZE=$(stat -c%s "$ZIP")
SHA=$(sha256sum "$ZIP" | awk '{print $1}')

python3 - "$VERSION" "$BRANCH" "$OWNER" "$REPO" "$ZIP_NAME" "$SHA" "$SIZE" "$ARCH" "$PKG_NAME" <<'PYEOF' > "$HERE/package_ardupic_index.json"
import json, sys
version, branch, owner, repo, zipname, sha, size, arch, pkgname = sys.argv[1:10]
base = f"https://raw.githubusercontent.com/{owner}/{repo}/{branch}"
pkg = {
    "name": pkgname,
    "maintainer": "Marcin Filipiak",
    "websiteURL": f"https://github.com/{owner}/{repo}",
    "email": f"{owner}@users.noreply.github.com",
    "help": {"online": f"https://github.com/{owner}/{repo}"},
    "platforms": [{
        "name": "ardupic (SDCC/gputils)",
        "architecture": arch,
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
echo "Indeks: package_ardupic_index.json"
echo "Adres URL dla Arduino IDE/CLI:"
echo "  https://raw.githubusercontent.com/$OWNER/$REPO/$BRANCH/package_ardupic_index.json"