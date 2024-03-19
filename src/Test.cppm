module;

export module Test;

import <GLFW/glfw3.h>;
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

export namespace Test {
	constexpr std::string_view MainPageName = "test";
	constexpr std::string_view BindPageName = "bind";

	void init(const int argc, char* argv[]) {
		//TODO move these into application loader
		//Register Cmd
		OS::args.reserve(argc);
		for(int i = 0; i < argc; ++i)OS::args.emplace_back(argv[0]);

		stbi::setFlipVertically_load(true);
		stbi::setFlipVertically_write(true);

		Core::initCore();

		OS::loadListeners(Core::mainWindow);
		//
		Assets::loadBasic();
		//
		OS::setApplicationIcon(Core::mainWindow, stbi::obtain_GLFWimage(Assets::assetsDir.subFile("icon.png")).get());

		Core::initCore_Post([] {
			Core::overlayBatch = new Core::SpriteBatch([](const Core::SpriteBatch<>& self) {
				auto* const shader = Assets::Shaders::screenSpace;

				shader->setUniformer([&self](const GL::Shader& s){
					s.setTexture2D("u_texture", self.getTexture());
					s.setMat3("view", *self.getProjection());
				});

				return shader;
			});

			Core::overlayBatch->setProjection(&Core::camera->getWorldToScreen());

			int w, h;
			glfwGetWindowSize(Core::mainWindow, &w, &h);
			Core::uiRoot = new UI::Root{};
			Core::renderer = new Core::Renderer{static_cast<unsigned>(w), static_cast<unsigned>(h)};

			Core::uiRoot->resize(w, h);
			Core::camera->resize(w, h);

			Core::renderer->registerSynchronizedResizableObject(Core::camera);
			Core::renderer->registerSynchronizedResizableObject(Core::uiRoot);

			Ctrl::registerCommands(Core::input);
			OS::registerListener(Core::uiRoot);
		});

		Graphic::Draw::defTexture(&Assets::Textures::whiteRegion);
		Graphic::Draw::texture();
		Graphic::Draw::rawMesh = Assets::Meshes::raw;
		Graphic::Draw::blitter = Assets::Shaders::blit;
	}

	void assetsLoad(){
		Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadPull>([](const auto& event) {
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
				Assets::textureDir.subFile(MainPageName).forAllSubs([mainPage](OS::File&& file) {
					mainPage->pushRequest(file);
				});
				mainPage->pushRequest(Assets::textureDir.find("white.png"));

				Assets::TexturePackPage* normalTexture = event.manager->getAtlas().registerAttachmentPage("normal", mainPage);
				Assets::TexturePackPage* lightTexture = event.manager->getAtlas().registerAttachmentPage("light", mainPage);

				Assets::textureDir.subFile(MainPageName).forAllSubs([normalTexture](OS::File&& file) {
					normalTexture->pushRequest(file);
				});

				Assets::textureDir.subFile(lightTexture->pageName).forAllSubs([lightTexture](OS::File&& file) {
					lightTexture->pushRequest(file);
				});
			}
		});

		Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadEnd>([](const Assets::AssetsLoadEnd& event) {
			Assets::Textures::whiteRegion = *event.manager->getAtlas().find("test-white");
			Assets::Textures::whiteRegion.shrinkEdge(15.0f);

			for (auto& texture : event.manager->getAtlas().getPage("ui").getTextures()) {
				texture->setScale(GL::TexParams::linear, GL::TexParams::linear);
			}

			for (auto& texture : event.manager->getAtlas().getPage(MainPageName).getTextures()) {
				texture->setScale(GL::TexParams::mipmap_linear_nearest, GL::TexParams::nearest);
			}

			Graphic::Draw::defTexture(&Assets::Textures::whiteRegion);
			Graphic::Draw::texture();

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
			Core::worldBatch = new Core::SpriteBatch<GL::VERT_GROUP_SIZE_WORLD>([](const Core::SpriteBatch<GL::VERT_GROUP_SIZE_WORLD>& self) {
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

			Core::worldBatch->setProjection(&Core::camera->getWorldToScreen());
		});

		//Majority Load
		Core::loadAssets();
	}

	void setupAudioTest() {

	}
}