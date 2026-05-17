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
        AddLog("LITENGINE, AN ECLIPSE SUPERSYSTEMS PRODUCT");
	}

    void Con::Display()
    {
        for (uint32_t i = lastIndex; i < logEntry.size(); i++) 
        {

            // convert to miliseconds
            // hour , min , sec ,ms
        }
       
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

        std::string fname = "Log" + firstLine + ".txt";
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

	void Con::Log(std::string information, uint8_t severity)
	{

		Con::logEntry.emplace_back(std::chrono::system_clock::now, information, 0);
	}
}