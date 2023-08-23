#ifndef CSETTINGS_H
#define CSETTINGS_H

#include <string>
#include <map>
#include "CProgramSetting.h"
#include <memory>

class CSettings
{
public:
	enum WindowTypes
	{
		Foregound,
		Background
	};

	CAlphaSettings alphaSettings;
	CAlphaSettings* GetAlphaSetting(TCHAR* processFileName, TCHAR* windowClassName);
	std::unique_ptr<TCHAR*> ToLowerCase(TCHAR* string);
	std::unique_ptr<std::map<t_string, std::unique_ptr<CProgramSetting>>> programs;
	CSettings();
	CSettings(const CSettings& c);
	~CSettings();
protected:
};

#endif

