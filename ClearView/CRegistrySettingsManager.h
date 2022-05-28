#include "ISettingsManager.h"
#pragma once

class CRegistrySettingsManager : public ISettingsManager
{
public:
	CRegistrySettingsManager();
	~CRegistrySettingsManager();
	void LoadSettings();
	bool SaveSettings();
	CSettings* GetSettings();
	void ApplyNewSettings(CSettings *newSettings);
protected:
	std::unique_ptr<CSettings> settings;
	void SaveValues(HKEY key, CAlphaSettings values);
	HKEY registryRootKey;
	BOOL ReadKeyValue(HKEY key, TCHAR* valueName, __out BYTE& value);
	void ReadAlphaValues(HKEY key, CAlphaSettings* settings);
	void ToLowerCase(TCHAR* string, size_t length);
};