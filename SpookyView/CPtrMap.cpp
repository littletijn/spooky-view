#include "stdafx.h"
#include "CPtrMap.h"
#include "CWindow.h"
#include "CDialog.h"

// The pointer map class
// Its purpose is to shorten the time for which critical section lock is required
// The lock is held only for the duration of searching or modifying the pointer map
// allowing for nested CreateWindow calls (ie, calling CreateWindow from within WM_CREATE handler)

// Constructor initializes our critical section
template <class T>
CPtrMap<T>::CPtrMap()
{
	InitializeCriticalSection(&m_cs);
}

// And destructor deletes it
template <class T>
CPtrMap<T>::~CPtrMap()
{
	DeleteCriticalSection(&m_cs);
}

// Add acquires a critical section lock and adds the current thread and the specified associated window to the map
template <class T>
void CPtrMap<T>::Add(T *w)
{
	EnterCriticalSection(&m_cs);
	m_map[GetCurrentThreadId()] = w;
	// Important: TODO: add error handling for std::bad_alloc, and leave critical section even if an exception is thrown
	LeaveCriticalSection(&m_cs);
}

// Extract acquires a critical section lock and retrieves the window associated with the current window from the map
template <class T>
T *CPtrMap<T>::Extract()
{
	EnterCriticalSection(&m_cs);
	T *w = m_map[GetCurrentThreadId()];
	// Important: leave critical section before returning
	LeaveCriticalSection(&m_cs);
	return w;
}

//http://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
template class CPtrMap<CWindow>;
template class CPtrMap<CDialog>;