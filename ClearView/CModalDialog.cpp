#include "stdafx.h"
#include "CModalDialog.h"
#include "CDialog.h"
#include "WndProc.h"

CModalDialog::CModalDialog(HINSTANCE hInstance, HWND hParent) : CDialog(hInstance)
{
	this->hParent = hParent;
}


CModalDialog::~CModalDialog()
{
}

BOOL CModalDialog::Create()
{
	this->result = DialogBox(this->hInstance, this->dialogResource, this->hParent, DlgProcMain);
	return TRUE;
}
