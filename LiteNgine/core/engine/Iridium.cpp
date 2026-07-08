#include "Iridium.h"
#include "../vulkan/EngineClasses/Lt_Gui.h"
#include "../vulkan/EngineClasses/Lt_Console.h"
namespace lte {
	bool Iridium::initialised = false;
	bool Iridium::frameStarted = false;
	uint32_t Iridium::frame = 0;
	std::string outputpath = "";
	float Iridium::ImmFps = 0;
	float Iridium::ImmFrameTime = 0;
	float Iridium::AvgFps = 0;
	float Iridium::AvgFrameTime = 0;
	float Iridium::PercentLowFps = 0;
	float Iridium::PercentHighFps = 0;
	std::list<Iridium::ProfFrame> Iridium::CompiledprofilingFrames = {};
	std::vector<float> Iridium::framesPerSecond = {};
	std::vector<float> Iridium::frameTimes = {};
	std::unordered_map<std::string, std::vector<Iridium::RegisterFunc>> Iridium::redirector = {};
	std::vector<std::unique_ptr<Iridium>> Iridium::profilingThreads = {};
	void Iridium::StartTime(const char* threadName, const char* name)
	{
		//searches existing profilers to see if it exists, if not it creates a profiler
		CheckExist(0, threadName, name);
	}

	void Iridium::EndTime(const char* threadName, const char* name)
	{
		CheckExist(1, threadName, name);
	}
	void Iridium::AddNote(const char* threadName, const char* name)
	{
		CheckExist(2, threadName, name);
	}
	void Iridium::StartFrame(uint32_t id)
	{
		if (!initialised) {
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
		frameStarted = true;
		FrameStart = std::chrono::high_resolution_clock::now();
	}
	void Iridium::EndFrame()
	{
		if (!initialised) {
			Con::LogError("Iridum profiler MUST be Initialised First!", HIGH_SEVERITY, TAG_ENGINE | TAG_PROFILING | TAG_USER);
			return;
		}
		if (!frameStarted) {
			Con::LogError("No Frame Has Been Started!", MED_SEVERITY, TAG_ENGINE | TAG_PROFILING | TAG_USER);
			return;
		}
		frameStarted = false;
		FrameEnd = std::chrono::high_resolution_clock::now();
		ImmFrameTime = (std::chrono::duration<float, std::milli>(FrameEnd - FrameStart).count());
		frameTimes.emplace_back(ImmFrameTime);
		ImmFps = 1000/ImmFrameTime;

		framesPerSecond.emplace_back(ImmFps);
	}
	void Iridium::StartLogging()
	{
	}
	void Iridium::EndLogging()
	{
	}
	void Iridium::ResetLogging()
	{
		//reset tracked data
	}
	void Iridium::WriteToFile()
	{
	}
	void Iridium::Init(IridiumCFG config_info)
	{
		if (initialised) {
			Con::LogWarning("Iridum profiler has already been initialised, updating configuration...", TAG_ENGINE | TAG_PROFILING);
		}
		initialised = true;
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
	void Iridium::CheckExist(uint8_t command, const char* threadName, const char* name)
	{
		if (!initialised) {
			Con::LogError("Iridum profiler MUST be Initialised First!", HIGH_SEVERITY, TAG_ENGINE | TAG_PROFILING | TAG_USER);
			return;
		}
		if (!frameStarted) {
			Con::LogError("Iridum profiler should be used in a frame!", MED_SEVERITY, TAG_ENGINE | TAG_PROFILING | TAG_USER);
			return;
		}
		if (redirector.contains(threadName))
		{
			redirector[threadName][command](name);
		}
		else {
			profilingThreads.emplace_back(std::make_unique<Iridium>(Iridium{}));
			Iridium* targetIridium = profilingThreads[profilingThreads.size() - 1].get();

			redirector[threadName].resize(3);
			redirector[threadName][0] = [targetIridium](const char* arg) {
				targetIridium->IStartTime(arg);
				};
			redirector[threadName][1] = [targetIridium](const char* arg) {
				targetIridium->IEndTime(arg);
				};
			redirector[threadName][2] = [targetIridium](const char* arg) {
				targetIridium->IAddNote(arg);
				};
			redirector[threadName][command](name);
		}
	}
	void Iridium::SubmitDrawCommands() 
	{
		//only this guy lives on the main thread
		ImGui::Begin("Iridium Profiler");
		ImGui::Text("performance metrics go here!");
		ImGui::Text(("ImmediateData: \n Fps :" + std::to_string(ImmFps) + "| FrameTime: " + std::to_string(ImmFrameTime) + "ms").c_str());
		if (ImGui::Button("Change Settings", ImVec2(120, 40)))
		{

		}
		ImGui::SameLine();
		if (ImGui::Button("Clear Logging", ImVec2(120, 40)))
		{
			ResetLogging();
		}	
		ImGui::End();

	}

	//

}