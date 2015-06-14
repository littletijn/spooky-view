#include "stdafx.h"
#include "ListView.h"
#include <CommCtrl.h>
#include <vector>

using namespace std;

ListView::ListView(HWND parent, int intResource)
{
	this->hWnd = GetDlgItem(parent, intResource);
}


ListView::~ListView()
{
}

LPWSTR ListView::GetTextByIndex(int index, TCHAR* textBuffer)
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

int ListView::GetSelectedIndex(LPARAM lParam)
{
	LPNMHDR notifyMessage = (LPNMHDR)lParam;
	int index = -1;
	switch (notifyMessage->code)
	{
	case NM_CLICK:
	{
		LPNMITEMACTIVATE item = (LPNMITEMACTIVATE)lParam;
		index = item->iItem;
	}
	break;

	case LVN_ITEMACTIVATE:
		index = ListView_GetNextItem(notifyMessage->hwndFrom, -1, LVNI_SELECTED);
		break;

	}
	return index;
}

int ListView::AddItem(t_string text)
{
	LVITEM item;
	
	std::vector<wchar_t>* textBuffer = new std::vector<wchar_t>(text.begin(), text.end());
	textBuffer->push_back(0); //Add null terminator for string
	SecureZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT;
	item.pszText = textBuffer->data();

	int result = ListView_InsertItem(this->hWnd, &item);
	delete item.pszText;
	return result;
}

void ListView::DeleteAllItems()
{
	ListView_DeleteAllItems(this->hWnd);
}
