#include "FormatLastError.h"

#include <cstdio>
#include <climits>

CHAR* FormatLastError(CHAR szBuffer[], DWORD dwError)
{
    LPSTR lpError = nullptr;

    DWORD charsWritten = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        dwError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&lpError),
        0,
        nullptr
    );

    if (charsWritten == 0 || lpError == nullptr)
    {
        sprintf_s(szBuffer, USHRT_MAX + 1, "Error %lu", dwError);
        return szBuffer;
    }

    sprintf_s(szBuffer, USHRT_MAX + 1, "Error %lu: %s", dwError, lpError);
    LocalFree(lpError);
    return szBuffer;
}