module;

#include <ranges>
#include <chrono>
#include <iostream>

module Assets.Manager;

import Core.Renderer;
import Assets.Graphic;
import Assets.Loader;
import Assets.LoaderRenderer;
import Assets.TexturePacker;
import Graphic.TextureAtlas;
import GL.Shader.Manager;
import Font;
import OS.File;
import Font.GlyphArrangement;
import OS;
import Event;

void Assets::Manager::pullRequest() {
	loadEvents.fire(AssetsLoadPull{this});

	Assets::Shaders::load(&shaders);

	Assets::Fonts::loadPreivous(tempFontLoader.get());
	Assets::Fonts::load(&fonts);

	Assets::Sounds::load(Assets::soundDir, soundLoader);

	for(auto& page : atlas.getPages() | std::ranges::views::values) {
		loader.push(&page);
	}

	loader.push(&shaders);
	loader.push(&fonts);
	loader.push(&soundLoader);
}

void Assets::Manager::load_Visible(const unsigned width, const unsigned height, GLFWwindow* window, Core::Renderer* renderer) {
	loadEvents.fire(AssetsLoadBegin{this});
	loader.begin();

	auto loadRenderer = Assets::LoaderRenderer{width, height, &loader};
	renderer->registerSynchronizedResizableObject(&loadRenderer);
	while (OS::continueLoop(window)){
		if(loader.finished()) {
			if(loadRenderer.lastProgress > 0.999f) {
				break;
			}
		}else{
			loader.processRequests();
		}

		loadRenderer.draw();

		OS::poll(window);
	}

	if(!loader.finished()) { //TODO better interruption
		loader.interrupt();
	}
	renderer->removeSizeSynchronizedResizableObject(&loadRenderer);
}

void Assets::Manager::load_Quiet(const bool forceGet) {
	loadEvents.fire(AssetsLoadBegin{this});
	loader.begin();
	if(forceGet)loader.forceGet();
}

void Assets::Manager::loadPost() {
	loadEvents.fire(AssetsLoadPost{this});

	atlas.flush();
	Assets::loadAfter();

	Font::glyphParser->context.defaultFont = Assets::Fonts::telegrama;
	Font::glyphParser->fontLib = fonts.manager.get();
}

void Assets::Manager::loadEnd() {
	loadEvents.fire(AssetsLoadEnd{this});
	tempFontLoader.reset(nullptr);

	for (auto& page : atlas.getPages() | std::ranges::views::values) {
		page.clearData();
	}

	std::cout << std::format("[Info]: Loading Cost Time: {}.sec", static_cast<float>(loader.getTimer().toMark().count()) / 1000.0f) << std::endl;
}

GL::ShaderManager& Assets::Manager::getShaders() {
	return shaders;
}

Graphic::TextureAtlas& Assets::Manager::getAtlas() {
	return atlas;
}

Font::FontCache& Assets::Manager::getonts() const{
	return *fonts.manager;
}

Assets::AssetsLoader& Assets::Manager::getLoader() {
	return loader;
}

Event::EventManager& Assets::Manager::getEventTrigger() {
	return loadEvents;
}

Assets::SoundLoader& Assets::Manager::getSoundLoader() {
	return soundLoader;
}
