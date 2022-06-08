#include "stdafx.h"
#include "CSettings.h"
#include <memory>


CSettings::CSettings()
{
	this->programs = std::make_unique<std::map<t_string, CProgramSetting*>>();
}

CSettings::CSettings(const CSettings& c) : CSettings::CSettings()
{
	this->alphaSettings = CAlphaSettings(c.alphaSettings);
	for (auto program = c.programs.get()->begin(); program != c.programs.get()->end(); ++program)
	{
		t_string firstCopy = t_string(program->first);
		CProgramSetting* secondCopy = new CProgramSetting (*program->second);
		this->programs->insert(std::pair<t_string, CProgramSetting*>(firstCopy, secondCopy));
	}
}

CSettings::~CSettings()
{
}

// Search settings maps for alpha settings for given process and window class
BOOL CSettings::GetAlphaSetting(TCHAR* processFileName, TCHAR* windowClassName, WindowTypes type, BYTE& alpha)
{
	CAlphaSettings alphaSettings;
	ToLowerCase(processFileName);

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
	if (!alphaSettings.enabled) {
		return false;
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

void CSettings::ToLowerCase(TCHAR* string)
{
	_tcslwr_s(string = _tcsdup(string), _tcslen(string) + 1);
}
