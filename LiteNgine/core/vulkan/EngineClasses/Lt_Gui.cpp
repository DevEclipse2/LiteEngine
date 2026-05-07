#include "Lt_Gui.h"

void CheckVKResult(VkResult err) {
	if (err == 0) {
		return;
	}
	std::cout << stderr << "[vulkan] error : VkResult = %d\n" << err;
	if (err < 0) {
		abort();
	}
}


namespace lte {
	void Lt_Gui::Instantiate()
	{
		

	}
}