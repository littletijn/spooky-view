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
	BOOL GetAlphaSetting(TCHAR* processFileName, TCHAR* windowClassName, WindowTypes type, __out BYTE& alpha);
	std::unique_ptr<std::map<t_string, CProgramSetting*>> programs;
	CSettings();
	~CSettings();
protected:
	void ToLowerCase(TCHAR* string, size_t length);
};

