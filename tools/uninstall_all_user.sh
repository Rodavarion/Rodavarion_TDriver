#!/usr/bin/env bash
set -euo pipefail

STATE_DIR="${HOME}/.local/share/rodavarion-tdriver/install"
MANIFEST_FILE="${STATE_DIR}/managed-files.txt"
PURGE_DATA=1
KEEP_DATA=0
REMOVE_SYSTEM=0
ASSUME_YES=0

usage() {
    cat <<USAGE
Безпечна деінсталяція Rodavarion TDriver
Використання: $0 [--purge-data] [--remove-system-rules] [--yes]
  без параметрів          повністю видалити програму та її дані
  --keep-data             зберегти налаштування/профілі/журнали
  --purge-data            явно виконати повне очищення
  --remove-system-rules   запропонувати видалити системні udev/modules-load файли
  --yes                   підтвердити без додаткового запиту
USAGE
}

while (($#)); do
    case "$1" in
        --purge-data) PURGE_DATA=1; KEEP_DATA=0; shift ;;
        --keep-data) PURGE_DATA=0; KEEP_DATA=1; shift ;;
        --remove-system-rules) REMOVE_SYSTEM=1; shift ;;
        --yes|-y) ASSUME_YES=1; shift ;;
        --help|-h) usage; exit 0 ;;
        *) echo "Невідомий параметр: $1" >&2; usage >&2; exit 2 ;;
    esac
done

echo "Буде видалено лише компоненти Rodavarion TDriver з профілю ${USER}."
if (( PURGE_DATA )); then
    echo "УВАГА: також буде видалено налаштування, профілі, кеш і журнали Rodavarion TDriver."
else
    echo "Налаштування, профілі та журнали буде збережено для можливого повторного встановлення."
fi
if (( ! ASSUME_YES )); then
    read -r -p "Продовжити деінсталяцію? [y/N] " answer
    case "${answer:-N}" in
        y|Y|yes|YES|т|Т|так|ТАК) ;;
        *) echo "Деінсталяцію скасовано."; exit 0 ;;
    esac
fi

systemctl --user disable --now rodavarion-tdriverd.service 2>/dev/null || true
if command -v kwriteconfig6 >/dev/null 2>&1; then
    kwriteconfig6 --file kwinrc --group Plugins --key rodavarion-contextEnabled false
fi

stop_owned_executable() {
    local executable="$1" pid exe
    for proc in /proc/[0-9]*; do
        pid="${proc##*/}"
        [[ "$(awk '/^Uid:/{print $2}' "${proc}/status" 2>/dev/null)" == "${UID}" ]] || continue
        exe="$(readlink -f "${proc}/exe" 2>/dev/null || true)"
        [[ "$exe" == "$executable" ]] || continue
        kill -TERM "$pid" 2>/dev/null || true
    done
    sleep 0.3
    for proc in /proc/[0-9]*; do
        pid="${proc##*/}"
        exe="$(readlink -f "${proc}/exe" 2>/dev/null || true)"
        [[ "$exe" == "$executable" ]] && kill -KILL "$pid" 2>/dev/null || true
    done
}
stop_owned_executable "${HOME}/.local/bin/rodavarion-tdriver"
stop_owned_executable "${HOME}/.local/bin/rodavarion-tdriverd"

safe_remove_file() {
    local path="$1"
    case "$path" in
        "${HOME}/.local/bin/rodavarion-tdriver"|\
        "${HOME}/.local/bin/rodavarion-tdriverd"|\
        "${HOME}/.local/bin/rodavarion-tdriverctl"|\
        "${HOME}/.local/bin/rodavarion-tdriver-uninstall"|\
        "${HOME}/.config/systemd/user/rodavarion-tdriverd.service"|\
        "${HOME}/.config/autostart/rodavarion-tdriver.desktop"|\
        "${HOME}/.local/share/applications/rodavarion-tdriver.desktop"|\
        "${HOME}/.local/share/applications/rodavarion-tdriver-uninstall.desktop"|\
        "${HOME}/.local/share/icons/hicolor/scalable/apps/rodavarion-tdriver.svg"|\
        "${HOME}/.local/share/icons/hicolor/scalable/apps/rodavarion-tdriver-uninstall.svg"|\
        "${HOME}/.local/share/rodavarion-tdriver/resources/icons/rodavarion-tdriver.png"|\
        "${HOME}/.local/share/rodavarion-tdriver/resources/icons/rodavarion-tdriver-uninstall.png"|\
        "${HOME}/.local/share/desktop-directories/rodavarion-tdriver.directory"|\
        "${HOME}/.config/menus/applications-merged/rodavarion-tdriver.menu"|\
        "${HOME}/.local/share/kwin/scripts/rodavarion-context/metadata.json"|\
        "${HOME}/.local/share/kwin/scripts/rodavarion-context/contents/code/main.js"|\
        "${HOME}/Desktop/Rodavarion TDriver.desktop"|\
        "${HOME}/Desktop/Rodavarion-TDriver.desktop"|\
        "${HOME}/Робочий стіл/Rodavarion TDriver.desktop"|\
        "${HOME}/Робочий стіл/Rodavarion-TDriver.desktop") rm -f -- "$path" ;;
        "${HOME}"/*/"Rodavarion TDriver.desktop"|\
        "${HOME}"/*/"Rodavarion-TDriver.desktop") rm -f -- "$path" ;;
        "${HOME}/.local/share/icons/hicolor/"*"x"*"/apps/rodavarion-tdriver.png"|\
        "${HOME}/.local/share/icons/hicolor/"*"x"*"/apps/rodavarion-tdriver-uninstall.png") rm -f -- "$path" ;;
        *) echo "Пропущено незареєстрований або небезпечний шлях: $path" >&2 ;;
    esac
}

if [[ -r "$MANIFEST_FILE" ]]; then
    while IFS= read -r path; do
        [[ -n "$path" ]] && safe_remove_file "$path"
    done < "$MANIFEST_FILE"
else
    echo "Реєстр старої версії відсутній; видаляються лише фіксовані шляхи Rodavarion TDriver."
    for path in \
        "${HOME}/.local/bin/rodavarion-tdriver" \
        "${HOME}/.local/bin/rodavarion-tdriverd" \
        "${HOME}/.local/bin/rodavarion-tdriverctl" \
        "${HOME}/.local/bin/rodavarion-tdriver-uninstall" \
        "${HOME}/.config/systemd/user/rodavarion-tdriverd.service" \
        "${HOME}/.config/autostart/rodavarion-tdriver.desktop" \
        "${HOME}/.local/share/applications/rodavarion-tdriver.desktop" \
        "${HOME}/.local/share/applications/rodavarion-tdriver-uninstall.desktop" \
        "${HOME}/.local/share/icons/hicolor/scalable/apps/rodavarion-tdriver.svg" \
        "${HOME}/.local/share/icons/hicolor/scalable/apps/rodavarion-tdriver-uninstall.svg" \
        "${HOME}/.local/share/rodavarion-tdriver/resources/icons/rodavarion-tdriver.png" \
        "${HOME}/.local/share/rodavarion-tdriver/resources/icons/rodavarion-tdriver-uninstall.png" \
        "${HOME}/.local/share/desktop-directories/rodavarion-tdriver.directory" \
        "${HOME}/.config/menus/applications-merged/rodavarion-tdriver.menu" \
        "${HOME}/Desktop/Rodavarion TDriver.desktop" \
        "${HOME}/Desktop/Rodavarion-TDriver.desktop" \
        "${HOME}/Робочий стіл/Rodavarion TDriver.desktop" \
        "${HOME}/Робочий стіл/Rodavarion-TDriver.desktop"; do
        safe_remove_file "$path"
    done
    if command -v xdg-user-dir >/dev/null 2>&1; then
        desktop_dir="$(xdg-user-dir DESKTOP 2>/dev/null || true)"
        [[ -n "$desktop_dir" && "$desktop_dir" != "$HOME" ]] && safe_remove_file "${desktop_dir}/Rodavarion TDriver.desktop"
        safe_remove_file "${desktop_dir}/Rodavarion-TDriver.desktop"
    fi
fi

systemctl --user daemon-reload
for icon_size in 16 24 32 48 64 128 256; do
    safe_remove_file "${HOME}/.local/share/icons/hicolor/${icon_size}x${icon_size}/apps/rodavarion-tdriver.png"
    safe_remove_file "${HOME}/.local/share/icons/hicolor/${icon_size}x${icon_size}/apps/rodavarion-tdriver-uninstall.png"
done

command -v update-desktop-database >/dev/null 2>&1 && update-desktop-database "${HOME}/.local/share/applications" || true
command -v gtk-update-icon-cache >/dev/null 2>&1 && gtk-update-icon-cache -f -t "${HOME}/.local/share/icons/hicolor" >/dev/null 2>&1 || true

if (( PURGE_DATA )); then
    # Лише власні каталоги застосунку; батьківські каталоги не видаляються.
    rm -rf -- \
        "${HOME}/.config/Rodavarion" \
        "${HOME}/.config/rodavarion-tdriver" \
        "${HOME}/.local/share/Rodavarion" \
        "${HOME}/.local/share/rodavarion-tdriver" \
        "${HOME}/.cache/Rodavarion" \
        "${HOME}/.cache/rodavarion-tdriver" \
        "${HOME}/.config/Rodavarion Technologies" \
        "${HOME}/.local/state/Rodavarion Technologies" \
        "${HOME}/.local/state/rodavarion-tdriver"
else
    rm -rf -- "$STATE_DIR"
fi

if (( REMOVE_SYSTEM )); then
    helper="/usr/local/lib/rodavarion-tdriver/uninstall-system.sh"
    if [[ -x "$helper" ]]; then
        sudo "$helper"
    else
        echo "Системний деінсталятор не знайдено. Правила не змінено." >&2
    fi
fi

echo "Rodavarion TDriver видалено з профілю користувача."
(( PURGE_DATA )) && echo "Особисті дані Rodavarion TDriver також видалено." || echo "Особисті дані збережено."
echo "Пакунки Arch Linux не видалялися: вони можуть використовуватися іншими програмами."
