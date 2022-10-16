#include <windows.h>
#include "fileoperation.h"
#include "logger.h"
#include "stringmanip.h"
#include "system.h"


bool createDirectories();

namespace fs = std::filesystem;
namespace fo = my::fileoperations;
namespace sm = my::stringmanip;
namespace sys = my::system;

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    Log::setDestination(sys::getCurrentProgramPath() + "ltspath\\logs.log");
#ifdef _DEBUG
    Log::setReportingLevel(LOG_DEBUG);
#else
    Log::setReportingLevel(LOG_INFO);
#endif
    Log::clear();

    if (!createDirectories()) {
        return EXIT_FAILURE;
    }
 
    if (!sys::getPrivileges()) { // not sure if still necessary
        //return EXIT_FAILURE;
    }

    std::string program = sys::getProgramPath();
    fs::path programPath = fs::u8path(program);
    if (!fs::is_regular_file(programPath)) {
        Log().Get(LOG_ERROR) << "Program missing or not specified! (create a file ltspath.txt with the path of the absolute path of the program)";
        return EXIT_FAILURE;
    }

    // Change arguments paths to avoid unicode
    std::string args;
    for (int i = 1; i < __argc; ++i) {
        std::string arg = sm::toString(std::wstring(__wargv[i]));
        arg = sys::getPathWithoutUnicode(arg);
        arg = "\"" + arg + "\"";
        args += arg;
    }

    // Change current dir to avoid unicode as it can crash non unicode apps
    std::string currentDirectory = sys::getCurrentDirectory();
    if (!currentDirectory.empty())
    {
        currentDirectory = sys::getPathWithoutUnicode(currentDirectory);
        sys::setCurrentDirectory(currentDirectory);
    }

    std::string commandLine = "\"" + program + "\" " + args;
    bool ok = sys::startProcess(commandLine);
    if (!ok)
    {
        Log().Get(LOG_ERROR) << "Could not launch the application " << program << " with arguments " << args;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

bool createDirectories()
{
    fs::path path = fs::u8path(sys::getCurrentProgramPath() + "ltspath\\links"); // no trailing slash or error on creation of dir even if it works
    bool ok = fo::createDirectories(path);
    if (!ok) {
        Log().Get(LOG_ERROR) << "Failed to create directory " << sys::toString(path);
    }
    return ok;
}
