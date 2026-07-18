#!/usr/bin/env bash
set -euo pipefail

if (( EUID != 0 )); then
    echo "Цей скрипт потребує sudo." >&2
    exit 1
fi

STATE_DIR="/var/lib/rodavarion-tdriver"
HASH_FILE="${STATE_DIR}/system-files.sha256"
FILES=(
    "/etc/udev/rules.d/70-rodavarion-input.rules"
    "/etc/modules-load.d/rodavarion-uinput.conf"
)

if [[ ! -r "$HASH_FILE" ]]; then
    echo "Реєстр системних файлів Rodavarion TDriver відсутній. Нічого не видалено." >&2
    exit 2
fi

removed=0
while read -r expected path; do
    [[ -n "${expected:-}" && -n "${path:-}" ]] || continue
    allowed=0
    for known in "${FILES[@]}"; do [[ "$path" == "$known" ]] && allowed=1; done
    if (( ! allowed )); then
        echo "Пропущено шлях поза дозволеним списком: $path" >&2
        continue
    fi
    if [[ ! -e "$path" ]]; then
        continue
    fi
    actual="$(sha256sum "$path" | awk '{print $1}')"
    if [[ "$actual" == "$expected" ]]; then
        rm -f -- "$path"
        echo "Видалено: $path"
        removed=1
    else
        echo "Не видалено змінений файл: $path" >&2
        echo "Його вміст відрізняється від встановленого Rodavarion TDriver." >&2
    fi
done < "$HASH_FILE"

rm -f -- "$HASH_FILE"
rmdir "$STATE_DIR" 2>/dev/null || true
udevadm control --reload-rules
udevadm trigger --subsystem-match=input || true

if (( removed )); then
    echo "Системні правила Rodavarion TDriver безпечно видалено."
else
    echo "Системних файлів із підтвердженою належністю не видалено."
fi
