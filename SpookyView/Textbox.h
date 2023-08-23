#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <memory>

class Textbox
{
public:
	Textbox(HWND parent, int intResource);
	void SetText(LPTSTR text);
	std::unique_ptr<TCHAR[]> GetText(int* textLength);
protected:
	HWND hWnd;
};

#endif