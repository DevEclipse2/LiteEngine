#include "Lt_Console.h"

namespace lte 
{
	std::vector<std::tuple<std::chrono::system_clock::time_point, std::string, uint8_t >> Con::logEntry = {};
	bool Con::Ready = false;
    uint32_t Con::lastIndex = 0;
    std::string Con::debugBoilerPlate = "";
    const std::time_t Con::now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

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
        if (logEntry.size() > 0)
        {
            for (uint32_t i = lastIndex; i < logEntry.size(); i++) 
            {
                auto logentry = logEntry[i];
                std::string Time = std::format("{:%Y-%m-%d %H:%M:%S.%.3f}", std::get<1>(logentry));
                Time += std::get<2>(logentry);

                //Time += ;
                AddLog(Time);
                // convert to miliseconds
                // hour , min , sec ,ms
            }
        }
        
        lastIndex = logEntry.size();
    }

    void Con::OutputFile()
    {
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
        for (int i = 0; i < firstLine.size(); i++) {
            if (firstLine[i] == ':') {
                firstLine[i] = '-';
            }
        }
        std::string fname = "log at " + firstLine + ".txt";
        std::stringstream buffer;
        buffer << data.rdbuf();
        std::string fileContent = buffer.str();
        data.close();
        std::ofstream outFile(fname);
        if (outFile.is_open()) {
            outFile << fileContent;
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

    void Con::AddLog(std::string data)
    {
        debugBoilerPlate += data + '\n';
    }

    void Con::AddLogTimed(std::string data)
    {
        //i have no clue how efficient this is
        std::tm time_info;
        #ifdef _WIN32
                localtime_s(&time_info, &now); // Windows secure alternative
        #else
                localtime_r(&now_c, &time_info); // POSIX secure alternative
        #endif

        char buffer[26];
        std::strftime(buffer, sizeof(buffer), "%c\n", &time_info);
        std::string timestamp(&buffer[11],&buffer[20]);
        debugBoilerPlate += timestamp + data + '\n';
    }

    std::string Con::convertSeverity(uint8_t severity, std::string& descriptor)
    {
        std::string output = "";
        descriptor = " ";
        switch (severity & 7)// bit mask
        {
        case UDEF_SEVERITY:
            output += "severity: undefined ,";
            descriptor += "the developer is too lazy to define how problematic the problem is/";
            break;
        case LOG_LOW_SEVERITY:
            output += "severity: low ,";
            descriptor += "not really important but should be viewed as a symptom; start looking here if you got a problem/";
            break;
        case LOG_MED_SEVERITY:
            output += "severity: medium ,";
            descriptor += "you should look into this/";
            break;
        case LOG_HIGH_SEVERITY:
            output += "severity: high ,";
            descriptor += "hey i mean what can i say the title speaks for itself/";
            break;
        case LOG_CRIT_SEVERITY:
            output += "severity: critical ,";
            descriptor += "a critical error indicates something wrong with the engine, not your code. unless you're me , the developer/";
            break;
        case LOG_FATAL_SEVERITY:
            output += "fatal error ,";
            descriptor += "very bad. the engine will probably crash/";
            break;
        }
        switch (severity & 56)
        {
        case LOG_VERBOSE:
            output += "vulkan verbose";
            break;
        case LOG_INFO:
            output += "vulkan info";
            break;
        case LOG_WARN:
            output += "vulkan warning";
            break;
        case LOG_ERR:
            output += "vulkan error";
            break;
        case LOG_NOPT:
            output += "vulkan suboptimal usage";
            break;
        }
        return output;
    }

	void Con::Log(std::string information, uint8_t severity)
	{

		Con::logEntry.emplace_back(std::chrono::system_clock::now(), information, severity);
        std::string description;
        AddLogTimed(convertSeverity(severity, description) + "    " + information + "\n");
        
	}

    void Con::LogVB(std::string information, uint8_t severity, std::string notes, std::string documentation)
    {
        Con::logEntry.emplace_back(std::chrono::system_clock::now(), information, severity);
        std::string description;
        AddLogTimed( convertSeverity(severity, description) + "    " + information + "\n" + description + "\n" + notes + "\n" + "to know more, visit documentation at" + documentation);
    }
}