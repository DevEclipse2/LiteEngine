#pragma once
#include <vector>
#include <string>
#include <chrono>
#include <tuple>
#include <ctime>
#include <iostream>
#define UDEF_SEVERITY		0 // 0
#define LOG_LOW_SEVERITY	1 // 1
#define LOG_MED_SEVERITY	2 // 01 
#define LOG_HIGH_SEVERITY	3 // 11
#define LOG_CRIT_SEVERITY	4 // 001
#define LOG_FATAL_SEVERITY	5 // 101

#define LOG_UDEF			8 // 0001
#define LOG_WARN			16// 00001
#define LOG_ERR				24// 00011
#define LOG_NOPT			32// 000001


#include <fstream>
namespace lte {
	class Con
	{
	public:
		// yy,xx
		//low med hi , warning , error , suboptimal usage
										//time , info , severity
		static std::vector<std::tuple< std::chrono::system_clock::time_point, std::string, uint8_t >> logEntry;
		static uint32_t lastIndex;
		static std::vector<std::string>	ConsoleEntry;
		static const std::time_t now;
		static bool Ready;
		static void Log(std::string information, uint8_t severity);
		/*
		static void LogWarn(std::string information, uint8_t severity);
		static void LogErr(std::string information, uint8_t severity);
		static void LogNOpt(std::string information, uint8_t severity);*/
		static void Init();
		static void Display();
		static void OutputFile();// log files are stored using time and build version
		static void RenameFile(std::ifstream& data);
	private:
		static std::string debugBoilerPlate;
		static void AddLog(std::string data);
	};

}