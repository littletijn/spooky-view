#include <string>
#include <map>
#include "CProgramSetting.h"
#include <memory>
#pragma once


class CSettings
{
public:
	enum WindowTypes
	{
		Foregound,
		Background
	};

	CAlphaSettings alphaSettings;
	CAlphaSettings* CSettings::GetAlphaSetting(TCHAR* processFileName, TCHAR* windowClassName);
	std::unique_ptr<TCHAR*> ToLowerCase(TCHAR* string);
	std::unique_ptr<std::map<t_string, std::unique_ptr<CProgramSetting>>> programs;
	CSettings();
	CSettings(const CSettings& c);
	~CSettings();
protected:
};

