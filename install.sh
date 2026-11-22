#!/usr/bin/env bash
set -euo pipefail

SRC="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ $EUID -ne 0 ]]; then
    echo "Run with sudo: sudo ./install.sh" >&2
    exit 1
fi

install -m 755 "$SRC/evision-rgb"        /usr/local/bin/evision-rgb
install -m 755 "$SRC/evision-sdk"        /usr/local/bin/evision-sdk
install -m 755 "$SRC/udev/kbd-fix-unbind-if1" /usr/local/bin/kbd-fix-unbind-if1
install -m 644 "$SRC/udev/99-evision-kbd-fix.rules" /etc/udev/rules.d/
install -m 644 "$SRC/udev/60-evision-rgb.rules"     /etc/udev/rules.d/
install -m 644 "$SRC/evision-sdk.service"           /usr/lib/systemd/user/

udevadm control --reload-rules
udevadm trigger /sys/bus/usb/devices/1-1.6 2>/dev/null || true

echo "Installed. Verify with: evision-rgb status"
