#include "Lt_Console.h"

namespace lte 
{
	std::vector<logEntry> Con::entries = {};
	bool Con::Ready = false;
    uint32_t Con::lastIndex = 0;
    std::string Con::debugBoilerPlate = "";
    std::string Con::newFilename = "";
    std::string Con::oldLogContent = "";
    const std::time_t Con::now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::unordered_map<std::string, uint16_t> Con::SuboperatorIndexes = {};
    std::vector<std::string> Con::Suboperators = {};
    std::vector<std::string> notes = { "" };

	void Con::Init() 
	{
        std::ifstream file("latest_log.txt");

        if (!file.is_open()) {
            std::cerr << "Failed to open debug log file" << std::endl;
        }
        // Read the file's buffer into a stringstream, then to string
       
		//moves content of files to a new one and clears current
		RenameFile(file);
        //more boilerplate here


        std::tm time_info;
        #ifdef _WIN32
            localtime_s(&time_info, &now); // Windows secure alternative
        #else
            localtime_r(&now_c, &time_info); // POSIX secure alternative
        #endif

        // 3. Format into a string safely
        char buffer[26];
        std::strftime(buffer, sizeof(buffer), "%c\n", &time_info);
        AddLog(buffer);
        AddLog("LITENGINE, A BITEBYBYTE SOFTWARE DIVISION PRODUCT");
        Con::OutputFile();
        //so you don't get an empty file
	}

    void Con::Display()
    {
        if (entries.size() > 0 && lastIndex < (entries.size()-1))
        {
            for (uint32_t i = lastIndex; i < entries.size(); i++)
            {
                const auto& logentry = entries[i];
                std::string outputline;
                convertTime(outputline, logentry.time);
                if (logentry.code != 0)
                {
                    std::string outputMsg = "";
                    if (logentry.SubOpID != -1)
                    {
                        outputMsg += "suboperator " + Suboperators[logentry.SubOpID];
                    }
                    if (logentry.AdditionalNotes != -1 && logentry.AdditionalNotes != 0)
                    {
                        outputMsg += "note :" + notes[logentry.AdditionalNotes];
                    }
                    outputMsg += logentry.message;
                    switch (logentry.type) {
                    case TYPE_ERROR:
                        outputline += " error: " + convertSeverity(logentry.severity) + "\t" + convertTags(logentry.tags) + outputMsg;
                        std::cout << "\033[31m" + outputline + "\033[0m" << std::endl;
                        break;
                    case TYPE_EVENT:
                        outputline += " event: " + convertTags(logentry.tags) + logentry.message;
                        std::cout << "\033[105m" + outputline + "\033[0m" << std::endl;
                        break;
                    case TYPE_FAILURE:
                        outputline += " failed: " + convertSeverity(logentry.severity) + "\t" + convertTags(logentry.tags) + outputMsg;
                        std::cout << "\033[41m" + outputline + "\033[0m" << std::endl;

                        break;
                    case TYPE_INFORMATION:
                        outputline += " info: " + convertTags(logentry.tags) + outputMsg;
                        std::cout << "\033[4:2m" + outputline + "\033[0m" << std::endl;
                        break;
                    case TYPE_SUBOPTIMAL:
                        outputline += " suboptimal usage: " + convertSeverity(logentry.severity) + "\t" + convertTags(logentry.tags) + outputMsg;
                        std::cout << "\033[4:4m" + outputline + "\033[0m" << std::endl;
                        break;
                    case TYPE_SUCCESS:
                        outputline += " success: " + convertTags(logentry.tags) + outputMsg;
                        std::cout << "\033[32m" + outputline + "\033[0m" << std::endl;
                        break;
                    case TYPE_WARNING:
                        outputline += " event: " + convertSeverity(logentry.severity) + "\t" + convertTags(logentry.tags) + outputMsg;
                        std::cout << "\033[33m" + outputline + "\033[0m" << std::endl;

                        break;
                    default:
                        std::cerr << "unhandled exception : log type handling failed" << std::endl;
                        LogError("failed to handle log type, please submit ticket" + logentry.type, HIGH_SEVERITY, TAG_ENGINE);
                        outputline += "unknown : " + convertSeverity(logentry.severity) + "\t" + convertTags(logentry.tags) + outputMsg;
                        std::cout << "\033[106m" + outputline + "\033[0m" << std::endl;

                        break;
                    }
                }
                else
                {
                    std::string outputMsg = "";
                    if (logentry.SubOpID != -1)
                    {
                        outputMsg += "suboperator " + Suboperators[logentry.SubOpID];
                    }
                    if (logentry.AdditionalNotes != -1 && logentry.AdditionalNotes != 0)
                    {
                        outputMsg += "note :" + notes[logentry.AdditionalNotes];
                    }
                    outputMsg += logentry.message;
                    switch (logentry.type) {
                        case TYPE_ERROR:
                            outputline += " error: " + convertSeverity(logentry.severity) + "\t" + convertTags(logentry.tags) + outputMsg;
                            std::cout<< "\033[31m" + outputline + "\033[0m" << std::endl;
                        break;
                        case TYPE_EVENT:
                            outputline += " event: " + convertTags(logentry.tags) + logentry.message;
                            std::cout << "\033[105m" + outputline + "\033[0m" << std::endl;
                            break;
                        case TYPE_FAILURE:
                            outputline += " failed: " + convertSeverity(logentry.severity) + "\t" + convertTags(logentry.tags) + outputMsg;
                            std::cout << "\033[41m" + outputline + "\033[0m" << std::endl;

                            break;
                        case TYPE_INFORMATION:
                            outputline += " info: " + convertTags(logentry.tags) + outputMsg;
                            std::cout << "\033[4:2m" + outputline + "\033[0m" << std::endl;
                            break;
                        case TYPE_SUBOPTIMAL:
                            outputline += " suboptimal usage: " + convertSeverity(logentry.severity) + "\t" + convertTags(logentry.tags) + outputMsg;
                            std::cout << "\033[4:4m" + outputline + "\033[0m" << std::endl;
                            break;
                        case TYPE_SUCCESS:
                            outputline += " success: " + convertTags(logentry.tags) + outputMsg;
                            std::cout << "\033[32m" + outputline + "\033[0m" << std::endl;
                            break;
                        case TYPE_WARNING:
                            outputline += " event: " + convertSeverity(logentry.severity) + "\t" + convertTags(logentry.tags) + outputMsg;
                            std::cout << "\033[33m" + outputline + "\033[0m" << std::endl;

                            break;
                        default:
                            std::cerr << "unhandled exception : log type handling failed" << std::endl;
                            LogError("failed to handle log type, please submit ticket" + logentry.type, HIGH_SEVERITY, TAG_ENGINE);
                            outputline += "unknown : " + convertSeverity(logentry.severity) + "\t" + convertTags(logentry.tags) + outputMsg;
                            std::cout << "\033[106m" + outputline + "\033[0m" << std::endl;

                            break;
                    }
                }
                //Time += ;
                AddLog(outputline);
                // convert to miliseconds
                // hour , min , sec ,ms

                //for in engine stuff , it goes here



            }
            lastIndex = entries.size();
        }
    }

    void Con::OutputFile()
    {
        Display();
        //this clears latest.log
        std::ofstream outFile("latest_log.txt");
        if (outFile.is_open()) {
            outFile << debugBoilerPlate;
            outFile.close();
        }
        else {
            std::cerr << "Could not open the file." << std::endl;
        }
    }

    void Con::RenameFile(std::ifstream& data)
    {
        std::string firstLine;
        std::getline(data, firstLine);
        firstLine.erase(0, 4);
        for (int i = 6; i < firstLine.size(); i++) {
            if (firstLine[i] == ':') {
                firstLine[i] = '-';
            }
        }
        //duplicates in case of bootstrapper crash
        newFilename = "log at " + firstLine + ".txt";
        std::stringstream buffer;
        buffer << data.rdbuf();
        std::string bufferdata = buffer.str();
        data.close();
        std::ofstream outFile(newFilename);
        if (outFile.is_open()) {
            outFile << bufferdata;
            outFile.close();
        }
        else {
            std::cerr << "Could not open the file." << std::endl;
        }

        //this clears latest.log
        outFile = std::ofstream("latest_log.txt");
        if (outFile.is_open()) {
            outFile << "";
            outFile.close();
        }
        else {
            std::cerr << "Could not open the file." << std::endl;
        }
    }

    void Con::AddSubOpLogs(std::vector<logEntry> entries, std::string name, std::string Notes)
    {
        uint16_t nameIndex = 0;
        uint16_t noteIndex = 0;
        if (!SuboperatorIndexes.contains(name))
        {
            nameIndex = Suboperators.size();
            Suboperators.emplace_back(name);
        }
        else
        {
            nameIndex = SuboperatorIndexes[name];
        }
        if (Notes != "")
        {
            noteIndex = notes.size();
            notes.emplace_back(Notes);
        }
        for(auto & entry : entries)
        {
            entry.SubOpID = nameIndex;
            entry.AdditionalNotes = noteIndex;
        }
        Con::entries.insert(Con::entries.end(), entries.begin(), entries.end());
        //this can be faster but i am lazy
    }

    void Con::BootstrapDone()
    {
        std::ifstream file(newFilename);
        
        if (!file.is_open()) {
            std::cerr << "Failed to open new log file" << std::endl;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string bufferdata = buffer.str();
        file.close();
        std::string newpath = Bootstrapper::data["[debug]"]["log_filepath"].value;
        //check file path here
        Log("loaded new path from preferences" + newpath, TAG_DEBUG | TAG_GENERIC);
        if (!std::filesystem::exists(newpath)) {
            LogError("Directory does not exist! attempting to create :" + newpath ,LOW_SEVERITY,TAG_GENERIC|TAG_DEBUG);

            bool success = std::filesystem::create_directories(newpath);

            if (success) {
                LogSuccess("Directory created", TAG_GENERIC | TAG_DEBUG);
            }
            else {
                LogFailure("failed to create Directory",HIGH_SEVERITY, TAG_GENERIC | TAG_DEBUG);
            }
        }
        std::ofstream outFile(newpath + newFilename);
        if (outFile.is_open()) {
            outFile << bufferdata;
            outFile.close();
            LogSuccess("successfully written to new address : " + newpath + newFilename, TAG_GENERIC | TAG_DEBUG);
        }
        else {
            std::cerr << "Could not open the file." << std::endl;
            LogFailure("failed to open from path" + newpath + newFilename, CRIT_SEVERITY, TAG_GENERIC | TAG_DEBUG);
        }

        std::filesystem::path target_file = newFilename;
        std::error_code ec;
        bool deleted = std::filesystem::remove(target_file, ec);

        if (ec) {
            LogFailure("could not delete temporary file : " + ec.message(), MED_SEVERITY, TAG_GENERIC | TAG_DEBUG);
        }
        else if (deleted) {
            LogSuccess("temporary file deleted successfully" + ec.message(), TAG_GENERIC | TAG_DEBUG);
        }
        else {
            LogError("temporary file cannot be found" + ec.message(), LOW_SEVERITY, TAG_GENERIC | TAG_DEBUG);
        }
    }

    void Con::loadErrorCodes()
    {

    }

    SubOp::SubOp(std::string designator,std::string notes)
    {
        Designation = designator;
        Notes = notes;
    }
    SubOp::~SubOp()
    {
        Con::AddSubOpLogs(entries, Designation, Notes);
    }

    void SubOp::Log(std::string message, uint64_t tags)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = 0;
        entry.severity = UDEF_SEVERITY;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_INFORMATION;
        entries.emplace_back(entry);
    }

    void SubOp::LogEvent(std::string message, uint64_t tags)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = 0;
        entry.severity = UDEF_SEVERITY;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_EVENT;
        entries.emplace_back(entry);
    }

    void SubOp::LogWarning(std::string message, uint64_t tags)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = 0;
        entry.severity = UDEF_SEVERITY;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_WARNING;
        entries.emplace_back(entry);
    }
    void SubOp::LogSuccess(std::string message, uint64_t tags)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = 0;
        entry.severity = UDEF_SEVERITY;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_SUCCESS;
        entries.emplace_back(entry);
    }

    void SubOp::LogError(std::string message, uint8_t severity, uint64_t tags)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = 0;
        entry.severity = severity;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_ERROR;
        entries.emplace_back(entry);
    }

    void SubOp::LogFailure(std::string message, uint8_t severity, uint64_t tags)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = 0;
        entry.severity = severity;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_FAILURE;
        entries.emplace_back(entry);
    }

    void SubOp::LogSuboptimal(std::string message, uint8_t severity, uint64_t tags)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = 0;
        entry.severity = severity;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_SUBOPTIMAL;
        entries.emplace_back(entry);
    }

    void SubOp::LogError(std::string message, uint8_t severity, uint64_t tags, uint16_t code)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = code;
        entry.severity = severity;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_ERROR;
        entries.emplace_back(entry);
    }

    void SubOp::LogWarning(std::string message, uint8_t severity, uint64_t tags, uint16_t code)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = code;
        entry.severity = severity;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_WARNING;
        entries.emplace_back(entry);
    }

    void SubOp::LogFailure(std::string message, uint8_t severity, uint64_t tags, uint16_t code)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = code;
        entry.severity = severity;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_FAILURE;
        entries.emplace_back(entry);
    }

    void SubOp::LogSuboptimal(std::string message, uint8_t severity, uint64_t tags, uint16_t code)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = code;
        entry.severity = severity;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_SUBOPTIMAL;
        entries.emplace_back(entry);
    }

    void Con::AddLog(std::string data)
    {
        debugBoilerPlate += data + '\n';
    }

    //void Con::AddLogTimed(std::string data)
    //{
    //    //i have no clue how efficient this is
    //    std::tm time_info;
    //    #ifdef _WIN32
    //            localtime_s(&time_info, &now); // Windows secure alternative
    //    #else
    //            localtime_r(&now_c, &time_info); // POSIX secure alternative
    //    #endif

    //    char buffer[26];
    //    std::strftime(buffer, sizeof(buffer), "%c\n", &time_info);
    //    std::string timestamp(&buffer[11],&buffer[20]);
    //    debugBoilerPlate += timestamp + data + '\n';
    //}

    std::string Con::convertSeverity(uint8_t severity)
    {
        std::string output = "";
        switch (severity)// bit mask
        {
        case UDEF_SEVERITY:
            return "undefined, ";
            //descriptor += "the developer is too lazy to define how problematic the problem is/";
            break;
        case LOW_SEVERITY:
            return "low     , ";
            //descriptor += "not really important but should be viewed as a symptom; start looking here if you got a problem/";
            break;
        case MED_SEVERITY:
            return "medium  , ";
            //descriptor += "you should look into this/";
            break;
        case HIGH_SEVERITY:
            return "high    , ";
            //descriptor += "hey i mean what can i say the title speaks for itself/";
            break;
        case CRIT_SEVERITY:
            return "critical, ";
            //descriptor += "a critical error indicates something wrong with the engine, not your code. unless you're me , the developer/";
            break;
        case FATAL_SEVERITY:
            return "fatal  , ";
            //descriptor += "very bad. the engine will probably crash/";
            break;
        }
    }

    std::string Con::convertTags(uint64_t tags)
    {
        std::string string = "";
        if (tags & TAG_ADDON)
        {
            string += "addons, ";
        }
        if (tags & TAG_DEBUG)
        {
            string += "debug, ";
        }
        if (tags & TAG_ENGINE)
        {
            string += "engine, ";
        }
        if (tags & TAG_GENERIC)
        {
            string += "generic, ";
        }
        if (tags & TAG_GL)
        {
            string += "opengl, ";
        }
        if (tags & TAG_PERFORMANCE)
        {
            string += "performance, ";
        }
        if (tags & TAG_PROFILING)
        {
            string += "profiling, ";
        }
        if (tags & TAG_THREADMGR)
        {
            string += "multithreading, ";
        }
        if (tags & TAG_VULKAN)
        {
            string += "vulkan, ";
        }
        if (string.length() == 0)
        {
            string += "unspecified";
        }
        return string;
    }

    void Con::convertTime(std::string& string, std::chrono::system_clock::time_point time)
    {

        // Convert to time_t for the date/time part
        auto tt = std::chrono::system_clock::to_time_t(time);

        // Extract milliseconds
        auto ms = duration_cast<std::chrono::milliseconds>(time.time_since_epoch()) % 1000;

        // Thread-safe localtime
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S")
            << '.' << std::setw(3) << std::setfill('0') << ms.count();

        string = oss.str();
    }

	void Con::Log(std::string message, uint64_t tags)
	{
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = 0;
        entry.severity = UDEF_SEVERITY;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_INFORMATION;
        Con::entries.emplace_back(entry);
	}

    void Con::LogEvent(std::string message, uint64_t tags)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = 0;
        entry.severity  = UDEF_SEVERITY;
        entry.tags      = tags;
        entry.message   = message;
        entry.type      = TYPE_EVENT;
        Con::entries.emplace_back(entry);
    }

    void Con::LogWarning(std::string message, uint64_t tags)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = 0;
        entry.severity = UDEF_SEVERITY;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_WARNING;
        Con::entries.emplace_back(entry);
    }

    void Con::LogSuccess(std::string message, uint64_t tags)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = 0;
        entry.severity = UDEF_SEVERITY;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_SUCCESS;
        Con::entries.emplace_back(entry);
    }

    void Con::LogError(std::string message, uint8_t severity, uint64_t tags)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = 0;
        entry.severity = severity;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_ERROR;
        Con::entries.emplace_back(entry);
    }

    void Con::LogFailure(std::string message, uint8_t severity, uint64_t tags)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = 0;
        entry.severity = severity;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_FAILURE;
        Con::entries.emplace_back(entry);
    }

    void Con::LogSuboptimal(std::string message, uint8_t severity, uint64_t tags)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = 0;
        entry.severity = severity;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_SUBOPTIMAL;
        Con::entries.emplace_back(entry);
    }

    void Con::LogError(std::string message, uint8_t severity, uint64_t tags, uint16_t code)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = code;
        entry.severity = severity;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_ERROR;
        Con::entries.emplace_back(entry);
    }

    void Con::LogWarning(std::string message, uint8_t severity, uint64_t tags, uint16_t code)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = code;
        entry.severity = severity;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_WARNING;
        Con::entries.emplace_back(entry);
    }

    void Con::LogFailure(std::string message, uint8_t severity, uint64_t tags, uint16_t code)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = code;
        entry.severity = severity;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_FAILURE;
        Con::entries.emplace_back(entry);
    }

    void Con::LogSuboptimal(std::string message, uint8_t severity, uint64_t tags, uint16_t code)
    {
        logEntry entry;
        entry.time = std::chrono::system_clock::now();
        entry.code = code;
        entry.severity = severity;
        entry.tags = tags;
        entry.message = message;
        entry.type = TYPE_SUBOPTIMAL;
        Con::entries.emplace_back(entry);
    }
    
}