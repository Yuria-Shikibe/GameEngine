module Core;

import Core.MainLoopManager;
import ext.RuntimeException;
import GL;
import GL.Constants;
import OS;
import Geom.Vector2D;

import std;
import Font.GlyphArrangement;
import Assets.Graphic;

void Core::initPlatform(const int argc, char* argv[]){
	initCurrentPlatform(platform, title, argc, argv);
}

void Core::initMainWindow(const bool maximizeWinOnInit) {
	platform->window->initWindow(title);
	platform->window->setVerticalSync();
	platform->window->setWindowCallback();

	if(maximizeWinOnInit) {
		platform->window->setMaximumizedWindow();
	}

	GL::init();
	const auto size = Core::platform->window->equrySize();
	GL::viewport(size);
	platform->window->lastScreenBound.setSize(size);
	GL::enable(GL_MULTISAMPLE);
}

void Core::initFileSystem() {
#if defined(ASSETS_DIR)
	rootFileTree = OS::FileTree(ASSETS_DIR);
#else
	const auto dir = Core::platform->getCurrentParentDir();

	std::cout << "Targeted Resource Root:" << dir.absolutePath() << std::endl;

	rootFileTree = OS::FileTree{dir};

#endif
}

void Core::initCore_Kernal(const std::function<void()>& initializer) {
	initFileSystem();

	if(initializer){
		initializer();
	}

	if(!log){
		log = new Log{rootFileTree.findDir("logs")};
	}

	if(!loopManager){
		loopManager = std::make_unique<MainLoopManager>();
	}

	loopManager->timer.setDeltaSetter(platform->getGlobalDeltaSetter());
	loopManager->timer.setTimerSetter(platform->getGlobalTimeSetter());
}

void Core::initCore_Interactive(const std::function<void()>& initializer) {
	if(!camera){
		camera = std::make_unique<Camera2D>();
	}

	if(!audio){
		audio = new Core::Audio;
	}

	Core::focus.camera.current = Core::focus.camera.fallback = Core::camera.get();

	if(initializer) {
		initializer();
	}

	if(!batchGroup.overlay)throw ext::NullPointerException{"Empty Overlay Batch!"};
}

void Core::beginLoadAssets(){
	assetsLoader.fontLoader.setBindedPage(assetsManager.atlas.getUIPage());
	assetsLoader.soundLoader.setEngine(audio->engine);

	for (auto && page : assetsManager.atlas.getPages() | std::views::values){
		assetsLoader.texturePacker.pushPage(&page);
	}


	assetsLoader.loadWith(renderer);

	assetsLoader.phaseArriveEventManager.on<Assets::Load::Phase::post_load>([]{

	});
}

void Core::endLoadAssets(){
	assetsManager.fonts = std::move(assetsLoader.fontLoader).cropStorage();
	assetsManager.atlas.flush();

	Font::defGlyphParser->context.defaultFont = Assets::Fonts::telegrama;
	Font::defGlyphParser->loadedFonts = &assetsManager.fonts;
}

void Core::dispose() {
	for (const auto & destructor : destructors){
		destructor();
	}

	loopManager->clearListeners();

	delete renderer;

	delete audio;
	delete settings;
	delete uiRoot;
	delete log;

	//Reset
	batchGroup.~BatchGroup();
	new (&batchGroup) BatchGroup;

	platform.reset();
}

Core::MainLoopManager* Core::getLoopManager(){
	return loopManager.get();
}
