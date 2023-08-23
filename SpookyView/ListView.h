#ifndef LISTVIEW_H
#define LISTVIEW_H

#include "MultiPlatformString.h"

class ListView
{
public:
	ListView(HWND parent, int intResource);
	~ListView();
	LPTSTR GetTextByIndex(int index, TCHAR* textBuffer);
	void InsertColumn(int index, TCHAR* textBuffer);
	int GetSelectedIndex();
	void SetSelectedItem(int index);
	int AddItem(LPTSTR text);
	void SetItem(int itemIndex, int subItemIndex, t_string text);
	int AddItem(t_string text);
	void DeleteAllItems();
	void DeleteSelectedItem();
protected:
	HWND hWnd;
	int intResouce;
};

#endif