#pragma once

namespace lte {
	static class UUID
	{
	public:
		static void generate(uuid* pUuid);


	};

	struct uuid {

		unsigned char uuid[16];
	};
}


