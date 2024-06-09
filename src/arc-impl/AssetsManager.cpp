module Assets.Manager;

import std;

import Core;

import Assets.Directories;
import Assets.Graphic;
import Assets.Sound;
import Assets.Loader;
import Assets.LoaderRenderer;
import Assets.TexturePacker;
import Graphic.TextureAtlas;
import GL.Shader.Manager;
import Font;
import OS.File;
import Font.GlyphArrangement;
import OS;
import ext.Event;

void Assets::Manager::pullRequest() {
	loadEvents.fire(AssetsLoadInit{this});

	Assets::Shaders::load(&shaders);
	Assets::Fonts::loadPreivous(tempFontLoader.get());
	Assets::Fonts::load(&fonts);

	Assets::Sounds::load(Assets::Dir::sound, soundLoader);

	for(auto& page : atlas.getPages() | std::ranges::views::values) {
		loader.push(&page);
	}


	loader.push(&shaders);
	loader.push(&fonts);

	loader.push(&soundLoader);

	loadEvents.fire(AssetsLoadPull{this});

	fonts.texturePage->setHandler(fonts.getHandler());
	loader.begin();
}

void Assets::Manager::load_Visible(const int width, const int height, Core::Renderer* renderer) {

	auto loadRenderer = Assets::LoaderRenderer{width, height, &loader};
	renderer->registerSynchronizedResizableObject(&loadRenderer);
	Core::platform->pollEvents();

	while (!Core::platform->shouldExit()){
		if(loader.finished()) {
			if(loadRenderer.lastProgress > 0.999f) {
				break;
			}
		}else{
			loader.processRequests();
		}

		loadRenderer.update();
		loadRenderer.draw();

		Core::platform->pollEvents();
		OS::pollWindowEvent();
	}

	Core::batchGroup.overlay->reset();

	if(!loader.finished()) { //TODO better interruption
		loader.interrupt();
	}
	renderer->removeSizeSynchronizedResizableObject(&loadRenderer);
}

void Assets::Manager::load_Quiet(const bool forceGet) {
	loadEvents.fire(AssetsLoadPull{this});
	loader.begin();
	if(forceGet)loader.forceGet();
}

void Assets::Manager::loadPost() {
	loadEvents.fire(AssetsLoadPost{this});

	atlas.flush();
	Assets::loadAfter();

	Font::defGlyphParser->context.defaultFont = Assets::Fonts::telegrama;
	Font::defGlyphParser->loadedFonts = fonts.atlas.get();
}

void Assets::Manager::loadEnd() {
	loadEvents.fire(AssetsLoadEnd{this});
	tempFontLoader.reset(nullptr);

	for (auto& page : atlas.getPages() | std::ranges::views::values) {
		page.clearData();
	}

	std::cout << std::format("[Info]: Loading Cost Time: {}.sec", static_cast<float>(loader.getTimer().toMark().count()) / 1000.0f) << std::endl;
}
