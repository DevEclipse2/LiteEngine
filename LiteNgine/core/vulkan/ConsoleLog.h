#pragma once
#include <cstring>
#include<iostream>
namespace lte {
	class ConsoleLog
	{


	public:
		static void printU64(const char* label, uint64_t v) {
			std::cout << label << ": " << v << "\n";
		}
		static void printU32(const char* label, uint32_t v) {
			std::cout << label << ": " << v << "\n";
		}

		static void printStr(const char* label, const char* s) {
			std::cout << label << ": " << s << "\n";
		}

		static void printByteArrayHex(const char* label, const uint8_t* data, size_t n) {
			std::cout << label << ": ";
			for (size_t i = 0; i < n; ++i) {
				std::cout << std::hex << (int)data[i];
				if (i + 1 != n) std::cout << ":";
			}
			std::cout << std::dec << "\n";
		}

	};
}
