#include "fileoperation.h"
#include "stringmanip.h"
#include "system.h"
#include <ctime>
#include <Windows.h>

namespace fs = std::filesystem;
namespace sm = my::stringmanip;

namespace my::fileoperations
{
time_t getSymbolicLinkCreationDate(const std::string& fullPath)
{
    std::wstring wFullPath = sm::toWString(fullPath);
    HANDLE hFile = CreateFile(
        wFullPath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OPEN_REPARSE_POINT,
        NULL
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        return 0;
    }

    FILETIME ftModified;
    time_t timeSinceEpoch = 0;

    // Retrieve the file creation date in the local timezone for the file.
    if (GetFileTime(hFile, NULL, NULL, &ftModified)) {
        FILETIME localFileTime;
        FileTimeToLocalFileTime(&ftModified, &localFileTime);
        SYSTEMTIME sysTime;
        FileTimeToSystemTime(&localFileTime, &sysTime);
        struct tm tmtime = { 0 };
        tmtime.tm_year = sysTime.wYear - 1900;
        tmtime.tm_mon = sysTime.wMonth - 1;
        tmtime.tm_mday = sysTime.wDay;
        tmtime.tm_hour = sysTime.wHour;
        tmtime.tm_min = sysTime.wMinute;
        tmtime.tm_sec = sysTime.wSecond;
        tmtime.tm_wday = 0;
        tmtime.tm_yday = 0;
        tmtime.tm_isdst = -1;
        timeSinceEpoch = mktime(&tmtime);
    }
    CloseHandle(hFile);

    return timeSinceEpoch;
}

bool updateSymbolicLinkCreationDate(const std::string& fullPath)
{
    std::wstring wFullPath = sm::toWString(fullPath);
    HANDLE hFile = CreateFileW(
        wFullPath.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OPEN_REPARSE_POINT,
        NULL
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    FILETIME ftModified;
    SYSTEMTIME st;

    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ftModified);
    BOOL ok = SetFileTime(
        hFile,
        NULL,
        NULL,
        &ftModified
    );
    CloseHandle(hFile);

    return ok;
}

bool createDirectories(const fs::path& path)
{
    bool ok = true;
    if (!fs::is_directory(path)) {
        ok = fs::create_directories(path);
    }
    return ok;
}
}
