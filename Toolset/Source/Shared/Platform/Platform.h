#pragma once
#include <string>
#include <vector> 
#include <algorithm> 
#include <sstream>


#ifdef _WIN32
#define _WINSOCKAPI_ 
#define NOMINMAX
#include <windows.h>
#else
#include <dirent.h>
#endif

inline std::string ReplaceString(std::string subject, const std::string& search, const std::string& replace) 
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

inline std::vector<std::string> SplitString(const std::string& s, char delim) 
{
	std::stringstream ss(s);
	std::string item;
	std::vector<std::string> tokens;
	while (std::getline(ss, item, delim)) 
	{
		tokens.push_back(item);
	}
	return tokens;
}

inline std::vector<std::string> SplitString(std::string s, std::string delim) 
{
	std::vector<std::string> vect;
	size_t pos = 0;
	std::string token;

	while ((pos = s.find(delim)) != std::string::npos) 
	{
		token = s.substr(0, pos);
		vect.push_back(token);
		s.erase(0, pos + s.length());
	}
	vect.push_back(s);
	return vect;
}

inline bool MatchStringPattern(std::string Target, std::string Pattern) 
{
	if (Target.empty() && Pattern.empty())
		return true;

	if (Pattern[0] == '*' && Pattern.length() > 1 && Target.empty())
		return false;

	if (Pattern[0] == '?' || Pattern[0] == Target[0])
		return MatchStringPattern(Target.substr(1), Pattern.substr(1));

	if (Pattern[0] == '*' && !Pattern.empty() && Target.empty())
		return MatchStringPattern(Target, Pattern.substr(1));

	if (Pattern[0] == '*' && !Target.empty() && Pattern.empty())
		return MatchStringPattern(Target.substr(1), Pattern);

	if (Pattern[0] == '*')
		return MatchStringPattern(Target.substr(1), Pattern) || MatchStringPattern(Target, Pattern.substr(1));

	return false;
}

inline std::string FormatPath(std::string path)
{
	std::replace(path.begin(), path.end(), '/', '\\');
	path = ReplaceString(path, "\\\\", "\\");
	return path;
}

inline std::string GetExecutableDirectory()
{
	std::string directory;

#ifdef _WIN32
	char filename[MAX_PATH];
	GetModuleFileNameA(nullptr, filename, MAX_PATH);
	directory = filename;
#else ifdef LINUX
	//TODO https://linux.die.net/man/2/readlink for /proc/self/exe
#endif

	const size_t last_slash_idx = directory.rfind('\\');
	if (std::string::npos != last_slash_idx)
	{
		directory = directory.substr(0, last_slash_idx);
	}
	return FormatPath(directory);
}

inline std::string GetExecutableExtension()
{
#ifdef _WIN32
	return ".exe";
#else ifdef LINUX
	//TODO implement
#endif
}

inline bool DirectoryExists(std::string path)
{
	struct stat info;
	return stat(path.c_str(), &info) == 0;
}

inline void CreateDirectoryTree(std::string path)
{
	if(DirectoryExists(path)) return;

	std::string current_path = FormatPath(path);
	std::string to_check;
	while(true)
	{
		to_check = current_path.substr(0, current_path.find_last_of("/\\"));
		if (DirectoryExists(to_check))
		{
#ifdef _WIN32
			CreateDirectoryA(current_path.c_str(), nullptr);
#else ifdef LINUX
			//TODO implement directory creation
#endif

			if (current_path == FormatPath(path)) return;

			current_path = FormatPath(path);
		} 
		else
		{
			current_path = to_check;
		}
	}
}

inline std::string GetPreferencesPath()
{
	std::string base_dir;

#ifdef _WIN32
	base_dir = getenv("APPDATA");
#else ifdef LINUX
	base_dir = "~\\Library\\Preferences";
#endif

	base_dir += "\\DataWise";

	CreateDirectoryTree(base_dir);

	return FormatPath(base_dir);
}

inline std::vector<std::string> FindFiles(std::string path, std::string pattern = "*.*")
{
#ifdef _WIN32
	std::vector<std::string> result;

	std::string search_path = path + "\\" + pattern;

	WIN32_FIND_DATA data;
	HANDLE handle;
	if ((handle = FindFirstFile(search_path.c_str(), &data)) != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			result.push_back(data.cFileName);
		} while (FindNextFile(handle, &data) != 0);
		FindClose(handle);
	}

	return result;
#else
	//TODO implement
	return std::vector<std::string>();
#endif
}

inline void RunExecutable(std::string file)
{

#ifdef _WIN32
	system(("start " + file).c_str());
#else ifdef LINUX
	//TODO implement
#endif
}