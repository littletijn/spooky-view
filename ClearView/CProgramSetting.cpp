#include "stdafx.h"
#include "CProgramSetting.h"

#pragma once

CProgramSetting::CProgramSetting()
{
	this->windows = std::make_unique<std::map<t_string, CWindowSetting *>>();
}

CProgramSetting::~CProgramSetting()
{
}