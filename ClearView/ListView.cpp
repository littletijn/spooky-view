#include "stdafx.h"
#include "ListView.h"
#include <commctrl.h>
#include <vector>
#include <memory>
#include "Unicode.h"

ListView::ListView(HWND parent, int intResource)
{
	this->hWnd = GetDlgItem(parent, intResource);
}


ListView::~ListView()
{
}

LPTSTR ListView::GetTextByIndex(int index, TCHAR* textBuffer)
{
	LVITEM item;
	SecureZeroMemory(&item, sizeof(item));
	item.iItem = index;
	item.mask = LVIF_TEXT;
	item.cchTextMax = MAX_PATH;
	item.pszText = textBuffer;
	ListView_GetItem(hWnd, &item);
	return item.pszText;
}

void ListView::InsertColumn(int index, TCHAR* textBuffer)
{
	LVCOLUMN column;
	column.mask = LVCF_TEXT | LVCF_WIDTH;
	column.pszText = textBuffer;
	column.cx = 150;
	ListView_InsertColumn(this->hWnd, index, &column);

}

void ListView::SetSelectedItem(int index)
{
	ListView_SetItemState(this->hWnd, index,  LVIS_SELECTED,  LVIS_SELECTED);
}

int ListView::GetSelectedIndex()
{
	return ListView_GetNextItem(this->hWnd, -1, LVNI_SELECTED);
}

int ListView::AddItem(LPTSTR text)
{
	LVITEM item;
	SecureZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT;
	item.pszText = text;

	int result = ListView_InsertItem(this->hWnd, &item);
	return result;
}

void ListView::SetItem(int itemIndex, int subItemIndex, t_string text)
{
	auto textBuffer = std::make_unique<std::vector<TCHAR>>(text.begin(), text.end());
	textBuffer->push_back(0); //Add null terminator for string
	ListView_SetItemText(this->hWnd, itemIndex, subItemIndex, textBuffer->data());
}

int ListView::AddItem(t_string text)
{	
	auto textBuffer = std::make_unique<std::vector<TCHAR>>(text.begin(), text.end());
	textBuffer->push_back(0); //Add null terminator for string
	int result = this->AddItem(textBuffer->data());
	return result;
}

void ListView::DeleteAllItems()
{
	ListView_DeleteAllItems(this->hWnd);
}

void ListView::DeleteSelectedItem()
{
	int index = GetSelectedIndex();
	ListView_DeleteItem(this->hWnd, index);
}
