#pragma once
#include <filesystem>
#include <string>


namespace my::system
{
	std::string getCurrentProgramFullPath();
	std::string getCurrentProgramPath();
	std::string getProgramPath();
	bool getPrivileges();
	bool isPathUnicode(const std::string& path);
	std::string getShortPath(const std::string& strPath);
	std::string getSymbolicLink(const std::string& target);
	std::string getPathWithoutUnicode(std::string strPath);
	std::string getCurrentDirectory();
	bool setCurrentDirectory(const std::string& path);
	bool startProcess(const std::string& commandLine);
	std::string toString(const std::filesystem::path& path);
}