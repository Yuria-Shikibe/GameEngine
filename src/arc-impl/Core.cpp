module Core;

import ext.RuntimeException;
import GL;
import GL.Constants;
import OS.KeyBind;
import OS;
import Geom.Vector2D;

import std;

using namespace Core;



void Core::initMainWindow() {
	platform->window->initWindow(title);
	platform->window->setVerticalSync();
	platform->window->setWindowCallback();

	if(maximizeWinOnInit) {
		platform->window->setMaximumizedWindow();
	}

	GL::init();
	auto [x, y] = Core::platform->window->equrySize();
	GL::viewport(x, y);
	platform->window->lastScreenBound.set(0, 0, x, y);
	GL::enable(GL_MULTISAMPLE);
}

void Core::initFileSystem() {
#if defined(_DEBUG) && defined(ASSETS_DIR)
	rootFileTree = std::make_unique<OS::FileTree>(ASSETS_DIR);
#else
	const auto dir = Core::platform->getProcessFileDir().subFile("resource");

	std::cout << "Targeted Resource Root:" << dir.absolutePath() << std::endl;

	rootFileTree = std::make_unique<OS::FileTree>(dir);

#endif
	OS::crashFileGetter = std::bind(&Core::Log::generateCrashFilePath, log);
}

void Core::initCore(const std::function<void()>& initializer) {
	initMainWindow();

	OS::launchApplication();

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
