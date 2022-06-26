#include "stdafx.h"
#include "UnicodeConversion.h"
#include <memory>
#include <string>

#ifdef UNICODE
// Convert std::string to a wchar_t* string.
std::unique_ptr<wchar_t[]> string_to_wchar_t(std::string string)
{
	size_t newsize = strlen(string.c_str()) + 1;
	auto wcstring = std::make_unique<wchar_t[]>(newsize);
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wcstring.get(), newsize, string.c_str(), _TRUNCATE);
	return wcstring;
}
#endif //UNICODE
