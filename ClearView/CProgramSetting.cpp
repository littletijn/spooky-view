#include "stdafx.h"
#include "CProgramSetting.h"

#pragma once

CProgramSetting::CProgramSetting()
{
	this->windows = std::make_unique<std::map<t_string, CWindowSetting *>>();
}

CProgramSetting::CProgramSetting(const CProgramSetting& c) : CProgramSetting::CProgramSetting()
{
	this->alphaSettings = CAlphaSettings(c.alphaSettings);
	for (auto window = c.windows.get()->begin(); window != c.windows.get()->end(); ++window)
	{
		t_string firstCopy = t_string(window->first);
		CWindowSetting* secondCopy = new CWindowSetting(*window->second);
		this->windows->insert(std::pair<t_string, CWindowSetting*>(firstCopy, secondCopy));
	}
}

CProgramSetting::~CProgramSetting()
{
}