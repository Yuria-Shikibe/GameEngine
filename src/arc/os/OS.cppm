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
import OS.ApplicationListener;

static float deltaTime_ = 0.0f;    //InTick
static float deltaTick_ = 0.0f;    //InTick

static float globalTime_ = 0.0f;     //Sec
static float globalTick_ = 0.0f;     //Tick

static float updateTime_ = 0.0f;     //Sec   | No Pause
static float updateTick_ = 0.0f;     //Tick  | NO Pause

inline void exit_(int s, const std::string& what);

inline void exit_(const int s) {
	exit_(s, "");
}

export namespace OS{
	// extern inline int width, height;
	Event::ApplicationMainLoop_Pre preEvent;
	Event::ApplicationMainLoop_After afterEvent;

	//Should Be Done In Application Launcher
	std::function<void(float&)> deltaSetter = nullptr;
	std::function<void(float&)> globalTimeSetter = nullptr;
	std::function<void(float&)> updateTimeSetter = nullptr;

	std::function<File()> crashFileGetter = nullptr;

	constexpr float Frames = 60.0f;

	inline float delta(){
		return deltaTime_;
	}

	inline float deltaTick(){
		return deltaTick_;
	}

	inline float globalTime(){
		return globalTime_;
	}

	inline float globalTick(){
		return globalTick_;
	}

	inline float updateTime(){
		return updateTime_;
	}

	inline float updateTick(){
		return updateTick_;
	}

	inline std::vector<ApplicationListener*> applicationListeners;

	inline std::vector<std::function<void()>> postTasks;

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

		deltaSetter = [](float& f){
			f = static_cast<float>(glfwGetTime()) - globalTime_;
		};

		updateTimeSetter = [](float& f){
			f += deltaTime_;
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

	inline void poll(GLFWwindow* window){
		glfwSwapBuffers(window);
		glfwPollEvents();

		deltaSetter(deltaTime_);
		deltaTick_ = deltaTime_ * Frames;

		globalTimeSetter(globalTime_);
		globalTick_ = globalTime_ * Frames;

		updateTimeSetter(deltaTime_);
		updateTick_ = updateTime_ * Frames;
	}

	inline void update(){
		for(const auto & postTask : postTasks){
			postTask();
		}

		postTasks.clear();

		Event::generalUpdateEvents.fire(preEvent);

		for(const auto & listener : applicationListeners){
			listener->update();
		}

//		getRenderer().draw();

		Event::generalUpdateEvents.fire(afterEvent);
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


