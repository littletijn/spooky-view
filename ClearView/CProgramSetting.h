#include "String.h"
#include "CWindowSetting.h"
#include "CAlphaSettings.h"
#include <map>
#include <memory>

#pragma once

class CProgramSetting
{
public:
	CProgramSetting();
	~CProgramSetting();
	t_string name;
	t_string path;
	CAlphaSettings alphaSettings;
	std::unique_ptr<std::map<t_string, CWindowSetting *>> windows;
};

