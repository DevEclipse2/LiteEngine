#include "DockingWindows.h"

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
#include "global_data.h"
#include <set>
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
        DrawFileExplorer();
        
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
                FILEOPENDIALOGOPTIONS options;
                hr = pFileOpen->GetOptions(&options);
                if (SUCCEEDED(hr)) {
                    // FOS_PICKFOLDERS allows folder selection
                    // FOS_FORCEFILESYSTEM ensures it's a real file system object, not a virtual one
                    pFileOpen->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
                }

                // 4. Set dialog title
                pFileOpen->SetTitle(L"Select a File or a Folder");

                // 5. Show the Dialog
                hr = pFileOpen->Show(NULL);

                // 6. Process the result if the user clicked "Open / Select"
                if (SUCCEEDED(hr)) {
                    IShellItem* pItem = nullptr;
                    hr = pFileOpen->GetResult(&pItem);

                    if (SUCCEEDED(hr)) {
                        PWSTR pszFilePath = nullptr;
                        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                        if (SUCCEEDED(hr)) {
                            // Success! Display or use the path
                            std::wcout << L"Selected Path: " << pszFilePath << std::endl;
                            filePath = pszFilePath;
                            // Free the string allocated by the shell
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
        std::vector<fs::path> filepathes = getDocumentsProject(GetOSDocumentPath());
        for (const auto& fp : filepathes)
        {
            m_DocumentProjects.push_back(fp.string());
        }
        m_IsOpen = true;
    }

    inline void FileManager::DrawStartpopUp()
    {
        SubOp StartPopUpOperation{"StartPopUP","a Imgui popup for the start menu, showing a list of projects to choose from. "};
        ImGui::SetNextWindowSize(ImVec2(800, 500), ImGuiCond_Appearing);
        if (selectedDirectory) return;
        m_IsOpen = true;
        if (ImGui::BeginPopupModal("Start or Open Project", &m_IsOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {

            // --- Top Bar: Search and Browse ---
            ImGui::SetNextItemWidth(300);
            ImGui::InputTextWithHint("##Search", "Search projects...", m_SearchBuffer, sizeof(m_SearchBuffer));
            ImGui::SameLine();
            if (ImGui::Button("Browse File Explorer...")) {
                // Trigger OS File Explorer here
                std::wstring filepath = L"";
                StartPopUpOperation.Log("browse file explorer", TAG_ENGINE);
#if defined(PLATFORM_WINDOWS)
                std::cout << "Running on Windows" << std::endl;
                filepath = OpenFileDialog();                
#elif defined(PLATFORM_LINUX)
                std::cout << "Running on Linux" << std::endl;
                filepath = StringToWString(OpenLinuxFileDialog());
#else 
                std::cout << "Running on an unsupported platform" << std::endl;
                StartPopUpOperation.LogError("unsupported platform!",CRIT_SEVERITY, TAG_ENGINE);
#endif
                if (filepath != L"")
                {
                    if (!fs::exists(filepath)) {
                        StartPopUpOperation.LogError("what. how could you have found the path in the system directory but not the engine", HIGH_SEVERITY, TAG_ENGINE);
                        return;
                    }
                    //check if its a directory or a file
                    //if its a directory, check for the .lite file
                    //if its a file, see if its a .lite
                    if (fs::is_directory(filepath)) {
                        
                        StartPopUpOperation.Log("user chose directory", TAG_USER);

                        bool found = false;
                        // Iterate through files inside the directory
                        for (const auto& entry : fs::directory_iterator(filepath)) {
                            if (entry.is_regular_file() && entry.path().extension() == ".lite") {
                                StartPopUpOperation.LogSuccess("found .lite file!", TAG_USER);
                                found = true;
                                ProjectDirectory = wstring_to_string(filepath);
                                m_RecentProjects.emplace_back(ProjectDirectory);
                                selectedDirectory = true;
                                SaveCache();
                            }
                        }
                        if (!found) {
                            StartPopUpOperation.LogFailure("didnt find .lite file!",HIGH_SEVERITY,TAG_ENGINE);
                        }
                    }
                    // 3. Check if it is a regular file
                    else if (fs::is_regular_file(filepath)) {
                        StartPopUpOperation.Log("user chose file", TAG_USER);

                        fs::path p = filepath;
                        if (p.extension() == ".lite") {
                            StartPopUpOperation.LogSuccess("extension correct!", TAG_USER);
                            ProjectDirectory = p.parent_path().string();
                            m_RecentProjects.emplace_back(ProjectDirectory);
                            selectedDirectory = true;
                            SaveCache();

                        }
                        else {
                            StartPopUpOperation.LogFailure("Incorrect extension. Expected .lite", HIGH_SEVERITY, TAG_ENGINE);
                        }
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("New Project")) {
                StartPopUpOperation.LogError("Some goober forgot to implement this", CRIT_SEVERITY, TAG_ENGINE);
                // Logic to create a new folder and .lite file
            }
            if (ImGui::Button("Select default directory")) {

                StartPopUpOperation.LogEvent("user select new default directory",TAG_ENGINE);
                // again trigger os file explorer
                std::wstring filepath = L"";
#if defined(PLATFORM_WINDOWS)
                std::cout << "Running on Windows" << std::endl;
                filepath = OpenFileDialog();
#elif defined(PLATFORM_LINUX)
                std::cout << "Running on Linux" << std::endl;
                filepath = StringToWString(OpenLinuxFileDialog());
#else 
                std::cout << "Running on an unsupported platform" << std::endl;
                StartPopUpOperation.LogError("unsupported platform!", CRIT_SEVERITY, TAG_ENGINE);
#endif
                if (filepath != L"")
                {
                    m_DefaultSearchDir = wstring_to_string(filepath);
                }
                SaveCache();
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
                    selectedDirectory = true;
                    ProjectDirectory = p.parent_path().string();

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

                    selectedDirectory = true;
                    ProjectDirectory = p.parent_path().string();
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

    std::vector<fs::path> FileManager::getDocumentsProject(const fs::path& root)
    {
        std::set<fs::path> unique_dirs;

        try {
            //ensure the root path exists and is a directory
            if (!fs::exists(root) || !fs::is_directory(root)) {
                return {};
            }

            for (const auto& entry : fs::recursive_directory_iterator(root)) {
                if (entry.is_regular_file() && entry.path().extension() == ".lite") {
                    // Add the parent directory of the .lite file to our set
                    unique_dirs.insert(entry.path().parent_path());
                }
            }
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << '\n';
        }

        //convert the unique set of directories back into a vector
        return std::vector<fs::path>(unique_dirs.begin(), unique_dirs.end());
    }
}