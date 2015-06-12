#include "ISettingsManager.h"
#pragma once

class CRegistrySettingsManager : public ISettingsManager
{
public:
	CRegistrySettingsManager();
	~CRegistrySettingsManager();
	CSettings* LoadSettings();
	bool SaveSettings(CSettings* settings);
protected:
	void SaveValues(HKEY key, CAlphaSettings values);
	HKEY registryRootKey;
	BOOL ReadKeyValue(HKEY key, TCHAR* valueName, __out BYTE& value);
	void ReadAlphaValues(HKEY key, CAlphaSettings* settings);
	void ToLowerCase(TCHAR* string, size_t length);
};