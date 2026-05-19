#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

CXX=${CXX:-clang++}
# Allow Homebrew-installed clang on systems where apt clang is unavailable.
if ! command -v "$CXX" >/dev/null 2>&1 && [ -x /home/linuxbrew/.linuxbrew/opt/llvm/bin/clang++ ]; then
    CXX=/home/linuxbrew/.linuxbrew/opt/llvm/bin/clang++
fi
CFLAGS=(-O1 -g -std=c++17 -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer)
JSON_INC=(-I../packages/nlohmann.json.3.11.3/build/native/include)

mkdir -p build
for tgt in url_allowlist_fuzz update_json_fuzz copydata_fuzz; do
    echo "[build] ${tgt}"
    "$CXX" "${CFLAGS[@]}" "${JSON_INC[@]}" "${tgt}.cc" -o "build/${tgt}"
done
echo "[build] done -> ./build/{url_allowlist_fuzz,update_json_fuzz,copydata_fuzz}"
