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
import ext.json.io;
import ext.Encoding;
import Image.Svg;

import UI.Icons;

import Assets.TexturePage;

export namespace Test{
	constexpr std::string_view MainPageName = "base";
	constexpr std::string_view BindPageName = "bind";

	void loadBasicAssets(){
		Assets::loadDir();
		Assets::loadBasic();
		Graphic::Draw::World::setDefTexture(&Assets::Textures::whiteRegion);
		Graphic::Draw::Overlay::setDefTexture(&Assets::Textures::whiteRegion);

		Graphic::Frame::rawMesh = Assets::Meshes::raw.get();
		Graphic::Frame::blitter = Assets::Shaders::blit;
	}

	void postSetBasicAssets(Graphic::TextureAtlas& atlas){
		atlas.setFail(atlas.getUIPage()->find("error"));

		Assets::Textures::whiteRegion = *atlas.find(MainPageName, "white-solid");
		Assets::Textures::whiteRegion.shrinkEdge(15.0f);

		const auto lightRegion = atlas.find(MainPageName, "white-light");
		lightRegion->shrinkEdge(15.0f);

		const auto uiWhite = atlas.find_ui("white");
		uiWhite->shrinkEdge(15.0f);

		Graphic::Draw::Overlay::setDefTexture(&Assets::Textures::whiteRegion);
		Graphic::Draw::Overlay::defaultUIWhiteTexture = uiWhite;

		Graphic::Draw::World::setDefTexture(&Assets::Textures::whiteRegion);
		Graphic::Draw::World::defaultLightTexture = lightRegion;
		Graphic::Draw::World::setDefTexture(lightRegion);
	}

	void setApplicationIcon(){
		const Graphic::Pixmap appIcon{Assets::Dir::assets.subFile("icon.png")};
		Core::platform->window->setApplicationIcon(appIcon.data(), appIcon.getWidth(), appIcon.getHeight(),
			appIcon.Channels, 1);
	}

	void initForwardParser(){
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
		Font::forwardParser->charParser->modifier[(Font::forwardParser->TokenEndCode)] = [
			](const Font::ModifierableData& data){
				if(data.context.currentColor == Graphic::Colors::GRAY){
					data.context.currentColor = data.context.fallbackColor;
				}
			};
	}

	void loadSettings(){
		//TODO move these to Settings
		Core::bundle.load(Assets::Dir::bundle.subFile("bundle.zh_cn.json"),
			Assets::Dir::bundle.subFile("bundle.def.json"));

		ext::json::JsonValue json{ext::json::parse(Assets::Dir::settings.subFile("ctrl.json").readString())};
		ext::json::getValueTo(Assets::Ctrl::basicGroup, json);

		Core::destructors.push_back([]{
			Assets::Dir::settings.subFile("ctrl.json").writeString(std::format("{:1}",
				ext::json::getJsonOf(Assets::Ctrl::basicGroup)));
		});
	}

	void initMisc(){
		ext::setFlipVertically_load(true);
		ext::setFlipVertically_write(true);

		loadBasicAssets();

		setApplicationIcon();
	}

	void init(const int argc, char* argv[]){
		Core::initPlatform(argc, argv);

		Core::initCore_Kernal();
		Core::initMainWindow(true);

		initMisc();

		Core::initCore_Interactive([]{
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

			Core::renderer->registerSynchronizedResizableObject(Core::camera.get());
		});

		Core::destructors.push_back([]{
			Game::EntityManage::realEntities.clear();
			Game::EntityManage::drawables.clear();
			Game::EntityManage::entities.clear();
			Game::Pools::entityPoolGroup.clear();
			Game::core.reset();
		});

		Assets::Ctrl::load();

		Game::core = std::make_unique<Game::Core>();
		Core::loopManager->setGameCore(Game::core.get());
	}

	void assetsLoad(){
		Game::Content::Builtin::load_SpaceCraft(Game::core->contentLoader.get());
		Game::Content::Builtin::load_Turrets(Game::core->contentLoader.get());

		auto* uiPage = Core::assetsManager.atlas.getUIPage();
		auto* mainPage = Core::assetsManager.atlas.registerPage(MainPageName);
		auto* normalPage = Core::assetsManager.atlas.registerPage("normal");
		auto* lightPage = Core::assetsManager.atlas.registerPage("light");

		normalPage->dependency = mainPage;
		lightPage->dependency = mainPage;

		mainPage->setMargin(2);

		using namespace std::string_view_literals;

		Core::assetsLoader.eventManager.on<Assets::Load::LoadEvent<Assets::Load::Phase::init>>([=](const auto& e){
			mainPage->pushRequest<Assets::FileImportData>("white-solid"sv, Assets::Dir::texture.find("white.png"));

			mainPage->pushRequest<Assets::FileImportData>("white-light"sv, Assets::Dir::texture.find("transparent.png"));

			lightPage->pushRequest<Assets::FileImportData>("white-light"sv, Assets::Dir::texture.find("white.png"));

			uiPage->pushRequest(Assets::Dir::texture.find("white.png"));
			uiPage->pushRequest(Assets::Dir::texture.find("error.png"));

			Assets::Dir::texture.subFile("ui").forAllSubs([uiPage](auto&& file){
				uiPage->pushRequest(file);
			});

			Assets::Dir::svg.subFile("binds").forAllSubs([uiPage](auto&& file){
				auto pixmap = ext::svgToBitmap(file, 64);

				if(pixmap.valid()) uiPage->pushRequest(file);
				UI::Icons::registerIconName(file.stem());
			});

			Assets::Dir::svg.subFile("icons").forAllSubs([uiPage](auto&& file){
				uiPage->pushRequest<Assets::SvgFileImportData>(file, Geom::Point2U{64, 0}, true);
				UI::Icons::registerIconName(file.stem());
			});

			Assets::Dir::texture.subFile("cursor").forAllSubs([uiPage](auto&& file){
				uiPage->pushRequest(file);
			});

			Game::core->contentLoader->loadTexture(Assets::textureTree, Core::assetsManager.atlas);

		});


		Core::assetsLoader.eventManager.on<Assets::Load::LoadEvent<Assets::Load::Phase::end>>([=](const Assets::Load::LoadEvent<Assets::Load::Phase::end>& event){
			auto& atlas = Core::assetsManager.atlas;

			postSetBasicAssets(atlas);

			UI::Styles::load(atlas);

			{
				auto& ptr = UI::getCursorRaw(UI::CursorType::select);
				ptr = std::make_unique<UI::Cursor>(atlas.find(atlas.getUIPage(), "cursor-aim"));
				ptr->drawer = std::make_unique<UI::CursorThoroughSightDrawer>();
			}

			{
				UI::setCursorRaw(UI::CursorType::select_regular,
					std::make_unique<UI::Cursor>(atlas.find(atlas.getUIPage(), "cursor-aim")));

				UI::setCursorRaw(UI::CursorType::regular,
					std::make_unique<UI::Cursor>(atlas.find(atlas.getUIPage(), "cursor-regular")));
				UI::setCursorRaw(UI::CursorType::regular_tip,
					std::make_unique<UI::Cursor>(atlas.find(atlas.getUIPage(), "cursor-regular-tip")));

				UI::setCursorRaw(UI::CursorType::clickable,
					std::make_unique<UI::Cursor>(atlas.find(atlas.getUIPage(), "cursor-click")));
				UI::setCursorRaw(UI::CursorType::clickable_tip,
					std::make_unique<UI::Cursor>(atlas.find(atlas.getUIPage(), "cursor-click-tip")));

				UI::setCursorRaw(UI::CursorType::textInput,
					std::make_unique<UI::Cursor>(atlas.find(atlas.getUIPage(), "cursor-input")));

				UI::setCursorRaw(UI::CursorType::scroll,
					std::make_unique<UI::Cursor>(atlas.find(atlas.getUIPage(), "cursor-scroll")));
				UI::setCursorRaw(UI::CursorType::scrollHori,
					std::make_unique<UI::Cursor>(atlas.find(atlas.getUIPage(), "cursor-scroll-hori")));
				UI::setCursorRaw(UI::CursorType::scrollVert,
					std::make_unique<UI::Cursor>(atlas.find(atlas.getUIPage(), "cursor-scroll-vert")));

				UI::setCursorRaw(UI::CursorType::drag,
					std::make_unique<UI::Cursor>(atlas.find(atlas.getUIPage(), "cursor-drag")));
			}

			event.loader.getTaskQueue().push([&atlas]{
				for(const auto& texture : atlas.getUIPage()->getTextures()){
					texture->setFilter(GL::TexParams::mipmap_linear_linear, GL::TexParams::linear);
				}

				for(const auto& texture : atlas.getPage(MainPageName).getTextures()){
					texture->setFilter(GL::TexParams::mipmap_linear_nearest, GL::TexParams::nearest);
				}

				atlas.bindTextureArray(BindPageName, {MainPageName, "normal", "light"},
					[](const GL::Texture2DArray* tex){
						tex->setFilter(GL::mipmap_linear_linear, GL::nearest);
					});
			}).get();


			Core::uiRoot = new UI::Root{};
			Core::uiRoot->resize(Core::renderer->getWidth(), Core::renderer->getHeight());
			Core::uiRoot->uiBasicBundle.load(Assets::Dir::bundle.subFile("ui.def.json"),
				Assets::Dir::bundle.subFile("ui.def.json"));

			Core::renderer->registerSynchronizedResizableObject(Core::uiRoot);
			Ctrl::registerCommands(Core::input);
		});

		Core::assetsLoader.miscs.push(Assets::Load::Phase::end, []{
			loadSettings();
			Assets::Ctrl::basicGroup.loadInstruction(Core::bundle);

			UI::Icons::load(Core::assetsManager.atlas);
		});


		//Majority Load
		Core::beginLoadAssets();
		Core::endLoadAssets();

		Assets::PostProcessors::bloom->setIntensity(1.f);

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

		// std::filesystem::path path = R"(D:\projects\GameEngine\src\arc\ui\components\Icons_Assets.cppm)";
		// std::ofstream stOfstream{path};
		// UI::Icons::genCode(stOfstream);
	}

	void setupAudioTest(){}
}
