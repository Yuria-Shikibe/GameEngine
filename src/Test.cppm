module;

#include <GLFW/glfw3.h>

export module Test;
import std;

import OS;
import OS.File;
import OS.ApplicationListenerSetter;
import Image;
import Core;
import Core.MainLoopManager;
import Core.Batch.Batch_Sprite;
import Assets.Directories;
import Assets.Graphic;
import Assets.Ctrl;
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
import UI.Palette;
import Core.Renderer;
import OS.Ctrl.Bind.Constants;
import OS.Ctrl.ControlCommands;

import Font.GlyphArrangement;

import Game.Core;
import Game.Content.Builtin.Turrets;
import Game.Content.Builtin.SpaceCrafts;
import Game.Pool;
import Game.Entity.Collision;
import Game.Entity.Controller;
import Game.Entity.Controller.Player;
import Game.Entity.Controller.AI;

import Core.IO.Specialized;
import Core.IO.JsonIO;
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
		Assets::loadDir();

		Assets::loadBasic();

		{
			const Graphic::Pixmap appIcon{Assets::Dir::assets.subFile("icon.png")};
			Core::platform->window->setApplicationIcon(appIcon.data(), appIcon.getWidth(), appIcon.getHeight(),
			                                           appIcon.Channels, 1);
		}

		Core::initCore_Post([]{
			Core::batchGroup.overlay = std::make_unique<Core::SpriteBatch<>>([](const Core::SpriteBatch<>& self){
				auto* const shader = Assets::Shaders::screenSpace;

				shader->setUniformer([&self](const GL::ShaderProgram& s){
					s.setTexture2D("u_texture", self.getTexture());
					s.setMat3("view", *self.getProjection());
					s.setMat3("localToWorld", self.getLocalToWorld());
				});

				return shader;
			});

			Core::batchGroup.overlay->setProjection(&Core::camera->getWorldToScreen());

			{
				// auto size = Core::platform
				auto [x, y] = Core::platform->window->equrySize();

				Core::renderer = new Core::Renderer{x, y};

				Core::camera->resize(x, y);
			}

			Core::renderer->registerSynchronizedResizableObject(Core::camera);

			Ctrl::registerCommands(Core::input);
		});

		Graphic::Draw::setDefTexture(&Assets::Textures::whiteRegion);
		Graphic::Draw::setTexture();
		Graphic::Frame::rawMesh = Assets::Meshes::raw.get();
		Graphic::Frame::blitter = Assets::Shaders::blit;

		Core::bundle.load(Assets::Dir::bundle.subFile("bundle.zh_cn.json"), Assets::Dir::bundle.subFile("bundle.def.json"));

		Game::core = std::make_unique<Game::Core>();
		Core::loopManager->setGameCore(Game::core.get());

		ext::json::Json json{Assets::Dir::settings.subFile("ctrl.json").readString()};
		ext::json::getValueTo(Assets::Ctrl::basicGroup, json.getData());

		Core::destructors.push_back([]{
			Assets::Dir::settings.subFile("ctrl.json").writeString(std::format("{:1}", ext::json::getJsonOf(Assets::Ctrl::basicGroup)));
		});

		Core::destructors.push_back([]{
			Game::EntityManage::realEntities.clear();
			Game::EntityManage::drawables.clear();
			Game::EntityManage::entities.clear();
			Game::Pools::entityPoolGroup.clear();
			Game::core.reset();
		});

		Assets::Ctrl::load();
	}

	void assetsLoad(){
		Game::Content::Builtin::load_SpaceCraft(Game::core->contentLoader.get());
		Game::Content::Builtin::load_Turrets(Game::core->contentLoader.get());

		Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadInit>([](const Assets::AssetsLoadInit& event){
			OS::File cacheDir = Assets::Dir::cache.subFile("font-load");
			if(!cacheDir.exist()) cacheDir.createDirQuiet();
			event.manager->getFontsManager_Load().rootCacheDir = cacheDir;
			event.manager->getFontsManager_Load().texturePage = event.manager->getAtlas().registerPage(
				"font-load", cacheDir);
			event.manager->getFontsManager_Load().texturePage->setMargin(1);


			cacheDir = Assets::Dir::cache.subFile("font");
			if(!cacheDir.exist()) cacheDir.createDirQuiet();
			event.manager->getFontsManager().rootCacheDir = cacheDir;
			event.manager->getFontsManager().texturePage = event.manager->getAtlas().registerPage("font", cacheDir);
			event.manager->getFontsManager_Load().texturePage->setMargin(1);


			{
				Assets::TexturePackPage* uiPage = event.manager->getAtlas().registerPage("ui", Assets::Dir::texCache);
				uiPage->forcePack = true;
				Assets::Dir::texture.subFile("ui").forAllSubs([uiPage](OS::File&& file){
					uiPage->pushRequest(file);
				});

				Assets::Dir::svg.subFile("binds").forAllSubs([uiPage](OS::File&& file){
					if(file.extension() != ".svg")return;

					auto pixmap = ext::svgToBitmap(file, 64);
					uiPage->pushRequest(file.stem(), std::move(pixmap));
				});

				Assets::Dir::svg.subFile("icons").forAllSubs([uiPage](OS::File&& file){
					auto pixmap = ext::svgToBitmap(file, 48);
					pixmap.mulWhite();
					uiPage->pushRequest(file.stem(), std::move(pixmap));
				});

				Assets::Dir::texture.subFile("cursor").forAllSubs([uiPage](OS::File&& file){
					uiPage->pushRequest(file);
				});
			}

			{
				Assets::TexturePackPage* mainPage = event.manager->getAtlas().registerPage(
					MainPageName, Assets::Dir::texCache);
				mainPage->pushRequest("white-solid", Assets::Dir::texture.find("white.png"));
				mainPage->setMargin(2);
				[[maybe_unused]] Assets::TexturePackPage* normalPage = event.manager->getAtlas().registerAttachmentPage(
					"normal", mainPage);
				Assets::TexturePackPage* lightPage = event.manager->getAtlas().
				registerAttachmentPage("light", mainPage);
				lightPage->pushRequest("white-light", Assets::Dir::texture.find("white.png"));
				mainPage->pushRequest("white-light", Assets::Dir::texture.find("transparent.png"));
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
				auto& ptr = UI::getCursorRaw(UI::CursorType::select_regular);
				ptr = std::make_unique<UI::Cursor>(event.manager->getAtlas().find("ui-cursor-aim"));
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

			Core::uiRoot = new UI::Root{};
			Core::uiRoot->resize(Core::renderer->getWidth(), Core::renderer->getHeight());
			Core::uiRoot->uiBasicBundle.load(Assets::Dir::bundle.subFile("ui.def.json"), Assets::Dir::bundle.subFile("ui.def.json"));

			Core::renderer->registerSynchronizedResizableObject(Core::uiRoot);
			//Core::loopManager->registerListener(Core::uiRoot);
		});

		Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadPost>([](const auto& event){
			Core::batchGroup.world = std::make_unique<Core::SpriteBatch<GL::VERT_GROUP_SIZE_WORLD>>(
				[](const Core::SpriteBatch<GL::VERT_GROUP_SIZE_WORLD>& self){
					auto* const shader = Assets::Shaders::world;

					shader->setUniformer([&self](const GL::ShaderProgram& s){
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

			Core::batchGroup.world->setProjection(&Core::camera->getWorldToScreen());

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
					                            ? UI::Pal::PALE_GREEN
					                            : UI::Pal::RED_DUSK;
				auto backItr = data.layout.getGlyphs().rbegin();

				for(int i = 0; i < 2; ++i){
					backItr->fontColor = UI::Pal::GRAY;
					++backItr;
				}

				for(const auto [index, charCode] : token | std::ranges::views::enumerate){
					const auto* charData = data.context.currentFont->getCharData(charCode);

					if(std::cmp_equal(index, subToken.size())){
						data.context.currentColor = UI::Pal::KEY_WORD;

					} else if(std::cmp_greater(index, subToken.size())){
						data.context.currentColor = UI::Pal::LIGHT_GRAY;
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

				data.context.currentColor = UI::Pal::GRAY;
			};

			Font::forwardParser->charParser->shouldNotContinueSet.insert(Font::forwardParser->TokenEndCode);
			Font::forwardParser->charParser->modifier[(Font::forwardParser->TokenEndCode)] = [](const Font::ModifierableData& data){
				if(data.context.currentColor == Graphic::Colors::GRAY){
					data.context.currentColor = data.context.fallbackColor;
				}
			};
		});

		//Misc
		Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadPost>([](const auto& event){
			Assets::Ctrl::basicGroup.loadInstruction(Core::bundle);
		});

		//Majority Load
		Core::loadAssets();
	}

	void setupAudioTest(){}
}
