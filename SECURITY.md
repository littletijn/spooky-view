# Security Policy

## Supported Versions

This repository is a **fork** of [`littletijn/spooky-view`](https://github.com/littletijn/spooky-view).
The actively maintained branch is `main` of [`SchwartzKamel/spooky-view`](https://github.com/SchwartzKamel/spooky-view).

Security fixes will be applied to `main` only. Older tags, release branches, and the
upstream repository are not maintained by this fork. If you are running an upstream
build, please report issues to the upstream maintainers as well.

| Branch / Source                       | Supported by this fork |
| ------------------------------------- | ---------------------- |
| `SchwartzKamel/spooky-view` `main`    | Yes                    |
| Tagged releases of this fork          | Latest only            |
| `littletijn/spooky-view` (upstream)   | No (report upstream)   |

## Reporting a Vulnerability

Please **do not** open a public GitHub issue for suspected vulnerabilities.

Instead, open a **private security advisory** on GitHub:

1. Go to the repository's **Security** tab.
2. Click **Report a vulnerability**.
3. Provide a clear description, reproduction steps, affected version/commit, and
   any proof-of-concept or crash dump you can share.

We will acknowledge reports on a **best-effort** basis and coordinate a fix and
disclosure timeline privately. No formal SLA is offered, but credible reports
will be triaged as quickly as maintainer availability allows.

## Scope

**In scope** (please report):

- Source code in this fork (`SpookyView/`, headers, resource scripts, manifest).
- Build configuration (MSBuild project files, top-level `Makefile`, packaging scripts).
- Installer behavior (MSI / MSIX) produced from this repository.
- Settings persistence and registry behavior under `HKCU`.
- The in-app update-check mechanism (request construction, response parsing,
  URL validation, browser launch).

**Out of scope** (please report elsewhere):

- The `updates.tyndomyn.net` update server itself. That endpoint is operated
  by the original upstream maintainer, not by this fork. Server-side issues
  should be reported upstream.
- Vulnerabilities in third-party libraries vendored or referenced by this
  project (for example, `nlohmann/json`). Please report those to the
  respective upstream projects. We will pick up fixes when they ship.
- General Windows platform issues unrelated to this application's code paths.

## Security Posture Summary

Spooky View runs as a normal desktop application: it performs no code
injection, installs no global hooks, and does not require administrator
privileges by default (the manifest requests `asInvoker`). User settings are
persisted to the per-user registry hive (`HKCU`) only; no `HKLM` writes are
performed during normal operation. The application makes a single outbound
network call by default — an HTTPS update check to `updates.tyndomyn.net` —
which can be disabled in Settings. The update flow validates the download URL
against the known release-host prefix before handing it to the user's default
browser; the app does **not** auto-download or auto-install updates.

## Recent Audit

A third-party security review of this fork was performed in 2026. Findings,
severity ratings, and a remediation table are documented in
[`docs/SECURITY-AUDIT-2026.md`](docs/SECURITY-AUDIT-2026.md). Please consult
that document before filing a report — your finding may already be tracked
there. See also the
[Post-audit stability hardening](docs/SECURITY-AUDIT-2026.md#post-audit-stability-hardening-2026-05)
section for the S1 / S2 / S3 follow-up findings closed after the main audit.

The audit was conducted against a **local snapshot** of this fork without a
line-by-line diff against `littletijn/spooky-view`. As a result, residual risk
relative to upstream — for example, regressions or divergences introduced by
either side — cannot be fully ruled out without a direct comparison against
the upstream tree.

## Hardening Recommendations for Users

- **Disable the update checker** in Settings if you do not want the
  application to make any outbound network calls.
- **Do not run as administrator** unless you specifically need to make
  windows owned by elevated processes transparent. The default `asInvoker`
  manifest is the safer choice for daily use.
- **Prefer the signed MSI or MSIX builds** over the portable EXE. The
  portable EXE is currently unsigned and offers no Authenticode integrity
  guarantee at launch time.