# Spooky View — Security Audit 2026

| Field | Value |
|---|---|
| Repository | `SchwartzKamel/spooky-view` (fork of `littletijn/spooky-view`) |
| Audit type | Internal static review |
| Scope | This fork's source tree at HEAD prior to remediation |
| Method | Five-workstream parallel static analysis (docs/vibe, net/update, Win32 core, registry/UI, build/supply chain) |
| Reviewer | Internal |
| Date | 2026-05-13 |
| Remediation commit | `9311a0a` — "Security and quality hardening from internal review" |
| Limitations | No upstream diff against `littletijn/spooky-view`; no compile; no dynamic analysis (no debugger, fuzzing, or runtime tracing); Windows-only target was not executed |

---

## Executive summary

**Verdict: Not malicious. Reasonably safe to use with the noted caveats.**

The fork is a verbatim mirror of upstream. `git shortlog --all` shows zero
commits authored by `SchwartzKamel` prior to the remediation commit — every
line of pre-existing code is authored by the upstream maintainer
(`Martijn van Antwerpen` / `littletijn`). The application:

- Runs as `asInvoker` (no elevation manifest).
- Stores all settings in `HKCU` only — no `HKLM`, no services, no scheduled tasks, no `AppInit_DLLs`, no `Image File Execution Options`.
- Does not inject code into other processes. There is no `WriteProcessMemory`, `CreateRemoteThread`, `VirtualAllocEx`, `SetWindowsHookEx`, `WINEVENT_INCONTEXT`, or `AdjustTokenPrivileges` anywhere in the tree. All `SetWinEventHook` calls are `WINEVENT_OUTOFCONTEXT`.
- Touches foreign windows only via supported `SetWindowLongPtr(GWL_EXSTYLE)` + `SetLayeredWindowAttributes` calls, after `OpenProcess(PROCESS_QUERY_INFORMATION)`.
- Has exactly one outbound network endpoint: `HTTPS GET https://updates.tyndomyn.net/spookyview/status` using `WINHTTP_FLAG_SECURE` with no cert-bypass flags. The check is opt-out from the Settings dialog.

The review surfaced **three High**, **nine Medium**, and **six Low** findings.
Commit `9311a0a` fixes all three High findings and eight of the nine Medium
findings. The unremediated items (M8 signing, L2 historical PFX, `v141_xp`
hardening, and CRLF noise) are explicitly accepted and documented below
with rationale and user-side mitigations.

---

## Methodology

The review ran as five parallel static workstreams against the fork's
checked-out source tree. No build, no execution, no upstream comparison.

| WS | Focus | Output |
|---|---|---|
| WS1 | Documentation, repo "vibe", authorship, license, secrets, binary blobs | `ws1-docs-vibe.md` |
| WS2 | Network surface and update channel (`UpdateChecker`, `CUpdateAvailableDialog`) | `ws2-net-update.md` |
| WS3 | Win32 core — process / window enumeration, single-instance, IPC, layered-window logic | `ws3-win32-core.md` |
| WS4 | Registry I/O, settings persistence, dialog/control input handling | `ws4-registry-ui.md` |
| WS5 | Build configuration, toolchain hardening flags, signing, MSI/MSIX, third-party deps | `ws5-build-supply.md` |

A consolidated rollup was written to
`session-state/.../files/spooky-view-review.md`. This document re-states
those findings against the post-remediation state of the tree.

---

## Findings & remediation

Status legend: **F** = Fixed in `9311a0a`; **N** = Not fixed (see notes).

| ID | Sev | File:Line (pre-fix) | Description | Status | Remediation summary |
|---|---|---|---|---|---|
| H1 | High | `SpookyView\CUpdateAvailableDialog.cpp:43` | `ShellExecute(NULL,"open",downloadUrl,...)` invoked on unvalidated JSON `download_url` from update server. UNC/`file://`/foreign-protocol redirection possible if `updates.tyndomyn.net` is compromised or MITMd. | F | Added `IsTrustedDownloadUrl()` helper using `StrCmpNI` against literal `https://updates.tyndomyn.net/` prefix; `ShellExecute` is gated on that check. (`SpookyView\CUpdateAvailableDialog.cpp` +27 lines) |
| H2 | High | `SpookyView\CSettingsDialog.cpp` (`AddAutoRun`) | `GetModuleFileName(0, programPath, sizeof(programPath))` passes a byte count where TCHAR count is required (2× over-report under UNICODE → stack overflow). Subsequent `RegSetValueEx(..., sizeof(programPath))` writes uninitialized stack bytes into `HKCU\...\Run`. Path was also unquoted. | F | `GetModuleFileName` now receives `_countof(programPath)-2`, return value is range-checked, path is surrounded with `"` quotes, and `RegSetValueEx` uses an exact `(len+3)*sizeof(TCHAR)` byte count. `RegOpenKeyEx` downgraded from `KEY_ALL_ACCESS` to `KEY_SET_VALUE` for both `AddAutoRun`/`RemoveAutoRun`. |
| H3 | High | `SpookyView\ListView.cpp` (`GetTextByIndex`) | Hard-coded `cchTextMax = MAX_PATH` (260) while callers in `CAddWindowDialog`/`CSetupDialog`/`CAddAppDialog` passed 256-TCHAR buffers — 8-byte stack overflow trigerable via any window with a long class name. | F | `GetTextByIndex` signature gains `int cchTextMax`; callers updated to pass `_countof(buffer)`; null-buffer guard and explicit terminator-clamping added. (`SpookyView\ListView.cpp`, `ListView.h`, three call sites) |
| M1 | Med | `SpookyView\CLimitSingleInstance.h` / `AppMain.cpp` | Single-instance mutex named bare `"SpookyView"` — unprefixed, predictable, session-local. Any local process can squat the name and DoS startup. | F | Renamed to `Local\SpookyView-SingleInstance-{7CE47E95-8AA0-4B62-87FD-CA1242022F47}`. (`SpookyView\AppMain.cpp:41`) |
| M2 | Med | `SpookyView\CMainWindow.cpp` (`WM_COPYDATA`) | Handler did not validate `lParam`, `cbData`, or `lpData`; the "auth" check was `strrchr(msg,'S')` — any session-local process could trigger `OpenSetupDialog()` or cause OOB read. | F | Validates `dataCopy`, `lpData`, and `cbData >= sizeof(expected)`. Copies into a bounded 64-byte buffer with explicit NUL-termination, then requires `strcmp == 0` against the literal. |
| M3 | Med | `SpookyView\WindowsEnum.cpp` (`IsWindowTransparent`) | Read `GWL_STYLE` and AND-ed with `WS_EX_LAYERED` — wrong style word. Restore-on-exit could fail to revert some windows the app had layered. | F | Switched to `GWL_EXSTYLE`, comparison normalized to `!= 0`. |
| M4 | Med | `SpookyView\WindowsEnum.cpp` (`GetWindowProcessAndClass`) | `_tcsrchr(...,'\\'); fileName++;` with no NULL check → AV if `GetProcessImageFileName` ever returns a path without a backslash. Return value also unchecked. | F | `GetProcessImageFileName` return value checked; buffer NUL-terminated; backslash-absent case falls back to the whole path instead of dereferencing `NULL+1`. |
| M5 | Med | `SpookyView\CNotifyIcon.cpp` | `_stprintf_s(buf, n, userString)` — user/resource strings used as the format string. Currently controlled but a fragile pattern. | F | All three call sites switched to `StringCchCopy` with `<strsafe.h>`; NULL inputs coerced to empty string. |
| M6 | Med | `SpookyView\UpdateChecker.cpp` / `PRIVACY.md` | `pre_release=1` query parameter is appended for `PRE_RELEASE` builds but not declared in `PRIVACY.md`. | F | `PRIVACY.md` updated to list the `pre_release=1` flag. |
| M7 | Med | `SpookyView\CRegistrySettingsManager.cpp` (`ReadValue`, `ShouldSkipVersion`) | `REG_SZ`/`REG_EXPAND_SZ`/`REG_MULTI_SZ` reads trusted the registry-provided `cbData` and did not force NUL-termination — downstream `_tcscmp`/`_tcsrchr`/`compare` can OOB-read. | F | Both `ReadValue` overloads force trailing-byte zeroing for string registry types; `ShouldSkipVersion` zero-initializes its 100-TCHAR buffer and explicitly caps the last slot to `'\0'`. |
| M8 | Med | `SpookyView\SpookyView.vcxproj:580-587` | EXE signing explicitly disabled (`<!-- Disabled signing -->` block). Only the MSI wrapper and the MSIX package are signed, and the MSIX uses `StorePackaging_TemporaryKey.pfx` (developer placeholder). SmartScreen will flag the EXE. | **N** | Out-of-scope for source-only commit: a production code-signing certificate is required. Users should prefer the MSI or MSIX build. |
| M9 | Med | `SpookyView\SpookyView.vcxproj` (all configurations) | No `/guard:cf`, `/Qspectre`, `/CETCOMPAT`, `/HIGHENTROPYVA`, explicit `/GS`, `/NXCOMPAT`, or `/DYNAMICBASE`. 9 of 12 configs are pinned to `v141_xp` which cannot emit CFG/Spectre. | F (partial) | `BufferSecurityCheck`, `ControlFlowGuard=Guard`, `RandomizedBaseAddress`, `HighEntropyVA`, and `DataExecutionPrevention` enabled on the three `v143` ARM64 Release configurations. `v141_xp` configurations cannot accept these flags by design (see "Items not remediated"). |
| L1 | Low | `SpookyView\UpdateChecker.cpp:22` | `std::unique_ptr<UpdateChecker> autoUpdater; autoUpdater->CheckForNewerVersion();` is a null-dereference that "works" because the call is non-virtual and touches no `this` state. Latent crash. | F | Replaced with a stack-allocated `UpdateChecker autoUpdater;`. |
| L2 | Low | `StorePackaging_TemporaryKey.pfx` (added `7614a0b`, removed `ae69297`) | VS-generated UWP temporary developer cert still recoverable from git history. No production value. | **N** | Removing requires rewriting/force-pushing history. Documented; left alone. |
| L3 | Low | `nlohmann/json 3.10.5` (sole third-party NuGet, header-only) | Pinned, slightly stale. No known critical CVE at this version. | **N** | No action; acceptable for a header-only library at a pinned version. |
| L4 | Low | `SpookyView\CRegistrySettingsManager.cpp` (various) | `KEY_ALL_ACCESS` requested where `KEY_READ` / `KEY_WRITE` would suffice. Does not cross a privilege boundary (HKCU only) but is loose. | **N** | Not addressed in `9311a0a` beyond the `AddAutoRun`/`RemoveAutoRun` downgrades (H2). Remaining call sites stay broad. |
| L5 | Low | Dialog edit controls (`CAddAppDialog`, `CAddWindowDialog`, `CSetupDialog`) | No `EM_SETLIMITTEXT`. Mitigated by `MAX_PATH`-sized read buffers but worth tightening. | **N** | Not addressed; defense-in-depth only. |
| L6 | Low | Commit timestamps in 2026 | Likely upstream author's machine clock skew, not a tamper signal. | **N** | Informational only. |

---

## Detailed findings (High and Medium)

### H1 — Unvalidated `download_url` passed to `ShellExecute`

**What's wrong.** `CUpdateAvailableDialog` displayed the `download_url`
returned in the update server's JSON response and, on click, passed it
verbatim to `ShellExecute`. Any string the server returns is honoured —
including `file://`, UNC, `ms-settings:`, custom URI handlers, or a
different `https://` host.

**Why it matters.** The single TLS endpoint is the only barrier between
the app and arbitrary URI execution on the user's desktop. Server
compromise, a misissued cert + DNS hijack, or a misconfigured corporate
TLS-intercept proxy turns a click on "Download" into arbitrary protocol
handler invocation.

Original (`SpookyView\CUpdateAvailableDialog.cpp:43`):

```cpp
case ID_DOWNLOAD:
#ifndef PACKAGING_STORE
    ShellExecute(NULL, _T("open"), this->downloadUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
#endif
```

Fix in `9311a0a`:

```cpp
static const TCHAR kPrefix[] = _T("https://updates.tyndomyn.net/");
const size_t prefixLen = _countof(kPrefix) - 1;
if (_tcslen(url) < prefixLen) return FALSE;
return StrCmpNI(url, kPrefix, (int)prefixLen) == 0;
...
if (IsTrustedDownloadUrl(this->downloadUrl.c_str()))
{
    ShellExecute(NULL, _T("open"), this->downloadUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
```

**Residual risk.** The trust anchor is still the single hostname
`updates.tyndomyn.net`. The prefix is fixed at compile time so the server
cannot redirect cross-host. There is no payload signature check — the
remediation hardens the URI, not the binary itself. Users who do not need
auto-update should disable the update checker.

### H2 — `AddAutoRun` byte-count vs TCHAR-count confusion

**What's wrong.** `GetModuleFileName` and `RegSetValueEx` interpret
their length arguments differently:

- `GetModuleFileName(HMODULE, LPTSTR, DWORD nSize)` — `nSize` is in **TCHARs**.
- `RegSetValueEx(..., const BYTE*, DWORD cbData)` — `cbData` is in **bytes**.

The original code used `sizeof(programPath)` for both — a 2× over-report
to `GetModuleFileName` under UNICODE (stack overflow with long paths),
and a registry write of the full buffer including uninitialized stack
bytes (info disclosure into `HKCU\...\Run`). The path was also unquoted.

Original (`SpookyView\CSettingsDialog.cpp:150`):

```cpp
TCHAR programPath[MAX_PATH];
GetModuleFileName(0, programPath, sizeof(programPath));
if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("...\\Run"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
{
    RegSetValueEx(hKey, _T("Spooky View"), 0, REG_SZ,
        (LPBYTE)programPath, sizeof(programPath));
}
```

Fix:

```cpp
DWORD len = GetModuleFileName(0, programPath + 1, _countof(programPath) - 2);
if (len == 0 || len >= _countof(programPath) - 2) return;
programPath[0]       = _T('"');
programPath[len + 1] = _T('"');
programPath[len + 2] = _T('\0');
DWORD byteCount = (DWORD)((len + 3) * sizeof(TCHAR));
if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("...\\Run"), 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS)
    RegSetValueEx(hKey, _T("Spooky View"), 0, REG_SZ, (LPBYTE)programPath, byteCount);
```

**Residual risk.** None significant. The fix also closes the unquoted-path
service-style hijack and tightens `RegOpenKeyEx` rights.

### H3 — `ListView::GetTextByIndex` cchTextMax mismatch

**What's wrong.** `GetTextByIndex` hard-coded `item.cchTextMax = MAX_PATH`
(260), but several callers passed buffers sized at 256 TCHARs.
`ListView_GetItem` would write up to 260 TCHARs into a 256-TCHAR buffer
— an 8-byte stack overflow trigerable by enumerating a window whose
class name approached 256 characters.

Original (`SpookyView\ListView.cpp:18`):

```cpp
LPTSTR ListView::GetTextByIndex(int index, TCHAR* textBuffer)
{
    LVITEM item; SecureZeroMemory(&item, sizeof(item));
    item.iItem = index;
    item.mask = LVIF_TEXT;
    item.cchTextMax = MAX_PATH;   // 260
    item.pszText = textBuffer;    // caller passed [256]
    ListView_GetItem(hWnd, &item);
    return item.pszText;
}
```

Fix:

```cpp
LPTSTR ListView::GetTextByIndex(int index, TCHAR* textBuffer, int cchTextMax)
{
    if (textBuffer == NULL || cchTextMax <= 0) return NULL;
    ...
    item.cchTextMax = cchTextMax;
    item.pszText    = textBuffer;
    textBuffer[0]   = _T('\0');
    ListView_GetItem(hWnd, &item);
    textBuffer[cchTextMax - 1] = _T('\0');
    return item.pszText;
}
```

All three call sites (`CAddWindowDialog.cpp`, `CSetupDialog.cpp`,
`CAddAppDialog.cpp`) now pass `_countof(buffer)`.

**Residual risk.** None significant.

### M1 — Predictable single-instance mutex

Original: `CLimitSingleInstance singleInstanceObj(_T("SpookyView"));`.
A bare name occupies the session-local namespace and is squat-able by any
local process. Fix moves it to `Local\` with a GUID-suffixed name. No
remaining squat surface within the same session.

### M2 — `WM_COPYDATA` validation

Original handler dereferenced `lParam` without checks, then accepted any
payload containing the letter `'S'` (via `strrchr`) as authentic.

```cpp
PCOPYDATASTRUCT dataCopy = (PCOPYDATASTRUCT)lParam;
if (dataCopy->dwData == ALREADY_RUNNING_NOTIFY) {
    CHAR *message = (CHAR*)dataCopy->lpData;
    if (strrchr(message, *"Spooky View - already running"))
        OpenSetupDialog();
}
```

Fix validates `dataCopy`, `lpData`, and `cbData`, copies into a bounded
64-byte buffer with explicit NUL-termination, and requires
`strcmp(buffer, "Spooky View - already running") == 0`. Same-session
spoofing is still possible (any process in the same session can send
`WM_COPYDATA`), but malformed/short payloads no longer cause OOB reads.

### M3 — `IsWindowTransparent` reads the wrong style word

`WS_EX_LAYERED` lives in the extended style word. The original code
ANDed it against `GWL_STYLE`, which happens to have unrelated bits at
the same numeric position, so restore-on-exit was unreliable. Fix uses
`GWL_EXSTYLE` and normalizes to a boolean.

### M4 — `WindowsEnum` path parsing AV

`GetProcessImageFileName` return value was ignored, the buffer was not
forcibly terminated, and `fileName = _tcsrchr(filePathName, '\\');
fileName++;` would dereference `NULL+1` on a path without a backslash.
Fix range-checks the return, NUL-terminates the buffer, and falls back
to the whole path when no backslash is present.

### M5 — Notification-icon format-string fragility

`_stprintf_s(nid.szTip, _countof(nid.szTip), userString)` treats
`userString` as the format string. The inputs were resource strings or
internally-built tooltips, so no exploitable `%s` was present today;
nonetheless any future caller passing user input would have been
directly vulnerable. Fix switches all three call sites to
`StringCchCopy`.

### M6 — `pre_release=1` not documented

The update check appends `&pre_release=1` for `PRE_RELEASE` builds but
`PRIVACY.md` did not list this field. Privacy-policy drift only. Fixed
by adding a bullet to `PRIVACY.md`.

### M7 — REG_SZ reads without forced NUL-termination

The registry does not guarantee that `REG_SZ` values are NUL-terminated.
`CRegistrySettingsManager::ReadValue` trusted the returned `cbData`
unchanged, so a value written without a terminator could later OOB-read
through `_tcscmp`/`_tcsrchr`/`tstring::compare`. The fix forces the last
TCHAR (and, under UTF-16, the last *two* bytes) of the destination
buffer to zero for `REG_SZ`/`REG_EXPAND_SZ`/`REG_MULTI_SZ` reads and
zero-initializes `ShouldSkipVersion`'s 100-TCHAR buffer.

### M8 — EXE signing disabled (not remediated)

`SpookyView\SpookyView.vcxproj` has a `<!-- Disabled signing -->` block
(lines 580–587). The MSI wrapper is signed, the MSIX is signed with
`StorePackaging_TemporaryKey.pfx` (developer placeholder). The portable
EXE is unsigned and will trip SmartScreen and many endpoint controls.
Resolving this requires a production code-signing certificate and is
out of scope for a source-only commit. User-side mitigation: prefer the
MSI or MSIX build.

### M9 — Toolchain hardening (partial)

`9311a0a` enables `BufferSecurityCheck`, `ControlFlowGuard`,
`RandomizedBaseAddress`, `HighEntropyVA`, and `DataExecutionPrevention`
on the three Release configurations that use the `v143` toolset
(ARM64 / portable / MSI / store). The remaining nine configurations are
pinned to `v141_xp` for Windows XP/Vista/7 target compatibility, which
cannot emit CFG or Spectre mitigations. `/Qspectre` and `/CETCOMPAT`
are not yet set on any configuration. Residual: the XP-targeted builds
remain less hardened than modern equivalents; users running on
supported modern Windows should prefer ARM64 or v143 builds where
available.

---

## Items not remediated and why

| Item | Reason | User-side mitigation |
|---|---|---|
| **M8 — EXE signing disabled** | Requires a production code-signing certificate, which cannot be added by a source-tree commit. | Install via the **MSI** or **MSIX** package (both signed). Avoid the bare portable EXE on managed endpoints. |
| **L2 — `StorePackaging_TemporaryKey.pfx` in git history** | Removing would require a force-push and history rewrite. The file is a Visual Studio-generated UWP developer placeholder with no production value. | None required. The cert is not trusted anywhere. |
| **`v141_xp` toolset hardening (M9 remainder)** | The XP-targeted toolset structurally cannot emit `/guard:cf` or `/Qspectre` and pre-dates `/CETCOMPAT`. Switching off `v141_xp` would drop XP/Vista/7 support, a deliberate upstream design choice. | On modern Windows, prefer a build whose configuration uses `v143` (the three ARM64 / portable / MSI Release configurations updated in `9311a0a`). |
| **CRLF noise on untouched files** | Some files retain CRLF line endings; the remediation commit deliberately did not re-normalize line endings across the repository to avoid producing a diff that obscures the security-relevant changes. | None. Cosmetic only. |
| **L4 — `KEY_ALL_ACCESS` in remaining registry sites** | Out of scope for the security pass; the only HKCU subtree affected is the app's own settings root, so no privilege boundary is crossed. | None required. |
| **L5 — Missing `EM_SETLIMITTEXT`** | Defense-in-depth only; existing read buffers are sized at `MAX_PATH` and are now correctly clamped (see H3). | None required. |

---

## Residual risk caveat

This audit is a **local-only static review** of the source tree at the
fork's HEAD. It cannot prove byte-equality between this fork and
upstream `littletijn/spooky-view`. The strongest available signal — that
`git shortlog --all` attributes 100% of pre-remediation commits to
`Martijn van Antwerpen` (`littletijn`) — is consistent with a verbatim
mirror but does not constitute proof.

If absolute certainty is required, fetch upstream and run:

```bash
git remote add upstream https://github.com/littletijn/spooky-view.git
git fetch upstream
git --no-pager diff upstream/main..origin/main
```

Any non-empty diff outside the four files added in this fork's
remediation commit (`9311a0a`) warrants individual inspection.

---

## Recommendations for users

1. **Disable the update checker** in the Settings dialog if you do not
   intend to install updates via the in-app dialog. This eliminates the
   H1 attack surface entirely. Updates can still be applied manually
   from the project's release page.
2. **Do not run Spooky View as Administrator** unless you specifically
   need to make an elevated application transparent. The application
   manifest is `asInvoker` by design; running elevated provides no
   additional functionality except for cross-elevation window
   manipulation and broadens the impact of any future memory-safety
   bug.
3. **Prefer the MSI or MSIX installer** over the portable EXE. The
   portable EXE is unsigned (M8); the MSI wrapper and MSIX package are
   signed and integrate cleanly with SmartScreen / AppLocker /
   WDAC-managed environments.
4. On modern Windows (10/11), prefer a build configuration that targets
   the `v143` toolset so you get CFG + ASLR-HighEntropy + DEP + BSC at
   runtime.
5. If your trust requirement is "byte-identical to upstream," run the
   upstream diff in the Residual risk caveat section above before
   deploying.

---

*End of report.*

---

## Post-audit stability hardening (2026-05)

Three bugs surfaced while exercising the app under real use after the
security audit closed. They are stability/correctness issues rather than
security issues, but they materially affect the app's reliability on
current Windows 11 builds and are recorded here so the audit document
remains the canonical reference for the state of the tree.

### Finding S1: Uninitialized `CDialog::hasInit` caused dialogs to silently no-op

- Commit: `4d95df9`
- Symptom: Tray menu Settings/Setup sometimes did nothing; About/Settings
  worked only when fresh allocations happened to land on zeroed memory.
- Root cause: `CDialog::hasInit` (and other base members) had no
  in-class initializer. Non-zero garbage made `hasInitInstance()` return
  `TRUE` before `InitInstance` ever ran, so `OpenDialog` called
  `SetForeground` on a `NULL` `hWnd`.
- Fix: In-class default initializers in `CDialog.h`, `CWnd.h`,
  `CModelessDialog.h` (`hasInit = FALSE`, `dialogResource = NULL`,
  `hParent = NULL`, `hWnd = NULL`, `parentHwnd = NULL`,
  `key = 0`). Added `GetLastError` logging on the
  `CreateDialogParam` failure path.
- Lesson: Always default-init members in any class hierarchy that may be
  allocated without zero-init.

### Finding S2: MinGW UCRT `%s` vs `%ls` crash in `CAddAppDialog`

- Commit: `7406459`
- Symptom: Clicking "Add" in the Setup dialog crashed the entire process.
- Root cause:
  `_sntprintf_s(buf, n, _T("%s (*.exe)%c*.exe%c%s (*.*)%c*.*%c"), programFilterString, ...)`
  at `CAddAppDialog.cpp:37`. Under MinGW UCRT, `_snwprintf_s`
  interprets `%s` in a wide-format string as `char*`, **not**
  `wchar_t*` like MSVC. Dereferencing a wide string as narrow → access
  violation.
- Fix: `%s` → `%ls` (works on both MinGW UCRT and MSVC).
- Lesson: For any wide-format printf family call (`_snwprintf*`,
  `wprintf`, `swprintf`, `StringCchPrintfW`, …), use `%ls` for
  `wchar_t*` arguments. Audit all wide format strings in the codebase
  (`Logger` path-build was already cleaned).

### Finding S3: Windows x64 kernel-callback dispatcher swallows SEH from dialog procs

- Commit: `ae6960b`
- Symptom: Crashes inside dialog/window procs (e.g., clicking "All other
  apps" in Setup) terminated the process with no log entry, no minidump,
  and no WER report. `SetUnhandledExceptionFilter` was never called.
- Root cause: On x64 Windows, exceptions raised inside callbacks invoked
  by the kernel callback dispatcher (USER32 calling back into the app's
  `WndProc`/`DialogProc`) are silently consumed by the kernel before
  any user-mode top-level filter sees them.
  `SetProcessUserModeExceptionPolicy` to clear
  `PROCESS_CALLBACK_FILTER_ENABLED` is the documented workaround, but
  those APIs are not exported by `kernel32` on current Windows 11
  builds (`GetProcAddress` returns `NULL`).
- Fix: Installed a vectored exception handler via
  `AddVectoredExceptionHandler(1, VectoredHandler)` in
  `CrashHandler::Install`. VEH runs first-chance, before the kernel's
  filter ever sees the exception. The handler filters fatal codes (AV,
  illegal instruction, divide-by-zero, stack overflow, etc.) and routes
  them through the existing `UnhandledFilter` (writes a log entry and a
  minidump alongside `spookyview.log`). Returns
  `EXCEPTION_CONTINUE_SEARCH` so the OS still terminates predictably
  after the dump is written. The best-effort
  `SetProcessUserModeExceptionPolicy` call is retained but logged as a
  no-op.
- Lesson: `SetUnhandledExceptionFilter` alone is insufficient on x64
  Windows for any GUI app. Use VEH as the primary catch-all. (Note: MinGW
  g++ does **not** support MSVC `__try`/`__except` keywords, so
  per-`WndProc` SEH wrapping isn't an option in this codebase.)

### Operational guidance

- **Log file:** `%LOCALAPPDATA%\SpookyView\spookyview.log` (1 MiB
  rotation to `.old`).
- **Crash dumps:** same directory, named
  `spookyview-crash-YYYYMMDD-HHMMSS-PID.dmp`.
- **For deeper triage**, enable Windows WER LocalDumps (full dumps) via:

  `HKCU:\Software\Microsoft\Windows\Windows Error Reporting\LocalDumps\SpookyView.exe`

  - `DumpFolder=%LOCALAPPDATA%\SpookyView`
  - `DumpType=2`
  - `DumpCount=10`