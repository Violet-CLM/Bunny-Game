#include <cstdarg>
#include <sys/timeb.h>
#include "Misc.h"

std::wstring wsprintf_z(const wchar_t *format, ...) {
	std::wstring buffer;
	va_list args;
	int length;

	va_start(args, format);
	length = _vscwprintf(format, args);// + 1; // _vscprintf doesn't count terminating '\0'

	if (length > 0) {
		wchar_t *tempBuf = NULL;
		tempBuf = new wchar_t[length + 1];

		//vsprintf_s(tempBuf, len, format, args);
		_vsnwprintf_s(tempBuf, length + 1, _TRUNCATE, format, args);
		buffer = tempBuf;
		delete[] tempBuf;
	}

	va_end(args);
	return buffer;
}
std::string sprintf_z(const char *format, ...) {
	std::string buffer;
	va_list args;
	int length;

	va_start(args, format);
	length = _vscprintf(format, args);// + 1; // _vscprintf doesn't count terminating '\0'

	if (length > 0) {
		char *tempBuf = NULL;
		tempBuf = new char[length + 1];

		//vsprintf_s(tempBuf, len, format, args);
		_vsnprintf_s(tempBuf, length + 1, _TRUNCATE, format, args);
		buffer = tempBuf;
		delete[] tempBuf;
	}

	va_end(args);
	return buffer;
}

std::wstring WStringFromCharArray(const char * charArray, unsigned int length)
{
	const std::string narrowString(charArray, length ? length : strlen(charArray));
	return std::wstring(narrowString.begin(), narrowString.end());
}

double getCurrentTime() {
	static struct _timeb timebuffer;
	_ftime64_s(&timebuffer);
	return (timebuffer.time * 1000.0 + timebuffer.millitm);
}