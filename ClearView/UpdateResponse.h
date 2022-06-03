#pragma once
#include <nlohmann/json.hpp>

struct UpdateResponse {
	std::string version;
	std::string download_url;
	std::string message;
	BOOL update_available;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UpdateResponse, version, download_url, message, update_available)