module;

#include <GLFW/glfw3.h>

export module Test;
import std;

import OS;
import OS.File;
import OS.ApplicationListenerSetter;
import Image;
import Core;
import Core.Batch.Batch_Sprite;
import Assets.Graphic;
import Assets.Sound;
import Assets.Loader;
import Assets.TexturePacker;
import Assets.Manager;
import Assets.Cursor;
import Graphic.Draw;

import GL;
import GL.Constants;
import GL.Texture;
import GL.VertexArray;
import GL.TextureArray;
import GL.Shader;
import UI.Root;
import UI.Styles;
import Core.Renderer;
import Ctrl.Constants;
import Ctrl.ControlCommands;

import Game.Core;
import Game.Content.Builtin.SpaceCrafts;

export namespace Test{
	constexpr std::string_view MainPageName = "base";
	constexpr std::string_view BindPageName = "bind";

	void init(const int argc, char* argv[]){
		//TODO move these into application loader
		//Register Cmd
		OS::args.reserve(argc);
		for(int i = 0; i < argc; ++i) OS::args.emplace_back(argv[0]);

		stbi::setFlipVertically_load(true);
		stbi::setFlipVertically_write(true);

		Core::initCore();

		OS::loadListeners(Core::mainWindow);
		//
		Assets::loadBasic();
		//
		OS::setApplicationIcon(Core::mainWindow, stbi::obtain_GLFWimage(Assets::assetsDir.subFile("icon.png")).get());

		Core::initCore_Post([]{
			Core::batchGroup.batchOverlay = std::make_unique<Core::SpriteBatch<>>([](const Core::SpriteBatch<>& self){
				auto* const shader = Assets::Shaders::screenSpace;

				shader->setUniformer([&self](const GL::Shader& s){
					s.setTexture2D("u_texture", self.getTexture());
					s.setMat3("view", *self.getProjection());
				});

				return shader;
			});

			Core::batchGroup.batchOverlay->setProjection(&Core::camera->getWorldToScreen());

			{
				int w, h;
			   glfwGetWindowSize(Core::mainWindow, &w, &h);
			   Core::uiRoot = new UI::Root{};
			   Core::renderer = new Core::Renderer{static_cast<unsigned>(w), static_cast<unsigned>(h)};

			   Core::uiRoot->resize(w, h);
			   Core::camera->resize(w, h);
			}

			Core::renderer->registerSynchronizedResizableObject(Core::camera);
			Core::renderer->registerSynchronizedResizableObject(Core::uiRoot);

			Ctrl::registerCommands(Core::input);
			OS::registerListener(Core::uiRoot);

			Core::input->registerSubInput(Core::uiRoot->uiInput.get());
		});

		Graphic::Draw::setDefTexture(&Assets::Textures::whiteRegion);
		Graphic::Draw::setTexture();
		Graphic::Frame::rawMesh = Assets::Meshes::raw;
		Graphic::Frame::blitter = Assets::Shaders::blit;

		Game::core = std::make_unique<Game::Core>();
		OS::registerListener(Game::core.get());
	}

	void assetsLoad(){
		Game::Content::Builtin::load_SpaceCraft(Game::core->contentLoader.get());

		Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadInit>([](const auto& event) {
			{
				Assets::TexturePackPage* uiPage = event.manager->getAtlas().registerPage("ui", Assets::texCacheDir);
				uiPage->forcePack = true;
				Assets::textureDir.subFile("ui").forAllSubs([uiPage](OS::File&& file) {
					uiPage->pushRequest(file);
				});

				Assets::TexturePackPage* cursorPage = event.manager->getAtlas().registerPage("cursor", Assets::texCacheDir);
				cursorPage->forcePack = true;
				Assets::textureDir.subFile("cursor").forAllSubs([cursorPage](OS::File&& file) {
					cursorPage->pushRequest(file);
				});
			}

			{
				Assets::TexturePackPage* mainPage = event.manager->getAtlas().registerPage(MainPageName, Assets::texCacheDir);
				mainPage->pushRequest("white-solid", Assets::textureDir.find("white.png"));
				Assets::TexturePackPage* normalPage = event.manager->getAtlas().registerAttachmentPage("normal", mainPage);
				Assets::TexturePackPage* lightPage = event.manager->getAtlas().registerAttachmentPage("light", mainPage);
				lightPage->pushRequest("white-light", Assets::textureDir.find("white.light.png"));
				mainPage ->pushRequest("white-light", Assets::textureDir.find("transparent.png"));
			}
		});

		Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadPull>([](const auto& event) {
			Game::core->contentLoader->loadTexture(Assets::textureTree, event.manager->getAtlas());
		});

		Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadEnd>([](const Assets::AssetsLoadEnd& event) {
			Assets::Textures::whiteRegion = *event.manager->getAtlas().find("base-white-solid");
			Assets::Textures::whiteRegion.shrinkEdge(15.0f);
			Graphic::Draw::defaultSolidTexture = Graphic::Draw::defaultTexture;

			auto lightRegion = event.manager->getAtlas().find("base-white-light");
			Graphic::Draw::defaultLightTexture = lightRegion;
			const_cast<GL::TextureRegionRect*>(lightRegion)->shrinkEdge(15.0f);

			for (auto& texture : event.manager->getAtlas().getPage("ui").getTextures()) {
				texture->setScale(GL::TexParams::linear, GL::TexParams::linear);
			}

			for (auto& texture : event.manager->getAtlas().getPage(MainPageName).getTextures()) {
				texture->setScale(GL::TexParams::mipmap_linear_nearest, GL::TexParams::nearest);
			}

			Graphic::Draw::setDefTexture(&Assets::Textures::whiteRegion);
			Graphic::Draw::setTexture();

			UI::Styles::load(event.manager->getAtlas());

			{
				auto& ptr = Assets::getCursorRaw(Assets::CursorType::regular);
				ptr = std::make_unique<Assets::Cursor>();
				ptr->setImage(event.manager->getAtlas().find("cursor-regular"));
			}

			{
				auto& ptr = Assets::getCursorRaw(Assets::CursorType::select);
				ptr = std::make_unique<Assets::Cursor>();
				ptr->setImage(event.manager->getAtlas().find("cursor-select"));
				ptr->drawer = std::make_unique<Assets::CursorThoroughSightDrawer>();
			}

			event.manager->getAtlas().bindTextureArray(BindPageName, {MainPageName, "normal", "light"}, [](GL::Texture2DArray* tex){
				tex->setScale(GL::mipmap_linear_linear, GL::nearest);
			});

		});

		Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadPost>([](const auto& event) {
			Core::batchGroup.batchWorld = std::make_unique<Core::SpriteBatch<GL::VERT_GROUP_SIZE_WORLD>>([](const Core::SpriteBatch<GL::VERT_GROUP_SIZE_WORLD>& self) {
				auto* const shader = Assets::Shaders::world;

				shader->setUniformer([&self](const GL::Shader& s){
					s.setTexture2D("texArray", self.getTexture());
					s.setMat3("view", *self.getProjection());
				});

				return shader;
			}, [](GL::AttributeLayout& layout){
				layout.addFloat(3);
				layout.addFloat(2);
				layout.addFloat(4);
				layout.addFloat(4);
			});

			Core::batchGroup.batchWorld->setProjection(&Core::camera->getWorldToScreen());
		});

		//Majority Load
		Core::loadAssets();
	}

	void setupAudioTest(){}
}
