#pragma once


#include <string>
#include <vector>
#include <list>
#include <chrono>
#include <unordered_map>
#include <functional>
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
		bool showImmediateFps = false;
		bool useSynchronousWrite = false; // constantly writes to the file on a separate thread
	};


	class Iridium
	{
		struct ProfFrame {
			float totalTime = 0;
			uint32_t id;
			std::vector<std::string> name;
			// threadname | eventname
			std::vector<float> times;
			
		};
		public:
			std::string outputpath;
			static void StartTime(const char* threadName, const char* name);
			static void EndTime(const char* threadName,const char* name);
			static void AddNote(const char* threadName,const char* name);
			static void StartFrame(uint32_t id);
			static void EndFrame();
			static void StartLogging();
			static void EndLogging();
			static void ResetLogging();
			static void WriteToFile();
			static void Init(IridiumCFG config_info);
			static void SubmitDrawCommands();
			static std::list<ProfFrame> profilingFrames;
			static std::vector<float> framesPerSecond;
			static std::vector<float> frameTimes;
			static float ImmFps;
			static float ImmFrameTime;
			static float AvgFps;
			static float AvgFrameTime;
			static float PercentLowFps;
			static float PercentHighFps;
			static std::chrono::steady_clock::time_point FrameStart;
			static std::chrono::steady_clock::time_point FrameEnd;

			using RegisterFunc = std::function<void(const char*)>;

			//main uses hash maps to redirect calls
			std::unordered_map<std::string,RegisterFunc> redirector;


			//non static, probably one iridium instance per thread
			std::string assignedThreadName;
			std::list<std::tuple<std::chrono::steady_clock::time_point, std::chrono::steady_clock::time_point>> event;
			std::list<std::string> eventName;
			std::list<ProfFrame> profilingFrames;
			void IStartTime(const char* name);
			void IEndTime( const char* name);
			void IAddNote(const char* note);
			void Register(); // registers to the redirector
			void DumpData(); // fills the data into the main 
		private:
			static bool initalised;
			static bool frameStarted;
			uint32_t id;

	};
}
//a frame is started, and on a separate thread,the profiler calculate average, writes to disk, and performs screen caps


//auto frameEnd = std::chrono::high_resolution_clock::now();
//float vulkanTime = std::chrono::duration<float, std::milli>(frameEnd - vulkanStart).count();