#include "DockingWindows.h"
#ifdef _WIN32 || _WIN64
#include <windows.h>
#include <shlobj.h>
#endif

#include <iostream>
#include <cstdio>
#include <memory>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#elif defined(__linux__) || defined(__gnu_linux__)
#define PLATFORM_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
#define PLATFORM_MACOS 1
#else
#define PLATFORM_UNKNOWN 1
#endif

#if defined(_MSC_VER) || defined(_WIN32)
#define popen _popen
#define pclose _pclose
#endif

#include <combaseapi.h>
#include <fstream>
#include "../EngineClasses/Lt_Console.h"
namespace lte{
    namespace fs = std::filesystem;
	void lte::FileManager::SubmitGUICommands()
	{
        if (!selectedDirectory)
        {
            //if no directory open popup
            //DrawStartpopUp();
            ImGui::OpenPopup("Start or Open Project");
        }
        DrawStartpopUp();

        /*
        ImGui::Begin("Content Browser");

        // Create a 2-column table for the layout
        if (ImGui::BeginTable("BrowserTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable)) {

            // Left Pane: Directory Tree
            ImGui::TableNextColumn();
            DrawDirectoryTree(m_DirPath + m_AddPath);

            // Right Pane: Files and Folders
            ImGui::TableNextColumn();
            DrawFileBrowser();
            ImGui::EndTable();
        }
       
        // Handle floating popups for creating/renaming outside the table structure
        DrawContextMenus();
        ImGui::End();
         */
        
	}
    void FileManager::LoadRecentCache() {
        m_RecentProjects.clear();

        // If cache doesn't exist, generate it with the OS Documents folder
        if (!fs::exists(CACHE_FILE)) {
            m_DefaultSearchDir = GetOSDocumentPath();
            SaveCache(); // Automatically creates the file
            return;
        }

        std::ifstream file(CACHE_FILE);
        std::string line;

        // Line 1: Read the default search directory
        if (std::getline(file, line) && !line.empty()) {
            m_DefaultSearchDir = line;
        }
        else {
            // Fallback if the file is corrupted or empty
            m_DefaultSearchDir = GetOSDocumentPath();
        }

        // Line 2+: Read recent projects
        while (std::getline(file, line)) {
            if (!line.empty() && fs::exists(line)) {
                m_RecentProjects.push_back(line);
            }
        }

        m_IndexedDir = true;
    }
    std::string FileManager::OpenLinuxFileDialog() 
    {
        char buffer[128];
        std::string result = "";

        // Command to open native Zenity file selection dialog
        // --file-selection: opens file picker
        // --title: sets window title
        std::string command = "zenity --file-selection --title=\"Select a File\" 2>/dev/null";

        // Open a pipe to execute the command and read stdout
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);

        if (!pipe) {
            std::cerr << "Failed to run zenity command." << std::endl;
            return "";
        }

        // Read the output path from zenity
        while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
            result += buffer;
        }

        // Strip trailing newline character added by the terminal output
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }

        return result; // Returns empty string if user clicked "Cancel"
    }
    std::wstring FileManager::OpenFileDialog()
    {
        //this opens the file dialog
        std::wstring filePath = L"";

        // Initialize the COM library
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        if (SUCCEEDED(hr)) {
            IFileOpenDialog* pFileOpen;

            // Create the FileOpenDialog object
            hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

            if (SUCCEEDED(hr)) {
                // Show the Open dialog box
                hr = pFileOpen->Show(NULL);

                // Get the file name from the dialog box
                if (SUCCEEDED(hr)) {
                    IShellItem* pItem;
                    hr = pFileOpen->GetResult(&pItem);
                    if (SUCCEEDED(hr)) {
                        PWSTR pszFilePath;
                        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                        // Display the file name to the user
                        if (SUCCEEDED(hr)) {
                            filePath = pszFilePath;
                            CoTaskMemFree(pszFilePath);
                        }
                        pItem->Release();
                    }
                }
                pFileOpen->Release();
            }
            CoUninitialize();
        }
        return filePath; //returns empty string if user cancelled
    }
    void FileManager::Init()
    {
        LoadRecentCache();
        m_IsOpen = true;
    }

    inline void FileManager::DrawStartpopUp()
    {
        SubOp StartPopUpOperation{"StartPopUP","a Imgui popup for the start menu, showing a list of projects to choose from"};
        ImGui::SetNextWindowSize(ImVec2(800, 500), ImGuiCond_Appearing);
        m_IsOpen = true;
        if (ImGui::BeginPopupModal("Start or Open Project"/*, &m_IsOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings*/)) {

            // --- Top Bar: Search and Browse ---
            ImGui::SetNextItemWidth(300);
            ImGui::InputTextWithHint("##Search", "Search projects...", m_SearchBuffer, sizeof(m_SearchBuffer));
            ImGui::SameLine();
            if (ImGui::Button("Browse File Explorer...")) {
                // Trigger OS File Explorer here
                StartPopUpOperation.Log("browse file explorer", TAG_ENGINE);

            }
            ImGui::SameLine();
            if (ImGui::Button("New Project")) {
                // Logic to create a new folder and .lite file
            }
            if (ImGui::Button("Select default directory")) {
                // again trigger os file explorer
            }
            ImGui::Separator();

            std::string searchStr = m_SearchBuffer;
            std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

            // --- Left Pane: Recent Projects ---
            ImGui::BeginChild("RecentPane", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, -1), true);
            ImGui::Text("Recent Projects");
            ImGui::Separator();

            for (const auto& path : m_RecentProjects) {
                fs::path p(path);
                std::string folderName = p.filename().string();

                // Search filter
                std::string lowerName = folderName;
                std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                if (!searchStr.empty() && lowerName.find(searchStr) == std::string::npos) continue;

                if (ImGui::Selectable(folderName.c_str())) {

                    //OpenProject(path);
                }
                //this shows teh full path as a tooltip or secondary text
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("%s", path.c_str());
                }
            }
            ImGui::EndChild();

            ImGui::SameLine();

            // --- Right Pane: Documents Directory ---
            ImGui::BeginChild("DocsPane", ImVec2(0, -1), true); // 0 width fills remaining space
            ImGui::Text("Projects in default folder");
            ImGui::Separator();

            for (const auto& path : m_DocumentProjects) {
                fs::path p(path);
                std::string folderName = p.filename().string();

                // Search filter
                std::string lowerName = folderName;
                std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                if (!searchStr.empty() && lowerName.find(searchStr) == std::string::npos) continue;

                if (ImGui::Selectable(folderName.c_str())) {


                    //OpenProject(path);
                }
            }
            ImGui::EndChild();

            //imgui button to select default directory  

            ImGui::EndPopup();
        }
        StartPopUpOperation.~SubOp();
    }

    void FileManager::SaveCache() 
    {
        std::ofstream file(CACHE_FILE);

        // Always write the search directory as the first line
        file << m_DefaultSearchDir.string() << "\n";

        // Write the recent projects below it
        for (const auto& p : m_RecentProjects) {
            file << p << "\n";
        }
    }
    std::filesystem::path FileManager::GetOSDocumentPath() {
#ifdef _WIN32
        PWSTR path = NULL;
        fs::path result;
        // FOLDERID_Documents gets the true user documents folder regardless of OneDrive/locale
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path))) {
            result = path; // std::filesystem::path natively accepts wide strings (PWSTR)
            CoTaskMemFree(path);
        }
        return result;
#else
        // Fallback for Linux / macOS
        const char* home = std::getenv("HOME");
        if (home) {
            return fs::path(home) / "Documents";
        }
        return fs::current_path(); // Absolute fallback
#endif
    }   
}