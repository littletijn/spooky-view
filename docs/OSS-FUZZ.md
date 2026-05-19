# Continuous Fuzzing for Spooky View

This document explains how Spooky View's portable, fuzz-amenable code paths
(URL allowlist, update-response JSON post-parse extraction, and the
`WM_COPYDATA` payload validator) are exercised by continuous fuzzing.

## Table of contents

- [Background: OSS-Fuzz vs. ClusterFuzzLite](#background-oss-fuzz-vs-clusterfuzzlite)
- [Repository layout](#repository-layout)
- [ClusterFuzzLite (in-repo, no submission)](#clusterfuzzlite-in-repo-no-submission)
  - [Workflows](#workflows)
  - [Storage repo for corpus and coverage history](#storage-repo-for-corpus-and-coverage-history)
- [Submitting to upstream OSS-Fuzz](#submitting-to-upstream-oss-fuzz)
  - [Preflight: required maintainer email](#preflight-required-maintainer-email)
  - [Local smoke test before opening the PR](#local-smoke-test-before-opening-the-pr)
  - [Opening the PR](#opening-the-pr)
  - [After merge](#after-merge)
- [Reproducing an OSS-Fuzz crash locally](#reproducing-an-oss-fuzz-crash-locally)
- [Coverage report](#coverage-report)
- [Adding a new fuzz harness](#adding-a-new-fuzz-harness)
- [Disclosure flow](#disclosure-flow)

## Background: OSS-Fuzz vs. ClusterFuzzLite

Both are Google-maintained fuzzing systems that share the same harness format
(libFuzzer-style `LLVMFuzzerTestOneInput`), Docker base image
(`gcr.io/oss-fuzz-base/base-builder`), and `build.sh` contract.

| | **OSS-Fuzz (upstream)** | **ClusterFuzzLite (in-repo)** |
|---|---|---|
| Hosting | Google's ClusterFuzz infrastructure | Your GitHub Actions runners |
| Submission | PR to `google/oss-fuzz` | None — files live in this repo |
| Cost | Free for OSS projects accepted to the program | GitHub Actions minutes |
| Bug tracker | `issues.oss-fuzz.com` (private 90-day window) | GitHub issues / Action logs |
| Crash email | Sent to `primary_contact` / `auto_ccs` | None (read Action output) |
| Coverage history | Hosted dashboard | You host a `storage-repo` |

You can run **both** simultaneously — the scaffolding here supports it.

## Repository layout

```text
fuzz/                         # libFuzzer harnesses (owned by the build)
  url_allowlist_fuzz.cc
  update_json_fuzz.cc
  copydata_fuzz.cc
  build.sh                    # local Linux/WSL convenience build
  README.md                   # local dev instructions
  corpus/<target>/            # optional seed corpora per target
  dict.txt                    # optional shared libFuzzer dictionary

.clusterfuzzlite/             # ClusterFuzzLite config (GitHub Actions)
  Dockerfile
  build.sh
  project.yaml

.github/workflows/
  cflite_pr.yml               # short fuzz on every PR
  cflite_batch.yml            # daily long fuzz

docs/oss-fuzz/                # upstream submission package
  Dockerfile
  build.sh
  project.yaml
```

The two `build.sh` files are intentionally identical — OSS-Fuzz copies
`docs/oss-fuzz/build.sh` to `$SRC/build.sh` inside the image, while
ClusterFuzzLite uses `.clusterfuzzlite/build.sh` the same way.

## ClusterFuzzLite (in-repo, no submission)

Nothing to submit. The workflows under `.github/workflows/cflite_*.yml`
run on this repository's Actions runners.

### Workflows

- **`cflite_pr.yml`** — runs on every PR that touches `SpookyView/**`,
  `fuzz/**`, `packages/nlohmann.json.**/**`, or `.clusterfuzzlite/**`.
  Builds the fuzzers with ASan + UBSan and fuzzes each for 5 minutes
  against the PR's code change. Fails the check on any crash.
- **`cflite_batch.yml`** — runs daily at 06:00 UTC (and on
  `workflow_dispatch`). Fuzzes for 30 minutes per sanitizer in `batch`
  mode, persisting corpus and coverage to the configured storage repo.

### Storage repo for corpus and coverage history

`cflite_batch.yml` references `secrets.CFLITE_STORAGE_REPO`. To enable
persistent corpus growth and a coverage dashboard:

1. Create a **separate** GitHub repo (e.g., `SchwartzKamel/spooky-view-fuzz-storage`).
2. Give that repo a `main` branch (corpus) and a `gh-pages` branch
   (coverage HTML).
3. Generate a fine-grained PAT with `contents: read/write` on that
   storage repo.
4. Add it as `CFLITE_STORAGE_REPO` (repo URL, e.g.
   `https://github.com/SchwartzKamel/spooky-view-fuzz-storage.git`) and
   a companion `PERSONAL_ACCESS_TOKEN` secret (refer to ClusterFuzzLite
   docs for the exact secret name your action version expects).

Without the storage repo the batch workflow still runs, but every run
starts from a cold corpus and coverage results are discarded.

## Submitting to upstream OSS-Fuzz

### Preflight: required maintainer email

OSS-Fuzz requires `primary_contact` (and any `auto_ccs`) to be **emails
controlled by project maintainers and recognized by Google** (Gmail or
a Google-Workspace account, so the contact can sign in to
`issues.oss-fuzz.com`). The `SchwartzKamel` fork must designate at least
one such email before submitting.

Edit `docs/oss-fuzz/project.yaml` and replace every `FILL_IN@example.com`.

### Local smoke test before opening the PR

```bash
# 1. Clone OSS-Fuzz
git clone --depth 1 https://github.com/google/oss-fuzz.git
cd oss-fuzz

# 2. Drop our scaffold into projects/spooky-view/
mkdir -p projects/spooky-view
cp /path/to/spooky-view/docs/oss-fuzz/Dockerfile  projects/spooky-view/
cp /path/to/spooky-view/docs/oss-fuzz/build.sh    projects/spooky-view/
cp /path/to/spooky-view/docs/oss-fuzz/project.yaml projects/spooky-view/

# 3. Build the image and the fuzzers
python3 infra/helper.py build_image spooky-view
python3 infra/helper.py build_fuzzers --sanitizer address spooky-view

# 4. Run one fuzzer for a few seconds
python3 infra/helper.py run_fuzzer spooky-view url_allowlist_fuzz -- -max_total_time=30

# 5. Repeat with UBSan
python3 infra/helper.py build_fuzzers --sanitizer undefined spooky-view
python3 infra/helper.py run_fuzzer spooky-view update_json_fuzz -- -max_total_time=30
```

All three targets must build and run cleanly before the PR is opened.

### Opening the PR

1. Fork `google/oss-fuzz`.
2. On a feature branch, commit `projects/spooky-view/{Dockerfile,build.sh,project.yaml}`.
3. Open a PR titled `[spooky-view] Initial integration`.
4. Reviewers will sanity-check the `primary_contact` email and may
   request it be added as an OSS-Fuzz-recognized contact before merge.

### After merge

Fuzzing starts within ~24 hours. Crash reports are filed at
`issues.oss-fuzz.com`, with a **90-day responsible-disclosure window**
during which they remain restricted. The `primary_contact` and
`auto_ccs` receive email notifications.

## Reproducing an OSS-Fuzz crash locally

When OSS-Fuzz files a bug it attaches a testcase. To reproduce:

```bash
git clone --depth 1 https://github.com/google/oss-fuzz.git
cd oss-fuzz

# Place the scaffold (see "Local smoke test" above) and rebuild fuzzers
# with the matching sanitizer reported by OSS-Fuzz.
python3 infra/helper.py build_fuzzers --sanitizer address spooky-view

# Reproduce
python3 infra/helper.py reproduce spooky-view url_allowlist_fuzz testcase.bin
```

For ClusterFuzzLite crashes the testcase is uploaded as an Actions
artifact on the failing workflow run. Download it, then use the same
`reproduce` command (or run the fuzzer binary directly under
`fuzz/build/` from `fuzz/README.md`).

## Coverage report

```bash
python3 infra/helper.py build_fuzzers --sanitizer coverage spooky-view
python3 infra/helper.py coverage spooky-view
```

Opens an HTML report in your browser. Use this to find untested
branches and to grow the seed corpus.

## Adding a new fuzz harness

1. Add `fuzz/<name>_fuzz.cc` implementing `LLVMFuzzerTestOneInput`.
2. Append `<name>_fuzz` to the `TARGETS=( ... )` array in **both**
   `.clusterfuzzlite/build.sh` and `docs/oss-fuzz/build.sh` (the two
   files are kept identical on purpose).
3. (Optional) Drop seed inputs at `fuzz/corpus/<name>_fuzz/`. The build
   script will zip them into `<name>_fuzz_seed_corpus.zip` automatically.
4. (Optional) Extend `fuzz/dict.txt` with tokens for the new format;
   it's shipped to every target as `<name>_fuzz.dict`.
5. Run the local smoke test above to confirm the new target builds
   under both sanitizers.

## Disclosure flow

If continuous fuzzing finds a real vulnerability:

- OSS-Fuzz: the bug is private for 90 days; coordinate disclosure with
  the maintainer email via the channel described in
  [`SECURITY.md`](../SECURITY.md).
- ClusterFuzzLite: the failing PR check / batch run output is visible
  to anyone with read access to this repo. If the crash is sensitive,
  **do not** push the reproducer publicly — file a private security
  advisory per [`SECURITY.md`](../SECURITY.md) and link the run.

The findings and remediation patterns from the 2026 audit are tracked
in [`docs/SECURITY-AUDIT-2026.md`](SECURITY-AUDIT-2026.md); preserve
those patterns when fixing fuzz-found issues in the same components
(`UpdateChecker`, `CRegistrySettingsManager`, `CUpdateAvailableDialog`,
`WindowsEnum`, `AppMain.cpp`).
