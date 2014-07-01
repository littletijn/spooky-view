class CSettings
{
public:
	enum WindowTypes
	{
		Foregound,
		Background
	};

	CSettings();
	~CSettings();
	BOOL GetAlphaSetting(TCHAR* processFileName, TCHAR* windowClassName, WindowTypes type, __out BYTE& alpha);
protected:
	HKEY registryRootKey;
	HKEY LoadSettingsKey(TCHAR* processFileName, TCHAR* windowClassName);
	BOOL ReadKeyValue(HKEY key, TCHAR* valueName, __out BYTE& value);
};