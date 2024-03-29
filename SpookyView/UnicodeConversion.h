#ifndef UNICODECONVERSION_H
#define UNICODECONVERSION_H

#include <string>
#include <memory>

std::unique_ptr<wchar_t[]> string_to_wchar_t(std::string string);

#endif