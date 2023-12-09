module ;

export module OS;

import <GLFW/glfw3.h>;
import <chrono>;
import <ctime>;
import <csignal>;
import <sstream>;
import <functional>;
import <iostream>;
import <vector>;
import Event;
import File;
import StackTrace;
import Concepts;
import OS.ApplicationListener;
import <future>;
import <utility>;

namespace OS{
	bool loopBegin = false;

	inline float _deltaTime = 0.0f;    //InTick
	inline float _deltaTick = 0.0f;    //InTick

	inline float _updateDeltaTime = 0.0f;    //InTick
	inline float _updateDeltaTick = 0.0f;    //InTick

	inline float _globalTime = 0.0f;     //Sec
	inline float _globalTick = 0.0f;     //Tick

	inline float _updateTime = 0.0f;     //Sec   | No Pause
	inline float _updateTick = 0.0f;     //Tick  | NO Pause

	std::vector<ApplicationListener*> applicationListeners;
	std::vector<std::function<void()>> postTasks;
	std::vector<std::pair<std::function<void()>, std::promise<void>>> postAsyncTasks;

}

inline void exit_(int s, const std::string& what);

inline void exit_(const int s) {
	exit_(s, "");
}

inline bool paused = false;

export namespace OS{
	std::vector<std::string> args{};

	enum class LoopSignal {
		begin, end,
		maxCount
	};

	Event::SignalManager<LoopSignal, LoopSignal::maxCount> updateSignalManager{};


	//Should Be Done In Application Launcher
	std::function<void(float&)> deltaSetter = nullptr;
	std::function<void(float&)> globalTimeSetter = nullptr;
	std::function<File()> crashFileGetter = nullptr;


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

	template <Concepts::Invokable<void()> Func>
	void post(const Func& func) {
		postTasks.push_back(&func);
	}

	template <Concepts::Invokable<void()> Func>
	std::future<void> postAsync(Func&& func) {
		std::promise<void> promise;
		std::future<void>&& fu = promise.get_future();
		postAsyncTasks.emplace_back(std::forward<Func>(func), std::move(promise));
		return fu;
	}

	template <Concepts::Invokable<void()> Func>
	void postAsync(Func&& func, std::promise<void>&& promise) {
		postAsyncTasks.emplace_back(std::forward<Func>(func), std::move(promise));
	}

	inline void registerListener(ApplicationListener* listener){
		applicationListeners.push_back(listener);
	}

	inline void removeListener(ApplicationListener* listener){
		std::erase(applicationListeners, listener);
	}

	inline void clearListeners(){
		applicationListeners.clear();
	}

	inline void launch(){
		std::signal(SIGABRT, exit_);
		std::signal(SIGILL, exit_);
		std::signal(SIGSEGV, exit_);
		std::signal(SIGINT, exit_);
		std::signal(SIGTERM, exit_);

		deltaSetter = [](float& f){
			f = static_cast<float>(glfwGetTime()) - _globalTime;
		};

		globalTimeSetter = [](float& f){
			f = static_cast<float>(glfwGetTime());
		};
	}

	inline void exitWith(const std::string& what) {
		exit_(SIGTERM, what);
	}

	inline bool continueLoop(GLFWwindow* window){
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

	inline void poll(GLFWwindow* window){
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

	inline void update(){
		for(const auto task : postTasks){
			task();
		}

		postTasks.clear();

		for(auto& [task, promise] : postAsyncTasks){
#ifdef DEBUG_LOCAL
			try {task();}catch(...) {promise.set_exception(std::current_exception());}
#else
			task();
#endif
			promise.set_value();
		}

		postAsyncTasks.clear();

		updateSignalManager.fire(LoopSignal::begin);

		for(const auto & listener : applicationListeners){
			listener->update(_deltaTick);
		}

		updateSignalManager.fire(LoopSignal::end);
	}
}


void exit_(const int s, const std::string& what = "") {
	std::stringstream ss;

	if(!what.empty()) {
		ss << what << "\n\n";
	}

	ss << "Crashed! : Code-" << s << '\n';

	ext::getStackTraceBrief(ss);

	const OS::File& file = OS::crashFileGetter();

	file.writeString(ss.str());

	// t.join();

	std::exit(s);
}


