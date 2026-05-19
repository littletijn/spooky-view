# Spooky View fuzz harnesses

This directory contains libFuzzer-style harnesses for the portable,
platform-independent code paths of Spooky View:

| Harness | Target code | What it fuzzes |
|---|---|---|
| `url_allowlist_fuzz.cc` | `CUpdateAvailableDialog::IsTrustedDownloadUrl` | URL classifier used to gate the "Download" button |
| `update_json_fuzz.cc` | `UpdateChecker` post-parse extraction | Field extraction from a parsed `UpdateResponse` JSON |
| `copydata_fuzz.cc` | `CMainWindow` `WM_COPYDATA` handler | Validation of the cross-instance ping payload |

The harnesses build and run on Linux (or WSL) using clang + libFuzzer.
The same `.cc` files are wired into continuous fuzzing via
[`.clusterfuzzlite/`](../.clusterfuzzlite) and
[`docs/oss-fuzz/`](../docs/oss-fuzz) — see
[`docs/OSS-FUZZ.md`](../docs/OSS-FUZZ.md) for the full continuous-fuzzing story.

## Prerequisites (Linux / WSL Ubuntu)

```bash
sudo apt-get install -y clang lld zip
```

`clang` ≥ 12 is recommended for libFuzzer + ASan / UBSan support.

## Building and running locally

From the repo root:

```bash
cd fuzz
./build.sh                # produces ./build/<target> binaries
./build/url_allowlist_fuzz -max_total_time=30
./build/update_json_fuzz  -max_total_time=30
./build/copydata_fuzz     -max_total_time=30
```

To use a seed corpus, drop inputs under `fuzz/corpus/<target>/` and pass
the directory as the first positional argument:

```bash
./build/url_allowlist_fuzz corpus/url_allowlist_fuzz -max_total_time=60
```

To use the dictionary:

```bash
./build/update_json_fuzz -dict=dict.txt corpus/update_json_fuzz
```

## Reproducing a crash

libFuzzer dumps a crashing input to `crash-<sha1>` in the current
directory. Feed it back into the same binary:

```bash
./build/url_allowlist_fuzz crash-deadbeef...
```

For crashes reported by ClusterFuzzLite (GitHub Actions artifact) or
OSS-Fuzz (issues.oss-fuzz.com testcase), follow the reproduction recipe
in [`docs/OSS-FUZZ.md`](../docs/OSS-FUZZ.md#reproducing-an-oss-fuzz-crash-locally).

## Notes

- The harnesses deliberately avoid Win32 dependencies. Any logic they
  exercise is either lifted into a small portable helper or stubbed with
  inline declarations inside the `.cc` file.
- Do **not** add Win32 code or DLL-injection paths to these harnesses;
  the goal is fast, sanitizer-friendly fuzzing of pure data parsers and
  validators.
- The continuous-fuzzing `build.sh` files
  (`.clusterfuzzlite/build.sh` and `docs/oss-fuzz/build.sh`) are kept
  identical. When adding a new harness, update both alongside this
  directory's local `build.sh`.
