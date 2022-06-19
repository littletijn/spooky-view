#include <memory>

#pragma once
class Textbox
{
public:
	Textbox(HWND parent, int intResource);
	void SetText(LPTSTR text);
	std::unique_ptr<TCHAR[]> GetText(int* textLength);
protected:
	HWND hWnd;
};

