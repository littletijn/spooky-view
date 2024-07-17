#include "stdafx.h"
#include "Textbox.h"
#include <memory>

Textbox::Textbox(HWND parent, int intResource)
{
	this->hWnd = GetDlgItem(parent, intResource);
}

void Textbox::SetText(LPTSTR text)
{
	SendMessage(this->hWnd, WM_SETTEXT, 0, (LPARAM)text);
}

std::unique_ptr<TCHAR[]> Textbox::GetText(int* textLength)
{
	*textLength = GetWindowTextLength(this->hWnd);
	auto textBuffer = std::make_unique<TCHAR[]>(*textLength + 1);
	GetWindowText(this->hWnd, textBuffer.get(), *textLength + 1);
	return textBuffer;
}
