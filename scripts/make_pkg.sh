#!/usr/bin/env bash
# Builds a macOS .pkg installer that installs DOL BENGKULU.vst3 to the
# standard system VST3 location:
#   /Library/Audio/Plug-Ins/VST3/DOL BENGKULU.vst3
#
# Usage:
#   ./scripts/make_pkg.sh
#
# Output:
#   dist/macOS/DOL_BKL-<version>-macos.pkg
#
# Notes:
# - Installing asks for the admin password once (normal for system installers).
# - The pkg is unsigned; on other machines Gatekeeper may ask to confirm
#   (right-click -> Open). For distribution, sign with a Developer ID.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

VST3_NAME="DOL BENGKULU.vst3"
VERSION="0.1.0"
BUILD_DIR="Builds/DOL_BKL"
# Lokasi artefak: dengan CMAKE_BUILD_TYPE=Release masuk subfolder Release/.
ARTEFACT_DIR="$BUILD_DIR/DOL_BKL_artefacts/Release/VST3"
if [ ! -d "$ARTEFACT_DIR/$VST3_NAME" ]; then
    ARTEFACT_DIR="$BUILD_DIR/DOL_BKL_artefacts/VST3"
fi
DIST_DIR="dist/macOS"
JOBS="$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo "==> Building VST3 (Release)..."
cmake --build "$BUILD_DIR" --config Release --target DOL_BKL_VST3 -j "$JOBS"

if [ ! -d "$ARTEFACT_DIR/$VST3_NAME" ]; then
    echo "ERROR: VST3 bundle not found at $ARTEFACT_DIR/$VST3_NAME" >&2
    exit 1
fi

echo "==> Staging payload..."
STAGE="$(mktemp -d)"
# Hapus sidecar AppleDouble (._) yang dihasilkan drive exFAT: file "._x.wav"
# ikut terhitung sebagai wav dan bisa mengacaukan pemuatan sampel di mesin lain.
find "$ARTEFACT_DIR/$VST3_NAME" -name "._*" -delete
mkdir -p "$STAGE/payload/Library/Audio/Plug-Ins/VST3"
cp -R "$ARTEFACT_DIR/$VST3_NAME" "$STAGE/payload/Library/Audio/Plug-Ins/VST3/"
# Pastikan semua file/folder bisa dibaca siapa pun (DAW berjalan sebagai user;
# drive exFAT bisa menghasilkan izin 700 yang membuat plugin tak termuat).
chmod -R a+rX,u+rw "$STAGE/payload"

echo "==> Building pkg..."
pkgbuild \
    --root "$STAGE/payload" \
    --identifier com.digiethnica.dolbengkulu \
    --version "$VERSION" \
    --install-location / \
    "$STAGE/DOL_BKL-component.pkg"

mkdir -p "$DIST_DIR"
PKG_PATH="$DIST_DIR/DOL_BKL-${VERSION}-macos.pkg"
rm -f "$PKG_PATH"

productbuild \
    --package "$STAGE/DOL_BKL-component.pkg" \
    "$PKG_PATH"

rm -rf "$STAGE"
echo "==> Selesai: $PKG_PATH"
