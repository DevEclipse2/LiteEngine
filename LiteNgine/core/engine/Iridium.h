#pragma once


#include <string>
#include <vector>
#include <list>
#include <chrono>
#include <unordered_map>
#include <functional>
#include <thread>
#include <mutex>
//the iridium profiler
//its gud
//image snapshot
//frame tracker
//frame time tracker
//1% lows and averages
//injectable profiler


//multithreading handles average fps calculation, data logging and screenshotting
//use mutex to lock the 
namespace lte {

	struct IridiumCFG {
		float ProfilingRetainTime = 0;
		int SamplingRate = 1; //once per x frames
		bool AlertFrameDrop;
		float AlertPercentage = 0; //alert when fps drops under x% of average
		float AlertFps = 0; //alert when fps drops under this value
		bool autoWriteToDisk = false;
		float autoSaveTimer = 30.0f;
		float UpdateRate = 1.0f; //how many seconds per update
		float RetainTime = 10.0f; //how many seconds to retain as average
		bool showImmediateFps = false;
		bool useSynchronousWrite = false; // constantly writes to the file on a separate thread
	};


	class Iridium
	{
		
	public:
		struct ProfFrame {
			float totalTime = 0;
			uint32_t id;
			std::vector<std::string> name;
			// threadname | eventname
			std::vector<float> times;

		};
		static std::string outputpath;
		static std::mutex FrameAvgLock;
		static void StartTime(const char* threadName, const char* name);
		static void EndTime(const char* threadName, const char* name);
		static void AddNote(const char* threadName, const char* name);
		static void StartFrame(uint32_t id);
		static void EndFrame();
		static void StartLogging();
		static void EndLogging();
		static void ResetLogging();
		static void WriteToFile();
		static void Init(IridiumCFG config_info);
		static void SubmitDrawCommands();
		static void CalculateAverages();
		static void Terminate();
		static std::vector<float> framesPerSecond;
		static std::vector<float> frameTimes;
		static float ImmFps;
		static float ImmFrameTime;
		static std::atomic<float> AvgFps;
		static std::atomic<float> AvgFrameTime;
		static std::atomic<float> PercentLowFps;
		static std::atomic<float> PercentHighFps;
		inline static std::chrono::steady_clock::time_point FrameStart;
		inline static std::chrono::steady_clock::time_point FrameEnd;
		inline static std::thread AvgFrameWorker;
		using RegisterFunc = std::function<void(std::string)>;

		//main uses hash maps to redirect calls
		static std::unordered_map<std::string, std::vector<RegisterFunc>> redirector;
		static std::vector<std::unique_ptr<Iridium>> profilingThreads;

		//non static, probably one iridium instance per thread
		std::string assignedThreadName = "";

		std::list<std::tuple<std::chrono::steady_clock::time_point, std::chrono::steady_clock::time_point>> event = {};
		std::list<std::string> eventName = {};
		std::list<ProfFrame> profilingFrames = {};
			void IStartTime(std::string name);
			void IEndTime(std::string name);
			void IAddNote(std::string note);
			void Register(); // registers to the redirector
			void DumpData(); // fills the data into the main 

		static IridiumCFG CurrentSettings;
		static IridiumCFG NewSettings;
		private:
			static bool initialised;
			static bool OpenMenu; 
			static bool frameStarted;
			static bool active;
			static uint32_t frame;
			static void CheckExist(uint8_t command,const char* thread, const char* name);
			
	};
}
//a frame is started, and on a separate thread,the profiler calculate average, writes to disk, and performs screen caps
//

//auto frameEnd = std::chrono::high_resolution_clock::now();
//float vulkanTime = std::chrono::duration<float, std::milli>(frameEnd - vulkanStart).count();