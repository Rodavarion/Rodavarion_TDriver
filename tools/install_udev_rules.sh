#!/usr/bin/env bash
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
sudo install -Dm644 "${ROOT_DIR}/packaging/udev/70-rodavarion-input.rules" /etc/udev/rules.d/70-rodavarion-input.rules
sudo udevadm control --reload-rules
sudo udevadm trigger --subsystem-match=input
sudo udevadm trigger --name-match=uinput || true
echo "Rule installed. Reconnect the device and restart Rodavarion TDriver."
