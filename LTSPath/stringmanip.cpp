#include "stringmanip.h"
#include <string_view>
#include <Windows.h>


namespace my::stringmanip
{
std::wstring toWString(const std::string& str)
{
    std::wstring wstrTo;
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (DWORD) str.size(), NULL, 0);
    if (size > 0) {
        wstrTo.resize(size);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (DWORD) str.size(), &wstrTo[0], size);
    }
    return wstrTo;
}

std::string toString(const std::wstring& wstr)
{
    std::string strTo;
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (DWORD) wstr.size(), NULL, 0, NULL, NULL);
    if (size > 0) {
        strTo.resize(size);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (DWORD) wstr.size(), &strTo[0], size, NULL, NULL);
    }
    return strTo;
}

std::string toString(const std::u8string& u8Str)
{
    return std::string(std::string_view(reinterpret_cast<const char*>(u8Str.c_str()), u8Str.size()));
}

void removeTrailingNull(std::string& str)
{
    str = str.c_str();
    str.shrink_to_fit();
}

void removeTrailingNull(std::wstring& str)
{
    str = str.c_str();
    str.shrink_to_fit();
}
}