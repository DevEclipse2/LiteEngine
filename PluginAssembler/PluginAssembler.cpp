// PluginAssembler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
namespace fs = std::filesystem;

#ifdef _WIN32
#include <windows.h>
#endif

void enableAnsiSupport() {
#ifdef _WIN32
    // Fetch the standard output handle
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    // Fetch current console mode
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;

    // Enable virtual terminal processing (ANSI escape sequences)
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}

//remember to use abi stuff to extract internal name, abi version etc.


struct PluginMetadata {
    std::string abiVersion = "223";
    std::string displayName = "MyPlugin";
    std::string internalName = "plugin_core";
    std::string description = "Plugin description";
    std::string engineSupportedMin = "0";
    std::string engineSupportedMax = "1";
    std::string fallbackResponse = "failskip";
    std::string loadFailResponse = "failwarn";
    std::string revMajor = "1";
    std::string revMinor = "0";
    std::string revPatch = "0";
};

std::string escapeJson(const std::string& input) {
    std::string output;
    for (char c : input) {
        switch (c) {
        case '\"': output += "\\\""; break;
        case '\\': output += "\\\\"; break;
        case '\b': output += "\\b";  break;
        case '\f': output += "\\f";  break;
        case '\n': output += "\\n";  break;
        case '\r': output += "\\r";  break;
        case '\t': output += "\\t";  break;
        default:   output += c;      break;
        }
    }
    return output;
}

std::vector<std::string> scanDependencies(const fs::path& targetDir, const std::string& manifestFilename = "plugin.json") {
    std::vector<std::string> dependencyPaths;

    if (!fs::exists(targetDir) || !fs::is_directory(targetDir)) {
        std::cerr << "[Error] Target path is not a valid directory: " << targetDir << '\n';
        return dependencyPaths;
    }

    for (const auto& entry : fs::recursive_directory_iterator(targetDir)) {
        if (entry.is_regular_file()) {
            // Exclude the manifest file itself if generated in the same directory
            if (entry.path().filename() == manifestFilename) {
                continue;
            }

            // Store relative normalized paths using forward slashes
            fs::path relativePath = fs::relative(entry.path(), targetDir);
            dependencyPaths.push_back(relativePath.generic_string());
        }
    }

    return dependencyPaths;
}

bool generateManifest(const fs::path& targetDir, const PluginMetadata& meta, const fs::path& outputPath) {
    const std::vector<std::string> internalPaths = scanDependencies(targetDir, outputPath.filename().string());

    std::ofstream out(outputPath);
    if (!out.is_open()) {
        std::cerr << "[Error] Failed to open " << outputPath << " for writing.\n";
        return false;
    }

    out << "{\n";
    out << "  \"ABI_version\": \"" << escapeJson(meta.abiVersion) << "\",\n";
    out << "  \"display_name\": \"" << escapeJson(meta.displayName) << "\",\n";
    out << "  \"Internal_name\": \"" << escapeJson(meta.internalName) << "\",\n";
    out << "  \"description\": \"" << escapeJson(meta.description) << "\",\n";
    out << "  \"engine_supported_min\": \"" << escapeJson(meta.engineSupportedMin) << "\",\n";
    out << "  \"engine_supported_max\": \"" << escapeJson(meta.engineSupportedMax) << "\",\n";
    out << "  \"fallback_response\": \"" << escapeJson(meta.fallbackResponse) << "\",\n";
    out << "  \"load_fail_response\": \"" << escapeJson(meta.loadFailResponse) << "\",\n";
    out << "  \"Internal_Revision_Major\": \"" << escapeJson(meta.revMajor) << "\",\n";
    out << "  \"Internal_Revision_Minor\": \"" << escapeJson(meta.revMinor) << "\",\n";
    out << "  \"Internal_Revision_Patch\": \"" << escapeJson(meta.revPatch) << "\",\n";

    out << "  \"Dependencies\": {\n";
    out << "    \"Soft_dependencies_silent\": [],\n";
    out << "    \"Soft_dependencies_warn\": [],\n";
    out << "    \"Hard_dependencies\": []\n";
    out << "  },\n";

    out << "  \"Dependencies_version_requirement\": {\n";
    out << "    \"Min_inclusive\": {\n";
    out << "      \"Soft_silent\": [],\n";
    out << "      \"Soft_warn\": [],\n";
    out << "      \"Hard\": []\n";
    out << "    },\n";
    out << "    \"Max_inclusive\": {\n";
    out << "      \"Soft_silent\": [],\n";
    out << "      \"Soft_warn\": [],\n";
    out << "      \"Hard\": []\n";
    out << "    }\n";
    out << "  },\n";

    out << "  \"internal_dependency_pathes\": [\n";
    for (size_t i = 0; i < internalPaths.size(); ++i) {
        out << "    \"" << escapeJson(internalPaths[i]) << "\"";
        if (i + 1 < internalPaths.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "  ],\n";

    out << "  \"dep_missing_response\": [\n";
    out << "    \"warn\",\n";
    out << "    \"silent\",\n";
    out << "    \"throw\"\n";
    out << "  ]\n";
    out << "}\n";

    return true;
}

// Integration hook into your command loop
void runManifestCommand(const std::string& inputDir) {
    fs::path targetPath(inputDir);
    fs::path manifestOut = targetPath / "plugin.LiteMeta"; // or plugin.json

    PluginMetadata meta;
    meta.displayName = targetPath.filename().string();
    meta.internalName = targetPath.filename().string();

    if (generateManifest(targetPath, meta, manifestOut)) {
        std::cout << "[liteNgine Assembler] Successfully generated manifest at: " << manifestOut << '\n';
    }
}



//need to include the plugin abi to ask for the version

// --- State Definitions ---
enum class AppState {
    Page0_Startup,
    Page1_Metadata,
    Page2_ExtDeps,
    Page3_IntDeps,
    Page4_Confirm,
    Exit
};

// --- Data Structures ---
struct ExternalDependencies {
    std::vector<std::string> softSilent;
    std::vector<std::string> softWarn;
    std::vector<std::string> hard;
};

struct AssemblerState {
    // Page 1
    std::string displayName = "NewPlugin";
    std::string internalName = "plugin_core";
    std::string description = "Description here";
    std::string engineMin = "0";
    std::string engineMax = "1";
    std::string fallbackResp = "failskip";
    std::string loadFailResp = "failwarn";
    std::string revMaj = "1", revMin = "0", revPatch = "0";

    // Page 2
    ExternalDependencies extDeps;

    // Page 3
    fs::path targetDir;
    std::vector<std::string> internalFiles;
    // Maps a file path to its dependency response: 0=warn, 1=silent, 2=throw, 3=Exclude
    std::map<std::string, int> fileDepLevels;
};

// --- Helper Functions ---
void clearScreen() {
    // Cross-platform clear screen using ANSI escape codes
    std::cout << "\033[2J\033[1;1H";
}

void awaitEnter() {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(10000, '\n');
    std::cin.get();
}

std::string promptString(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::cin >> std::ws; // Clear leading whitespace
    std::getline(std::cin, input);
    return input;
}

// --- Page Implementations ---

void renderPage0(AssemblerState& data, AppState& nextState)
{
    clearScreen();
    std::string inputbuf = "";
    std::cout << "Welcome to the liteNgine Assembler!\n This program is intended to assist the user in assembling their plugins for liteNgine\n";
    std::cout << "This program can help the user sort out dependancies for their plugin, as well as generating the .LiteMeta manifest for the plugins!\n";
    std::cout << "The assembler also helps users add additional binaries dependancies for multiple Addons to share\n";
    std::cout << "To begin, make sure all files are placed within clean directories, as all files will be treated as dependancies, even the ones that contain your childhood photos and the funny numbers at the back of your credit card \n";
    std::cout << "Commands Available : " << std::endl;
    std::cout << "!help              : lists available commands, add help to the back of other commands to get specific tips" << std::endl;
    std::cout << "!back              : the fix all to everything. actually no. the fix all is restarting the program" << std::endl;
    std::cout << "!about             : what is it about anyways?" << std::endl;
    std::cout << "!generateTemplate  : allows user to input path to a .LiteMeta file that acts as a template for the new one" << std::endl;
    std::cout << "!manidoc           : runs a quick check and diagnosis of problematic .LiteMeta file" << std::endl;
    std::cout << "!fixer             : attempts repair of problematic .LiteMeta file " << std::endl;
    std::cout << "!builder           : the actual system for the plugin" << std::endl;
    std::cout << "Input the file directory below or use \"!\" to specify a command: " << std::endl;
    std::cin >> inputbuf;
    if (inputbuf.size() > 0 && inputbuf[0] == '!')
    {
        std::cout << "we're missing commands for now, press enter carat to return to main" << std::endl;
        std::cin >> inputbuf;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

    }
    else
    {
        std::cout << "using input as a valid directory path. if you believe this is a mistake, return to the main screen" << std::endl;
        data.targetDir = inputbuf;
        nextState = AppState::Page1_Metadata;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void renderPage1(AssemblerState& data, AppState& nextState) {
    clearScreen();
    std::cout << "=== [Page 1/4] Plugin Metadata ===\n";
    std::cout << "[1] Display Name       : " << data.displayName << "\n";
    std::cout << "[2] Internal Name      : " << data.internalName << "\n";
    std::cout << "[3] Description        : " << data.description << "\n";
    std::cout << "[4] Engine Support Min : " << data.engineMin << "\n";
    std::cout << "[5] Engine Support Max : " << data.engineMax << "\n";
    std::cout << "[6] Fallback Response  : " << data.fallbackResp << "\n";
    std::cout << "[7] Load Fail Response : " << data.loadFailResp << "\n";
    std::cout << "[8] Revision (M/m/p)   : " << data.revMaj << "." << data.revMin << "." << data.revPatch << "\n";
    std::cout << "----------------------------------\n";
    std::cout << "[N] Next Page   [Q] Quit  [B] Back\n\n";

    std::cout << "Select item to edit (or N to proceed): ";
    std::string choice;
    std::cin >> choice;

    if (choice == "1") data.displayName = promptString("New Display Name: ");
    else if (choice == "2") data.internalName = promptString("New Internal Name: ");
    else if (choice == "3") data.description = promptString("New Description: ");
    else if (choice == "4") data.engineMin = promptString("New Engine Min: ");
    else if (choice == "5") data.engineMax = promptString("New Engine Max: ");
    else if (choice == "6") data.fallbackResp = promptString("Fallback (failskip/failerror/failthrow): ");
    else if (choice == "7") data.loadFailResp = promptString("Load Fail (failwarn/failthrow): ");
    else if (choice == "8") {
        data.revMaj = promptString("Major: ");
        data.revMin = promptString("Minor: ");
        data.revPatch = promptString("Patch: ");
    }
    else if (choice == "N" || choice == "n") nextState = AppState::Page2_ExtDeps;
    else if (choice == "Q" || choice == "q") nextState = AppState::Exit;
    else if (choice == "B") nextState = AppState::Page0_Startup;
}

void renderPage2(AssemblerState& data, AppState& nextState) {
    clearScreen();
    std::cout << "=== [Page 2/4] External Dependencies ===\n";

    std::cout << "Soft Silent: ";
    for (const auto& d : data.extDeps.softSilent) std::cout << "[" << d << "] ";
    std::cout << "\nSoft Warn  : ";
    for (const auto& d : data.extDeps.softWarn) std::cout << "[" << d << "] ";
    std::cout << "\nHard       : ";
    for (const auto& d : data.extDeps.hard) std::cout << "[" << d << "] ";
    std::cout << "\n----------------------------------------\n";

    std::cout << "[1] Add Soft Silent Dependency\n";
    std::cout << "[2] Add Soft Warn Dependency\n";
    std::cout << "[3] Add Hard Dependency\n";
    std::cout << "[C] Clear All\n";
    std::cout << "[B] Back   [N] Next Page   [Q] Quit\n\n";

    std::cout << "Select action: ";
    std::string choice;
    std::cin >> choice;

    if (choice == "1") data.extDeps.softSilent.push_back(promptString("Dependency Name: "));
    else if (choice == "2") data.extDeps.softWarn.push_back(promptString("Dependency Name: "));
    else if (choice == "3") data.extDeps.hard.push_back(promptString("Dependency Name: "));
    else if (choice == "C" || choice == "c") {
        data.extDeps.softSilent.clear(); data.extDeps.softWarn.clear(); data.extDeps.hard.clear();
    }
    else if (choice == "B" || choice == "b") nextState = AppState::Page1_Metadata;
    else if (choice == "N" || choice == "n") nextState = AppState::Page3_IntDeps;
    else if (choice == "Q" || choice == "q") nextState = AppState::Exit;
}

void renderPage3(AssemblerState& data, AppState& nextState) {
    clearScreen();
    std::cout << "=== [Page 3/4] Internal File Dependencies ===\n";
    std::cout << "Target Directory: " << data.targetDir.string() << "\n\n";

    const char* levelStrs[] = { "WARN", "SILENT", "THROW", "EXCLUDE" };

    for (size_t i = 0; i < data.internalFiles.size(); ++i) {
        int lvl = data.fileDepLevels[data.internalFiles[i]];
        std::cout << "[" << i << "] " << levelStrs[lvl] << " \t| " << data.internalFiles[i] << "\n";
    }
    std::cout << "---------------------------------------------\n";
    std::cout << "Type a file number to cycle its dependency level (Warn -> Silent -> Throw -> Exclude).\n";
    std::cout << "[B] Back   [N] Next Page   [Q] Quit\n\n";

    std::cout << "Action: ";
    std::string choice;
    std::cin >> choice;

    if (choice == "B" || choice == "b") nextState = AppState::Page2_ExtDeps;
    else if (choice == "N" || choice == "n") nextState = AppState::Page4_Confirm;
    else if (choice == "Q" || choice == "q") nextState = AppState::Exit;
    else {
        try {
            size_t idx = std::stoull(choice);
            if (idx < data.internalFiles.size()) {
                // Cycle the enum state (0, 1, 2, 3)
                int& lvl = data.fileDepLevels[data.internalFiles[idx]];
                lvl = (lvl + 1) % 4;
            }
        }
        catch (...) { /* Ignore invalid non-numeric input */ }
    }
}

void renderPage4(AssemblerState& data, AppState& nextState) {
    clearScreen();
    std::cout << "=== [Page 4/4] Build Configuration ===\n";
    std::cout << "Ready to assemble liteNgine plugin: " << data.displayName << "\n\n";
    std::cout << "Included Internal Files : " << data.internalFiles.size() << "\n";
    std::cout << "Total External Deps     : " << (data.extDeps.hard.size() + data.extDeps.softSilent.size() + data.extDeps.softWarn.size()) << "\n";
    std::cout << "--------------------------------------\n";
    std::cout << "[1] Build as .zip Archive\n";
    std::cout << "[2] Build as Binary Payload (.litePak)\n";
    std::cout << "[B] Back   [Q] Quit Without Building\n\n";

    std::cout << "Select Output Format: ";
    std::string choice;
    std::cin >> choice;

    if (choice == "1") {
        std::cout << "\n[OK] Zipping dependencies into plugin.zip...\n";
        // TODO: Call miniz or libzip logic here
        awaitEnter();
        nextState = AppState::Exit;
    }
    else if (choice == "2") {
        std::cout << "\n[OK] Packing dependencies into custom binary format...\n";
        // TODO: Call custom binary packaging logic here
        awaitEnter();
        nextState = AppState::Exit;
    }
    else if (choice == "B" || choice == "b") nextState = AppState::Page3_IntDeps;
    else if (choice == "Q" || choice == "q") nextState = AppState::Exit;
}

void handleCommand(std::string& command)
{

}

// --- Main Engine Bootstrapper ---
int main() {
    
    enableAnsiSupport();
    AssemblerState sessionData;
    // Mocking file population for Page 3 visualization
    sessionData.internalFiles = { "shaders/lit.frag", "shaders/lit.vert", "scripts/ik_solver.lua", "assets/model.obj" };

    for (const auto& file : sessionData.internalFiles) {
        sessionData.fileDepLevels[file] = 0; // Default to WARN
    }

    AppState currentState = AppState::Page0_Startup;

    while (currentState != AppState::Exit) {
        switch (currentState) {
        case AppState::Page0_Startup:   renderPage0(sessionData, currentState); break;
        case AppState::Page1_Metadata:  renderPage1(sessionData, currentState); break;
        case AppState::Page2_ExtDeps:   renderPage2(sessionData, currentState); break;
        case AppState::Page3_IntDeps:   renderPage3(sessionData, currentState); break;
        case AppState::Page4_Confirm:   renderPage4(sessionData, currentState); break;
        default: break;
        }
    }

    clearScreen();
    std::cout << "Exiting liteNgine Assembler. Goodbye!\n";
    return 0;
}