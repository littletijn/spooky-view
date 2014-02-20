#include "stdafx.h"
#include "CSettings.h"


CSettings::CSettings()
{
	DWORD result = RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\ClearView\\Programs"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &registryRootKey, NULL);
	if (result == ERROR_SUCCESS)
	{
	}
}

CSettings::~CSettings()
{
	if (registryRootKey != NULL)
	{
		RegCloseKey(registryRootKey);
	}
}

BOOL CSettings::ReadKeyValue(HKEY key, TCHAR* valueName, __out BYTE& value)
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

HKEY CSettings::LoadSettingsKey(TCHAR* processFileName, TCHAR* windowClassName)
{
	//Create variables for return values
	HKEY currentSetting;
	LSTATUS result;

	//http://msdn.microsoft.com/en-us/library/windows/desktop/ms724872(v=vs.85).aspx
	// The size of a path in the registry is max 255 characters
	TCHAR defaultWindowKeyPath[255];
	TCHAR classWindowKeyPath[255];

	//Copy Process name to buffer and concat "\Windows" to name
	_tcscpy_s(defaultWindowKeyPath, processFileName);
	_tcscat_s(defaultWindowKeyPath, _T("\\Windows"));
	//Copy default window path and concat Class name to it
	_tcscpy_s(classWindowKeyPath, defaultWindowKeyPath);
	_tcscat_s(classWindowKeyPath, _T("\\"));
	_tcscat_s(classWindowKeyPath, windowClassName);


	//Open key PROCESSNAME\Windows\CLASSNAME
	result = RegOpenKeyEx(registryRootKey, classWindowKeyPath, 0, KEY_READ, &currentSetting);
	if (result != ERROR_SUCCESS)
	{
		//Open default window key instead
		RegCloseKey(currentSetting);
		result = RegOpenKeyEx(registryRootKey, defaultWindowKeyPath, 0, KEY_READ, &currentSetting);
		if (result != ERROR_SUCCESS)
		{
			//Read global settings
			currentSetting = registryRootKey;
		}
	}
	return currentSetting;
}

BOOL CSettings::GetAlphaSetting(TCHAR* processFileName, TCHAR* windowClassName, WindowTypes type, __out BYTE& alpha)
{
	//Create buffer and variables for return values
	TCHAR* valueName = NULL;
	BOOL result = FALSE;
	BYTE alphaKeyValue;

	HKEY currentSetting = CSettings::LoadSettingsKey(processFileName, windowClassName);

	//Get valueName
	switch (type)
	{
	case Foregound:
		valueName = _T("AlphaForeground");
		break;
	case Background:
		valueName = _T("AlphaBackground");
		break;
	}

	if (ReadKeyValue(currentSetting, valueName, alphaKeyValue))
	{
		alpha = alphaKeyValue;
		result = TRUE;
	}

	//Close the key if it is not the root key
	if (currentSetting != registryRootKey)
	{
		RegCloseKey(currentSetting);
	}
	return result;
}

BOOL CSettings::GetEnableGlassBlurrSetting(TCHAR* processFileName, TCHAR* windowClassName)
{
	BYTE result;

	HKEY currentSetting = CSettings::LoadSettingsKey(processFileName, windowClassName);
	if (ReadKeyValue(currentSetting, _T("EnableGlass"), result))
	{
		return (result == 1);
	}
	return FALSE;
}