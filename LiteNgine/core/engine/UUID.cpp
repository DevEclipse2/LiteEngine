#include "UUID.h"
namespace lte {

	void UUID::generate(lte::uuid* pUuid) {
		std::vector<unsigned char> data(16);
		std::generate(begin(data), end(data), std::ref(rb));
	}
	
}