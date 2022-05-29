#pragma once
#include "nlohmann/json.hpp"
#include "Unicode.h"

class AutoUpdater
{
public:
	void CheckForNewerVersion();
	bool GetProductVersion(tstring* version);
protected:
	void DownloadAndParseJson();
	void AsyncCallback(WINHTTP_STATUS_CALLBACK test);
};

