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
	BOOL GetAlphaSetting(TCHAR* processFileName, TCHAR* windowClassName, WindowTypes type, BYTE& alpha);
	std::unique_ptr<std::map<t_string, std::unique_ptr<CProgramSetting>>> programs;
	CSettings();
	CSettings(const CSettings& c);
	~CSettings();
	void ToLowerCase(TCHAR* string);
protected:
};

