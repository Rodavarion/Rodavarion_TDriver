#!/usr/bin/env bash
set -euo pipefail

pkill -x rodavarion-tdriver 2>/dev/null || true
rm -f -- \
    "${HOME}/.local/bin/rodavarion-tdriver" \
    "${HOME}/.local/share/applications/rodavarion-tdriver.desktop" \
    "${HOME}/.local/share/icons/hicolor/scalable/apps/rodavarion-tdriver.svg" \
    "${HOME}/Desktop/Rodavarion TDriver.desktop" \
    "${HOME}/Робочий стіл/Rodavarion TDriver.desktop"
if command -v xdg-user-dir >/dev/null 2>&1; then
    desktop_dir="$(xdg-user-dir DESKTOP 2>/dev/null || true)"
    [[ -n "$desktop_dir" && "$desktop_dir" != "$HOME" ]] && rm -f -- "${desktop_dir}/Rodavarion TDriver.desktop"
fi
command -v update-desktop-database >/dev/null 2>&1 && update-desktop-database "${HOME}/.local/share/applications" || true

echo "Графічний інтерфейс і його ярлики видалено."
echo "Фонова служба та призначення кнопок залишилися активними."
systemctl --user --no-pager --full status rodavarion-tdriverd.service || true
