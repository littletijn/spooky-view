#include "stdafx.h"
#include <memory>
#include <algorithm>
#include "CRegistrySettingsManager.h"
#include "Unicode.h"
#include <shlwapi.h>
#include <intsafe.h>

#pragma once


CRegistrySettingsManager::CRegistrySettingsManager()
{
	settings = std::make_unique<CSettings>();
	DWORD result = RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\ClearView"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &registryRootKey, NULL);
	if (result == ERROR_SUCCESS)
	{
	}
}

CRegistrySettingsManager::~CRegistrySettingsManager()
{
	if (registryRootKey != NULL)
	{
		RegCloseKey(registryRootKey);
	}
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
		std::unique_ptr<TCHAR[]> processKeyName(new TCHAR[programsSubKeyLength]);

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
				CProgramSetting *progSettings = new CProgramSetting();

				//Make process name lower case
				settings->ToLowerCase(processKeyName.get());
				settings->programs->insert(std::pair<t_string, CProgramSetting*>(processKeyName.get(), progSettings));

				//Open the Programs\PROCESSNAME\Windows key
				LSTATUS windowsKeyResult = RegOpenKeyEx(processKey, _T("Windows"), 0, KEY_READ, &windowsKey);
				if (windowsKeyResult == ERROR_SUCCESS)
				{
					ReadAlphaValues(windowsKey, &progSettings->alphaSettings);
					DWORD windowsSubKeyLength;
					DWORD windowsSubKeyIndex = 0;
					RegQueryInfoKey(windowsKey, NULL, NULL, NULL, NULL, &windowsSubKeyLength, NULL, NULL, NULL, NULL, NULL, NULL);
					windowsSubKeyLength++;
					std::unique_ptr<TCHAR[]> windowKeyName(new TCHAR[windowsSubKeyLength]);

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
							CWindowSetting *windowSettings = new CWindowSetting();
							ReadAlphaValues(windowKey, &windowSettings->alphaSettings);
							progSettings->windows->insert(std::pair<t_string, CWindowSetting*>(windowKeyName.get(), windowSettings));
							RegCloseKey(windowKey);
						}
						
						windowsSubKeyIndex++;
					} while (windowsEnumResult != ERROR_NO_MORE_ITEMS && windowsEnumResult == ERROR_SUCCESS);
					RegCloseKey(windowsKey);
				}
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
	if (ReadKeyByteValue(key, _T("AlphaForeground"), value))
	{
		settings->foreground = value;
	}
	if (ReadKeyByteValue(key, _T("AlphaBackground"), value))
	{
		settings->background = value;
	}
	if (ReadKeyByteValue(key, _T("Enabled"), value))
	{
		settings->enabled = value != 0;
	}
}

LSTATUS CRegistrySettingsManager::ClearProgramSettings()
{
	HKEY programsKey;
	auto result = RegOpenKeyEx(registryRootKey, _T("Programs"), 0, KEY_READ, &programsKey);
	if (result == ERROR_SUCCESS)
	{
		RegCloseKey(programsKey);
		//Remove current Programs key
		return SHDeleteKey(registryRootKey, _T("Programs"));
	}
	return result;
}

bool CRegistrySettingsManager::SaveSettings()
{
	//Remove programs key
	CRegistrySettingsManager::ClearProgramSettings();
	//Recreate keys
	//Create the Programs key
	HKEY programsKey;
	if (RegCreateKeyEx(registryRootKey, _T("Programs"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &programsKey, NULL) == ERROR_SUCCESS)
	{
		//Save global settings
		this->SaveValues(programsKey, settings->alphaSettings);
		for (auto const &program : *settings->programs)
		{
			//Create key for program
			HKEY programKey;
			if (RegCreateKeyEx(programsKey, program.first.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &programKey, NULL) == ERROR_SUCCESS)
			{
				//Create Windows key of program
				HKEY windowsKey;
				if (RegCreateKeyEx(programKey, _T("Windows"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &windowsKey, NULL) == ERROR_SUCCESS)
				{
					//Save global settings of program
					this->SaveValues(windowsKey, program.second->alphaSettings);

					for (auto const &window : *program.second->windows)
					{
						HKEY windowKey;
						//Create key of window
						if (RegCreateKeyEx(windowsKey, window.first.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &windowKey, NULL) == ERROR_SUCCESS)
						{
							this->SaveValues(windowKey, window.second->alphaSettings);
						}
					}
				}
			}
		}
	}
	return true;
}

void CRegistrySettingsManager::SaveValues(HKEY key, CAlphaSettings values)
{
	RegSetValueEx(key, _T("AlphaForeground"), 0, REG_BINARY, &values.foreground, sizeof(values.foreground));
	RegSetValueEx(key, _T("AlphaBackground"), 0, REG_BINARY, &values.background, sizeof(values.background));
	BYTE enabled = values.enabled ? '\x1' : '\x0';
	RegSetValueEx(key, _T("Enabled"), 0, REG_BINARY, &enabled, sizeof(enabled));
}

BOOL CRegistrySettingsManager::ReadKeyByteValue(HKEY key, TCHAR* valueName, BYTE& value)
{
	//Create buffer and variables for return values
	BOOL readResult = FALSE;
	DWORD keyType;
	BYTE keyData[1];
	DWORD keyDataSize = sizeof(keyData);
	LSTATUS result;

	//Read the data of the value based on given type
	result = RegQueryValueEx(key, valueName, NULL, &keyType, keyData, &keyDataSize);
	if (result != ERROR_SUCCESS || keyType != REG_BINARY)
	{
		readResult = FALSE;
	}
	else
	{
		value = keyData[0];
		readResult = TRUE;
	}
	return readResult;
}

void CRegistrySettingsManager::AddSkipVersionKey(tstring versionNumber)
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\ClearView"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		DWORD versionNumberSize;
		//This will convert the size_t to DWORD. a size_t in x64 is larger than a DWORD. Because our string will not be that long, we only need the size of a DWORD
		if (SIZETToDWord(versionNumber.size() * sizeof(TCHAR), &versionNumberSize) != S_OK)
		{
			//Show error message
		}
		else if (RegSetValueEx(hKey, _T("SkipVersion"), 0, REG_SZ, (LPBYTE)versionNumber.c_str(), versionNumberSize) != ERROR_SUCCESS)
		{
			//Show error message
		}
		RegCloseKey(hKey);
	}
}

BOOL CRegistrySettingsManager::ShouldSkipVersion(tstring versionNumber)
{
	DWORD keyType;
	TCHAR keyData[100];
	DWORD keyDataSize = sizeof(keyData);
	auto result = SHGetValue(HKEY_CURRENT_USER, _T("Software\\ClearView"), _T("SkipVersion"), &keyType, keyData, &keyDataSize);
	if (result == ERROR_SUCCESS && keyType == REG_SZ)
	{
		return versionNumber.compare(keyData) == 0;
	}
	return FALSE;
}

BOOL CRegistrySettingsManager::GetDisableUpdateCheck()
{
	DWORD keyType;
	BYTE keyData[1];
	DWORD keyDataSize = sizeof(keyData);
	auto result = SHGetValue(HKEY_CURRENT_USER, _T("Software\\ClearView"), _T("DisableUpdateCheck"), &keyType, keyData, &keyDataSize);
	if (result == ERROR_SUCCESS && keyType == REG_BINARY)
	{
		return keyData[0];
	}
	return FALSE;
}

void CRegistrySettingsManager::SetDisableUpdateCheck(BOOL state)
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\ClearView"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		BYTE stateByte = state ? '\x1' : '\x0';
		if (RegSetValueEx(hKey, _T("DisableUpdateCheck"), 0, REG_BINARY, &stateByte, sizeof(stateByte)) != ERROR_SUCCESS)
		{
			//Show error message
		}
		RegCloseKey(hKey);
	}
}