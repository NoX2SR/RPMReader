#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$(mktemp -d "${TMPDIR:-/tmp}/rpm-reader-tests.XXXXXX")"
trap 'rm -rf "${BUILD_DIR}"' EXIT

g++ -std=c++11 -Wall -Wextra -Werror -O0 -g --coverage \
  -I "${ROOT_DIR}/tests/fakes" \
  "${ROOT_DIR}/tests/fakes/Arduino.cpp" \
  "${ROOT_DIR}/tests/test_rpm_reader.cpp" \
  -o "${BUILD_DIR}/test_rpm_reader"

"${BUILD_DIR}/test_rpm_reader"
echo "PASS test_rpm_reader"

echo "Production line coverage:"
(cd "${BUILD_DIR}" && gcov -b -c "${BUILD_DIR}/test_rpm_reader-test_rpm_reader.gcno" 2>&1) | awk '
  /^File .*RPMReader\.ino/ { selected = 1; next }
  selected && /^Lines executed:/ { print "RPMReader.ino: " substr($0, 16); exit }
'
