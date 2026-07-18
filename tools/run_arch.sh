#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

if [[ -x "${BUILD_DIR}/rodavarion-tdriver" ]]; then
    exec "${BUILD_DIR}/rodavarion-tdriver"
fi

echo "GUI у цій збірці відсутній."
echo "Запуск headless-служби в передньому плані..."
exec "${BUILD_DIR}/rodavarion-tdriverd"
