#include "stdafx.h"
#include "CProgramSetting.h"

#pragma once

CProgramSetting::CProgramSetting()
{
	this->windows = std::make_unique<std::map<t_string, std::unique_ptr<CWindowSetting>>>();
}

CProgramSetting::CProgramSetting(const CProgramSetting& c) : CProgramSetting::CProgramSetting()
{
	this->alphaSettings = CAlphaSettings(c.alphaSettings);
	for (auto window = c.windows.get()->begin(); window != c.windows.get()->end(); ++window)
	{
		t_string firstCopy = t_string(window->first);
		this->windows->insert(std::pair<t_string, std::unique_ptr<CWindowSetting>>(firstCopy, std::make_unique<CWindowSetting>(*window->second)));
	}
}

CProgramSetting::~CProgramSetting()
{
}