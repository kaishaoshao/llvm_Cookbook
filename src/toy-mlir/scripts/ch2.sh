#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-${ROOT_DIR}/build/mlir-ch2}"
CHAPTER_TARGET="mlir-ch2"
AST_TEST_NAME="mlir-ch2-ast-smoke"
TODO_TEST_NAME="mlir-ch2-emit-mlir-todo"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}" --target "${CHAPTER_TARGET}" -j "$(nproc)"

ctest --test-dir "${BUILD_DIR}" --output-on-failure -R "^(${AST_TEST_NAME}|${TODO_TEST_NAME})$"

AST_OUTPUT="$("${BUILD_DIR}/${CHAPTER_TARGET}" "${ROOT_DIR}/test/ch2/codegen.toy" -emit=ast 2>&1)"

echo "${AST_OUTPUT}"

grep -q "Module:" <<< "${AST_OUTPUT}"
grep -q "Proto 'main'" <<< "${AST_OUTPUT}"
grep -q "Print" <<< "${AST_OUTPUT}"

MLIR_STATUS=0
MLIR_OUTPUT="$("${BUILD_DIR}/${CHAPTER_TARGET}" "${ROOT_DIR}/test/ch2/codegen.toy" -emit=mlir 2>&1)" || MLIR_STATUS=$?

echo "${MLIR_OUTPUT}"

if [[ "${MLIR_STATUS}" -eq 0 ]]; then
  echo "expected ${CHAPTER_TARGET} -emit=mlir to be unimplemented" >&2
  exit 1
fi

grep -q "MLIR emission for ${CHAPTER_TARGET} is not implemented yet" <<< "${MLIR_OUTPUT}"

echo "${CHAPTER_TARGET} interface verification passed"
