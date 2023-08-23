#ifndef CPROGRAMSETTING_H
#define CPROGRAMSETTING_H

#include "MultiPlatformString.h"
#include "CWindowSetting.h"
#include "CAlphaSettings.h"
#include <map>
#include <memory>

class CProgramSetting
{
public:
	CProgramSetting();
	CProgramSetting(const CProgramSetting& c);
	~CProgramSetting();
	t_string name;
	t_string path;
	CAlphaSettings alphaSettings;
	std::unique_ptr<std::map<t_string, std::unique_ptr<CWindowSetting>>> windows;
};

#endif