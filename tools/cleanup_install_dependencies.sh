#!/usr/bin/env bash
set -euo pipefail
ledger="${HOME}/.local/share/rodavarion-tdriver/package-ledger.arch.json"
[[ -f "$ledger" ]] || exit 0
mapfile -t candidates < <(python - "$ledger" <<'PY2'
import json,sys
try:d=json.load(open(sys.argv[1]))
except Exception:raise SystemExit
runtime={"qt6-base","hidapi","libevdev","polkit","ydotool","xdotool","wtype","usbutils","libusb","pciutils","bluez","bluez-utils"}
for p in d.get("installed_by_rodavarion",[]):
    if p not in runtime: print(p)
PY2
)
((${#candidates[@]})) || exit 0
mapfile -t orphans < <(pacman -Qdtq 2>/dev/null || true)
remove=()
for p in "${candidates[@]}"; do
  printf '%s
' "${orphans[@]}" | grep -Fqx "$p" && remove+=("$p") || true
done
if ((${#remove[@]})); then
  echo "Очищення тимчасових залежностей збірки: ${remove[*]}"
  sudo pacman -Rns --noconfirm "${remove[@]}" || true
fi
