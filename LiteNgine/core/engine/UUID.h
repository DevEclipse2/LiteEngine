#pragma once
#include <random>
#include <climits>
#include <algorithm>
#include <functional>

using randomBytes = std::independent_bits_engine<
	std::default_random_engine, CHAR_BIT, unsigned int>;

namespace lte {
	struct uuid {

		unsigned char uuid[16];
	};
	class UUID
	{
	public:
		UUID();
		static void generate(lte::uuid* pUuid);
		
	private:
		static randomBytes rb;
		static std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned int> rand;
		//uint8_t randomByte = static_cast<uint8_t>(rand());

	};

	
}


