module;

export module Core;

import <functional>;
import <GLFW/glfw3.h>;

import Graphic;

import Core.Audio;
import Core.AssetsLoader;
import Core.Bundle;
import Core.Settings;
import Core.UIRoot;
import Core.Log;

import RuntimeException;

export import Core.Batch.Batch_Sprite;
export import Core.Batch;
export import Core.Input;
export import Core.Camera;
export import Core.Renderer;
export import GL.Constants;
export import OS.FileTree;
export import OS.Key;
export import OS;
import <iostream>;

export namespace Core{
	inline GLFWwindow* window = nullptr;
	inline GLFWmonitor* monitor = nullptr;
	//using unique ptr?

	/* Almost Done */
	inline Input* input = nullptr;
	/* Basically Done */
	inline Camera* camera = nullptr;
	/* Basically Done */
	inline Batch* batch = nullptr;
	/* Basically Done */
	inline Renderer* renderer = nullptr;
	/* 68.00% */
	inline OS::FileTree* rootFileTree = nullptr;

	//TODO main components... maybe more

	/* 0.00% */
	inline Audio* audio = nullptr;
	/* 0.00% */
	inline AssetsLoader* assetsLoader = nullptr;
	/* 0.00% */
	inline Settings* settings = nullptr;
	/* 0.00% */
	inline UIRoot* uiRoot = nullptr;
	/* 0.00% */
	inline Bundle* bundle = nullptr;
	/* 0.00% */
	inline Log* log = nullptr;

	//TODO maybe some objects for task management, if necessary
	//TODO Async Impl...

	inline void initCore(const std::function<void()>& initializer = nullptr){
		monitor = glfwGetPrimaryMonitor();

		Graphic::initGLFW();

#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

		window = Graphic::initWindow(APPLICATION_NAME);

		OS::launch();

#ifdef DEBUG_LOCAL
#ifdef ASSETS_DIR
		rootFileTree = new OS::FileTree{ASSETS_DIR};
#else
		rootFileTree = new OS::FileTree{};
#endif
#else
		rootFileTree = new OS::FileTree{};
#endif
		OS::crashFileGetter = [] {
			return log->generateCrashFile();
		};

		if(initializer){
			initializer();
		}

		{
			if(!input)input = new Input(window);
			if(!camera)camera = new Camera();
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

		if(!batch)throw ext::RuntimeException{};

		batch->setProjection(&camera->worldToScreen);

		input->scrollListener.emplace_back([]([[maybe_unused]] float x, const float y) -> void {
			camera->setScale(camera->getScale() + y * OS::delta() * 5.0f);
		});

		static float baseMoveSpeed = 50;

		input->registerKeyBind(true , new OS::KeyBind(GLFW_KEY_LEFT_SHIFT, GLFW_PRESS, [](const int k){baseMoveSpeed = 200;}));
		input->registerKeyBind(false, new OS::KeyBind(GLFW_KEY_LEFT_SHIFT, GLFW_RELEASE, [](const int k){baseMoveSpeed =  50;}));

		input->registerKeyBind(true , new OS::KeyBind(GLFW_KEY_A, GLFW_PRESS, [](const int k){camera->trans(-baseMoveSpeed * OS::delta(), 0);}));
		input->registerKeyBind(true , new OS::KeyBind(GLFW_KEY_D, GLFW_PRESS, [](const int k){camera->trans( baseMoveSpeed * OS::delta(), 0);}));
		input->registerKeyBind(true , new OS::KeyBind(GLFW_KEY_W, GLFW_PRESS, [](const int k){camera->trans(0,  baseMoveSpeed * OS::delta());}));
		input->registerKeyBind(true , new OS::KeyBind(GLFW_KEY_S, GLFW_PRESS, [](const int k){camera->trans(0, -baseMoveSpeed * OS::delta());}));

		auto keys = std::array{OS::KeyBind(GLFW_KEY_LEFT_SHIFT, GLFW_PRESS), OS::KeyBind(GLFW_KEY_SPACE, GLFW_PRESS) };
		input->registerKeyBindMulti(false, keys, []() {camera->setScale(1.0f);});

		OS::registerListener(input);
	}

	inline void dispose(){
		OS::clearListeners();

		delete input;
		delete camera;
		delete batch;
		delete renderer;
		delete rootFileTree;

		delete audio;
		delete assetsLoader;
		delete settings;
		delete uiRoot;
		delete bundle;
		delete log;

		glfwTerminate();
	}
}
