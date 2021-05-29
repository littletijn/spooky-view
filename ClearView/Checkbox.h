#pragma once
class Checkbox
{
public:
	Checkbox(HWND parent, int intResource);
	void SetCheckState(bool state);
	BOOL GetCheckState();
protected:
	HWND hWnd;
	int intResouce;
};

