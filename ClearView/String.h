#include <string>
#pragma once

#ifndef UNICODE  
	typedef std::string t_string;
#else
	typedef std::wstring t_string;
#endif