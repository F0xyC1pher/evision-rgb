#!/usr/bin/env bash
# Build OpenRGB with support for EVision 320F:5055 keyboards.
set -euo pipefail

PATCH_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC="${1:-$HOME/openrgb-custom}"

if [ ! -d "$SRC/.git" ]; then
    git clone --depth 1 https://gitlab.com/CalcProgrammer1/OpenRGB.git "$SRC"
fi

TARGET="$SRC/Controllers/EVisionKeyboardController/EVisionV2KeyboardController"
cp "$PATCH_DIR/Evision5055Layout.h"              "$TARGET/"
cp "$PATCH_DIR/EVisionV2KeyboardController.h"    "$TARGET/"
cp "$PATCH_DIR/EVisionV2KeyboardController.cpp"  "$TARGET/"
cp "$PATCH_DIR/EVisionV2KeyboardControllerDetect.cpp" "$TARGET/"
cp "$PATCH_DIR/RGBController_EVisionV2Keyboard.cpp"   "$TARGET/"
cp "$PATCH_DIR/hidapi_wrapper.h"                      "$SRC/hidapi_wrapper/"
cp "$PATCH_DIR/DetectionManager.cpp"                  "$SRC/"

cd "$SRC"
qmake6 OpenRGB.pro -o Makefile.custom
make -f Makefile.custom -j"$(nproc)"

echo
echo "Built: $SRC/openrgb"
echo "Install: sudo install -m 755 $SRC/openrgb /usr/local/bin/openrgb-evision"
