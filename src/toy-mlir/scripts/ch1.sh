#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-${ROOT_DIR}/build/mlir-ch1}"
CHAPTER_TARGET="mlir-ch1"
TEST_TARGET="test_mlir_ch1_lexer"
TEST_NAME="mlir-ch1-lexer-smoke"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}" --target "${CHAPTER_TARGET}" "${TEST_TARGET}" -j "$(nproc)"

ctest --test-dir "${BUILD_DIR}" --output-on-failure -R "^${TEST_NAME}$"

AST_OUTPUT="$("${BUILD_DIR}/${CHAPTER_TARGET}" "${ROOT_DIR}/test/ch1/basic.toy" -emit=ast 2>&1)"

echo "${AST_OUTPUT}"

grep -q "Module:" <<< "${AST_OUTPUT}"
grep -q "Proto 'main'" <<< "${AST_OUTPUT}"
grep -q "Print" <<< "${AST_OUTPUT}"

echo "${CHAPTER_TARGET} verification passed"
