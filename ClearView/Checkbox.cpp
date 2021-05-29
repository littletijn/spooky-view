#include "stdafx.h"
#include "Checkbox.h"
#include <windowsx.h>


Checkbox::Checkbox(HWND parent, int intResource)
{
	this->hWnd = GetDlgItem(parent, intResource);
}

void Checkbox::SetCheckState(bool state)
{
	if (state)
	{
		Button_SetCheck(this->hWnd, BST_CHECKED);
	} 
	else
	{
		Button_SetCheck(this->hWnd, BST_UNCHECKED);
	}
}