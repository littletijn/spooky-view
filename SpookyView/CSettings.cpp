#include "stdafx.h"
#include "CSettings.h"
#include <memory>


CSettings::CSettings()
{
	this->programs = std::make_unique<std::map<t_string, std::unique_ptr<CProgramSetting>>>();
}

CSettings::CSettings(const CSettings& c) : CSettings::CSettings()
{
	this->modificationSettings = CModificationSettings(c.modificationSettings);
	for (auto program = c.programs.get()->begin(); program != c.programs.get()->end(); ++program)
	{
		t_string firstCopy = t_string(program->first);
		this->programs->insert(std::pair<t_string, std::unique_ptr<CProgramSetting>>(firstCopy, std::make_unique<CProgramSetting>(*program->second)));
	}
}

CSettings::~CSettings()
{
}

// Search settings maps for alpha settings for given process and window class
CModificationSettings* CSettings::GetModificationSetting(TCHAR* processFileName, TCHAR* windowClassName)
{
	CModificationSettings* foundModificationSettings;
	auto lowerCaseProcessKeyName = ToLowerCase(processFileName);

	auto result = this->programs->find(*lowerCaseProcessKeyName);
	if (result != this->programs->end())
	{
		auto& setting = result->second;
		auto windowResult = setting->windows->find(windowClassName);
		if (windowResult != setting->windows->end())
		{
			auto& window = windowResult->second;
			foundModificationSettings = &window->modificationSettings;
		}
		else{
			foundModificationSettings = &setting->modificationSettings;
		}
	}
	else{
		foundModificationSettings = &this->modificationSettings;
	}
	if (!foundModificationSettings->enabled) {
		return NULL;
	}
	return foundModificationSettings;
}

std::unique_ptr<TCHAR*> CSettings::ToLowerCase(TCHAR* string)
{
	TCHAR* lowerCase;
	_tcslwr_s(lowerCase = _tcsdup(string), _tcslen(string) + 1);
	return std::make_unique<TCHAR*>(lowerCase);
}
