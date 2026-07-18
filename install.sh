#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EDITION="full"
ASSUME_YES=0
SKIP_BOOTSTRAP=0

usage() {
    cat <<USAGE
Rodavarion TDriver — єдиний інсталятор для Arch Linux

Використання: ./install.sh [параметри]
  --edition full|minimal  редакція (типово: full)
  --yes, -y                підтвердити заміну старої версії
  --skip-bootstrap         не перевіряти й не встановлювати системні залежності
  --help, -h               показати довідку

Інсталятор послідовно виконує:
  1) підготовку системи;
  2) чисту збірку та тести;
  3) встановлення перевірених файлів без повторної збірки.
USAGE
}

while (($#)); do
    case "$1" in
        --edition) EDITION="${2:-}"; shift 2 ;;
        --yes|-y) ASSUME_YES=1; shift ;;
        --skip-bootstrap) SKIP_BOOTSTRAP=1; shift ;;
        --help|-h) usage; exit 0 ;;
        *) echo "Невідомий параметр: $1" >&2; usage >&2; exit 2 ;;
    esac
done

if [[ "$EDITION" != "full" && "$EDITION" != "minimal" ]]; then
    echo "Допустимі редакції: minimal або full." >&2
    exit 2
fi

printf '\n=== Rodavarion TDriver: автоматичне встановлення (%s) ===\n' "$EDITION"

if (( ! SKIP_BOOTSTRAP )); then
    echo "[1/3] Підготовка системи"
    "${ROOT_DIR}/tools/bootstrap_arch.sh" --edition "$EDITION"
else
    echo "[1/3] Підготовку системи пропущено за параметром --skip-bootstrap"
fi

echo "[2/3] Перевірка, чиста збірка й тести"
"${ROOT_DIR}/tools/preflight_arch.sh" --edition "$EDITION"

echo "[3/3] Встановлення перевіреної збірки"
install_args=(--edition "$EDITION")
(( ASSUME_YES )) && install_args+=(--yes)
"${ROOT_DIR}/tools/install_user.sh" "${install_args[@]}"

echo
echo "Установлення успішно завершено."
if [[ "$EDITION" == "full" ]]; then
    echo "Запуск GUI: ~/.local/bin/rodavarion-tdriver"
fi
