#include <string>
#include <map>
#include "CProgramSetting.h"
using namespace std;
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
	map<t_string, CProgramSetting *> *programs;
	CSettings();
	~CSettings();
protected:
	void ToLowerCase(TCHAR* string, size_t length);
};

