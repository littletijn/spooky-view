#pragma once
#include "nlohmann/json.hpp"
#include "Unicode.h"

void CreateUpdateCheckerThread();
DWORD WINAPI UpdateCheckerThread(LPVOID lpParam);

class UpdateChecker
{
public:
	void CheckForNewerVersion();
	bool GetProductVersion(tstring* version);
protected:
	void DownloadAndParseJson();
};

