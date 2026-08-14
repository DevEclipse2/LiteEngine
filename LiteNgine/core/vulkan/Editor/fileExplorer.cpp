#include "DockingWindows.h"
#include "global_data.h"
namespace lte
{
	void FileManager::DrawFileExplorer()
	{
        ImGui::Begin("Content Browser");

        // Create a 2-column table for the layout
        if (ImGui::BeginTable("BrowserTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable)) {

            // Left Pane: Directory Tree
            ImGui::TableNextColumn();
            DrawDirectoryTree(ProjectDirectory);

            // Right Pane: Files and Folders
            ImGui::TableNextColumn();
            DrawFileBrowser();

            ImGui::EndTable();
        }

        // Handle floating popups for creating/renaming outside the table structure
        DrawContextMenus();

        ImGui::End();
	}
    void FileManager::DrawContextMenus() 
    {
        SubOp ContextMenu{ "context menu","from file explorer" };
        if (m_IsRenaming) ImGui::OpenPopup("Rename Item");
        if (m_IsCreatingFolder) ImGui::OpenPopup("New Folder");
        if (m_IsCreatingFile) ImGui::OpenPopup("New File");

        if (ImGui::BeginPopupModal("Rename Item", &m_IsRenaming, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("New Name", m_InputBuffer, sizeof(m_InputBuffer));
            if (ImGui::Button("Apply") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
                ContextMenu.Log("Rename file", TAG_ENGINE);
                fs::path newPath = m_SelectedFile.parent_path() / m_InputBuffer;
                fs::rename(m_SelectedFile, newPath);
                m_SelectedFile = newPath;
                m_IsRenaming = false;
            }
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupModal("New Folder", &m_IsCreatingFolder, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("Folder Name", m_InputBuffer, sizeof(m_InputBuffer));
            if (ImGui::Button("Create") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
                ContextMenu.Log("Create folder", TAG_ENGINE);

                fs::create_directory(m_CurrentPath / m_InputBuffer);
                m_IsCreatingFolder = false;
            }
            ImGui::EndPopup();
        }
        if (ImGui::BeginPopupModal("New File", &m_IsCreatingFile, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("File Name", m_InputBuffer, sizeof(m_InputBuffer));
            if (ImGui::Button("Create") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
                ContextMenu.Log("Create File", TAG_ENGINE);

                std::ofstream outputFile(m_CurrentPath / m_InputBuffer);

                if (!outputFile.is_open()) {
                    ContextMenu.LogError("Failed to open file!", HIGH_SEVERITY, TAG_ENGINE);
                }
                outputFile << "-- LiteNgine!" << std::endl;
                outputFile.close();
                m_IsCreatingFile = false;
            }
            ImGui::EndPopup();
        }
        //use std ofstream to make empty file
        ContextMenu.~SubOp();
    }
    void FileManager::DrawDirectoryTree(const fs::path& directoryPath) 
    {
        if (directoryPath == "")return;
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        
        //highlight if this is the directory currently being viewed in the right pane
        if (m_CurrentPath == directoryPath) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        //extract the folder name
        std::string folderName = directoryPath.filename().string();
        if (folderName.empty()) folderName = directoryPath.string(); // Fallback for root drives

        bool isOpen = ImGui::TreeNodeEx(directoryPath.string().c_str(), flags, "%s", folderName.c_str());

        //if clicked, update the right pane to show this folder's contents
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            m_CurrentPath = directoryPath;
            m_SelectedFile = ""; // Clear selection when changing folders
        }

        if (isOpen) {
            for (const auto& entry : fs::directory_iterator(directoryPath)) {
                if (entry.is_directory()) {
                    DrawDirectoryTree(entry.path());
                }
            }
            ImGui::TreePop();
        }
    }

    void FileManager::DrawFileBrowser() {
        //top bar showing current path
        ImGui::Text("%s", m_CurrentPath.string().c_str());
        ImGui::Separator();

        //right-click on empty space (Create New...)
        if (ImGui::BeginPopupContextWindow("RightPaneContext")) {
            if (ImGui::MenuItem("New Folder")) { m_IsCreatingFolder = true; m_InputBuffer[0] = '\0'; }
            if (ImGui::MenuItem("New File")) { m_IsCreatingFile = true; m_InputBuffer[0] = '\0'; }
            ImGui::EndPopup();
        }
        //list 
        if (m_CurrentPath == L"")
        {
            if (ProjectDirectory == "") return;
            else m_CurrentPath = ProjectDirectory;
        }
        for (const auto& entry : fs::directory_iterator(m_CurrentPath)) {
            const auto& path = entry.path();
            auto relativePath = fs::relative(path, ProjectDirectory);
            std::string filenameString = relativePath.filename().string();

            //shows file or folder
            std::string displayPrefix = entry.is_directory() ? "[Folder] " : "[File] ";

            bool isSelected = (m_SelectedFile == path);
            if (ImGui::Selectable((displayPrefix + filenameString).c_str(), isSelected)) {
                m_SelectedFile = path;
            }

            // Double-click to enter a folder
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (entry.is_directory()) {
                    m_CurrentPath = path;
                    m_SelectedFile = "";
                }
                else{
                    fs::path filepath = path;
                    if (filepath.has_extension())
                    {
                        if (filepath.extension() == ".lua")
                        {
                            ShellExecuteW(NULL, L"open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                        }
                    }
                }
            }

            //shows context menu.
            if (ImGui::BeginPopupContextItem(filenameString.c_str())) {
                m_SelectedFile = path; // Ensure right-clicked item is selected
                if (ImGui::MenuItem("Rename")) {
                    m_IsRenaming = true;
                    strncpy(m_InputBuffer, filenameString.c_str(), sizeof(m_InputBuffer));
                }
                if (ImGui::MenuItem("Show in Files")) {
                    fs::path absolutePath = fs::absolute(m_SelectedFile);
                    
                    absolutePath.make_preferred();
                    //converts path into preferred format for digestion
                    std::string command;
                    bool isFile = fs::is_regular_file(absolutePath);
                    //wrap in quotes for security
#if defined(_WIN32)
                    if (isFile) {
                        // Opens the folder AND highlights the file
                        command = "explorer /select,\"" + absolutePath.string() + "\"";
                    }
                    else {
                        // Just opens the directory
                        command = "explorer \"" + absolutePath.string() + "\"";
                    }

#elif defined(__APPLE__)
                    if (isFile) {
                        // The -R flag in macOS reveals/highlights the file in Finder
                        command = "open -R \"" + absolutePath.string() + "\"";
                    }
                    else {
                        command = "open \"" + absolutePath.string() + "\"";
                    }

#elif defined(__linux__)
    // standard xdg-open doesn't have a universal "highlight file" flag.
    // If it's a file, we extract its enclosing folder using parent_path()
                    fs::path directoryToOpen = isFile ? absolutePath.parent_path() : absolutePath;
                    command = "xdg-open \"" + directoryToOpen.string() + "\"";
#else
                    std::cerr << "Unsupported operating system.\n";
                    return;
#endif
                    std::system(command.c_str());
                }
                //other stuff here later
                ImGui::EndPopup();
            }
            

        }
    }
    void FileManager::moveFile(fs::path& source)
    {
        SubOp MoveFiles{ "Moving files","drag and drop handler" };
        if (!fs::exists(source)) {
            MoveFiles.LogError("Src path missing!", MED_SEVERITY, TAG_ENGINE);
        }
        fs::path destinationDirectory = m_CurrentPath;
        if (m_CurrentPath == L"")
        {
            MoveFiles.LogError("CurrentPath is not yet initalised!", HIGH_SEVERITY, TAG_ENGINE);
            return;
        }
        fs::create_directories(destinationDirectory);
        fs::path finalDestination = destinationDirectory / source.filename();

        try {
            //OS-level move (works for files AND whole directories)
            fs::rename(source, finalDestination);
            MoveFiles.LogSuccess("Successfully did copy ", TAG_ENGINE);
        }
        catch (const fs::filesystem_error& e) {
            // Error code 18 (EXDEV) means cross-device link (different hard drives)
            // If it fails for this or any other reason, fallback to deep copy + delete
            std::string error = e.what();
            MoveFiles.LogFailure("Rename failed (" + error + "). Falling back to deep copy...", LOW_SEVERITY, TAG_ENGINE);
            try {
                // copy_options::recursive ensures every sub-folder and file is copied
                fs::copy(
                    source,
                    finalDestination,
                    fs::copy_options::recursive | fs::copy_options::overwrite_existing
                );

                // Once the copy is successful, delete the original
                fs::remove_all(source);
                MoveFiles.LogSuccess("deep copy successful!", TAG_ENGINE);
            }
            catch (const fs::filesystem_error& copyErr) {
                error = copyErr.what();
                MoveFiles.LogFailure("Fatal error: " + error, HIGH_SEVERITY, TAG_ENGINE);
            }
        }
        MoveFiles.~SubOp();
    }
}