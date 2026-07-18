#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MANIFEST="${ROOT_DIR}/config/dependencies.arch.json"
feature="${1:-}"

if [[ -z "${feature}" ]]; then
    echo "Вкажіть профіль функції." >&2
    exit 2
fi

mapfile -t packages < <(python - "${MANIFEST}" "${feature}" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as handle:
    data = json.load(handle)

profile = data.get("profiles", {}).get(sys.argv[2])
if profile is None:
    raise SystemExit(2)

for package in profile.get("packages", []):
    print(package)
PY
)

missing=()
for package in "${packages[@]}"; do
    pacman -Q "${package}" >/dev/null 2>&1 \
        || missing+=("${package}")
done

if (( ${#missing[@]} > 0 )); then
    printf 'Буде встановлено:\n  - %s\n' "${missing[@]}"
    sudo pacman -S --needed "${missing[@]}"
fi

ledger_dir="${HOME}/.local/share/rodavarion-tdriver"
feature_dir="${HOME}/.config/rodavarion-tdriver"
ledger_path="${ledger_dir}/package-ledger.arch.json"
feature_path="${feature_dir}/enabled-features.json"
mkdir -p "${ledger_dir}" "${feature_dir}"

python - "${ledger_path}" "${feature_path}" "${feature}" \
    "${missing[@]}" <<'PY'
import json
import os
import sys
from datetime import datetime, timezone

ledger_path, state_path, feature = sys.argv[1:4]
packages = sys.argv[4:]

def load(path):
    if not os.path.exists(path):
        return {}
    try:
        with open(path, encoding="utf-8") as handle:
            return json.load(handle)
    except Exception:
        return {}

ledger = load(ledger_path)
installed = set(ledger.get("installed_by_rodavarion", []))
installed.update(packages)
ledger.update({
    "schema_version": 1,
    "installed_by_rodavarion": sorted(installed),
    "updated_at": datetime.now(timezone.utc).isoformat()
})
with open(ledger_path, "w", encoding="utf-8") as handle:
    json.dump(ledger, handle, ensure_ascii=False, indent=2)
    handle.write("\n")

state = load(state_path)
profiles = set(state.get("enabled_profiles", []))
profiles.add("runtime-core")
profiles.add(feature)
state.update({
    "schema_version": 1,
    "enabled_profiles": sorted(profiles),
    "updated_at": datetime.now(timezone.utc).isoformat()
})
with open(state_path, "w", encoding="utf-8") as handle:
    json.dump(state, handle, ensure_ascii=False, indent=2)
    handle.write("\n")
PY

echo "Профіль '${feature}' готовий."
