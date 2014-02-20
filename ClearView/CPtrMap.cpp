#include "stdafx.h"
#include "CPtrMap.h"

// The pointer map class
// Its purpose is to shorten the time for which critical section lock is required
// The lock is held only for the duration of searching or modifying the pointer map
// allowing for nested CreateWindow calls (ie, calling CreateWindow from within WM_CREATE handler)

// Constructor initializes our critical section
CPtrMap::CPtrMap()
{
	InitializeCriticalSection(&m_cs);
}

// And destructor deletes it
CPtrMap::~CPtrMap()
{
	DeleteCriticalSection(&m_cs);
}

// Add acquires a critical section lock and adds the current thread and the specified associated window to the map
void CPtrMap::Add(CWnd *w)
{
	EnterCriticalSection(&m_cs);
	m_map[GetCurrentThreadId()] = w;
	// Important: TODO: add error handling for std::bad_alloc, and leave critical section even if an exception is thrown
	LeaveCriticalSection(&m_cs);
}

// Extract acquires a critical section lock and retrieves the window associated with the current window from the map
CWnd *CPtrMap::Extract()
{
	EnterCriticalSection(&m_cs);
	CWnd *w = m_map[GetCurrentThreadId()];
	// Important: leave critical section before returning
	LeaveCriticalSection(&m_cs);
	return w;
}

CPtrMap g_ptrmap;