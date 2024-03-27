#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include "nlohmann/json.hpp"
#include "Unicode.h"

void CreateUpdateCheckerThread();
DWORD WINAPI UpdateCheckerThread(LPVOID lpParam);

class UpdateChecker
{
public:
	void CheckForNewerVersion();
	bool GetProductVersion(tstring* version);
	void GetUserLanguage(tstring* version);
protected:
	void DownloadAndParseJson();
};

#endif