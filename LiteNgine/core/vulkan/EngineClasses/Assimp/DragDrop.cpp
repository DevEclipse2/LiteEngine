#include "DragDrop.h"
namespace lte
{
    void DragDrop::OnFilesDropped(const std::vector<std::string>& paths) {
        for (const auto& path : paths) {
            std::cout << "Processing dropped file: " << path << std::endl;

            //TODO: Pass path to your Assimp loader, texture loader, script parser, etc.
        }
    }
}