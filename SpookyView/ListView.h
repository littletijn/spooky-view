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
	int AddItem(LPTSTR text, int index = -1);
	int AddItem(t_string text, int index = -1);
	void SetItem(int itemIndex, int subItemIndex, t_string text);
	void DeleteAllItems();
	void DeleteSelectedItem();
	int GetItemCount();
protected:
	HWND hWnd;
	int intResouce;
};

#endif