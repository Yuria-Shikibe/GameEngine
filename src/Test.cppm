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
import UI.Cursor;
import Graphic.Draw;
import Graphic.Pixmap;

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

import Font.GlyphArrangement;

import Game.Core;
import Game.Content.Builtin.SpaceCrafts;

import ext.Encoding;
import Image.Svg;

export namespace Test{
	constexpr std::string_view MainPageName = "base";
	constexpr std::string_view BindPageName = "bind";

	void init(const int argc, char* argv[]){
		Core::initPlatform(argc, argv);
		OS::timerSetter = Core::platform->getGlobalTimeSetter();
		OS::deltaSetter = Core::platform->getGlobalDeltaSetter();

		ext::setFlipVertically_load(true);
		ext::setFlipVertically_write(true);

		Core::initCore();

		Assets::loadBasic();

		{
			const Graphic::Pixmap appIcon{Assets::assetsDir.subFile("icon.png")};
			Core::platform->window->setApplicationIcon(appIcon.data(), appIcon.getWidth(), appIcon.getHeight(),
			                                           appIcon.Channels, 1);
		}

		Core::initCore_Post([]{
			Core::batchGroup.batchOverlay = std::make_unique<Core::SpriteBatch<>>([](const Core::SpriteBatch<>& self){
				auto* const shader = Assets::Shaders::screenSpace;

				shader->setUniformer([&self](const GL::Shader& s){
					s.setTexture2D("u_texture", self.getTexture());
					s.setMat3("view", *self.getProjection());
					s.setMat3("localToWorld", self.getLocalToWorld());
				});

				return shader;
			});

			Core::batchGroup.batchOverlay->setProjection(&Core::camera->getWorldToScreen());

			{
				// auto size = Core::platform
				auto [x, y] = Core::platform->window->equrySize();

				Core::uiRoot = new UI::Root{};
				Core::renderer = new Core::Renderer{x, y};

				Core::uiRoot->resize(x, y);
				Core::camera->resize(x, y);
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

		Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadInit>([](const Assets::AssetsLoadInit& event){
			OS::File cacheDir = Assets::fontDir.subFile("cache-load");
			if(!cacheDir.exist()) cacheDir.createDirQuiet();
			event.manager->getFontsManager_Load().rootCacheDir = cacheDir;
			event.manager->getFontsManager_Load().texturePage = event.manager->getAtlas().registerPage(
				"font-load", cacheDir);
			event.manager->getFontsManager_Load().texturePage->setMargin(1);


			cacheDir = Assets::fontDir.subFile("cache");
			if(!cacheDir.exist()) cacheDir.createDirQuiet();
			event.manager->getFontsManager().rootCacheDir = cacheDir;
			event.manager->getFontsManager().texturePage = event.manager->getAtlas().registerPage("font", cacheDir);
			event.manager->getFontsManager_Load().texturePage->setMargin(1);


			{
				Assets::TexturePackPage* uiPage = event.manager->getAtlas().registerPage("ui", Assets::texCacheDir);
				uiPage->forcePack = true;
				Assets::textureDir.subFile("ui").forAllSubs([uiPage](OS::File&& file){
					uiPage->pushRequest(file);
				});

				Assets::assetsDir.subFile("svg\\icons").forAllSubs([uiPage](OS::File&& file){
					auto pixmap = ext::svgToBitmap(file, 48);
					pixmap.mulWhite();
					uiPage->pushRequest(file.stem(), std::move(pixmap));
				});

				Assets::textureDir.subFile("cursor").forAllSubs([uiPage](OS::File&& file){
					uiPage->pushRequest(file);
				});
			}

			{
				Assets::TexturePackPage* mainPage = event.manager->getAtlas().registerPage(
					MainPageName, Assets::texCacheDir);
				mainPage->pushRequest("white-solid", Assets::textureDir.find("white.png"));
				Assets::TexturePackPage* normalPage = event.manager->getAtlas().registerAttachmentPage(
					"normal", mainPage);
				Assets::TexturePackPage* lightPage = event.manager->getAtlas().
				                                           registerAttachmentPage("light", mainPage);
				lightPage->pushRequest("white-light", Assets::textureDir.find("white.light.png"));
				mainPage->pushRequest("white-light", Assets::textureDir.find("transparent.png"));
			}
		});

		Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadPull>([](const Assets::AssetsLoadPull& event){
			Game::core->contentLoader->loadTexture(Assets::textureTree, event.manager->getAtlas());
		});

		Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadEnd>([](const Assets::AssetsLoadEnd& event){
			for(auto& texture2D : event.manager->getAtlas().getPage("font").getTextures()){
				texture2D->setScale(GL::TexParams::mipmap_linear_linear);
			}

			Assets::Textures::whiteRegion = *event.manager->getAtlas().find("base-white-solid");
			Assets::Textures::whiteRegion.shrinkEdge(15.0f);
			Graphic::Draw::globalState.defaultSolidTexture = Graphic::Draw::getDefaultTexture();

			auto lightRegion = event.manager->getAtlas().find("base-white-light");
			Graphic::Draw::globalState.defaultLightTexture = lightRegion;



			const_cast<GL::TextureRegionRect*>(lightRegion)->shrinkEdge(15.0f);

			for(auto& texture : event.manager->getAtlas().getPage("ui").getTextures()){
				texture->setScale(GL::TexParams::linear, GL::TexParams::linear);
			}

			for(auto& texture : event.manager->getAtlas().getPage(MainPageName).getTextures()){
				texture->setScale(GL::TexParams::mipmap_linear_nearest, GL::TexParams::nearest);
			}

			Graphic::Draw::setDefTexture(&Assets::Textures::whiteRegion);
			Graphic::Draw::setTexture();

			UI::Styles::load(event.manager->getAtlas());

			{
				auto& ptr = UI::getCursorRaw(UI::CursorType::select);
				ptr = std::make_unique<UI::Cursor>(event.manager->getAtlas().find("ui-cursor-aim"));
				ptr->drawer = std::make_unique<UI::CursorThoroughSightDrawer>();
			}

			{

				UI::setCursorRaw(UI::CursorType::regular,
					std::make_unique<UI::Cursor>(event.manager->getAtlas().find("ui-cursor-regular")));
				UI::setCursorRaw(UI::CursorType::regular_tip,
					std::make_unique<UI::Cursor>(event.manager->getAtlas().find("ui-cursor-regular-tip")));

				UI::setCursorRaw(UI::CursorType::clickable,
					std::make_unique<UI::Cursor>(event.manager->getAtlas().find("ui-cursor-click")));
				UI::setCursorRaw(UI::CursorType::clickable_tip,
					std::make_unique<UI::Cursor>(event.manager->getAtlas().find("ui-cursor-click-tip")));

				UI::setCursorRaw(UI::CursorType::textInput,
				                 std::make_unique<UI::Cursor>(event.manager->getAtlas().find("ui-cursor-input")));

				UI::setCursorRaw(UI::CursorType::scroll,
				                 std::make_unique<UI::Cursor>(event.manager->getAtlas().find("ui-cursor-scroll")));
				UI::setCursorRaw(UI::CursorType::scrollHori,
				                 std::make_unique<UI::Cursor>(event.manager->getAtlas().find("ui-cursor-scroll-hori")));
				UI::setCursorRaw(UI::CursorType::scrollVert,
				                 std::make_unique<UI::Cursor>(event.manager->getAtlas().find("ui-cursor-scroll-vert")));

				UI::setCursorRaw(UI::CursorType::drag,
				                 std::make_unique<UI::Cursor>(event.manager->getAtlas().find("ui-cursor-drag")));
			}

			event.manager->getAtlas().bindTextureArray(BindPageName, {MainPageName, "normal", "light"},
			                                           [](GL::Texture2DArray* tex){
				                                           tex->setScale(GL::mipmap_linear_linear, GL::nearest);
			                                           });
		});

		Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadPost>([](const auto& event){
			Core::batchGroup.batchWorld = std::make_unique<Core::SpriteBatch<GL::VERT_GROUP_SIZE_WORLD>>(
				[](const Core::SpriteBatch<GL::VERT_GROUP_SIZE_WORLD>& self){
					auto* const shader = Assets::Shaders::world;

					shader->setUniformer([&self](const GL::Shader& s){
						s.setTexture2D("texArray", self.getTexture());
						s.setMat3("view", *self.getProjection());
						s.setMat3("localToWorld", self.getLocalToWorld());
					});

					return shader;
				}, [](GL::AttributeLayout& layout){
					layout.addFloat(3);
					layout.addFloat(2);
					layout.addFloat(4);
					layout.addFloat(4);
				});

			Core::batchGroup.batchWorld->setProjection(&Core::camera->getWorldToScreen());

			Font::forwardParser = std::make_unique<Font::GlyphParser>(Assets::Fonts::telegrama);
			Font::forwardParser->charParser->registerDefParser();
			Font::forwardParser->tokenParser->reserveTokenSentinal = true;
			Font::forwardParser->tokenParser->fallBackModifier = [](const int curIndex, const Font::TextView token,
			                                                        const Font::ModifierableData& data){
				const auto hasType = token.find('#');

				Font::TextView subToken = token;

				if(hasType != Font::TextView::npos){
					subToken = token.substr(0, hasType);
				}

				data.context.fallbackColor = data.context.currentColor;

				data.context.currentColor = Font::defGlyphParser->tokenParser->modifier.contains(subToken)
					                            ? Graphic::Color{0xa1ecabff}
					                            : Graphic::Colors::RED_DUSK;
				auto backItr = data.layout.getGlyphs().rbegin();

				for(int i = 0; i < 2; ++i){
					backItr->fontColor = Graphic::Colors::GRAY;
					++backItr;
				}

				for(const auto [index, charCode] : token | std::ranges::views::enumerate){
					const auto* charData = data.context.currentFont->getCharData(charCode);

					if(index == subToken.size()){
						data.context.currentColor = Graphic::Colors::ROYAL;
					} else if(index > subToken.size()){
						data.context.currentColor = Graphic::Colors::LIGHT_GRAY;
					}

					const bool hasCharToken = Font::forwardParser->charParser->contains(charCode);

					if(hasCharToken){
						if(Font::forwardParser->charParser->shouldNotContinue(charCode)){
							Font::ParserFunctions::pushData(charCode, curIndex - static_cast<int>(token.size() - index),
							                                charData, data);
							Font::forwardParser->charParser->parse(charCode, data);
						} else{
							Font::forwardParser->charParser->parse(charCode, data);
						}
					} else{
						Font::ParserFunctions::pushData(charCode, curIndex - static_cast<int>(token.size() - index),
						                                charData, data);
					}
				}

				data.context.currentColor = Graphic::Colors::GRAY;
			};

			Font::forwardParser->charParser->shouldNotContinueSet.insert('}');
			Font::forwardParser->charParser->modifier['}'] = [](const Font::ModifierableData& data){
				if(data.context.currentColor == Graphic::Colors::GRAY){
					data.context.currentColor = data.context.fallbackColor;
				}
			};
		});

		//Majority Load
		Core::loadAssets();
	}

	void setupAudioTest(){}
}
