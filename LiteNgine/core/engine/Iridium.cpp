#include "Iridium.h"
#include "../vulkan/EngineClasses/Lt_Gui.h"
#include "../vulkan/EngineClasses/Lt_Console.h"
#include <numeric>
namespace lte {
	bool Iridium::initialised = false;
	bool Iridium::frameStarted = false;
	std::mutex Iridium::FrameAvgLock;

	uint32_t Iridium::frame = 0;
	std::string outputpath = "";
	float Iridium::ImmFps = 0;
	float Iridium::ImmFrameTime = 0;
	std::atomic<float> Iridium::AvgFps = 0;
	std::atomic<float> Iridium::AvgFrameTime = 0;
	std::atomic<float> Iridium::PercentLowFps = 0;
	std::atomic<float> Iridium::PercentHighFps = 0;
	//std::list<Iridium::ProfFrame> Iridium::CompiledprofilingFrames = {};
	std::vector<float> Iridium::framesPerSecond = {};
	std::vector<float> Iridium::frameTimes = {};
	std::unordered_map<std::string, std::vector<Iridium::RegisterFunc>> Iridium::redirector = {};
	std::vector<std::unique_ptr<Iridium>> Iridium::profilingThreads = {};
	bool Iridium::OpenMenu = false;
	IridiumCFG Iridium::CurrentSettings;
	bool Iridium::active = false;
	IridiumCFG Iridium::NewSettings;
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


		//use mutex to open a new frame and grab all of the profiling threads

	}
	void Iridium::StartLogging()
	{
		active = true;
	}
	void Iridium::EndLogging()
	{
		active = false;
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
			Con::LogWarning("Iridium profiler has already been initialised, updating configuration...", TAG_ENGINE | TAG_PROFILING);
		}
		else {
			initialised = true;
			AvgFrameWorker = std::thread(&CalculateAverages);
			Con::LogEvent("Iridium profiler has been initalised.",TAG_ENGINE|TAG_PROFILING);
		}
		//pause logging and apply using mutex

		CurrentSettings = config_info;
	}
	void Iridium::IStartTime(std::string name)
	{
		eventName.emplace_back(name);
		event.emplace_back(std::tuple<std::chrono::steady_clock::time_point, std::chrono::steady_clock::time_point>(std::chrono::high_resolution_clock::now(), std::chrono::high_resolution_clock::now()));
	}
	void Iridium::IEndTime(std::string name)
	{
		auto it = std::find(eventName.begin(), eventName.end(), name);
		if (it != eventName.end()) {
			int index = std::distance(eventName.begin(), it);
			std::tuple<std::chrono::steady_clock::time_point, std::chrono::steady_clock::time_point>& val = *std::next(event.begin(), index);
			std::get<1>(val) = std::chrono::high_resolution_clock::now();
		}
		else {
			Con::LogFailure("Could not find event with name!", MED_SEVERITY, TAG_PROFILING | TAG_ENGINE);
		}
	}
	void Iridium::IAddNote(std::string note)
	{
		eventName.emplace_back( "Note :" + note);
		event.emplace_back(std::tuple<std::chrono::steady_clock::time_point, std::chrono::steady_clock::time_point>(std::chrono::high_resolution_clock::now(), std::chrono::high_resolution_clock::now()));
	}

	//new changes 
		
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
			redirector[threadName][0] = [targetIridium](std::string arg) {
				targetIridium->IStartTime(arg);
				};
			redirector[threadName][1] = [targetIridium](std::string arg) {
				targetIridium->IEndTime(arg);
				};
			redirector[threadName][2] = [targetIridium](std::string arg) {
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
		
		if (ImGui::Button("Start Logging", ImVec2(220, 40)))
		{
			StartLogging();
		}ImGui::SameLine();
		if (ImGui::Button("Stop Logging", ImVec2(220, 40)))
		{
			EndLogging();
		}
			ImGui::Text(("ImmediateData: \n Fps :" + std::to_string(ImmFps) + "| FrameTime: " + std::to_string(ImmFrameTime) + "ms").c_str());
			ImGui::Text(("AverageData: \n Fps :" +	 std::to_string(AvgFps.load()) + "| FrameTime: " + std::to_string(AvgFrameTime.load()) + "ms").c_str());
			if (ImGui::Button("Change Settings", ImVec2(320, 40)))
			{
				OpenMenu = !OpenMenu;
			}
			ImGui::SameLine();
			if (ImGui::Button("Clear Logging", ImVec2(320, 40)))
			{
				ResetLogging();
			}
			if (OpenMenu) {
				if (ImGui::BeginChild("Logger Settings"))
				{
					ImGui::SetNextItemWidth(160);
					ImGui::Checkbox("Show ConstantFps", &NewSettings.showImmediateFps);
					if (ImGui::Button("Apply", ImVec2(120, 40)))
					{
						Init(NewSettings);
					}
				}
				ImGui::EndChild();
			}
			ImGui::End();
	}

	void Iridium::CalculateAverages()
	{
		while (initialised) {
			std::chrono::duration<float> float_seconds(CurrentSettings.UpdateRate);
			std::this_thread::sleep_for(std::chrono::milliseconds(std::chrono::duration_cast<std::chrono::milliseconds>(float_seconds )));

			float avg = 0.0f;
			float avgFT = 0.0f;
			{
				// --- CRITICAL SECTION START ---
				// std::lock_guard automatically locks the mutex, and unlocks it 
				// when it goes out of scope at the closing brace.
				std::lock_guard<std::mutex> lock(FrameAvgLock);
				if (!frameTimes.empty()) {
					float sum = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0f);
					avgFT = sum / frameTimes.size();
					avg = 1000.0f / avgFT; // Assuming times are in ms
					frameTimes.clear(); // Reset for the next batch
				}
				// --- CRITICAL SECTION END (Mutex unlocks here) ---
			}
			AvgFps.store(avg);
			AvgFrameTime.store(avgFT);
		}
	}

	void Iridium::Terminate()
	{
		initialised = false;
		if (AvgFrameWorker.joinable()) AvgFrameWorker.join();
	}

	//

}