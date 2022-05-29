#pragma once
#include <nlohmann/json.hpp>

struct UpdateResponse {
	std::string version;
	std::string download_url;
	std::string message;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UpdateResponse, version, download_url, message)