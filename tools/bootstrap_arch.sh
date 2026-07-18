#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MANIFEST="${ROOT_DIR}/config/dependencies.arch.json"

edition="${1:-}"

if [[ "${edition}" == "--edition" ]]; then
    edition="${2:-}"
fi

if [[ -z "${edition}" ]]; then
    echo "Оберіть редакцію:"
    echo "  1) minimal — headless/server, мінімум залежностей"
    echo "  2) full    — GUI та основні класи периферії"
    read -r -p "Вибір [1/2]: " answer
    case "${answer}" in
        1) edition="minimal" ;;
        2) edition="full" ;;
        *) echo "Невірний вибір." >&2; exit 2 ;;
    esac
fi

if [[ "${edition}" != "minimal" && "${edition}" != "full" ]]; then
    echo "Допустимі редакції: minimal або full." >&2
    exit 2
fi

if [[ ! -f /etc/arch-release ]] || ! command -v pacman >/dev/null 2>&1; then
    echo "Bootstrap наразі підтримує Arch Linux та сумісні системи з pacman." >&2
    exit 3
fi

mapfile -t packages < <(python - "${MANIFEST}" "${edition}" <<'PY'
import json
import sys

manifest, edition = sys.argv[1], sys.argv[2]
with open(manifest, encoding="utf-8") as handle:
    data = json.load(handle)

seen = set()
for profile_name in data["editions"][edition]["profiles"]:
    profile = data["profiles"][profile_name]
    for package in profile.get("packages", []):
        if package not in seen:
            seen.add(package)
            print(package)
PY
)

missing=()
for package in "${packages[@]}"; do
    pacman -Q "${package}" >/dev/null 2>&1 || missing+=("${package}")
done
installed_by_this_run=("${missing[@]}")

echo "=== Rodavarion TDriver: підготовка редакції ${edition} ==="
echo "Пакетів у профілі: ${#packages[@]}"
echo "Відсутніх: ${#missing[@]}"

if (( ${#missing[@]} > 0 )); then
    printf '  - %s\n' "${missing[@]}"
    echo
    read -r -p "Встановити відсутні офіційні пакети? [Y/n] " answer
    answer="${answer:-Y}"

    if [[ "${answer}" =~ ^[YyТт]$ ]]; then
        sudo pacman -S --needed "${missing[@]}"
    else
        echo "Встановлення скасовано."
        exit 4
    fi
fi

ledger_dir="${HOME}/.local/share/rodavarion-tdriver"
ledger_path="${ledger_dir}/package-ledger.arch.json"
feature_dir="${HOME}/.config/rodavarion-tdriver"
feature_path="${feature_dir}/enabled-features.json"

mkdir -p "${ledger_dir}" "${feature_dir}"

python - "${ledger_path}" "${edition}" \
    "${installed_by_this_run[@]}" <<'PY'
import json
import os
import sys
from datetime import datetime, timezone

path = sys.argv[1]
edition = sys.argv[2]
new_packages = sys.argv[3:]

data = {}
if os.path.exists(path):
    try:
        with open(path, encoding="utf-8") as handle:
            data = json.load(handle)
    except Exception:
        data = {}

installed = set(data.get("installed_by_rodavarion", []))
installed.update(new_packages)

data.update({
    "schema_version": 1,
    "edition": edition,
    "installed_by_rodavarion": sorted(installed),
    "updated_at": datetime.now(timezone.utc).isoformat()
})

with open(path, "w", encoding="utf-8") as handle:
    json.dump(data, handle, ensure_ascii=False, indent=2)
    handle.write("\n")
PY

python - "${feature_path}" <<'PY'
import json
import os
import sys
from datetime import datetime, timezone

path = sys.argv[1]
data = {}
if os.path.exists(path):
    try:
        with open(path, encoding="utf-8") as handle:
            data = json.load(handle)
    except Exception:
        data = {}

profiles = set(data.get("enabled_profiles", []))
profiles.add("runtime-core")

data.update({
    "schema_version": 1,
    "enabled_profiles": sorted(profiles),
    "updated_at": datetime.now(timezone.utc).isoformat()
})

with open(path, "w", encoding="utf-8") as handle:
    json.dump(data, handle, ensure_ascii=False, indent=2)
    handle.write("\n")
PY

sudo modprobe uinput
printf 'uinput\n' |
    sudo tee /etc/modules-load.d/rodavarion-uinput.conf >/dev/null

sudo install -Dm644 \
    "${ROOT_DIR}/packaging/udev/70-rodavarion-input.rules" \
    /etc/udev/rules.d/70-rodavarion-input.rules

# Реєструємо лише власні системні файли та їхні контрольні суми.
sudo install -d -m755 /var/lib/rodavarion-tdriver /usr/local/lib/rodavarion-tdriver
sudo install -m755 "${ROOT_DIR}/tools/uninstall_system.sh" \
    /usr/local/lib/rodavarion-tdriver/uninstall-system.sh
{
    sha256sum /etc/udev/rules.d/70-rodavarion-input.rules
    sha256sum /etc/modules-load.d/rodavarion-uinput.conf
} | sudo tee /var/lib/rodavarion-tdriver/system-files.sha256 >/dev/null

sudo udevadm control --reload-rules
sudo udevadm trigger --name-match=uinput || true

if systemctl --user list-unit-files ydotool.service >/dev/null 2>&1; then
    systemctl --user daemon-reload
    systemctl --user enable --now ydotool.service || true
fi

echo
echo "Підготовку редакції ${edition} завершено."
echo "Підготовка системи завершена."
