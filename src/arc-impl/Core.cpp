module Core;

import Graphic;
import RuntimeException;
import GL;
import GL.Constants;
import OS.Key;
import OS;

using namespace Core;

void Core::setScreenBound(GLFWwindow* win) {
	glfwGetWindowSize(win, lastScreenBound.getWidthRaw(), lastScreenBound.getHeightRaw());
	glfwGetWindowPos(win, lastScreenBound.getSrcXRaw(), lastScreenBound.getSrcYRaw());
}

void Core::initMainWindow() {
	mainMonitor = glfwGetPrimaryMonitor();

	mainScreenMode = Graphic::getVideoMode(mainMonitor);

	OS::screenWidth = mainScreenMode->width;
	OS::screenHeight = mainScreenMode->height;
	OS::refreshRate = mainScreenMode->refreshRate;

#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	mainWindow = Graphic::initWindow(title, OS::screenWidth, OS::screenHeight, nullptr);

	if(maximizedWin) {
		glfwSetWindowSizeCallback(mainWindow, [](GLFWwindow* win, const int width, const int height){
			Core::lastScreenBound.setSize(width, height);
		});

		glfwMaximizeWindow(mainWindow);
		setScreenBound();

		glViewport(0, 0, lastScreenBound.getWidth(), lastScreenBound.getHeight());
		glfwSwapInterval(1);
		GL::enable(GL_MULTISAMPLE);

		lastScreenBound.set(100, 100, OS::screenWidth * 0.75f, OS::screenHeight * 0.75f);

		glfwSetWindowSizeCallback(mainWindow, nullptr);
	}

}

void Core::initFileSystem() {
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
	OS::crashFileGetter = std::bind(&Core::Log::generateCrashFilePath, log);
}

void Core::initCore(const std::function<void()>& initializer) {
	Graphic::initOpenGL();

	initMainWindow();

	OS::launch();
	GL::init();

	initFileSystem();

	if(initializer){
		initializer();
	}

	{
		if(!input)input = new Input(mainWindow);
		if(!camera)camera = new Camera2D();
		if(!log)log = new Log{rootFileTree->findDir("logs")};
	}
}

void Core::initCore_Post(const std::function<void()>& initializer) {
	if(initializer) {
		initializer();
	}

	if(!batch)throw ext::NullPointerException{"Empty Default Batch!"};

	OS::registerListener(input);
	OS::registerListener(camera);

	assetsManager = new Assets::Manager{};
}

void Core::loadAssets() {
	assetsManager->pullRequest();
	assetsManager->load_Visible(renderer->getWidth(), renderer->getHeight(), mainWindow, renderer);
	assetsManager->loadPost();
	assetsManager->loadEnd();
}

void Core::dispose() {
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
