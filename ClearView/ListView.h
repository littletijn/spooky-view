#include "String.h"

#pragma once
class ListView
{
public:
	ListView(HWND parent, int intResource);
	~ListView();
	LPWSTR GetTextByIndex(int index, TCHAR* textBuffer);
	int GetSelectedIndex(LPARAM lParam);
	int GetSelectedIndex();
	int AddItem(LPWSTR text);
	int AddItem(t_string text);
	void DeleteAllItems();
protected:
	HWND hWnd;
	int intResouce;
};

