#pragma once
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <iostream>
#include <filesystem>
#include <map>
#include <unordered_map>
#include "../InterfaceLayers/Bootstrapper.h"


#define UDEF_SEVERITY		0 // undefined 
#define LOW_SEVERITY		1 // has existing workaround / no reprecussions
#define MED_SEVERITY		2 // might lead to reprecussions
#define HIGH_SEVERITY		3 // very large reprecussions
#define CRIT_SEVERITY		4 // will result in errors
#define FATAL_SEVERITY		5 // will result in crash / unhandled exception
#define NOTE				6 // something to keep track of



#define TYPE_EVENT			1 // events and timestamps
#define TYPE_WARNING		2 // 
#define TYPE_ERROR			3 // 
#define TYPE_INFORMATION	4 // 
#define TYPE_FAILURE		5 // 
#define TYPE_SUCCESS		6 // 
#define TYPE_SUBOPTIMAL		7 // optimisation warning

#define TAG_PROFILING		1   //
#define TAG_GENERIC			2   //
#define TAG_VULKAN			4   //
#define TAG_GL				8   //
#define TAG_DEBUG			16  // 
#define TAG_ENGINE			32  // 
#define TAG_ADDON			64	// 
#define TAG_THREADMGR		128	// 
#define TAG_PERFORMANCE		256 // 
#define TAG_USER			512 // 
/*
new severities
low med high critical fatal information
new types
event, warning, error, generic, success, failure, suboptimal
new tags
profiling, performance, vulkan, enginecore, addon, threadmanager

*/

struct logEntry
{
	std::chrono::system_clock::time_point	time;
	std::string								message;
	uint8_t									severity;
	uint8_t									type;
	uint64_t								tags;
	uint16_t								code;

	//suboperator stuff
	uint16_t								SubOpID = -1;
	uint16_t								AdditionalNotes = -1;
};


#include <fstream>
namespace lte {
	class Con
	{
	public:
		// yy,xx
		//low med hi , warning , error , suboptimal usage
										//time , info , severity
		static std::vector<logEntry> entries;
		static std::map<uint16_t, std::string> errorcodes;
		static uint32_t lastIndex;
		static std::vector<std::string>	ConsoleEntry;
		static const std::time_t now;
		static bool Ready;


		static void Log				(std::string message				  , uint64_t tags);
		static void LogEvent		(std::string message				  , uint64_t tags);
		static void LogWarning		(std::string message				  , uint64_t tags);
		static void LogSuccess		(std::string message				  , uint64_t tags);
		static void LogError		(std::string message, uint8_t severity, uint64_t tags);
		static void LogFailure		(std::string message, uint8_t severity, uint64_t tags);
		static void LogSuboptimal	(std::string message, uint8_t severity, uint64_t tags);



		static void LogError		(std::string message, uint8_t severity, uint64_t tags, uint16_t code);
		static void LogWarning		(std::string message, uint8_t severity, uint64_t tags, uint16_t code);
		static void LogFailure		(std::string message, uint8_t severity, uint64_t tags, uint16_t code);
		static void LogSuboptimal	(std::string message, uint8_t severity, uint64_t tags, uint16_t code);


		static void Init();
		static void Display();
		static void OutputFile();// log files are stored using time and build version
		static void RenameFile(std::ifstream& data);


		static void AddSubOpLogs(std::vector<logEntry> entries, std::string name, std::string notes);
		static void BootstrapDone();
	private:
		static void loadErrorCodes();


		static std::string debugBoilerPlate;
		static void AddLog(std::string data);
		static std::string convertSeverity(uint8_t severity);
		static std::string convertTags(uint64_t tags);

		static void convertTime(std::string& string, std::chrono::system_clock::time_point time);


		static std::string newFilename;
		static std::string oldLogContent;


		//for suboperators, one unordered map to eliminate duplicates
		static std::unordered_map<std::string, uint16_t> SuboperatorIndexes;
		static std::vector<std::string> Suboperators;
		static std::vector<std::string> notes;
	};
	class SubOp 
	{
		//suboperations class
	public:
		SubOp(std::string designation, std::string notes);
		~SubOp();
		//basically a subtask, helps with sorting debug stuff
		//subtasks should fit within a single frame and have their logs submitted all at once.
		void Log							(std::string message, uint64_t tags);
		void LogEvent						(std::string message, uint64_t tags);
		void LogWarning						(std::string message, uint64_t tags);
		void LogSuccess						(std::string message, uint64_t tags);
		void LogError						(std::string message, uint8_t severity, uint64_t tags);
		void LogFailure						(std::string message, uint8_t severity, uint64_t tags);
		void LogSuboptimal					(std::string message, uint8_t severity, uint64_t tags);

		void LogError						(std::string message, uint8_t severity, uint64_t tags, uint16_t code);
		void LogWarning						(std::string message, uint8_t severity, uint64_t tags, uint16_t code);
		void LogFailure						(std::string message, uint8_t severity, uint64_t tags, uint16_t code);
		void LogSuboptimal					(std::string message, uint8_t severity, uint64_t tags, uint16_t code);
	private:
		std::string Designation;
		std::string Notes;
		std::vector<logEntry> entries;
		
	};
}