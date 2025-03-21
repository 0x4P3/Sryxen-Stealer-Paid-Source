#ifndef GET_HPP
#define GET_HPP

#include <windows.h>
#include <winhttp.h>
#include "obfusheader.h"

#pragma comment(lib, "winhttp.lib")

std::string FetchUrl(const wchar_t* host, const wchar_t* path, bool secure = false);

#endif
