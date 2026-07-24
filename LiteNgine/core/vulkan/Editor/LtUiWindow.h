#pragma once

// base class of all ui windows
namespace lte {
	class LtUiWindow
	{
	public:
		bool Enabled = true;
		virtual void SubmitGUICommands(){}
		const char* name;
	};
}