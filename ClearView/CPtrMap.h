#pragma once

#include "stdafx.h"
#include <map>

template <class T>
class CPtrMap
{
public:
	CPtrMap();
	~CPtrMap();
	void Add(T *w);
	T *Extract();
private:
	// thread identifier to Window pointer map
	std::map<DWORD, T *> m_map;
	// Critical section used to synchronize access to the map
	CRITICAL_SECTION m_cs;
};