#ifndef CREGISTRYSETTINGSMANAGER_H
#define CREGISTRYSETTINGSMANAGER_H

#include "ISettingsManager.h"
#include "Unicode.h"
class CRegistrySettingsManager : public ISettingsManager
{
public:
	CRegistrySettingsManager();
	BOOL Init();
	~CRegistrySettingsManager();
	void LoadSettings();
	bool SaveSettings();
	CProgramSetting* AddProgramSettings(TCHAR* programName);
	bool SaveModificationSettings(CModificationSettings* modificationSettings, TCHAR* processFileName, TCHAR* windowClassName, HotkeyType type);
	CSettings* GetSettings();
	void ApplyNewSettings(CSettings *newSettings);
	void AddSkipVersionKey(tstring versionNumber);
	BOOL ShouldSkipVersion(tstring versionNumber);
	BOOL GetDisableUpdateCheck();
	void SetDisableUpdateCheck(BOOL state);
	int GetSkipWelcome();
	void SetSkipWelcome(BOOL state);
	int GetEnableHotkeys();
	void SetEnableHotkeys(BOOL state);
	int GetEnableFullTransparent();
	void SetEnableFullTransparent(BOOL state);
protected:
	std::unique_ptr<CSettings> settings;
	BOOL fullTransparentEnabled;
	void SaveModificationValues(BOOL globalSettings, HKEY key, CModificationSettings values);
	HKEY registryRootKey;
	BOOL ReadKeyByteValue(HKEY key, TCHAR* valueName, BYTE& value);
	void ReadModificationValues(HKEY key, CModificationSettings* settings);
	BOOL ClearProgramSettings();
	BOOL ReadValue(HKEY key, TCHAR* valueName, DWORD expectedKeyType, BYTE* dataBuffer, DWORD dataBufferSize);
	BOOL ReadValue(TCHAR* subkey, TCHAR* valueName, DWORD expectedKeyType, BYTE* dataBuffer, DWORD dataBufferSize);
	BOOL SaveValue(HKEY hKey, TCHAR* valueName, DWORD keyType, BYTE* value, DWORD valueSize = 0);
	BOOL SaveValue(TCHAR* subkey, TCHAR* valueName, DWORD keyType, BYTE* value);
	BOOL SaveStringValue(HKEY hKey, TCHAR* valueName, tstring value);
};

#endif