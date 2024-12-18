#include "stdafx.h"
#include <memory>
#include <algorithm>
#include "CRegistrySettingsManager.h"
#include "Unicode.h"
#include <shlwapi.h>
#include <intsafe.h>
#include "Defines.h"


CRegistrySettingsManager::CRegistrySettingsManager()
{
	settings = std::make_unique<CSettings>();
}

CRegistrySettingsManager::~CRegistrySettingsManager()
{
	if (registryRootKey != NULL)
	{
		RegCloseKey(registryRootKey);
	}
}

BOOL CRegistrySettingsManager::Init()
{
	DWORD result = RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Spooky View"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &registryRootKey, NULL);
	return result == ERROR_SUCCESS;
}

CSettings* CRegistrySettingsManager::GetSettings()
{
	return settings.get();
}

void CRegistrySettingsManager::ApplyNewSettings(CSettings* newSettings)
{
	if (settings) {
		settings.release();
	}
	//Copy the newSettings to our settings
	settings = std::make_unique<CSettings>(*newSettings);
}

void CRegistrySettingsManager::LoadSettings()
{
	//Create variables for return values
	HKEY programsKey;

	LSTATUS result = RegOpenKeyEx(registryRootKey, _T("Programs"), 0, KEY_READ, &programsKey);
	if (result == ERROR_SUCCESS)
	{
		//Read global settings
		ReadAlphaValues(programsKey, &settings->alphaSettings);

		DWORD programsSubKeyLength;
		DWORD programsSubKeyIndex = 0;

		RegQueryInfoKey(programsKey, NULL, NULL, NULL, NULL, &programsSubKeyLength, NULL, NULL, NULL, NULL, NULL, NULL);
		//Create a TCHAR buffer with the maximum allowed key name length as size, make it as a heap stored array and put it in a smart pointer on the stack
		//so it will be delete-d when the smart pointer is going out of scope
		programsSubKeyLength++;
		auto processKeyName = std::make_unique<TCHAR[]>(programsSubKeyLength);
		auto fileName = std::make_unique<TCHAR[]>(MAX_PATH);

		//Enum the process sub keys in the Programs key
		LSTATUS programsEnumResult;
		do
		{
			DWORD programKeyLength = programsSubKeyLength;
			programsEnumResult = RegEnumKeyEx(programsKey, programsSubKeyIndex, processKeyName.get(), &programKeyLength, NULL, NULL, NULL, NULL);
			if (programsEnumResult != ERROR_SUCCESS)
			{
				break;
			}
			HKEY processKey;
			//Open the Programs\PROCESSNAME key
			LSTATUS processKeyResult = RegOpenKeyEx(programsKey, processKeyName.get(), 0, KEY_READ, &processKey);
			if (processKeyResult == ERROR_SUCCESS)
			{
				HKEY windowsKey;
				auto progSettings = std::make_unique<CProgramSetting>();

				//Open the Programs\PROCESSNAME\Windows key
				LSTATUS windowsKeyResult = RegOpenKeyEx(processKey, _T("Windows"), 0, KEY_READ, &windowsKey);
				if (windowsKeyResult == ERROR_SUCCESS)
				{
					ReadValue(processKey, _T("File name"), REG_SZ, (BYTE*)fileName.get(), MAX_PATH * sizeof(TCHAR));
					ReadAlphaValues(windowsKey, &progSettings->alphaSettings);
					DWORD windowsSubKeyLength;
					DWORD windowsSubKeyIndex = 0;
					RegQueryInfoKey(windowsKey, NULL, NULL, NULL, NULL, &windowsSubKeyLength, NULL, NULL, NULL, NULL, NULL, NULL);
					windowsSubKeyLength++;
					auto windowKeyName = std::make_unique<TCHAR[]>(windowsSubKeyLength);
					auto windowClassName = std::make_unique<TCHAR[]>(MAX_WINDOW_CLASS_NAME);

					LSTATUS windowsEnumResult;
					//Enum the window sub keys in the Windows key
					do
					{
						DWORD windowKeyLength = windowsSubKeyLength;
						windowsEnumResult = RegEnumKeyEx(windowsKey, windowsSubKeyIndex, windowKeyName.get(), &windowKeyLength, NULL, NULL, NULL, NULL);
						if (windowsEnumResult != ERROR_SUCCESS)
						{
							break;
						}
						HKEY windowKey;

						//Open the Programs\PROCESSNAME\Windows\WINDOWCLASSNAME key
						LSTATUS windowKeyResult = RegOpenKeyEx(windowsKey, windowKeyName.get(), 0, KEY_READ, &windowKey);
						if (windowKeyResult == ERROR_SUCCESS)
						{
							auto windowSettings = std::make_unique<CWindowSetting>();
							ReadValue(windowKey, _T("Window class name"), REG_SZ, (BYTE*)windowClassName.get(), MAX_WINDOW_CLASS_NAME * sizeof(TCHAR));
							ReadAlphaValues(windowKey, &windowSettings->alphaSettings);
							progSettings->windows->insert(std::pair<t_string, std::unique_ptr<CWindowSetting>>(windowClassName.get(), std::move(windowSettings)));
							RegCloseKey(windowKey);
						}
						
						windowsSubKeyIndex++;
					} while (windowsEnumResult != ERROR_NO_MORE_ITEMS && windowsEnumResult == ERROR_SUCCESS);
					RegCloseKey(windowsKey);
				}
				//Make process name lower case
				auto lowerCaseProcessKeyName = settings->ToLowerCase(fileName.get());
				settings->programs->insert(std::pair<t_string, std::unique_ptr<CProgramSetting>>(*lowerCaseProcessKeyName, std::move(progSettings)));
				RegCloseKey(processKey);
			}
			programsSubKeyIndex++;
		} while (programsEnumResult != ERROR_NO_MORE_ITEMS && programsEnumResult == ERROR_SUCCESS);
		RegCloseKey(programsKey);
	}
}

void CRegistrySettingsManager::ReadAlphaValues(HKEY key, CAlphaSettings* settings)
{
	BYTE value;
	if (ReadKeyByteValue(key, _T("Alpha foreground"), value))
	{
		settings->foreground = value;
	}
	if (ReadKeyByteValue(key, _T("Alpha background"), value))
	{
		settings->background = value;
	}
	if (ReadKeyByteValue(key, _T("Enabled"), value))
	{
		settings->enabled = value != 0;
	}
	if (ReadKeyByteValue(key, _T("Separate background value"), value))
	{
		settings->separateBackgroundValue = value != 0;
	}
}

BOOL CRegistrySettingsManager::ClearProgramSettings()
{
	HKEY programsKey;
	if (RegOpenKeyEx(registryRootKey, _T("Programs"), 0, KEY_READ, &programsKey) == ERROR_SUCCESS)
	{
		RegCloseKey(programsKey);
		//Remove current Programs key
		return SHDeleteKey(registryRootKey, _T("Programs")) == ERROR_SUCCESS;
	}
	return TRUE;
}

bool CRegistrySettingsManager::SaveSettings()
{
	//Remove programs key
	if (!CRegistrySettingsManager::ClearProgramSettings())
	{
		return false;
	}
	//Recreate keys
	//Create the Programs key
	HKEY programsKey;
	if (RegCreateKeyEx(registryRootKey, _T("Programs"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &programsKey, NULL) == ERROR_SUCCESS)
	{
		//Save global settings
		this->SaveAlphaSettingsValues(programsKey, settings->alphaSettings);
		for (auto const &program : *settings->programs)
		{
			//Create key for program
			HKEY programKey;
			if (RegCreateKeyEx(programsKey, program.first.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &programKey, NULL) == ERROR_SUCCESS)
			{
				SaveStringValue(programKey, _T("File name"), program.first);
				//Create Windows key of program
				HKEY windowsKey;
				if (RegCreateKeyEx(programKey, _T("Windows"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &windowsKey, NULL) == ERROR_SUCCESS)
				{
					//Save global settings of program
					this->SaveAlphaSettingsValues(windowsKey, program.second->alphaSettings);

					for (auto const &window : *program.second->windows)
					{
						HKEY windowKey;
						//Create key of window
						if (RegCreateKeyEx(windowsKey, window.first.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &windowKey, NULL) == ERROR_SUCCESS)
						{
							SaveStringValue(windowKey, _T("Window class name"),window.first);
							this->SaveAlphaSettingsValues(windowKey, window.second->alphaSettings);
						}
					}
				}
			}
		}
		return true;
	}
	return false;
}

void CRegistrySettingsManager::SaveAlphaSettingsValues(HKEY key, CAlphaSettings values)
{
	BYTE enabled = values.enabled ? '\x1' : '\x0';
	BYTE separateBackgroundValue = values.separateBackgroundValue ? '\x1' : '\x0';
	SaveValue(key, _T("Enabled"), REG_BINARY, &enabled);
	SaveValue(key, _T("Alpha foreground"), REG_BINARY, &values.foreground);
	SaveValue(key, _T("Alpha background"), REG_BINARY, &values.background);
	SaveValue(key, _T("Separate background value"), REG_BINARY, &separateBackgroundValue);
}

BOOL CRegistrySettingsManager::ReadKeyByteValue(HKEY key, TCHAR* valueName, BYTE& value)
{
	DWORD keyType;
	DWORD dataBufferSize = sizeof(value);
	auto result = RegQueryValueEx(key, valueName, NULL, &keyType, &value, &dataBufferSize);
	return result == ERROR_SUCCESS && keyType == REG_BINARY;
}

BOOL CRegistrySettingsManager::ReadValue(HKEY key, TCHAR *valueName, DWORD expectedKeyType, BYTE *dataBuffer, DWORD dataBufferSize)
{
	DWORD keyType;
	auto result = RegQueryValueEx(key, valueName, NULL, &keyType, dataBuffer, &dataBufferSize);
	return result == ERROR_SUCCESS && keyType == expectedKeyType;
}

BOOL CRegistrySettingsManager::ReadValue(TCHAR *subkey, TCHAR *valueName, DWORD expectedKeyType, BYTE *dataBuffer, DWORD dataBufferSize)
{
	DWORD keyType;
	auto result = SHGetValue(HKEY_CURRENT_USER, subkey, valueName, &keyType, dataBuffer, &dataBufferSize);
	return result == ERROR_SUCCESS && keyType == expectedKeyType;
}

BOOL CRegistrySettingsManager::SaveValue(HKEY hKey, TCHAR* valueName, DWORD keyType, BYTE* value, DWORD valueSize)
{
	if (valueSize == 0) {
		valueSize = sizeof(*value);
	}
	return RegSetValueEx(hKey, valueName, 0, keyType, value, valueSize) == ERROR_SUCCESS;
}

BOOL CRegistrySettingsManager::SaveValue(TCHAR* subkey, TCHAR* valueName, DWORD keyType, BYTE* value)
{
	BOOL result = false;
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, subkey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		BOOL result = SaveValue(hKey, valueName, keyType, value);
		RegCloseKey(hKey);
	}
	return result;
}

BOOL CRegistrySettingsManager::SaveStringValue(HKEY hKey, TCHAR* valueName, tstring value)
{
	DWORD valueSize;
	//This will convert the size_t to DWORD. a size_t in x64 is larger than a DWORD. Because our string will not be that long, we only need the size of a DWORD
	if (SSIZETToDWord(value.size() * sizeof(TCHAR), &valueSize) != S_OK)
	{
		return FALSE;
	}
	return RegSetValueEx(hKey, valueName, 0, REG_SZ, (LPBYTE)value.c_str(), valueSize) == ERROR_SUCCESS;
}

BOOL CRegistrySettingsManager::ShouldSkipVersion(tstring versionNumber)
{
	TCHAR keyData[100];
	if (ReadValue(_T("Software\\Spooky View"), _T("Skip version"), REG_SZ, (BYTE*)keyData, sizeof(keyData)))
	{
		return versionNumber.compare(keyData) == 0;
	}
	return FALSE;
}

void CRegistrySettingsManager::AddSkipVersionKey(tstring versionNumber)
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Spooky View"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		SaveStringValue(hKey, _T("Skip version"), versionNumber);
		RegCloseKey(hKey);
	}
}

BOOL CRegistrySettingsManager::GetDisableUpdateCheck()
{
	BYTE keyData[1];
	if (ReadValue(_T("Software\\Spooky View"), _T("Disable update check"), REG_BINARY, keyData, sizeof(keyData)))
	{
		return keyData[0];
	}
	return FALSE;
}

void CRegistrySettingsManager::SetDisableUpdateCheck(BOOL state)
{
	BYTE stateByte = state ? '\x1' : '\x0';
	SaveValue(_T("Software\\Spooky View"), _T("Disable update check"), REG_BINARY, &stateByte);
}

int CRegistrySettingsManager::GetSkipWelcome()
{
	BYTE keyData[1];
	if (ReadValue(_T("Software\\Spooky View"), _T("Skip welcome"), REG_BINARY, keyData, sizeof(keyData)))
	{
		return keyData[0];
	}
	return -1;
}

void CRegistrySettingsManager::SetSkipWelcome(BOOL state)
{
	BYTE stateByte = state ? '\x1' : '\x0';
	SaveValue(_T("Software\\Spooky View"), _T("Skip welcome"), REG_BINARY, &stateByte);
}
