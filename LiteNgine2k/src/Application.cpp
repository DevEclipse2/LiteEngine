#include "Application.h"
#include "forScrap/Bootstrapper.h"
#include "forScrap/lt_Console.h"
#include "jraphics/windowTracker.h"
namespace ltCore {
	enum Result {
		Continue,
		Exit
	};
	void Application::End()
	{
		lte::Con::LogEvent("Engine Shutdown initiated", TAG_ENGINE);
	}
	void Application::run()
	{
		lte::Con::Init();
		uint8_t result = 0;
		lte::Bootstrapper::OnWake(&result);
		switch (result) {
		case Continue:
			lte::Con::Log("program to continue execution", TAG_ENGINE);
			break;
		case Exit:
			lte::Con::Log("requested program abort launch, terminating...", TAG_ENGINE);
			End();
			break;
		default:
			lte::Con::LogError("unknown on wake result, possible programming oversight , please submit an issue on github,  interface layers / bootstrapper class", MED_SEVERITY, TAG_ENGINE);
			break;
		}
		lte::Bootstrapper::DumpPreferences();
		lte::Bootstrapper::SavePrefs("LiteNginePref.ini");
		lte::Con::BootstrapDone();
		lte::Con::OutputFile();
		//this is main engine
		dllBridge.Startup();

		windowTracker::Init();
		windowTracker::DefaultWindow();
		instance.Init("LiteNgine core");
		
		//create commandpool
		//create commandbuffers


		//here more stuff
		while (!windowTracker::SubWindows[windowTracker::mainWindowIndex]->shouldClose())
		{
			//main loop 
			lte::Con::Display();
		}


		//shutdown

		dllBridge.Shutdown();

		//ends

		lte::Con::Display();
		lte::Con::OutputFile();

		//stuff

	}
}
