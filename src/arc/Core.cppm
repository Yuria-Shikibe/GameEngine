module;

export module Core;

import <functional>;
import <glad/glad.h>;
import <GLFW/glfw3.h>;

import Graphic;
import Geom.Shape.Rect_Orthogonal;

import Core.Audio;
import Assets.Manager;
import Assets.Bundle;
import Core.Settings;
import Core.Log;

import UI.Root;

import RuntimeException;

export import Core.Batch.Batch_Sprite;
export import Core.Batch;
export import Core.Input;
export import Core.Camera;
export import Core.Renderer;
export import GL;
export import GL.Constants;
export import OS.FileTree;
export import OS.Key;
export import OS;
import <iostream>;



export namespace Core{
	const std::string title = APPLICATION_NAME;

	inline GLFWwindow* window = nullptr;
	inline GLFWmonitor* mainMonitor = nullptr;
	inline GLFWmonitor* currentMonitor = nullptr;

	inline Geom::Shape::OrthoRectInt lastScreenBound{};
	inline bool maximizedWin = true;
	inline void setScreenBound(GLFWwindow* win = window) {
		glfwGetWindowSize(win, lastScreenBound.getWidthRaw(), lastScreenBound.getHeightRaw());
		glfwGetWindowPos(win, lastScreenBound.getSrcXRaw(), lastScreenBound.getSrcYRaw());
	}

	inline const GLFWvidmode* mainScreenMode = nullptr;
	//TODO using unique ptr?

	/* Almost Done */
	inline Input* input = nullptr;
	/* Basically Done */
	inline Camera2D* camera = nullptr;
	/* Basically Done */
	inline Batch* batch = nullptr;
	/* Basically Done */
	inline Renderer* renderer = nullptr;
	/* 88.00% */
	inline OS::FileTree* rootFileTree = nullptr;

	//TODO main components... maybe more

	/* 0.00% */
	inline Audio* audio = nullptr;
	/* 90.00% */
	inline Assets::Manager* assetsManager = nullptr;
	/* 0.00% */
	inline Settings* settings = nullptr;
	/* 3.00% */
	inline UI::Root* uiRoot = nullptr;
	/* 0.00% */
	inline Assets::Bundle* bundle = nullptr;
	/* 30.00% */
	inline Log* log = nullptr;

	//TODO maybe some objects for task management, if necessary
	//TODO Async Impl...

	inline void initMainWindow() {
		mainMonitor = glfwGetPrimaryMonitor();

		mainScreenMode = Graphic::getVideoMode(mainMonitor);

		OS::screenWidth = mainScreenMode->width;
		OS::screenHeight = mainScreenMode->height;
		OS::refreshRate = mainScreenMode->refreshRate;

#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

		window = Graphic::initWindow(title, OS::screenWidth, OS::screenHeight, nullptr);

		if(maximizedWin) {
			glfwSetWindowSizeCallback(window, [](GLFWwindow* win, const int width, const int height){
				Core::lastScreenBound.setSize(width, height);
			});

			// Graphic::fullScreen(window, currentMonitor, title);
			glfwMaximizeWindow(window);
			setScreenBound();

			glViewport(0, 0, lastScreenBound.getWidth(), lastScreenBound.getHeight());
			glfwSwapInterval(1);
			GL::enable(GL_MULTISAMPLE);

			lastScreenBound.set(100, 100, OS::screenWidth * 0.75f, OS::screenHeight * 0.75f);

			glfwSetWindowSizeCallback(window, nullptr);
		}

	}

	inline void initFileSystem() {
#ifdef DEBUG_LOCAL
	#ifdef ASSETS_DIR
		rootFileTree = new OS::FileTree{ASSETS_DIR};
	#else
		rootFileTree = new OS::FileTree{OS::args[0]};
	#endif
#else
		//TODO release build assets pacakge
	#ifdef ASSETS_DIR
		rootFileTree = new OS::FileTree{ASSETS_DIR};
	#else
		rootFileTree = new OS::FileTree{OS::args[0]};
	#endif
#endif
		OS::crashFileGetter = [] {
			return log->generateCrashFile();
		};
	}

	inline void initCore(const std::function<void()>& initializer = nullptr){
		Graphic::initOpenGL();

		initMainWindow();

		OS::launch();
		GL::init();

		initFileSystem();

		if(initializer){
			initializer();
		}

		{
			if(!input)input = new Input(window);
			if(!camera)camera = new Camera2D();
			if(!log)log = new Log{rootFileTree->findDir("logs")};
		}
	}

	/**
	* \brief Invoke this after assets load!
	**/
	inline void initCore_Post(const std::function<void()>& initializer = nullptr) {
		if(initializer) {
			initializer();
		}

		if(!batch)throw ext::NullPointerException{"Empty Default Batch!"};

		OS::registerListener(input);
		OS::registerListener(camera);

		assetsManager = new Assets::Manager{};
	}

	void loadAssets() {
		assetsManager->pullRequest();
		assetsManager->load_Visible(renderer->getWidth(), renderer->getHeight(), window, renderer);
		assetsManager->loadPost();
		assetsManager->loadEnd();
	}

	inline void dispose(){
		OS::clearListeners();

		delete input;
		delete camera;
		delete batch;
		delete renderer;
		delete rootFileTree;

		delete audio;
		delete assetsManager;
		delete settings;
		delete uiRoot;
		delete bundle;
		delete log;

		glfwTerminate();
	}
}