#include "system.h"
#include "fileoperation.h"
#include "logger.h"
#include "stringmanip.h"
#include <fstream>
#include <Windows.h>

namespace fs = std::filesystem;
namespace fo = my::fileoperations;
namespace sm = my::stringmanip;

namespace my::system
{
std::string getCurrentProgramFullPath()
{
    std::string strPath;
    HMODULE hModule = GetModuleHandleW(NULL);
    if (hModule != NULL) {
        std::wstring buffer;
        buffer.resize(MAX_PATH);

        GetModuleFileNameW(hModule, &buffer[0], (DWORD)buffer.size());
        sm::removeTrailingNull(buffer);
        strPath = sm::toString(buffer);
    }
    return strPath;
}

std::string getCurrentProgramPath()
{
    static std::string strPath;

    if (strPath.empty()) {
        fs::path path = fs::u8path(getCurrentProgramFullPath());
        path.remove_filename();
        strPath = toString(path);
    }
    return strPath;
}

std::string getProgramPath()
{
    std::string strPath;
    std::ifstream fProgram(getCurrentProgramPath() + "ltspath\\path.txt", std::ios::in);
    if (fProgram.is_open()) {
        std::getline(fProgram, strPath);
        fs::path path = fs::u8path(strPath);
        if (path.has_filename() && !path.has_parent_path()) {
            strPath = getCurrentProgramPath() + strPath;
        }
    }
    return strPath;
}

bool getPrivileges()
{
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        Log().Get(LOG_ERROR) << "OpenProcessToken failed: error=" << GetLastError() << std::endl;
        return false;
    }

    LUID luid;
    if (LookupPrivilegeValueW(NULL, L"SeCreateSymbolicLinkPrivilege", &luid))
    {
        TOKEN_PRIVILEGES token_privileges;
        token_privileges.PrivilegeCount = 1;
        token_privileges.Privileges[0].Luid = luid;
        token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        if (!AdjustTokenPrivileges(hToken, FALSE, &token_privileges, 0, NULL, NULL))
        {
            Log().Get(LOG_ERROR) << "AdjustTokenPrivileges failed: error=" << GetLastError() << std::endl;
            return false;
        }
    }
    return true;
}

bool isPathUnicode(const std::string& path)
{
    // Might not be accurate
    std::wstring wPath = sm::toWString(path);
    return std::any_of(wPath.begin(), wPath.end(), [](wchar_t c) {
        return c > 127;
    });
}

std::string getShortPath(const std::string& strPath)
{
    std::wstring wStrShortPath;
    std::wstring wStrPath = sm::toWString(strPath);
    int size = GetShortPathNameW(wStrPath.c_str(), NULL, 0);
    if (size > 0) {
        wStrShortPath.resize(size);
        size = GetShortPathNameW(wStrPath.c_str(), &wStrShortPath[0], (DWORD)wStrShortPath.size());
        if (size == 0) {
            Log().Get(LOG_ERROR) << "Could not get short name: error=" << GetLastError() << std::endl;
        }
    }
    return sm::toString(wStrShortPath);
}

std::string getSymbolicLink(const std::string& target)
{
    const int maxSymbolicLinksSaved = 255;

    fs::path targetPath = fs::u8path(target);
    fs::path destPath = fs::u8path(getCurrentProgramPath() + "ltspath\\links\\");

    // In case the target is a symlink of LTSPath
    if (targetPath.parent_path() == destPath) {
        return target;
    }

    time_t oldest = 0;
    fs::path oldestPath;
    int nbLinks = 0;
    std::vector<std::string> stems;

    // Get oldest and most recent symlink
    for (const auto& entry : fs::directory_iterator(destPath)) {
        if (!entry.is_symlink()) {
            continue;
        }

        fs::path linkPath = entry.path();
        std::string strLinkPath = toString(linkPath);
        // A symlink has already been created for this target
        fs::path linkTargetPath = fs::read_symlink(linkPath);
        if (linkTargetPath == targetPath) {
            fo::updateSymbolicLinkCreationDate(strLinkPath);
            return strLinkPath;
        }

        time_t lastModified = fo::getSymbolicLinkCreationDate(strLinkPath);
        if (oldestPath.empty() || lastModified < oldest) {
            oldest = lastModified;
            oldestPath = linkPath;
        }
        ++nbLinks;
        stems.push_back(toString(linkPath.stem()));
    }

    // Remove oldest symbolic link once limit is reached
    if (nbLinks >= maxSymbolicLinksSaved && !oldestPath.empty()) {
        bool ok = fs::remove(oldestPath);
        if (!ok) {
            Log().Get(LOG_ERROR) << "Failed to remove symbolic link " << toString(oldestPath) << std::endl;
        }
        else {
            stems.erase(std::remove(stems.begin(), stems.end(), toString(oldestPath.stem())), stems.end());
        }
    }

    // Get new symbolic link filename
    std::string destStem = "0";
    for (int i = 0; i < maxSymbolicLinksSaved; ++i) {
        std::string stem = std::to_string(i);
        if (std::find(stems.begin(), stems.end(), stem) == stems.end()) {
            destStem = stem;
            break;
        }
    }

    std::string destFileName = destStem;
    std::error_code errCode;
    if (fs::is_regular_file(targetPath)) {
        destFileName += toString(targetPath.extension());
        destPath /= destFileName;
        fs::create_symlink(targetPath, destPath, errCode);
    }
    else {
        destPath /= destFileName;
        fs::create_directory_symlink(targetPath, destPath, errCode);
    }
    if (errCode) {
        Log().Get(LOG_ERROR) << "Failed to create a symbolic link " << toString(destPath) << std::endl;
        return "";
    }

    return toString(destPath);
}

std::string getPathWithoutUnicode(std::string strPath)
{
    fs::path path = fs::u8path(strPath);
    strPath = toString(path); // correct the formatting

    if (!path.empty() && isPathUnicode(strPath)) { // the arg is a path
        std::string newPath = getSymbolicLink(strPath);
        if (newPath.empty()) { // Try short path method if no rights to create symbolic links
            newPath = getShortPath(strPath);
        }
        if (newPath.empty()) {
            // nothing works, try to launch with unicode path anyway...
            Log().Get(LOG_WARNING) << "Path could not be changed: " << toString(path) << std::endl;
        }
        else {
            strPath = newPath;
            /*if (i == 1 && fs::is_regular_file(newPath)) {
                std::string currentDir = toString(fs::u8path(newPath).parent_path());
                setCurrentDirectory(currentDir);
            }*/
            Log().Get(LOG_INFO) << "Path changed: " << toString(path) << " --> " << newPath << std::endl;
        }
    }
    return strPath;
}

std::string getCurrentDirectory()
{
    std::wstring wStrPath;
    int size = GetCurrentDirectoryW(0, NULL);
    if (size > 0) {
        wStrPath.resize(size);
        size = GetCurrentDirectoryW((DWORD)wStrPath.size(), &wStrPath[0]);
        if (size == 0) {
            Log().Get(LOG_ERROR) << "Could not get the current directory: error=" << GetLastError() << std::endl;
        }
    }
    return sm::toString(wStrPath);
}

bool setCurrentDirectory(const std::string& path)
{
    std::wstring wPath = sm::toWString(path);
    BOOL ok = SetCurrentDirectoryW(wPath.c_str());
    if (!ok) {
        Log().Get(LOG_ERROR) << "Could not set the current directory to " << path << ": error=" << GetLastError() << std::endl;
    }
    return ok;
}

bool startProcess(const std::string& commandLine)
{
    std::wstring WCommandLine = sm::toWString(commandLine);
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessW(
        NULL,
        &WCommandLine[0],
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
        return false;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

std::string toString(const fs::path& path)
{
    std::u8string u8Path = path.u8string();
    return sm::toString(u8Path);
}
}