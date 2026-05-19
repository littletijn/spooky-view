# Spooky View — Copilot Instructions

Win32 C++ desktop application that makes any window transparent via
`SetLayeredWindowAttributes` / `WS_EX_LAYERED`. Targets Windows XP and newer.
This is the `SchwartzKamel` fork of `littletijn/spooky-view`.

## Build

Two supported toolchains. Pick one — they share sources but not output paths.

**Visual Studio 2022 (primary, all configs)**
- Workload: *Desktop Development with C++*
- Component: *C++ Windows XP Support for VS 2017 (v141) tools* (required — the
  `Release` configurations use the `v141_xp` platform toolset; only the ARM64
  configs use `v143`).
- Open `SpookyView.sln`, build the `SpookyView` project.
- Solution platforms: `Win32`, `x64`, `ARM64`. Solution configurations include
  `Debug`, `Release`, `Release MSI`, `Release Store` (the last two feed
  `WiXSetup/` and `StorePackaging/`).

**MinGW-w64 (Debug only)**
From the repo root: `make restore && make`. The top-level `Makefile`
delegates to `SpookyView/makefile`, which now compiles the `.rc` files via
`windres` as part of the dependency graph (no more manual `windres` step).
Other targets: `make clean`, `make rebuild`, `make check-tools`,
`make help`. See `BUILDING.md` for choco / scoop instructions to install
the toolchain. The VS Code task **"Build Debug version with MinGW-w64"**
still works.

## Test / lint

There are no unit tests, no lint config, and no CI. "Testing" is manual: run
the built `SpookyView.exe`, right-click the tray icon, exercise the
Settings/Setup/Add-App/Add-Window dialogs against a few target windows.

## Architecture

Single-instance Win32 app, no DLL injection. Entry point `AppMain.cpp`
(`_tWinMain`):
1. `CLimitSingleInstance` (mutex in the `Local\` namespace) — if another
   instance exists, the new process sends a `WM_COPYDATA` ping (literal
   `"Spooky View - already running"`) to the existing main window and exits.
2. Initializes globals declared in `SpookyView.h` — `settingsManager`,
   `mainWindow`, `windowsEnum`, dialog singletons. All owned by
   `std::unique_ptr` and referenced from anywhere via `extern`.
3. Pumps a standard message loop; modeless dialogs are dispatched through
   `hWndDialogCurrent` and respond to `WM_MODELESS_DIALOG_DESTROYED`.

Core components:
- **`CMainWindow`** — owns the message-only window that hosts the tray icon
  (`CNotifyIcon`), receives `WM_COPYDATA`, hotkey events, and update
  notifications (`WM_UPDATE_AVAILABLE`, see `Defines.h`).
- **`WindowsEnum`** — `EnumWindows` walker that drives the transparency
  effect. `IsWindowTransparent` checks `WS_EX_LAYERED` via `GWL_EXSTYLE`;
  process-name matching uses `GetProcessImageFileName` with bounded parsing.
- **`ISettingsManager`** — abstract settings interface. The concrete
  `CRegistrySettingsManager` persists to `HKCU` (Run key for autorun,
  per-program settings, "skip version" list). REG_SZ reads always force
  NUL-termination; the `ReadValue` overloads in
  `CRegistrySettingsManager.cpp` are the canonical pattern — copy it for new
  registry reads.
- **`UpdateChecker`** — WinHTTP client that issues an HTTPS GET to
  `updates.tyndomyn.net` with query-string parameters (version, locale,
  arch, packaging, pre_release — see `PRIVACY.md`), parses the JSON
  response via `nlohmann::json` into `UpdateResponse`, and posts
  `WM_UPDATE_AVAILABLE` to `mainHwnd`. **The "Download" button in
  `CUpdateAvailableDialog` is gated by `IsTrustedDownloadUrl`** — extend
  that allowlist if the update host ever changes; never `ShellExecute` an
  unverified URL.
- **Dialogs** (`CSettingsDialog`, `CSetupDialog`, `CAddAppDialog`,
  `CAddWindowDialog`, `CIntroDialog`, `CAbout`, `CUpdateAvailableDialog`) —
  all inherit from `CDialog`. Owner-draw list interactions go through the
  `ListView` wrapper.

## Conventions

- **TCHAR everywhere, never raw `char` or `wchar_t`.** The codebase compiles
  both ANSI and Unicode (`UNICODE`/`_UNICODE`). Use `TCHAR`, `_T("…")`,
  `tstring` (from `Unicode.h` — wraps `std::wstring`/`std::string`), and
  `to_tstring`. `MultiPlatformString.h` defines an older `t_string` alias;
  prefer `tstring` for new code.
- **Pass character counts, not byte counts, to string APIs.** When a buffer
  is fixed-size, pass `_countof(buf)` (TCHAR count) to bounded string
  helpers; multiply by `sizeof(TCHAR)` only when the API wants bytes
  (`RegSetValueEx`, `WM_COPYDATA cbData`). The `ListView::GetTextByIndex(int,
  TCHAR*, int cchTextMax)` signature is the canonical pattern for new
  helpers — always take an explicit char-count parameter.
- **Use StrSafe, not `_stprintf_s` / `_tcscpy`.** `CNotifyIcon.cpp` uses
  `StringCchCopy` / `StringCchPrintf` from `<strsafe.h>`; do the same.
- **Registry strings must be NUL-terminated by the reader.** `RegQueryValueEx`
  does not guarantee termination for `REG_SZ`/`REG_EXPAND_SZ`/`REG_MULTI_SZ`.
  Allocate `cbData + sizeof(TCHAR)`, zero it, then read.
- **Single-instance mutex must be `Local\`-namespaced** (see `AppMain.cpp`)
  to avoid cross-session squatting.
- **No DLL injection.** Transparency works in-process on the target HWND
  via `SetLayeredWindowAttributes`. Don't introduce `SetWindowsHookEx`
  global hooks or `CreateRemoteThread` patterns.
- **Hardening flags live in `SpookyView.vcxproj`**. The v143 ARM64 Release
  configs carry `ControlFlowGuard`, `BufferSecurityCheck`,
  `RandomizedBaseAddress`, `HighEntropyVA`, `DataExecutionPrevention`.
  The `v141_xp` configs cannot emit CFG / `/Qspectre` / `/CETCOMPAT` — do
  not try to add them there.
- **Logging.** `Logger.h` / `Logger.cpp` is the canonical logger. Call
  `Logger::Instance().Init()` at startup (already wired in
  `AppMain::Run`), then use `LOG_INFO` / `LOG_WARN` / `LOG_ERROR`. Logs
  land at `%LOCALAPPDATA%\SpookyView\spookyview.log` (with naive 1 MiB
  rotation → `.old`). Path building inside `Logger::Init` uses only
  kernel32 string APIs (`lstrcpynW`/`lstrcatW`) — **do not switch to
  `_snwprintf_s` with `%s` for wchar_t args**; MinGW's UCRT interprets
  `%s` as `char*` even in wide-format strings and the call silently
  truncates. Use `%ls` for wchar_t* in wide formats if you need
  `_snwprintf`.
- **Crash handler.** `CrashHandler::Install()` (called immediately after
  `Logger::Init`) registers **both** a Vectored Exception Handler via
  `AddVectoredExceptionHandler` **and** a last-chance
  `SetUnhandledExceptionFilter`, and also calls
  `SetProcessUserModeExceptionPolicy` to clear
  `PROCESS_CALLBACK_FILTER_ENABLED`. The VEH is the primary catch-all on
  x64, where the kernel's user-mode callback dispatcher will otherwise
  swallow exceptions raised inside dialog procs before
  `SetUnhandledExceptionFilter` ever sees them — see audit finding S3 in
  `docs/SECURITY-AUDIT-2026.md`. On a fatal exception the handler writes
  a minidump (`spookyview-crash-YYYYMMDD-HHMMSS-PID.dmp`) next to the
  log. Requires linking `dbghelp` (MSVC: `#pragma comment(lib,
  "dbghelp.lib")` is already there; MinGW: the makefile passes
  `-ldbghelp`).
- **MinGW link flags.** `SpookyView/makefile` static-links the C/C++
  runtime (`-static -static-libgcc -static-libstdc++`) so the produced
  exe has no `libgcc_s_seh-1.dll` / `libstdc++-6.dll` dependency.
  Removing `-static*` flags will reintroduce "missing DLL" errors on
  clean machines. **Do not add `-nostdlib`** — it was in the original
  makefile and broke secure-CRT functions; static linking achieves the
  same goal cleanly.
- **Line endings:** Most source files are CRLF (Windows defaults). New docs
  under `docs/` and root-level Markdown (`SECURITY.md`, `PRIVACY.md`) are
  LF. There is no `.gitattributes`; **stage explicitly by path** rather
  than `git add -A` to avoid sweeping in unrelated CRLF churn.
- **No history rewrites.** A historical `.pfx` exists in past commits;
  the policy is to leave it and rely on cert revocation, not force-push.

## Security review artifacts

- `SECURITY.md` — disclosure policy, scope, posture.
- `docs/SECURITY-AUDIT-2026.md` — full findings + remediation table from the
  internal review; cross-references commit `9311a0a` for every fix. Read
  this before touching `UpdateChecker`, `CRegistrySettingsManager`,
  `CUpdateAvailableDialog`, `WindowsEnum`, or `AppMain.cpp` — there are
  fix patterns there you should preserve.
- `PRIVACY.md` — exact list of fields the update check transmits. If you
  change the update request payload, update this file in the same commit.

## Release / packaging

- `WiXSetup/Product.wxs` produces the MSI (`Release MSI` configuration).
- `StorePackaging/Package.appxmanifest` produces the MSIX for the Microsoft
  Store (`Release Store` configuration).
- EXE signing is currently disabled in `SpookyView.vcxproj`
  (`<SignMode>Off</SignMode>`); enabling it requires a code-signing cert
  and is intentionally not wired up.
