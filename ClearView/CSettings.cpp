#include "stdafx.h"
#include "CSettings.h"


CSettings::CSettings()
{
	this->programs = new map<t_string, CProgramSetting *>();
}


CSettings::~CSettings()
{
	delete this->programs;
}

// Search settings maps for alpha settings for given process and window class
BOOL CSettings::GetAlphaSetting(TCHAR* processFileName, TCHAR* windowClassName, WindowTypes type, __out BYTE& alpha)
{
	CAlphaSettings alphaSettings;
	ToLowerCase(processFileName, MAX_PATH);

	auto result = this->programs->find(processFileName);
	if (result != this->programs->end())
	{
		auto setting = result->second;
		auto windowResult = setting->windows->find(windowClassName);
		if (windowResult != setting->windows->end())
		{
			auto window = windowResult->second;
			alphaSettings = window->alphaSettings;
		}
		else{
			alphaSettings = setting->alphaSettings;
		}
	}
	else{
		alphaSettings = this->alphaSettings;
	}

	switch (type)
	{
	case WindowTypes::Foregound:
		alpha = alphaSettings.foreground;
		break;

	case WindowTypes::Background:
		alpha = alphaSettings.background;
		break;
	}
	return true;
}

void CSettings::ToLowerCase(TCHAR* string, size_t length)
{
	//Make process name lower case
	for (size_t i = 0; i < _tcsnlen(string, length); i++){
		string[i] = tolower(string[i]);
	}
}
