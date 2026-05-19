// Fuzz harness for the WM_COPYDATA payload validator in CMainWindow.cpp.
//
// Replicates the bounded-copy + strcmp sequence from CMainWindow.cpp:142-150:
//     CHAR buffer[64];
//     DWORD copyLen = cbData < sizeof(buffer) ? cbData : sizeof(buffer) - 1;
//     memcpy(buffer, lpData, copyLen);
//     buffer[copyLen] = '\0';
//     return strcmp(buffer, "Spooky View - already running") == 0;
//
// The entry gate (dataCopy != NULL && dwData == ALREADY_RUNNING_NOTIFY &&
// lpData != NULL && cbData >= sizeof(literal)) is checked here too because
// only cbData >= 30 reaches the strcmp path.

#include <cstddef>
#include <cstdint>
#include <cstring>

static const char kExpected[] = "Spooky View - already running";

bool ValidateCopyData(const uint8_t* data, size_t len) {
    if (data == nullptr) return false;
    if (len < sizeof(kExpected)) return false; // gate from CMainWindow.cpp:138

    char buffer[64];
    size_t copyLen = (len < sizeof(buffer)) ? len : (sizeof(buffer) - 1);
    std::memcpy(buffer, data, copyLen);
    buffer[copyLen] = '\0';
    return std::strcmp(buffer, kExpected) == 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size > 1 << 20) return 0;
    (void)ValidateCopyData(data, size);
    return 0;
}
