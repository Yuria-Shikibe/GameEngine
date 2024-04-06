module;

#include <GLFW/glfw3.h>

module Core;

import RuntimeException;
import GL;
import GL.Constants;
import OS.KeyBind;
import OS;
import Geom.Vector2D;

import std;

using namespace Core;


void Core::initMainWindow() {
#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	platform->window->initWindow(title);

	if(maximizeWinOnInit) {
		glfwSetWindowSizeCallback(platform->window->as<GLFWwindow>(), [](GLFWwindow* win, const int width, const int height){
			platform->window->windowBound.setSize(width, height);
		});

		glfwMaximizeWindow(platform->window->as<GLFWwindow>());

		GL::viewport(0, 0, platform->window->windowBound.getWidth(), platform->window->windowBound.getHeight());
		glfwSwapInterval(1);
		GL::enable(GL_MULTISAMPLE);

		platform->window->lastScreenBound.set(100, 100, platform->window->currentMonitor.width * 0.75f, platform->window->currentMonitor.height * 0.75f);

		glfwSetWindowSizeCallback(platform->window->as<GLFWwindow>(), nullptr);
	}

}

void Core::initFileSystem() {
#if defined(_DEBUG) && defined(ASSETS_DIR)
	rootFileTree = std::make_unique<OS::FileTree>(ASSETS_DIR);
#else
	const OS::File self{OS::args[0]};

	const auto dir = self.getParent().subFile("resource");

	std::cout << "Targeted Resource Root:" << dir.absolutePath() << std::endl;

	rootFileTree = std::make_unique<OS::FileTree>(dir);

#endif
	OS::crashFileGetter = std::bind(&Core::Log::generateCrashFilePath, log);
}

void Core::initCore(const std::function<void()>& initializer) {
	initMainWindow();

	OS::launchApplication();
	GL::init();

	initFileSystem();

	if(initializer){
		initializer();
	}

	{
		if(!input)input = new Input{};
		if(!camera)camera = new Camera2D{};
		if(!log)log = new Log{rootFileTree->findDir("logs")};
	}

	audio = new Core::Audio;
}

void Core::initCore_Post(const std::function<void()>& initializer) {
	if(initializer) {
		initializer();
	}

	if(!batchGroup.batchOverlay)throw ext::NullPointerException{"Empty Overlay Batch!"};

	OS::registerListener(input);
	OS::registerListener(camera);

	assetsManager = new Assets::Manager{};
}

void Core::loadAssets() {
	assetsManager->getSoundLoader().setEngine(audio->engine);
	assetsManager->pullRequest();
	assetsManager->load_Visible(renderer->getWidth(), renderer->getHeight(), renderer);
	assetsManager->loadPost();
	assetsManager->loadEnd();
}

void Core::dispose() {
	OS::clearListeners();

	delete input;
	delete camera;
	delete renderer;

	delete audio;
	delete assetsManager;
	delete settings;
	delete uiRoot;
	delete bundle;
	delete log;

	platform.reset();
}
