#include "stdafx.h"
#include "CProgramSetting.h"

#pragma once

CProgramSetting::CProgramSetting()
{
	this->windows = new std::map<t_string, CWindowSetting *>();
}

CProgramSetting::~CProgramSetting()
{
	delete this->windows;
}