#pragma once
#include <string>

std::wstring wsprintf_z(const wchar_t *format, ...);
std::wstring WStringFromCharArray(const char * charArray, unsigned int length = 0);
double getCurrentTime();

#define OutputDebugStringF(a, ...) OutputDebugString(wsprintf_z(a ## "\n", __VA_ARGS__).c_str())

//for now...
#ifdef DEBUG
#define ShowErrorMessage OutputDebugString
#else
#define ShowErrorMessage(e) MessageBoxW(nullptr, e, L"Error", MB_ICONEXCLAMATION)
#endif
