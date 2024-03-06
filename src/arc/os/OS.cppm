export module OS;

import <GLFW/glfw3.h>;
import <chrono>;
import <ctime>;
import <csignal>;
import <sstream>;
import <functional>;
import <iostream>;
import <vector>;
import <future>;
import <queue>;
import <utility>;

import Event;
import StackTrace;
import Concepts;
import OS.ApplicationListener;

namespace OS{
	inline bool loopBegin = false;

	inline float _deltaTime = 0.0f;    //InTick
	inline float _deltaTick = 0.0f;    //InTick

	inline float _updateDeltaTime = 0.0f;    //InTick
	inline float _updateDeltaTick = 0.0f;    //InTick

	inline float _globalTime = 0.0f;     //Sec    | No Pause
	inline float _globalTick = 0.0f;     //Tick   | NO Pause

	inline float _updateTime = 0.0f;     //Sec
	inline float _updateTick = 0.0f;     //Tick

	std::vector<::OS::ApplicationListener*> applicationListeners;

	std::mutex lockTask{};
	std::mutex lockAsyncTask{};
	std::mutex lockAsyncPackTask{};
	std::vector<std::function<void()>> postTasks;
	std::vector<std::pair<std::function<void()>, std::promise<void>>> postAsyncTasks;
	std::vector<std::packaged_task<void()>> postAsyncPackedTasks;

	std::thread::id mainThreadID{};

	inline bool paused = false;

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
		_globalTime = _globalTick = _updateTime = _updateTick = 0;
	}

	inline float delta(){
		return _deltaTime;
	}

	unsigned int getFPS() {
		FPS_reload += _deltaTime;

		totalFrames += 1.0f;
		if(FPS_reload > 1.0f) {
			FPS_last = static_cast<unsigned int>(totalFrames);
			FPS_reload = 0.0f;
			totalFrames = 0.0f;
		}

		return FPS_last;
	}

	// ReSharper disable once CppDFAConstantFunctionResult
	inline float deltaTick(){
		return _deltaTick;
	}

	inline float updateDelta(){
		return _updateTime;
	}

	// ReSharper disable once CppDFAConstantFunctionResult
	inline float updateDeltaTick(){
		return _updateDeltaTick;
	}

	inline float globalTime(){
		return _globalTime;
	}

	inline float globalTick(){
		return _globalTick;
	}

	inline float updateTime(){
		return _updateTime;
	}

	inline float updateTick(){
		return _updateTick;
	}

	void pause() {
		paused = true;
	}

	bool isPaused() {
		return paused;
	}

	void resume() {
		paused = false;
	}

	//TODO fire an event?
	void setPause(const bool v) {
		if(v)pause();
		else resume();
	}

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
	void registerListener(ApplicationListener* listener){
		applicationListeners.push_back(listener);
	}

	void removeListener(ApplicationListener* listener){
		std::erase(applicationListeners, listener);
	}

	void clearListeners(){
		applicationListeners.clear();
	}

	void launch(){
		std::signal(SIGABRT, exitApplication);
		std::signal(SIGILL, exitApplication);
		std::signal(SIGSEGV, exitApplication);
		std::signal(SIGINT, exitApplication);
		std::signal(SIGTERM, exitApplication);

		deltaSetter = [](float& f){
			f = static_cast<float>(glfwGetTime()) - _globalTime;
		};

		globalTimeSetter = [](float& f){
			f = static_cast<float>(glfwGetTime());
		};

		mainThreadID = std::this_thread::get_id();
	}

	std::thread::id getMainThreadID() {
		return mainThreadID;
	}

	void exitWith(const std::string& what) {
		exitApplication(SIGTERM, what);
	}

	bool continueLoop(GLFWwindow* window){
		return !glfwWindowShouldClose(window);
	}

	void setupLoop() {
		loopBegin = true;
	}

	bool getLoop() {
		return loopBegin;
	}

	void terminateLoop() {
		loopBegin = false;
	}

	void poll(GLFWwindow* window){
		glfwSwapBuffers(window);
		glfwPollEvents();

		deltaSetter(_deltaTime);
		_deltaTick = _deltaTime * TicksPerSecond;

		_updateDeltaTime = paused ? 0.0f : _deltaTime;
		_updateDeltaTick = paused ? 0.0f : _deltaTick;

		globalTimeSetter(_globalTime);
		_globalTick = _globalTime * TicksPerSecond;

		_updateTime += _updateDeltaTime;
		_updateTick = _updateTime * TicksPerSecond;
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
				if(!paused)listener->update(_updateDeltaTick);
			}else {
				listener->update(_deltaTick);
			}

			listener->updateGlobal(_deltaTick);
		}

		updateSignalManager.fire(Event::CycleSignalState::end);
	}
}


