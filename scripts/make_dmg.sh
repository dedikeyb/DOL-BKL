#!/usr/bin/env bash
# Builds the VST3 and packages it into a macOS DMG installer.
#
# Usage:
#   ./scripts/make_dmg.sh
#
# Output:
#   dist/DOL_BKL-<version>-macos.dmg
#
# Notes:
# - The DMG contains the .vst3 bundle plus short install instructions.
# - The plugin is ad-hoc signed (no Developer ID), so first launch in a
#   DAW may ask to confirm it — right-click -> Open once, then it runs.
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

echo "==> Staging DMG contents..."
STAGE="$(mktemp -d)"
mkdir -p "$STAGE/DOL BENGKULU PROTOTYPE"
cp -R "$ARTEFACT_DIR/$VST3_NAME" "$STAGE/DOL BENGKULU PROTOTYPE/"

cat > "$STAGE/INSTALL.txt" <<'EOF'
CARA INSTAL (macOS)
====================
1. Buka folder "DOL BENGKULU PROTOTYPE".
2. Salin "DOL BENGKULU.vst3" ke:
     ~/Library/Audio/Plug-Ins/VST3/
   (buat folder VST3 jika belum ada)
3. Tutup & buka ulang DAW, lalu rescan plugin jika perlu.

Catatan: build lokal tidak ditandatangani Developer ID —
jika Gatekeeper memperingatkan, klik kanan -> Open.
EOF

mkdir -p "$DIST_DIR"
DMG_PATH="$DIST_DIR/DOL_BKL-${VERSION}-macos.dmg"
rm -f "$DMG_PATH"

echo "==> Creating DMG..."
hdiutil create \
    -volname "DOL BENGKULU PROTOTYPE $VERSION" \
    -srcfolder "$STAGE" \
    -ov \
    -format UDZO \
    "$DMG_PATH"

rm -rf "$STAGE"
echo "==> Selesai: $DMG_PATH"
