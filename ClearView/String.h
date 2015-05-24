#include <string>
using namespace std;
#pragma once

#ifndef UNICODE  
	typedef std::string t_string;
#else
	typedef std::wstring t_string;
#endif