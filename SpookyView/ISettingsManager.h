#include "CSettings.h"
#include "Unicode.h"
#pragma once

class ISettingsManager
{

public:
	//A virtual implemented destructor is required. Otherwise the destructor of a derived class will not be called when a reference via a base class is delete-d.
	//Also this destructor will be called
	virtual ~ISettingsManager() { /* Empty implementation */ }
	virtual BOOL Init() = 0;
	virtual CSettings* GetSettings() = 0;
	virtual void LoadSettings() = 0;
	virtual bool SaveSettings() = 0;
	virtual void ApplyNewSettings(CSettings* newSettings) = 0;
	virtual void AddSkipVersionKey(tstring versionNumber) = 0;
	virtual BOOL ShouldSkipVersion(tstring versionNumber) = 0;
	virtual BOOL GetDisableUpdateCheck() = 0;
	virtual void SetDisableUpdateCheck(BOOL state) = 0;
	virtual BOOL GetSkipWelcome() = 0;
	virtual void SetSkipWelcome() = 0;
};