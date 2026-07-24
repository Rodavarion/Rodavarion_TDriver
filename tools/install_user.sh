#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
VERSION="$(cat "${ROOT_DIR}/VERSION" 2>/dev/null || printf 'unknown')"
STATE_DIR="${HOME}/.local/share/rodavarion-tdriver/install"
STATE_FILE="${STATE_DIR}/state.env"
MANIFEST_FILE="${STATE_DIR}/managed-files.txt"
EDITION="full"
ASSUME_YES=0

usage() {
    cat <<USAGE
Використання: $0 [--edition full|minimal] [--yes]
  --edition  редакція для встановлення
  --yes      підтвердити заміну попередньої версії без запиту

Цей скрипт лише встановлює вже перевірену збірку з каталогу build/.
Для повного автоматичного циклу використовуйте: ./install.sh
USAGE
}

while (($#)); do
    case "$1" in
        --edition) EDITION="${2:-}"; shift 2 ;;
        --yes|-y) ASSUME_YES=1; shift ;;
        --help|-h) usage; exit 0 ;;
        *) echo "Невідомий параметр: $1" >&2; usage >&2; exit 2 ;;
    esac
done

if [[ "$EDITION" != "full" && "$EDITION" != "minimal" ]]; then
    echo "Невідома редакція: $EDITION" >&2
    exit 2
fi

build_state="${BUILD_DIR}/.rodavarion-build.env"
checksums="${BUILD_DIR}/.rodavarion-artifacts.sha256"
if [[ ! -r "$build_state" || ! -r "$checksums" ]]; then
    echo "Немає підтвердженої збірки. Спочатку виконайте:" >&2
    echo "  ./tools/preflight_arch.sh --edition ${EDITION}" >&2
    echo "або скористайтеся єдиною командою:" >&2
    echo "  ./install.sh --edition ${EDITION}" >&2
    exit 6
fi

read_build_value() {
    local key="$1"
    sed -n "s/^${key}=//p" "$build_state" | tail -n1
}

built_version="$(read_build_value BUILD_VERSION)"
built_edition="$(read_build_value BUILD_EDITION)"
tests_passed="$(read_build_value BUILD_TESTS_PASSED)"

if [[ "$built_version" != "$VERSION" || "$built_edition" != "$EDITION" || "$tests_passed" != "1" ]]; then
    echo "Збірка не відповідає вибраному пакету або редакції." >&2
    echo "Очікується: версія ${VERSION}, редакція ${EDITION}, успішні тести." >&2
    echo "Знайдено: версія ${built_version:-?}, редакція ${built_edition:-?}, тести ${tests_passed:-?}." >&2
    echo "Повторіть preflight або запустіть ./install.sh." >&2
    exit 7
fi

if ! (cd "$BUILD_DIR" && sha256sum -c .rodavarion-artifacts.sha256 >/dev/null); then
    echo "Контрольні суми збірки не збігаються. Встановлення зупинено." >&2
    echo "Повторіть preflight або запустіть ./install.sh." >&2
    exit 8
fi

old_version=""
old_edition=""
if [[ -r "$STATE_FILE" ]]; then
    # shellcheck disable=SC1090
    source "$STATE_FILE"
    old_version="${INSTALLED_VERSION:-unknown}"
    old_edition="${INSTALLED_EDITION:-unknown}"
elif [[ -e "${HOME}/.local/bin/rodavarion-tdriverd" || -e "${HOME}/.local/bin/rodavarion-tdriver" ]]; then
    old_version="невідома (встановлення без реєстру)"
    old_edition="невідома"
fi

if [[ -n "$old_version" ]]; then
    echo "Виявлено встановлену версію Rodavarion TDriver: ${old_version}, редакція ${old_edition}."
    echo "Нова версія: ${VERSION}, редакція ${EDITION}."
    echo "Налаштування, профілі та журнали буде збережено."
    if (( ! ASSUME_YES )); then
        read -r -p "Замінити встановлену версію на нову? [Y/n] " answer
        case "${answer:-Y}" in
            y|Y|yes|YES|т|Т|так|ТАК) ;;
            *) echo "Оновлення скасовано."; exit 0 ;;
        esac
    fi
fi

systemctl --user stop rodavarion-tdriverd.service 2>/dev/null || true

stop_installed_process() {
    local executable="$1"
    local signal="${2:-TERM}"
    local pid exe
    for proc in /proc/[0-9]*; do
        pid="${proc##*/}"
        [[ -r "${proc}/status" ]] || continue
        [[ "$(awk '/^Uid:/{print $2}' "${proc}/status" 2>/dev/null)" == "${UID}" ]] || continue
        exe="$(readlink -f "${proc}/exe" 2>/dev/null || true)"
        [[ "$exe" == "$executable" ]] || continue
        kill "-${signal}" "$pid" 2>/dev/null || true
    done
}

old_gui="${HOME}/.local/bin/rodavarion-tdriver"
stop_installed_process "$old_gui" TERM
for _ in {1..30}; do
    running=0
    for proc in /proc/[0-9]*; do
        [[ "$(readlink -f "${proc}/exe" 2>/dev/null || true)" == "$old_gui" ]] && running=1 && break
    done
    (( running == 0 )) && break
    sleep 0.1
done
stop_installed_process "$old_gui" KILL

install -d "${HOME}/.local/bin" "${HOME}/.config/systemd/user" "$STATE_DIR"
old_manifest_content=""
[[ -r "$MANIFEST_FILE" ]] && old_manifest_content="$(cat "$MANIFEST_FILE")"
managed=()
install_managed() {
    local mode="$1" src="$2" dst="$3"
    install -D -m "$mode" "$src" "$dst"
    managed+=("$dst")
}

install_managed 755 "${BUILD_DIR}/rodavarion-tdriverd" "${HOME}/.local/bin/rodavarion-tdriverd"
install_managed 755 "${BUILD_DIR}/rodavarion-tdriverctl" "${HOME}/.local/bin/rodavarion-tdriverctl"
install_managed 644 "${ROOT_DIR}/packaging/systemd/user/rodavarion-tdriverd.service" "${HOME}/.config/systemd/user/rodavarion-tdriverd.service"
install_managed 755 "${ROOT_DIR}/tools/uninstall_all_user.sh" "${HOME}/.local/bin/rodavarion-tdriver-uninstall"

RESOURCE_DIR="${HOME}/.local/share/rodavarion-tdriver/resources"
APP_ICON="${RESOURCE_DIR}/icons/rodavarion-tdriver.png"
UNINSTALL_ICON="${RESOURCE_DIR}/icons/rodavarion-tdriver-uninstall.png"
APP_DESKTOP="${HOME}/.local/share/applications/rodavarion-tdriver.desktop"
UNINSTALL_DESKTOP="${HOME}/.local/share/applications/rodavarion-tdriver-uninstall.desktop"
AUTOSTART_DESKTOP="${HOME}/.config/autostart/rodavarion-tdriver.desktop"

render_desktop_entry() {
    local template="$1" destination="$2"
    sed \
        -e "s|@EXECUTABLE@|${HOME}/.local/bin/rodavarion-tdriver|g" \
        -e "s|@UNINSTALLER@|${HOME}/.local/bin/rodavarion-tdriver-uninstall|g" \
        -e "s|@ICON@|${APP_ICON}|g" \
        -e "s|@UNINSTALL_ICON@|${UNINSTALL_ICON}|g" \
        "$template" > "$destination"
}

if [[ "$EDITION" == "full" ]]; then
    install_managed 755 "${BUILD_DIR}/rodavarion-tdriver" "${HOME}/.local/bin/rodavarion-tdriver"

    # Канонічні ресурси застосунку. Ярлики використовують ці абсолютні шляхи,
    # тому їх робота не залежить від кешу або активної теми іконок.
    install_managed 644 "${ROOT_DIR}/packaging/icons/png/256x256/apps/rodavarion-tdriver.png" "$APP_ICON"
    install_managed 644 "${ROOT_DIR}/packaging/icons/png/256x256/apps/rodavarion-tdriver-uninstall.png" "$UNINSTALL_ICON"

    app_desktop_tmp="$(mktemp)"
    uninstall_desktop_tmp="$(mktemp)"
    render_desktop_entry "${ROOT_DIR}/packaging/applications/rodavarion-tdriver.desktop" "$app_desktop_tmp"
    render_desktop_entry "${ROOT_DIR}/packaging/applications/rodavarion-tdriver-uninstall.desktop" "$uninstall_desktop_tmp"
    install_managed 755 "$app_desktop_tmp" "$APP_DESKTOP"
    install_managed 755 "$uninstall_desktop_tmp" "$UNINSTALL_DESKTOP"
    rm -f -- "$app_desktop_tmp" "$uninstall_desktop_tmp"

    autostart_tmp="$(mktemp)"
    render_desktop_entry "${ROOT_DIR}/packaging/autostart/rodavarion-tdriver.desktop" "$autostart_tmp"
    install_managed 644 "$autostart_tmp" "$AUTOSTART_DESKTOP"
    rm -f -- "$autostart_tmp"

    # Копії для стандартного механізму hicolor і меню програм.
    install_managed 644 "${ROOT_DIR}/packaging/icons/rodavarion-tdriver.svg" "${HOME}/.local/share/icons/hicolor/scalable/apps/rodavarion-tdriver.svg"
    install_managed 644 "${ROOT_DIR}/packaging/icons/rodavarion-tdriver-uninstall.svg" "${HOME}/.local/share/icons/hicolor/scalable/apps/rodavarion-tdriver-uninstall.svg"
    for icon_size in 16 24 32 48 64 128 256; do
        install_managed 644 "${ROOT_DIR}/packaging/icons/png/${icon_size}x${icon_size}/apps/rodavarion-tdriver.png" "${HOME}/.local/share/icons/hicolor/${icon_size}x${icon_size}/apps/rodavarion-tdriver.png"
        install_managed 644 "${ROOT_DIR}/packaging/icons/png/${icon_size}x${icon_size}/apps/rodavarion-tdriver-uninstall.png" "${HOME}/.local/share/icons/hicolor/${icon_size}x${icon_size}/apps/rodavarion-tdriver-uninstall.png"
    done
    install_managed 644 "${ROOT_DIR}/packaging/desktop-directories/rodavarion-tdriver.directory" "${HOME}/.local/share/desktop-directories/rodavarion-tdriver.directory"
    install_managed 644 "${ROOT_DIR}/packaging/menus/rodavarion-tdriver.menu" "${HOME}/.config/menus/applications-merged/rodavarion-tdriver.menu"

    KWIN_SCRIPT_ROOT="${HOME}/.local/share/kwin/scripts/rodavarion-context"
    install_managed 644 "${ROOT_DIR}/packaging/kwin/rodavarion-context/metadata.json" "${KWIN_SCRIPT_ROOT}/metadata.json"
    install_managed 644 "${ROOT_DIR}/packaging/kwin/rodavarion-context/contents/code/main.js" "${KWIN_SCRIPT_ROOT}/contents/code/main.js"

    # За правилом Rodavarion інсталятор не створює ярлик на робочому столі.
    # Прибираємо ярлики, які могли залишитися від старих версій.
    desktop_dir=""
    if command -v xdg-user-dir >/dev/null 2>&1; then
        desktop_dir="$(xdg-user-dir DESKTOP 2>/dev/null || true)"
    fi
    for old_desktop in         "${HOME}/Desktop/Rodavarion TDriver.desktop"         "${HOME}/Desktop/Rodavarion-TDriver.desktop"         "${HOME}/Робочий стіл/Rodavarion TDriver.desktop"         "${HOME}/Робочий стіл/Rodavarion-TDriver.desktop"         "${desktop_dir}/Rodavarion TDriver.desktop"         "${desktop_dir}/Rodavarion-TDriver.desktop"; do
        [[ -n "$old_desktop" ]] && rm -f -- "$old_desktop"
    done
fi

if [[ "$EDITION" == "minimal" ]]; then
    rm -f -- "${HOME}/.local/bin/rodavarion-tdriver" \
        "${HOME}/.local/share/applications/rodavarion-tdriver.desktop" \
        "${HOME}/.local/share/applications/rodavarion-tdriver-uninstall.desktop" \
        "${HOME}/.config/autostart/rodavarion-tdriver.desktop" \
        "${HOME}/.local/share/icons/hicolor/scalable/apps/rodavarion-tdriver.svg" \
        "${HOME}/.local/share/icons/hicolor/scalable/apps/rodavarion-tdriver-uninstall.svg" \
        "${HOME}/.local/share/rodavarion-tdriver/resources/icons/rodavarion-tdriver.png" \
        "${HOME}/.local/share/rodavarion-tdriver/resources/icons/rodavarion-tdriver-uninstall.png" \
        "${HOME}/.local/share/desktop-directories/rodavarion-tdriver.directory" \
        "${HOME}/.config/menus/applications-merged/rodavarion-tdriver.menu" \
        "${HOME}/Desktop/Rodavarion TDriver.desktop" \
        "${HOME}/Desktop/Rodavarion-TDriver.desktop" \
        "${HOME}/Робочий стіл/Rodavarion TDriver.desktop" \
        "${HOME}/Робочий стіл/Rodavarion-TDriver.desktop"
    for icon_size in 16 24 32 48 64 128 256; do
        rm -f -- "${HOME}/.local/share/icons/hicolor/${icon_size}x${icon_size}/apps/rodavarion-tdriver.png" \
            "${HOME}/.local/share/icons/hicolor/${icon_size}x${icon_size}/apps/rodavarion-tdriver-uninstall.png"
    done
fi

new_manifest_tmp="${MANIFEST_FILE}.new"
printf '%s\n' "${managed[@]}" | awk 'NF && !seen[$0]++' > "$new_manifest_tmp"

safe_remove_obsolete() {
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
        *) echo "Застарілий незнайомий шлях не видалено: $path" >&2 ;;
    esac
}

if [[ -n "$old_manifest_content" ]]; then
    while IFS= read -r old_path; do
        [[ -n "$old_path" ]] || continue
        if ! grep -Fqx -- "$old_path" "$new_manifest_tmp"; then
            safe_remove_obsolete "$old_path"
            echo "Видалено застарілий компонент: $old_path"
        fi
    done <<< "$old_manifest_content"
fi

mv -f "$new_manifest_tmp" "${MANIFEST_FILE}.tmp"
mv -f "${MANIFEST_FILE}.tmp" "$MANIFEST_FILE"
cat > "${STATE_FILE}.tmp" <<STATE
INSTALLED_VERSION='${VERSION//\'/\'\\\'\'}'
INSTALLED_EDITION='${EDITION//\'/\'\\\'\'}'
INSTALLED_AT='$(date --iso-8601=seconds)'
STATE
mv -f "${STATE_FILE}.tmp" "$STATE_FILE"

systemctl --user daemon-reload
systemctl --user enable --now rodavarion-tdriverd.service
if command -v kwriteconfig6 >/dev/null 2>&1; then
    kwriteconfig6 --file kwinrc --group Plugins --key rodavarion-contextEnabled true
fi
if command -v qdbus6 >/dev/null 2>&1; then
    qdbus6 org.kde.KWin /KWin reconfigure >/dev/null 2>&1 || true
elif command -v qdbus >/dev/null 2>&1; then
    qdbus org.kde.KWin /KWin reconfigure >/dev/null 2>&1 || true
fi
command -v update-desktop-database >/dev/null 2>&1 && update-desktop-database "${HOME}/.local/share/applications" || true
command -v gtk-update-icon-cache >/dev/null 2>&1 && gtk-update-icon-cache -f -t "${HOME}/.local/share/icons/hicolor" >/dev/null 2>&1 || true
command -v kbuildsycoca6 >/dev/null 2>&1 && kbuildsycoca6 --noincremental >/dev/null 2>&1 || true
command -v kbuildsycoca5 >/dev/null 2>&1 && kbuildsycoca5 --noincremental >/dev/null 2>&1 || true

verify_file() {
    local path="$1" description="$2"
    if [[ ! -f "$path" ]]; then
        echo "ПОМИЛКА: не встановлено ${description}: ${path}" >&2
        return 1
    fi
}

verify_desktop_entry() {
    local path="$1" expected_exec="$2" expected_icon="$3"
    verify_file "$path" "ярлик" || return 1
    [[ -x "$path" ]] || { echo "ПОМИЛКА: ярлик не має права запуску: $path" >&2; return 1; }
    grep -Fqx "Type=Application" "$path" || { echo "ПОМИЛКА: некоректний Type у $path" >&2; return 1; }
    grep -Fqx "Exec=${expected_exec}" "$path" || { echo "ПОМИЛКА: некоректний Exec у $path" >&2; return 1; }
    grep -Fqx "Icon=${expected_icon}" "$path" || { echo "ПОМИЛКА: некоректний Icon у $path" >&2; return 1; }
    verify_file "$expected_exec" "виконуваний файл ярлика" || return 1
    verify_file "$expected_icon" "іконку ярлика" || return 1
    if command -v desktop-file-validate >/dev/null 2>&1; then
        desktop-file-validate "$path"
    fi
}

if [[ "$EDITION" == "full" ]]; then
    verify_desktop_entry "$APP_DESKTOP" "${HOME}/.local/bin/rodavarion-tdriver" "$APP_ICON"
    verify_desktop_entry "$UNINSTALL_DESKTOP" "${HOME}/.local/bin/rodavarion-tdriver-uninstall" "$UNINSTALL_ICON"
    verify_file "$AUTOSTART_DESKTOP" "файл автозапуску"
    grep -Fqx "Exec=${HOME}/.local/bin/rodavarion-tdriver --tray" "$AUTOSTART_DESKTOP" || { echo "ПОМИЛКА: некоректний Exec у файлі автозапуску" >&2; exit 1; }
    if command -v desktop-file-validate >/dev/null 2>&1; then desktop-file-validate "$AUTOSTART_DESKTOP"; fi
    echo "Перевірка інтеграції: GUI, автозапуск у треї, меню програм, іконки та деінсталятор встановлено коректно."
fi

echo
echo "Rodavarion TDriver ${VERSION} (${EDITION}) встановлено."
echo "Попередні налаштування та дані користувача збережено."
echo "Деінсталяція: rodavarion-tdriver-uninstall"
