#include "String.h"

#pragma once
class ListView
{
public:
	ListView(HWND parent, int intResource);
	~ListView();
	LPWSTR GetTextByIndex(int index, TCHAR* textBuffer);
	int GetSelectedIndex();
	void SetSelectedItem(int index);
	int AddItem(LPWSTR text);
	int AddItem(t_string text);
	void DeleteAllItems();
	void DeleteSelectedItem();
protected:
	HWND hWnd;
	int intResouce;
};

