#!/usr/bin/env bash
set -euo pipefail

cd "${SRC}/spooky-view"

JSON_INC="-Ipackages/nlohmann.json.3.11.3/build/native/include"

TARGETS=(url_allowlist_fuzz update_json_fuzz copydata_fuzz)

for tgt in "${TARGETS[@]}"; do
  "${CXX}" ${CXXFLAGS} -std=c++17 ${JSON_INC} \
    fuzz/${tgt}.cc \
    ${LIB_FUZZING_ENGINE} \
    -o "${OUT}/${tgt}"

  if [ -d "fuzz/corpus/${tgt}" ]; then
    (cd "fuzz/corpus/${tgt}" && zip -qr "${OUT}/${tgt}_seed_corpus.zip" .)
  fi
done

if [ -f fuzz/dict.txt ]; then
  for tgt in "${TARGETS[@]}"; do
    cp fuzz/dict.txt "${OUT}/${tgt}.dict"
  done
fi
