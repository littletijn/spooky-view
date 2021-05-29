#include "CSettings.h"
#pragma once

class ISettingsManager
{

public:
	//A virtual implemented destructor is required. Otherwise the destructor of a derived class will not be called when a reference via a base class is delete-d.
	//Also this destructor will be called
	virtual ~ISettingsManager() { /* Empty implementation */ }
	virtual CSettings* GetSettings() = 0;
	virtual void LoadSettings() = 0;
	virtual bool SaveSettings() = 0;
};