#ifndef UNICODE_H
#define UNICODE_H

#include <string>

#ifdef UNICODE
#define tstring std::wstring
#define to_tstring std::to_wstring
#else
#define tstring std::string
#define to_tstring std::to_string
#endif // !UNICODE

#endif