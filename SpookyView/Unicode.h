#include <string>

#ifdef UNICODE
#include <memory>
#define tstring std::wstring
#define to_tstring std::to_wstring
std::unique_ptr<wchar_t[]> string_to_wchar_t(std::string string);
#else
#define tstring std::string
#define to_tstring std::to_string
#endif // !UNICODE