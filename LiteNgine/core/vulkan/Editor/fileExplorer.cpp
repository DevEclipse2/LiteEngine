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
        if (m_IsRenaming) ImGui::OpenPopup("Rename Item");
        if (m_IsCreatingFolder) ImGui::OpenPopup("New Folder");
        if (m_IsCreatingFile) ImGui::OpenPopup("New File");

        if (ImGui::BeginPopupModal("Rename Item", &m_IsRenaming, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("New Name", m_InputBuffer, sizeof(m_InputBuffer));
            if (ImGui::Button("Apply") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
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
                fs::create_directory(m_CurrentPath / m_InputBuffer);
                m_IsCreatingFolder = false;
            }
            ImGui::EndPopup();
        }
        //use std ofstream to make empty file
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
        if (m_CurrentPath == "")return;
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
            }

            //shows context menu.
            if (ImGui::BeginPopupContextItem(filenameString.c_str())) {
                m_SelectedFile = path; // Ensure right-clicked item is selected
                if (ImGui::MenuItem("Rename")) {
                    m_IsRenaming = true;
                    strncpy(m_InputBuffer, filenameString.c_str(), sizeof(m_InputBuffer));
                }
                //other stuff here later
                ImGui::EndPopup();
            }

            
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {

                // Convert the absolute path to a string (or relative to your asset folder)
                std::string itemPath = path.string();

                // Package the data. We add +1 to the size to include the null terminator '\0'
                ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath.c_str(), itemPath.size() + 1);

                // Display a tooltip so the user sees what they are dragging
                ImGui::Text("Drop %s into scene", filenameString.c_str());

                ImGui::EndDragDropSource();
            }
            

        }
    }
}