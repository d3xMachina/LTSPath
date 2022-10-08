#include <fstream>
#include <filesystem>
#include <windows.h>


wchar_t* getPathBuffer();
std::wofstream& mylog();
std::wstring getCurrentProgramFullPath();
std::wstring getCurrentProgramPath();
std::wstring getProgramPath();
std::wstring getShortPath(const std::wstring& strPath);

namespace fs = std::filesystem;

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    std::wstring program = getProgramPath();
    fs::path programPath(program);
    if (!fs::is_regular_file(programPath)) {
        mylog() << "Program missing or not specified! (create a file ltspath.txt with the path of the absolute path of the program)" << std::endl;
        return EXIT_FAILURE;
    }

    std::wstring args;
    for (int i = 1; i < __argc; ++i) {
        std::wstring arg = std::wstring(__wargv[i]);
        fs::path path(arg);
        if (!path.empty()) { // the arg is a path
            std::wstring shortPath = getShortPath(arg);
            if (!shortPath.empty()) {
                arg = shortPath;

                //mylog() << "Path=" << path.wstring() << std::endl;
                //mylog() << "Short Path=" << path.wstring() << std::endl;
            }
        }
        if (arg.find_first_of(L' ') == std::wstring::npos) {
            arg = L"\"" + arg + L"\"";
        }
        args += arg;
    }

    std::wstring commandLine = L"\"" + program + L"\" " + args;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessW(NULL,
        &commandLine[0],
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi)
        )
    {
        mylog() << "Could not launch the application " << program << " with arguments " << args << std::endl;
        return EXIT_FAILURE;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return EXIT_SUCCESS;
}

wchar_t* getPathBuffer()
{
    static wchar_t pathBuffer[MAX_PATH];
    return pathBuffer;
}

std::wofstream& mylog()
{
    static std::wofstream fLog(getCurrentProgramPath() + L"ltspath.log", std::ios::out | std::ios::trunc);
    return fLog;
}

std::wstring getCurrentProgramFullPath()
{
    std::wstring strPath;

    HMODULE hModule = GetModuleHandleW(NULL);
    if (hModule != NULL)
    {
        GetModuleFileNameW(hModule, getPathBuffer(), MAX_PATH);
        strPath = std::wstring(getPathBuffer());
    }
    return strPath;
}

std::wstring getCurrentProgramPath()
{
    static std::wstring strPath;

    if (strPath.empty())
    {
        fs::path path(getCurrentProgramFullPath());
        path.remove_filename();
        strPath = path.wstring();
    }
    return strPath;
}

std::wstring getProgramPath()
{
    std::wstring strPath;
    std::wifstream fProgram(getCurrentProgramPath() + L"ltspath.txt", std::ios::in);
    if (fProgram.is_open())
    {
        std::getline(fProgram, strPath);
        fs::path path(strPath);
        if (path.has_filename() && !path.has_parent_path()) {
            strPath = getCurrentProgramPath() + strPath;
        }
    }
    return strPath;
}

std::wstring getShortPath(const std::wstring& strPath)
{
    std::wstring strShortPath;
    DWORD ret = GetShortPathNameW(strPath.c_str(), getPathBuffer(), MAX_PATH);
    if (ret == 0) {
        mylog() << "Could not get short name: error=" << GetLastError() << std::endl;
    }
    else {
        strShortPath = std::wstring(getPathBuffer());
    }
    return getPathBuffer();
}