#include "Iridium.h"
#include "../vulkan/EngineClasses/Lt_Gui.h"
#include "../vulkan/EngineClasses/Lt_Console.h"
namespace lte {
	bool Iridium::initalised = false;
	bool Iridium::frameStarted = false;
	uint32_t Iridium::frame = 0;

	void Iridium::StartTime(const char* threadName, const char* name)
	{
		if (!initalised) {
			Con::LogError("Iridum profiler MUST be Initialised First!", HIGH_SEVERITY, TAG_ENGINE | TAG_PROFILING | TAG_USER);
			return;
		}
		if (!frameStarted) {
			Con::LogError("Iridum profiler should be used in a frame!", MED_SEVERITY, TAG_ENGINE | TAG_PROFILING | TAG_USER);
			return;
		}
		//searches existing profilers to see if it exists, if not it creates a profiler
	}

	void Iridium::EndTime(const char* threadName, const char* name)
	{
		if (!initalised) {
			Con::LogError("Iridum profiler MUST be Initialised First!", HIGH_SEVERITY, TAG_ENGINE | TAG_PROFILING | TAG_USER);
			return;
		}
		if (!frameStarted) {
			Con::LogError("Iridum profiler should be used in a frame!", MED_SEVERITY, TAG_ENGINE | TAG_PROFILING | TAG_USER);
			return;
		}
	}
	void Iridium::AddNote(const char* threadName, const char* name)
	{
		if (!initalised) {
			Con::LogError("Iridum profiler MUST be Initialised First!", HIGH_SEVERITY, TAG_ENGINE | TAG_PROFILING | TAG_USER);
			return;
		}
		if (!frameStarted) {
			Con::LogError("Iridum profiler should be used in a frame!", MED_SEVERITY, TAG_ENGINE | TAG_PROFILING | TAG_USER);
			return;
		}

	}
	void Iridium::StartFrame(uint32_t id)
	{
		if (!initalised) {
			Con::LogError("Iridum profiler MUST be Initialised First!", HIGH_SEVERITY, TAG_ENGINE | TAG_PROFILING | TAG_USER);
			return;
		}
		if (frameStarted) {
			Con::LogError("StartFrame() has already been called! this function applies globally and should only be called once per frame, not per thread", MED_SEVERITY, TAG_ENGINE | TAG_PROFILING | TAG_USER);
			return;
		}
		if (frame >= id) {
			Con::LogError("Invalid frameID, submitted " + std::to_string(id) + " while profiler has completed frame " + std::to_string(frame), HIGH_SEVERITY, TAG_ENGINE | TAG_PROFILING | TAG_USER);
			return;
		}
	}
	void Iridium::EndFrame()
	{
		if (!initalised) {
			Con::LogError("Iridum profiler MUST be Initialised First!", HIGH_SEVERITY, TAG_ENGINE | TAG_PROFILING | TAG_USER);
			return;
		}
		if (!frameStarted) {
			Con::LogError("No Frame Has Been Started!", MED_SEVERITY, TAG_ENGINE | TAG_PROFILING | TAG_USER);
			return;
		}
	}
	void Iridium::StartLogging()
	{
	}
	void Iridium::EndLogging()
	{
	}
	void Iridium::ResetLogging()
	{
	}
	void Iridium::WriteToFile()
	{
	}
	void Iridium::Init(IridiumCFG config_info)
	{
		if (initalised) {
			Con::LogWarning("Iridum profiler has already been initialised, updating configuration...", TAG_ENGINE | TAG_PROFILING);
		}
	}
	void Iridium::IStartTime(const char* name)
	{
	}
	void Iridium::IEndTime(const char* name)
	{
	}
	void Iridium::IAddNote(const char* note)
	{
	}
	void Iridium::Register()
	{
	}
	void Iridium::DumpData()
	{
	}
	void Iridium::SubmitDrawCommands() 
	{
		//only this guy lives on the main thread	
	}

}