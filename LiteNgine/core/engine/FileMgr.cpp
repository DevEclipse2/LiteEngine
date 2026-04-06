#include "FileMgr.h"
namespace lte {

	FileMgr::FileMgr() {
		//load paging file


	}
	FileMgr::~FileMgr() {

	}
	uint8_t FileMgr::readBinFile(const std::string* path, std::vector<char>* charptr, uint16_t format)
	{	
		uint8_t result{};
		if (!checkPath(*path))
		{
			result |= FileMgr::noFile;
			return result | FileMgr::failure;
		}
		std::fstream bin(*path, std::ios::binary);
		if (charptr == nullptr) {
			charptr = &data;

		}

		return result | FileMgr::unknownError;
	}

	uint8_t FileMgr::readJsonFile(const std::string* path, std::string* charptr , std::string format)
	{
		uint8_t result{};
		if (!checkPath(*path))
		{
			result |= FileMgr::noFile;
			return result | FileMgr::failure;
		}
		if (charptr == nullptr) {
			charptr = &dataStr;

		}
		std::fstream input("dat.json");
		nlohmann::json file;
		input >> file;
		std::cout << file.dump(4) << std::endl;

		return result | FileMgr::unknownError;
	}
	bool FileMgr::checkPath(const std::string& path){
		//checks if the file even exists
		return std::filesystem::exists(path);
	}
	bool FileMgr::checkWrite() {
		//check if the file format is indeed correct and fits in with the attempted modifications
		return false;
	}
	bool FileMgr::checkFormat(){
		//checks if the file is correctly formatted with what we're expecting to read
		return false;
	}
	bool FileMgr::checkAccess() {
		//checks if the file to be written to is actually supposed to happen
		return false;
	}
}