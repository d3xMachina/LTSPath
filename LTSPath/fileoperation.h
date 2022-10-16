#pragma once
#include <filesystem>
#include <string>

namespace my::fileoperations
{
	time_t getSymbolicLinkCreationDate(const std::string& fullPath);
	bool updateSymbolicLinkCreationDate(const std::string& fullPath);
	bool createDirectories(const std::filesystem::path& path);
}