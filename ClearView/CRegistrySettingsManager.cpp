#include "stdafx.h"
#include <memory>
#include <algorithm>
#include "CRegistrySettingsManager.h"

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
				ToLowerCase(processKeyName.get(), programsSubKeyLength);
				(*settings->programs)[processKeyName.get()] = progSettings;

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
							(*progSettings->windows)[windowKeyName.get()] = windowSettings;
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
	BYTE alphaValue;
	if (ReadKeyValue(key, _T("AlphaForeground"), alphaValue))
	{
		settings->foreground = alphaValue;
	}
	if (ReadKeyValue(key, _T("AlphaBackground"), alphaValue))
	{
		settings->background = alphaValue;
	}
}

bool CRegistrySettingsManager::SaveSettings()
{
	if (RegDeleteTree(registryRootKey, _T("Programs")) == ERROR_SUCCESS)
	{
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
	}
	return true;
}

void CRegistrySettingsManager::SaveValues(HKEY key, CAlphaSettings values)
{
	RegSetValueEx(key, _T("AlphaForeground"), 0, REG_BINARY, &values.foreground, sizeof(values.foreground));
	RegSetValueEx(key, _T("AlphaBackground"), 0, REG_BINARY, &values.background, sizeof(values.background));
}

BOOL CRegistrySettingsManager::ReadKeyValue(HKEY key, TCHAR* valueName, __out BYTE& value)
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

void CRegistrySettingsManager::ToLowerCase(TCHAR* string, size_t length)
{
	//Make process name lower case
	for (size_t i = 0; i < _tcsnlen(string, length); i++){
		string[i] = tolower(string[i]);
	}
}