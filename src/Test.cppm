module;

export module Test;

import <GLFW/glfw3.h>;

import OS;
import OS.File;
import OS.ApplicationListenerSetter;
import Image;
import Core;
import Core.Batch;
import Core.Batch.Batch_Sprite;
import Assets.Graphic;
import Assets.Sound;
import Assets.Loader;
import Assets.TexturePacker;
import Assets.Manager;
import Assets.Cursor;
import Graphic.Draw;

import GL;
import GL.Texture;
import GL.Shader;
import UI.Root;
import UI.Styles;
import Core.Renderer;
import Ctrl.Constants;
import Ctrl.ControlCommands;

export namespace Test {
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
				Assets::TexturePackPage* testPage = event.manager->getAtlas().registerPage("test", Assets::texCacheDir);
				Assets::textureDir.subFile("test").forAllSubs([testPage](OS::File&& file) {
					testPage->pushRequest(file);
				});

				testPage->pushRequest(Assets::textureDir.find("white.png"));

				Assets::TexturePackPage* normalTexture = event.manager->getAtlas().registerPage("normal", Assets::texCacheDir);
				normalTexture->linkTarget = testPage;

				Assets::textureDir.subFile("test").forAllSubs([normalTexture](OS::File&& file) {
					normalTexture->pushRequest(file);
				});
			}
		});

		Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadEnd>([](const auto& event) {
			Assets::Textures::whiteRegion = *event.manager->getAtlas().find("test-white");
			Assets::Textures::whiteRegion.shrinkEdge(15.0f);

			for (auto& texture : event.manager->getAtlas().getPage("ui").getTextures()) {
				texture->setScale(GL::TexParams::nearest, GL::TexParams::linear);
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
		});

		//Majority Load
		Core::loadAssets();
	}

	void setupAudioTest() {
		Core::input->registerKeyBind(Ctrl::KEY_F, Ctrl::Act_Press, [] {

		});
	}
}