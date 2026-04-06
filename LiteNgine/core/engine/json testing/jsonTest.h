#pragma once
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
class jsonTest
{
public:
	void createJson() {
		nlohmann::json file;
		file["name"] = "jeffery epstein";
		file["status"] = "cia custody";
		std::cout << file.dump(4) << std::endl; 

		if (file.contains("name"))
		{
			std::cout << file["name"] << std::endl;
		}
		if (file.is_null()) {

		}
		if (file["name"].is_string()) {

		}
		try {
			std::string name = file.at("status").get<std::string>();
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
	}
	void writeJson() {
		std::fstream output("dat.json");
		nlohmann::json file;
		file["name"] = "jeffery epstein";
		file["status"] = "cia custody";
		output << file.dump(4);
		output.close();
	}
	void readJson() {
		std::fstream input("dat.json");
		nlohmann::json file;
		input >> file;
		std::cout << file.dump(4) << std::endl;
	}
	std::string name;
	std::string status;
	jsonTest(std::string name, std::string status) : name(name), status(status) {

	}
};	
	namespace nlohmann {
		template<>
		struct adl_serializer<jsonconverterstruct>
		{
			static void to_json(json& j, const jsonconverterstruct& c)
			{
				j = json{

					{"name", c.name},
					{"status" , c.status},
				};
			}

			static void from_json(const json& j, jsonconverterstruct& c)
			{
				try {
					c.name = j.at("name").get<std::string>();
				}
				catch(const std::exception &e)
				{
					std::cerr << e.what() << '\n';
				}
			}
		};
	}

	struct jsonconverterstruct
	{
		std::string name;  
		std::string status;
	};


