module;

#include <GLFW/glfw3.h>
#include <csignal>

export module OS;

import std;
import std.compat;

import Event;
import StackTrace;
import Concepts;
import OS.ApplicationListener;

namespace OS{
	bool loopBegin = false;

	float deltaTime_internal = 0.0f;    //InTick
	float deltaTick_internal = 0.0f;    //InTick

	float updateDeltaTime_internal = 0.0f;    //InTick
	float updateDeltaTick_internal = 0.0f;    //InTick

	float globalTime_internal = 0.0f;     //Sec    | No Pause
	float globalTick_internal = 0.0f;     //Tick   | NO Pause

	float updateTime_internal = 0.0f;     //Sec
	float updateTick_internal = 0.0f;     //Tick

	std::vector<::OS::ApplicationListener*> applicationListeners;

	std::mutex lockTask{};
	std::mutex lockAsyncTask{};
	std::mutex lockAsyncPackTask{};
	std::vector<std::function<void()>> postTasks;
	std::vector<std::pair<std::function<void()>, std::promise<void>>> postAsyncTasks;
	std::vector<std::packaged_task<void()>> postAsyncPackedTasks;

	std::thread::id mainThreadID{};

	bool paused = false;

	float FPS_reload = 0;
	float totalFrames = 0;
	unsigned int FPS_last = 0;
}

export namespace OS{
	std::vector<std::string> args{};

	void exitApplication(int s, std::string_view what);

	void exitApplication(const int s) {exitApplication(s, "");}

	Event::CycleSignalManager updateSignalManager{};


	//Should Be Done In Application Launcher
	//TODO isCallable good?
	std::function<void(float&)> deltaSetter = nullptr;
	std::function<void(float&)> globalTimeSetter = nullptr;
	std::function<std::filesystem::path()> crashFileGetter = nullptr;


	/**\brief READ ONLY*/
	int refreshRate = 60;
	/**\brief READ ONLY*/
	int screenWidth = 60;
	/**\brief READ ONLY*/
	int screenHeight = 60;

	constexpr float TicksPerSecond = 60.0f;

	void resetTimer() {
		globalTime_internal = globalTick_internal = updateTime_internal = updateTick_internal = 0;
	}

	inline float delta(){return deltaTime_internal;}

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

	inline float deltaTick(){return deltaTick_internal;}

	inline float updateDelta(){return updateTime_internal;}

	inline float updateDeltaTick(){return updateDeltaTick_internal;}

	inline float globalTime(){return globalTime_internal;}

	inline float globalTick(){return globalTick_internal;}

	inline float updateTime(){return updateTime_internal;}

	inline float updateTick(){return updateTick_internal;}

	inline void pause() {paused = true;}

	inline bool isPaused() {return paused;}

	inline void resume() {paused = false;}

	//TODO fire an event?
	inline void setPause(const bool v) {paused = v;}

	void post(Concepts::Invokable<void()> auto&& func) {
		std::lock_guard guard{lockTask};
		postTasks.push_back(std::forward<decltype(func)>(func));
	}

	void postAsync(Concepts::Invokable<void()> auto&& func, std::promise<void>&& promise) {
		std::lock_guard guard{lockAsyncTask};
		postAsyncTasks.emplace_back(std::forward<decltype(func)>(func), std::move(promise));
	}

	[[nodiscard]] std::future<void> postAsync(Concepts::Invokable<void()> auto&& func) {
		lockAsyncPackTask.lock();
		std::packaged_task<void()>& task = postAsyncPackedTasks.emplace_back(std::forward<decltype(func)>(func));
		lockAsyncPackTask.unlock();
		return task.get_future();
	}

	/**
	 * \brief Register an application listener.
	 * \param listener Should Has Static Lifetime or at least keeps alive before main loop exited.
	 */
	void registerListener(ApplicationListener* listener){applicationListeners.push_back(listener);}

	void removeListener(ApplicationListener* listener){std::erase(applicationListeners, listener);}

	void clearListeners(){applicationListeners.clear();}

	void launchApplication(){
		std::signal(SIGABRT, exitApplication);
		std::signal(SIGILL, exitApplication);
		std::signal(SIGSEGV, exitApplication);
		std::signal(SIGINT, exitApplication);
		std::signal(SIGTERM, exitApplication);

		deltaSetter = [](float& f){
			f = static_cast<float>(glfwGetTime()) - globalTime_internal;
		};

		globalTimeSetter = [](float& f){
			f = static_cast<float>(glfwGetTime());
		};

		mainThreadID = std::this_thread::get_id();
	}

	[[nodiscard]] std::thread::id getMainThreadID() {return mainThreadID;}

	void killApplication(const std::string_view what) {exitApplication(SIGTERM, what);}

	bool shouldContinueLoop(GLFWwindow* window){return !glfwWindowShouldClose(window);}

	void setupMainLoop() {loopBegin = true;}

	[[nodiscard]] bool mainLoopRunning() {return loopBegin;}

	void terminateMainLoop() {loopBegin = false;}

	void pollWindowEvent(GLFWwindow* window){
		glfwSwapBuffers(window);
		glfwPollEvents();

		deltaSetter(deltaTime_internal);
		deltaTick_internal = deltaTime_internal * TicksPerSecond;

		updateDeltaTime_internal = paused ? 0.0f : deltaTime_internal;
		updateDeltaTick_internal = paused ? 0.0f : deltaTick_internal;

		globalTimeSetter(globalTime_internal);
		globalTick_internal = globalTime_internal * TicksPerSecond;

		updateTime_internal += updateDeltaTime_internal;
		updateTick_internal = updateTime_internal * TicksPerSecond;
	}

	void update(){
		updateSignalManager.fire(Event::CycleSignalState::begin);

		{
			lockTask.lock();
			const decltype(postTasks) tempTask = std::move(postTasks);
			lockTask.unlock();

			for(auto&& task : tempTask){
				task();
			}
		}

		{
			lockAsyncPackTask.lock();
			decltype(postAsyncPackedTasks) tempTask = std::move(postAsyncPackedTasks);
			lockAsyncPackTask.unlock();

			for(auto&& task : tempTask){
				task();
			}
		}

		{
			lockAsyncTask.lock();
			const decltype(postAsyncTasks) tempTask = std::move(postAsyncTasks);
			lockAsyncTask.unlock();

			for(auto& [task, promise] : postAsyncTasks){
				try{
					task();
				}catch(...){
					promise.set_exception(std::current_exception());
				}
				promise.set_value();
			}
		}

		for(const auto & listener : applicationListeners){
			if(listener->pauseRestrictable) {
				if(!paused)listener->update(updateDeltaTick_internal);
			}else {
				listener->update(deltaTick_internal);
			}

			listener->updateGlobal(deltaTick_internal);
		}

		for(const auto & listener : applicationListeners){
			listener->updatePost(deltaTick_internal);
		}

		updateSignalManager.fire(Event::CycleSignalState::end);
	}
}


