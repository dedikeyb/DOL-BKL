#!/usr/bin/env bash
# Builds a macOS .pkg installer that installs DOL BENGKULU.vst3 to the
# standard system VST3 location:
#   /Library/Audio/Plug-Ins/VST3/DOL BENGKULU.vst3
#
# Usage:
#   ./scripts/make_pkg.sh
#
# Output:
#   dist/DOL_BKL-<version>-macos.pkg
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
ARTEFACT_DIR="$BUILD_DIR/DOL_BKL_artefacts/VST3"
DIST_DIR="dist"
JOBS="$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo "==> Building VST3 (Release)..."
cmake --build "$BUILD_DIR" --config Release --target DOL_BKL_VST3 -j "$JOBS"

if [ ! -d "$ARTEFACT_DIR/$VST3_NAME" ]; then
    echo "ERROR: VST3 bundle not found at $ARTEFACT_DIR/$VST3_NAME" >&2
    exit 1
fi

echo "==> Staging payload..."
STAGE="$(mktemp -d)"
mkdir -p "$STAGE/payload/Library/Audio/Plug-Ins/VST3"
cp -R "$ARTEFACT_DIR/$VST3_NAME" "$STAGE/payload/Library/Audio/Plug-Ins/VST3/"

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
