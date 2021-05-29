#pragma once
class Checkbox
{
public:
	Checkbox(HWND parent, int intResource);
	void SetCheckState(bool state);
protected:
	HWND hWnd;
	int intResouce;
};

