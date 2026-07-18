#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
VERSION="$(cat "${ROOT_DIR}/VERSION" 2>/dev/null || printf 'unknown')"

edition="full"
if [[ "${1:-}" == "--edition" ]]; then
    edition="${2:-full}"
elif [[ -n "${1:-}" ]]; then
    edition="${1}"
fi

case "${edition}" in
    minimal)
        gui="OFF"
        daemon="ON"
        cli="ON"
        ;;
    full)
        gui="ON"
        daemon="ON"
        cli="ON"
        ;;
    *)
        echo "Допустимі редакції: minimal або full." >&2
        exit 2
        ;;
esac

rm -rf "${BUILD_DIR}"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DRODAVARION_BUILD_GUI="${gui}" \
    -DRODAVARION_BUILD_DAEMON="${daemon}" \
    -DRODAVARION_BUILD_CLI="${cli}" \
    -DRODAVARION_BUILD_TESTS=ON

cmake --build "${BUILD_DIR}"
ctest --test-dir "${BUILD_DIR}" --output-on-failure

artifacts=(rodavarion-tdriverd rodavarion-tdriverctl)
[[ "${edition}" == "full" ]] && artifacts+=(rodavarion-tdriver)

for artifact in "${artifacts[@]}"; do
    [[ -x "${BUILD_DIR}/${artifact}" ]] || {
        echo "Після збірки відсутній виконуваний файл: build/${artifact}" >&2
        exit 5
    }
done

(
    cd "${BUILD_DIR}"
    sha256sum "${artifacts[@]}" > .rodavarion-artifacts.sha256
)

cat > "${BUILD_DIR}/.rodavarion-build.env" <<STATE
BUILD_SCHEMA=1
BUILD_VERSION=${VERSION}
BUILD_EDITION=${edition}
BUILD_TESTS_PASSED=1
STATE

echo
echo "Збірка редакції ${edition} завершена, тести пройдено."
