module Assets.Load.MainLoader;

import OS;
import Core;
import Core.MainLoopManager;
import Core.Renderer;
import Assets.LoaderRenderer;

import Assets.Directories;
import Assets.Graphic;
import Assets.Sound;

void Assets::Load::MainLoader::pull_builtin(){
	Assets::Shaders::load(shaderLoader);

	tempFontLoader.setBindedPage(&tempPage);
	Assets::Fonts::loadPreivous(tempFontLoader);

	Assets::Fonts::pull(fontLoader);

	Assets::Sounds::load(Assets::Dir::sound, soundLoader);
}

void Assets::Load::MainLoader::post_builtin(){
	Assets::loadAfter();
}

void Assets::Load::MainLoader::loadWith(Core::Renderer* renderer){
	pull_builtin();

	texturePacker.setCacheDir(Assets::Dir::cache.subFile("tex"));

	registerTask(texturePacker);
	registerTask(fontLoader);
	registerTask(miscs);
	registerTask(soundLoader);
	registerTask(shaderLoader);


	launch();

	if(renderer){
		auto loadRenderer = Assets::LoaderRenderer{renderer->getWidth(), renderer->getHeight(), this};
		renderer->registerSynchronizedResizableObject(&loadRenderer);
		Core::platform->pollEvents();

		while (!Core::platform->shouldExit()){
			if(isFinished()) {
				requestDone();
				if(loadRenderer.lastProgress > 0.999f) {
					break;
				}
			}else{
				processRequests();
			}

			loadRenderer.update();
			loadRenderer.draw();

			Core::platform->pollEvents();
			Core::loopManager->timer.fetchGlobalTime();
		}

		Core::batchGroup.overlay->reset();

		renderer->removeSizeSynchronizedResizableObject(&loadRenderer);
	}else{
		while(!isFinished()){
			processRequests();
		}
	}

	if(!isFinished()) { //TODO better interruption
		requestStop();
	}else{
		requestDone();
	}

	post_builtin();
}
