#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <iostream> 
#include "UUID.h"
namespace lte {
	
	class FileMgr
	{
	public:
		static const uint8_t sucess				= 0;
		static const uint8_t failure			= 1;
		static const uint8_t noFile				= 2;
		static const uint8_t incorrectFormat	= 4;
		static const uint8_t readFail			= 8;
		static const uint8_t unknownError		= 16;
		static const uint8_t noDestination		= 32;


		FileMgr();
		~FileMgr();
		uint8_t readBinFile(const std::string* path, std::vector<char>* charptr, uint16_t format);
		uint8_t readJsonFile(const std::string* path, nlohmann::json* pFile, std::string format);

		void readSlateFile(std::string* pName, uuid uuid, nlohmann::json* pData);

	private:
		bool checkPath(const std::string& path);
		bool checkWrite();
		bool checkFormat();
		bool checkAccess();
		std::vector<char> data;
		std::string dataStr;



	};

}