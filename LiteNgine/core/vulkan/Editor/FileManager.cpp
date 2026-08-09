#include "DockingWindows.h"
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif
#include <combaseapi.h>
namespace lte{
    namespace fs = std::filesystem;
	/*inline void lte::FileManager::SubmitGUICommands()
	{
        if (!m_IndexedDir)
        {
            //here opens cache file and also scans the document directory
        }
        if (!selectedDirectory)
        {
            //if no directory open popup
            DrawStartpopUp();
        }

        ImGui::Begin("Content Browser");

        // Create a 2-column table for the layout
        /*if (ImGui::BeginTable("BrowserTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable)) {

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
    }
    inline void FileManager::DrawStartpopUp()
    {

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
    */
}