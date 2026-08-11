#pragma once
#include "LtUiWindow.h"
#include "backends/imgui.h"
#include <string>
#include <vector>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <shlobj.h>
#endif
#include <filesystem>
#include "../EngineClasses/Lt_Console.h"

//docking windows is the main place for utility windows
namespace lte {	
	namespace fs = std::filesystem;

	class FileManager : LtUiWindow
	{
	public:
		static void SubmitGUICommands();
		static inline std::string m_DirPath = "";
		static inline std::string m_AddPath = "";
		static void Init();//new engine screen to create session or something idk
		static inline void DrawStartpopUp();
		static inline bool selectedDirectory = false;

		inline static std::vector<std::string> m_RecentProjects = {};
		inline static std::vector<std::string> m_DocumentProjects = {};
		//file paths are only 256 max length
		inline static char m_SearchBuffer[256] = "";
		inline static bool m_IsOpen = true;
		inline static bool m_IndexedDir = false;
		inline static fs::path m_DefaultSearchDir = "";
		// A simple text file next to the executable to store recent paths
		inline static const std::string CACHE_FILE = "recent_projects.cache";
		static std::filesystem::path GetOSDocumentPath();
		static void SaveCache();
		static void LoadRecentCache();
		static std::wstring OpenFileDialog();
		static std::string OpenLinuxFileDialog();
		static std::wstring StringToWString(const std::string& str) {
			if (str.empty()) return L"";

			int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);

			std::wstring wstrTo(size_needed, 0);
			MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);

			return wstrTo;
		}
		static std::string wstring_to_string(const std::wstring& wstr) {
			if (wstr.empty()) return "";

			size_t size_needed = std::wcstombs(nullptr, wstr.c_str(), 0);

			if (size_needed == static_cast<size_t>(-1)) {
				Con::LogFailure("Invalid character, wstring to string conversion failed!", HIGH_SEVERITY, TAG_ENGINE);
				return "";
			}

			std::vector<char> buffer(size_needed + 1);
			std::wcstombs(buffer.data(), wstr.c_str(), buffer.size());

			return std::string(buffer.data());
		}

	};


}