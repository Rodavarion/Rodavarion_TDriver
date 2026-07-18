#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
edition="${1:-full}"

if [[ "${edition}" == "--edition" ]]; then
    edition="${2:-full}"
fi

echo "[1/5] Перевірка JSON..."
python -m json.tool \
    "${ROOT_DIR}/config/dependencies.arch.json" >/dev/null
python -m json.tool \
    "${ROOT_DIR}/config/default.json" >/dev/null

echo "[2/5] Перевірка shell-скриптів..."
for script in "${ROOT_DIR}"/tools/*.sh; do
    bash -n "${script}"
done

echo "[3/5] Перевірка обов'язкових файлів..."

# Regression guard: GUI and daemon must share one context profile path.
grep -q 'GenericConfigLocation' "${ROOT_DIR}/src/context/ApplicationContext.cpp"
grep -q 'Rodavarion Technologies/Rodavarion TDriver' "${ROOT_DIR}/src/context/ApplicationContext.cpp"
if grep -q 'setApplicationName("Rodavarion TDriver Service")' "${ROOT_DIR}/src/daemon/main.cpp"; then
  echo "Помилка: daemon знову використовує окреме ім'я застосунку і окремі налаштування." >&2
  exit 1
fi
required=(
    "src/daemon/main.cpp"
    "src/cli/main.cpp"
    "packaging/systemd/user/rodavarion-tdriverd.service"
    "include/rodavarion/runtime/MouseRuntimeController.hpp"
)
for file in "${required[@]}"; do
    [[ -f "${ROOT_DIR}/${file}" ]] || {
        echo "Відсутній файл: ${file}" >&2
        exit 3
    }
done

[[ -f "${ROOT_DIR}/packaging/kwin/rodavarion-context/metadata.json" ]] || { echo "Відсутній metadata.json KWin bridge" >&2; exit 1; }
[[ -f "${ROOT_DIR}/packaging/kwin/rodavarion-context/contents/code/main.js" ]] || { echo "Відсутній main.js KWin bridge" >&2; exit 1; }
grep -Fq 'workspace.windowActivated.connect' "${ROOT_DIR}/packaging/kwin/rodavarion-context/contents/code/main.js" || { echo "KWin bridge не слухає активне вікно" >&2; exit 1; }
grep -Fq 'org.rodavarion.TDriver' "${ROOT_DIR}/packaging/kwin/rodavarion-context/contents/code/main.js" || { echo "KWin bridge не має D-Bus цілі" >&2; exit 1; }
echo "[4/5] Чиста збірка й тести..."
"${ROOT_DIR}/tools/build_arch.sh" --edition "${edition}"

# Architectural regression guards for the input pipeline.
grep -q "single owner of the physical mouse grab" src/gui/MainWindow.cpp || {
  echo "Preflight: відсутній захист єдиного власника EVIOCGRAB." >&2; exit 1;
}
grep -q "profiles.size() == 1" src/action/MouseMappingStore.cpp || {
  echo "Preflight: відсутній fallback профілю для різних interface keys." >&2; exit 1;
}
grep -q "Emit complete keyboard states in deterministic frames" src/runtime/DesktopActionExecutor.cpp || {
  echo "Preflight: відсутня атомарна передача модифікаторів uinput." >&2; exit 1;
}
if grep -q "QTimer::singleShot(45" src/runtime/MouseRuntimeController.cpp; then
  echo "Preflight: повернулася небезпечна затримка dispatch." >&2; exit 1;
fi

echo "[5/5] Preflight успішний."

# Direct keyboard backend invariants.
grep -q 'Rodavarion TDriver Virtual Keyboard' "${ROOT_DIR}/src/runtime/DesktopActionExecutor.cpp"
grep -q 'UI_DEV_CREATE' "${ROOT_DIR}/src/runtime/DesktopActionExecutor.cpp"
grep -q 'sendShortcut(portableShortcut)' "${ROOT_DIR}/src/runtime/DesktopActionExecutor.cpp"
