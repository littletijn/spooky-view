// Fuzz harness for the UpdateChecker JSON post-parse extraction.
//
// Mirrors SpookyView/UpdateChecker.cpp:322-336 which does:
//     auto j = nlohmann::json::parse(response);
//     updateResponse = j.get<UpdateResponse>();
//     if (updateResponse.update_available) { ... }
//
// UpdateResponse (see UpdateResponse.h) is:
//     struct UpdateResponse {
//         std::string version;
//         std::string download_url;
//         std::string message;
//         BOOL update_available;   // Win32 BOOL = int
//     };
//     NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UpdateResponse, version, download_url,
//                                        message, update_available)
//
// We replicate BOOL as int (the Win32 typedef) so nlohmann's generated
// from_json matches the production binding exactly.

#include <cstddef>
#include <cstdint>
#include <string>

using BOOL = int;

#include <nlohmann/json.hpp>

struct UpdateResponse {
    std::string version;
    std::string download_url;
    std::string message;
    BOOL update_available;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UpdateResponse, version, download_url, message, update_available)

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size == 0 || size > 1 << 16) return 0;
    std::string input(reinterpret_cast<const char*>(data), size);
    try {
        auto j = nlohmann::json::parse(input);
        UpdateResponse r = j.get<UpdateResponse>();
        // Mimic the production read of the bool gate; no further action.
        if (r.update_available) {
            // Touch the string fields the way ShowMessage/SetDownloadUrl will,
            // to make sure construction works and oversized strings are
            // tolerated.
            volatile size_t s = r.version.size() + r.download_url.size() + r.message.size();
            (void)s;
        }
    } catch (const nlohmann::json::exception&) {
        // Production catches nlohmann::json::exception and returns.
        return 0;
    } catch (const std::exception&) {
        // Any other std exception escaping json.hpp would be a real bug.
        // Re-raise so libFuzzer can record it.
        throw;
    }
    return 0;
}
