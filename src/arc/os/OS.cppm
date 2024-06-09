module;

#include <csignal>

export module OS;

import std;
import std.compat;

import ext.Event;
import StackTrace;
import ext.Concepts;
import OS.ApplicationListener;
export import OS.GlobalTaskQueue;

import Core.Platform;
import Core.Unit;

//TODO move these to other place
namespace OS{
	float deltaTime_internal = 0.0f;    //InTick | No Pause
	float deltaTick_internal = 0.0f;    //InTick | NO Pause

	float updateDeltaTime_internal = 0.0f;    //InTick
	float updateDeltaTick_internal = 0.0f;    //InTick

	float globalTime_internal = 0.0f;     //Sec  | No Pause
	float globalTick_internal = 0.0f;     //Tick | NO Pause

	float updateTime_internal = 0.0f;     //Sec
	float updateTick_internal = 0.0f;     //Tick

	// std::vector<::OS::ApplicationListener*> applicationListeners;

	std::thread::id mainThreadID{};

	bool paused = false;

	float FPS_reload = 0;
	float totalFrames = 0;
	unsigned int FPS_last = 0;
}

export namespace OS{
	void exitApplication(int s, std::string_view what);

	void exitApplication(const int s) {exitApplication(s, "");}

	ext::CycleSignalManager updateSignalManager{};

	Core::PlatformHandle::DeltaSetter deltaSetter;
	Core::PlatformHandle::TimerSetter timerSetter;

	//Should Be Done In Application Launcher
	std::function<std::filesystem::path()> crashFileGetter = nullptr;

	constexpr float TicksPerSecond{Core::Tick::period::den};

	void resetTimer() {
		globalTime_internal = globalTick_internal = updateTime_internal = updateTick_internal = 0;
	}

	inline float delta(){return deltaTime_internal;}

	[[nodiscard]] bool isMainThread(){
		return std::this_thread::get_id() == mainThreadID;
	}

	unsigned int getFPS() {
		FPS_reload += deltaTime_internal;

		totalFrames += 1.0f;
		if(FPS_reload > 1.0f) {
			FPS_last = static_cast<unsigned int>(totalFrames);
			FPS_reload = 0.0f;
			totalFrames = 0.0f;
		}

		return FPS_last;
	}

	inline Core::Tick deltaTick(){return {deltaTick_internal};}

	inline Core::Sec updateDelta(){return updateTime_internal;}

	inline Core::Tick updateDeltaTick(){return {updateDeltaTick_internal};}

	inline Core::Sec globalTime(){return globalTime_internal;}

	inline Core::Tick globalTick(){return {globalTick_internal};}

	inline Core::Sec updateTime(){return updateTime_internal;}

	inline Core::Tick updateTick(){return {updateTick_internal};}

	inline void pause() {paused = true;}

	inline bool isPaused() {return paused;}

	inline void resume() {paused = false;}

	//TODO fire an event?
	inline void setPause(const bool v) {paused = v;}


	// /**
	//  * \brief Register an application listener.
	//  * \param listener Should Has Static Lifetime or at least keeps alive before main loop exited.
	//  */
	// void registerListener(ApplicationListener* listener){applicationListeners.push_back(listener);}
	//
	// void removeListener(ApplicationListener* listener){std::erase(applicationListeners, listener);}

	// void clearListeners(){applicationListeners.clear();}

	void launchApplication(){
		std::signal(SIGABRT, exitApplication);
		std::signal(SIGILL, exitApplication);
		std::signal(SIGSEGV, exitApplication);
		std::signal(SIGINT, exitApplication);
		std::signal(SIGTERM, exitApplication);

		mainThreadID = std::this_thread::get_id();
	}

	[[nodiscard]] std::thread::id getMainThreadID() {return mainThreadID;}

	void killApplication(const std::string_view what) {exitApplication(SIGTERM, what);}

	void pollWindowEvent(){
		deltaTime_internal = deltaSetter(globalTime_internal);
		deltaTick_internal = deltaTime_internal * TicksPerSecond;

		updateDeltaTime_internal = paused ? 0.0f : deltaTime_internal;
		updateDeltaTick_internal = paused ? 0.0f : deltaTick_internal;

		globalTime_internal = timerSetter();
		globalTick_internal = globalTime_internal * TicksPerSecond;

		updateTime_internal += updateDeltaTime_internal;
		updateTick_internal = updateTime_internal * TicksPerSecond;
	}

	void update(){
		updateSignalManager.fire(ext::CycleSignalState::begin);

		handleTasks();

		// for(const auto & listener : applicationListeners){
		// 	if(listener->pauseRestrictable) {
		// 		if(!paused)listener->update(updateDeltaTick_internal);
		// 	}else {
		// 		listener->update(deltaTick_internal);
		// 	}
		//
		// 	listener->updateGlobal(deltaTick_internal);
		// }
		//
		// for(const auto & listener : applicationListeners){
		// 	listener->updatePost(deltaTick_internal);
		// }

		updateSignalManager.fire(ext::CycleSignalState::end);
	}
}


