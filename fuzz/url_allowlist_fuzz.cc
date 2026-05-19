// Fuzz harness for IsTrustedDownloadUrl in SpookyView/CUpdateAvailableDialog.cpp.
//
// Mirrors the production logic line-by-line. The production function uses:
//   * NULL check
//   * static prefix L"https://updates.tyndomyn.net/"
//   * _tcslen(url) < prefixLen -> reject
//   * StrCmpNI(url, kPrefix, prefixLen) == 0   (case-insensitive Windows ASCII)
//
// StrCmpNI is documented as a case-insensitive compare using the current
// locale (Windows behavior). The prefix is pure ASCII, so we replicate it
// with a portable ASCII tolower compare to keep the harness self-contained.

#include <cstddef>
#include <cstdint>
#include <cwchar>
#include <string>

namespace {

inline wchar_t ascii_tolower(wchar_t c) {
    return (c >= L'A' && c <= L'Z') ? static_cast<wchar_t>(c + (L'a' - L'A')) : c;
}

int StrCmpNI_replica(const wchar_t* a, const wchar_t* b, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        wchar_t ca = ascii_tolower(a[i]);
        wchar_t cb = ascii_tolower(b[i]);
        if (ca != cb) return (ca < cb) ? -1 : 1;
        if (ca == L'\0') return 0;
    }
    return 0;
}

bool IsTrustedDownloadUrl(const wchar_t* url) {
    if (url == nullptr) return false;
    static const wchar_t kPrefix[] = L"https://updates.tyndomyn.net/";
    const size_t prefixLen = (sizeof(kPrefix) / sizeof(kPrefix[0])) - 1;
    if (wcslen(url) < prefixLen) return false;
    return StrCmpNI_replica(url, kPrefix, prefixLen) == 0;
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 2 || size > 4096) return 0;
    size_t wlen = size / sizeof(wchar_t);
    std::wstring s(reinterpret_cast<const wchar_t*>(data), wlen);
    // Strip any embedded NULs so wcslen sees the full buffer; then re-terminate.
    for (auto& c : s) if (c == L'\0') c = L' ';
    s.push_back(L'\0');
    (void)IsTrustedDownloadUrl(s.c_str());
    return 0;
}
