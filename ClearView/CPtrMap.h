#pragma once

#include "stdafx.h"
#include <map>
#include "CWnd.h"

class CPtrMap
{
public:
	CPtrMap();
	~CPtrMap();
	void Add(CWnd *w);
	CWnd *Extract();
private:
	// thread identifier to Window pointer map
	std::map<DWORD, CWnd *> m_map;
	// Critical section used to synchronize access to the map
	CRITICAL_SECTION m_cs;
};