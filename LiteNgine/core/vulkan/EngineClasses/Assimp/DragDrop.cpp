#include "DragDrop.h"
#include "../Lt_Console.h"
#include "../../Editor/DockingWindows.h"
#include <filesystem>
namespace lte
{
    void DragDrop::OnFilesDropped(const std::vector<std::string>& paths) {
        SubOp fileproc{ "file processing","from drag and drop operation" };
        for (const auto& path : paths) {
            fileproc.Log("processing file" + path, TAG_ENGINE);
            std::filesystem::path fspath = path;
            FileManager::moveFile(fspath);
        }
        fileproc.~SubOp();
    }
}