#include <Windows.h>

#include "src/application_head.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../src/code-gen/ReflectData_Builtin.hpp"

import std;
import std.compat;

import Assets.LoaderRenderer;

import UI.Align;

import Core.Platform;
import OS.File;
import Concepts;
import ext.Container.ObjectPool;
import Event;
import StackTrace;

import Graphic.Draw;
import Graphic.Pixmap;
import Core.Renderer;
import Graphic.Viewport.Viewport_OrthoRect;
import Graphic.Viewport;

import Graphic.PostProcessor.BloomProcessor;
import Graphic.PostProcessor.ShaderProcessor;
import Graphic.PostProcessor.PipeProcessor;
import Graphic.PostProcessor.P4Processor;
import Graphic.TextureAtlas;

import Font;

import Math;
import Math.Rand;
import Math.StripPacker2D;
import Geom.Vector2D;
import Geom.Matrix3D;
import Geom.Rect_Orthogonal;

import OS;
import OS.ApplicationListenerSetter;

import Core;

import Core.Audio;
import Assets.Manager;
import Assets.Effects;
import Assets.Sound;
import Assets.Bundle;
import Core.Settings;
import Core.Log;

import UI.Root;
import Core.Batch;
import Core.Input;
import Core.Camera;
import Core.Renderer;

import Ctrl.ControlCommands;
import Ctrl.Constants;
import Core.Batch.Batch_Sprite;

import Assets.Graphic;
import Graphic.Color;

import GL.Buffer.MultiSampleFrameBuffer;
import GL.Buffer.FrameBuffer;
import GL.Texture.TextureRegionRect;
import GL.Texture.TextureNineRegion;
import GL.Blending;
import Event;
import Font.GlyphArrangement;
import ext.Timer;

import Assets.TexturePacker;
import Assets.Loader;
import Assets.Manager;

//TODO 模块分区
import UI.Widget;
import UI.Root;
import UI.Table;
import UI.Label;
import UI.ScrollPane;
import UI.Drawer;
import UI.Styles;
import UI.Button;
import UI.SliderBar;
import UI.ProgressBar;
import UI.InputArea;
import UI.FileTreeSelector;


import Geom.Shape.RectBox;
import Geom.QuadTree;

import Test;
import Game.Core;
import Game.Entity.EntityManager;
import Game.Entity.RealityEntity;
import Game.Entity.SpaceCraft;
import Game.Entity.Bullet;
import Game.Entity.Collision;
import Game.Entity.Turrets;


import Game.Content.Type.BasicBulletType;
import Game.Content.Type.Turret.BasicTurretType;
import Game.Content.Builtin.SpaceCrafts;

import Game.Graphic.CombinePostProcessor;

import Game.ChamberFrame;
import Game.Chamber.Util;

import ext.Encoding;
import ext.TreeStructure;
import ext.Algorithm;
import ext.Heterogeneous;
import ext.Json;
import ext.Base64;
import ext.StringParse;
import ext.StaticReflection;
import ext.ReflectData;

import Core.IO.Specialized;

using namespace Graphic;
using namespace GL;
using Geom::Vec2;


struct TestChamberFactory : Game::ChamberFactory{
	struct TestChamberData : Game::ChamberMetaDataBase{
		Vec2 targetPos{};
		float reload{};
	};


	struct TestChamberTrait : Game::ChamberTraitFallback<TestChamberData>{
		float reloadTime{};

		void update(Game::Chamber* chamber, TraitDataType& data, const float delta) const{
			chamber->update(delta);
			data.reload += delta;
			if(data.reload > reloadTime){
				data.reload = 0;
			}
		}

		void draw(const Game::Chamber* chamber, const TraitDataType& data) const{
			Graphic::Draw::Line::rectOrtho(chamber->getChamberBound());
		}
	} baseTraitTest;


	template <Concepts::Derived<TestChamberTrait> Trait = TestChamberTrait>
	using ChamberType = Game::ChamberVariant<typename Trait::TraitDataType, TestChamberTrait>;

	std::unique_ptr<Game::Chamber> genChamber() const override{
		return std::make_unique<ChamberType<>>(baseTraitTest);
	}
};


std::stringstream sstream{};
Game::ChamberFrame chamberFrame{};
std::unique_ptr<Game::ChamberFactory> testFactory{std::make_unique<TestChamberFactory>()};


void setupUITest(){
	const auto HUD = new UI::Table{};

	Core::uiRoot->currentScene->transferElem(HUD).fillParent().setAlign(Align::center);

	HUD->setRelativeLayoutFormat(false);
	HUD->setBorderZero();
	HUD->setDrawer(&UI::emptyDrawer);

	HUD->add<UI::Table>([](UI::Table& label){
		   label.add<UI::ScrollPane>([](UI::ScrollPane& pane){
			   pane.setItem<UI::Label>([](UI::Label& label){
				   label.setText([](){
					   sstream.str("");
					   sstream << "$<scl#[0.55]>(" << std::fixed << std::setprecision(2) << Core::camera->
						   getPosition().x << ", " <<
						   Core::camera->getPosition().y << ") | " << std::to_string(OS::getFPS());
					   sstream << "\n\nEntity count: " << Game::EntityManage::entities.idMap.size();
					   sstream << "\nDraw count: " << std::ranges::count_if(
						   Game::EntityManage::drawables.idMap | std::ranges::views::values, std::identity{},
						   &decltype(Game::EntityManage::drawables)::ValueType::isInScreen);

					   return sstream.str();
				   });

				   label.setFillparentX();
				   label.usingGlyphHeight = true;
			   });
			   // pane.setItem<UI::Elem>([](UI::Elem& area){
			   // 	area.setWidth(1000);
			   // 	area.setFillparentY();
			   // });
		   });
	   })
	   .setAlign(Align::top_left).setSizeScale(0.25f, 0.2f)
	   .setMargin(0, 10, 0, 10);


	HUD->add<UI::Table>([](UI::Table& table){
		   table.add<UI::Table>([](UI::Table& t){
			   t.selfMaskOpacity = 0.0f;
			   t.setBorderZero();
			   t.defaultCellLayout.setMargin({.left = 2.0f, .right = 2.f});
			   for(int i = 0; i < 8; ++i){
				   t.add<UI::Button>([i](UI::Button& button){
					   button.setCall([i](UI::Button& b, bool){
						   b.buildTooltip();
					   });

					   button.setTooltipBuilder({
							   .followTarget = UI::FollowTarget::parent,
							   .minHoverTime = UI::DisableAutoTooltip,
							   .followTargetAlign = Align::Mode::bottom_left,
							   .builder = [i](UI::Table& hint){
								   // hint.setMinimumSize({600, 300});
								   hint.setCellAlignMode(Align::Mode::top_left);
								   hint.add<UI::Label>([i](UI::Label& label){
									   label.usingGlyphHeight = label.usingGlyphWidth = true;
									   label.setText(
										   std::format("<Hint Hover Table>\nButton$<sub>$<c#PALE_GREEN>{}$<\\sub>", i));
									   label.getGlyphLayout()->setSCale(0.65f);
									   label.setEmptyDrawer();
									   label.getBorder().expand(4.0f);
								   }).expand().endLine();
								   hint.add<UI::Button>([i](UI::Button& button){
									   button.setCall([i](UI::Button&, bool){
										   Core::uiRoot->showDialog(true, [i](UI::Table& builder){
											   builder.add<UI::FileTreeSelector>([](UI::FileTreeSelector& selector){
												   const OS::File src{
														   R"(D:\projects\GameEngine\properties\resource\assets)"
													   };
												   selector.gotoFile(src, false);
											   }).fillParent().setAlign(Align::Mode::top_center);
										   });
									   });
									   button.setTooltipBuilder({
											   .followTarget = UI::FollowTarget::parent,
											   .followTargetAlign = Align::Mode::center_right,
											   .tooltipSrcAlign = Align::Mode::center_left,
											   .builder = [](UI::Table& hintInner){
												   hintInner.setCellAlignMode(Align::Mode::top_left);
												   hintInner.add<UI::Label>([](UI::Label& label){
													   label.usingGlyphHeight = label.usingGlyphWidth = true;
													   label.setText(std::format(
														   "<Hover Table>$<sub>$<c#PALE_GREEN>{}$<\\sub>", "Nesting"));
													   label.getGlyphLayout()->setSCale(0.65f);
													   label.setEmptyDrawer();
													   label.getBorder().expand(4.0f);
												   }).expand().endLine();
											   }
										   });
								   }).setHeight(120.0f).expandY().fillParentX();
								   hint.PointCheck = 180;
							   }
						   });
				   });
			   }
		   }).fillParent();

		   table.add<UI::Table>([](UI::Table& t){
			   t.setEmptyDrawer();
			   auto& slider = t.add<UI::SliderBar>([](UI::SliderBar& s){
				   s.setClampedOnHori();
			   }).fillParent().endLine().as<UI::SliderBar>();

			   t.add<UI::ProgressBar>([&slider](UI::ProgressBar& bar){
				   bar.progressGetter = [&slider]{
					   return slider.getProgress().x;
				   };

				   bar.PointCheck = 12;

				   bar.setTooltipBuilder({
						   .followTarget = UI::FollowTarget::parent,
						   .followTargetAlign = Align::Mode::bottom_center,
						   .tooltipSrcAlign = Align::Mode::top_center,
						   .builder = [&bar](UI::Table& hint){
							   // hint.setMinimumSize({600, 300});
							   hint.setCellAlignMode(Align::Mode::top_left);
							   hint.add<UI::Label>([&bar](UI::Label& label){
								   label.usingGlyphHeight = label.usingGlyphWidth = true;
								   label.setText([&bar]{
									   return std::format(
										   "$<c#GRAY>Progress: $<c#LIGHT_GRAY>{:.2f}$<scl#[0.75]>$<c#PALE_GREEN>%",
										   bar.getDrawProgress() * 100.0f);
								   });
								   label.getGlyphLayout()->setSCale(0.55f);
								   label.setEmptyDrawer();
								   label.getBorder().expand(2.0f);
							   }).expand().endLine();
						   }
					   });
			   });
		   }).fillParent().setPad({.left = 2.0f});
	   })
	   .setAlign(Align::Mode::top_left)
	   .setSizeScale(0.4f, 0.08f)
	   .setSrcScale(0.25f, 0.0f)
	   .setMargin(10, 0, 0, 0);


	HUD->transferElem(new UI::Table{})
	   .setAlign(Align::Mode::top_left)
	   .setSizeScale(0.1f, 0.6f)
	   .setSrcScale(0.0f, 0.2f)
	   .setMargin(0, 0, 10, 10);


	HUD->add<UI::Table>([](UI::Table& table){
		   table.add<UI::Widget>();
		   table.lineFeed();
		   table.add<UI::Widget>();
		   table.add<UI::Widget>();
	   })
	   .setAlign(Align::Mode::bottom_left)
	   .setSizeScale(0.25f, 0.2f)
	   .setMargin(0, 10, 10, 10);


	HUD->transferElem(new UI::Table{})
	   .setAlign(Align::Mode::bottom_left)
	   .setSizeScale(0.075f, 0.2f)
	   .setSrcScale(0.25f, 0.0f)
	   .setMargin(10, 0, 10, 10);


	{
		auto pane = new UI::ScrollPane{};

		pane->setItem<UI::Table>([](UI::Table& rt){
			rt.setSize(400, 900);
			rt.setFillparentX();

			rt.add<UI::ScrollPane>([](UI::ScrollPane& pane){
				pane.setItem<UI::Table>([](UI::Table& paneT){
					paneT.setHeight(600);
					paneT.setFillparentX();

					paneT.add<UI::Widget>();
					paneT.lineFeed();
					paneT.add<UI::Widget>();
					paneT.add<UI::Widget>();
				});
			});
			// rt->add(new UI::Elem);
			rt.lineFeed();
			rt.transferElem(new UI::Widget{});
			rt.transferElem(new UI::Widget{});
		});

		HUD->transferElem(pane).setAlign(Align::Mode::top_right).setSizeScale(0.225f, 0.25f).setMargin(10, 0, 0, 10);
	}

	HUD->add<UI::Table>([](UI::Table& table){
		   table.add<UI::ScrollPane>([](UI::ScrollPane& pane){
			   pane.setItem<UI::InputArea>([](UI::InputArea& area){
				   area.usingGlyphWidth = area.usingGlyphHeight = true;
				   area.setMaxTextLength(1000);

				   area.getGlyphLayout()->setSCale(0.75f);
				   area.setText("Test\n123123123\nsadaDSAda");
			   });
		   }).fillParent();
	   })
	   .setAlign(Align::top_right)
	   .setSizeScale(0.185f, 0.575f).setSrcScale(0.0f, 0.25f)
	   .setMargin(10, 0, 10, 0);
	//
	HUD->transferElem(new UI::Table{})
	   .setAlign(Align::top_right)
	   .setSizeScale(0.225f - 0.185f, 0.45f).setSrcScale(0.185f, 0.25f)
	   .setMargin(10, 10, 10, 0);
	//
	HUD->add<UI::Table>([](UI::Table& table){})
	   .setAlign(Align::Mode::bottom_right)
	   .setSizeScale(0.3f, 0.15f)
	   .setMargin(10, 0, 10, 0);
}

void setupCtrl(){
	// Core::input->registerMouseBind(Ctrl::MOUSE_BUTTON_1, Ctrl::Act_Press, [] {
	// 	const auto pos = Game::core->overlayManager->getMouseInWorld();
	// 	auto* effect = Game::core->effectManager->suspend();
	// 	effect->setDrawer(&Assets::Effects::CircleDrawer)->set(pos, 0, Graphic::Colors::SKY);
	// });

	Core::input->registerMouseBind(Ctrl::MOUSE_BUTTON_2, Ctrl::Act_Press, []{
		auto transed = (Core::input->getCursorPos() - Core::renderer->getSize().div(2)).div(Core::camera->getScale()) + Core::camera->getPosition();

		Draw::Line::line(transed, {0, 0});
		const auto pos = chamberFrame.getNearby(transed);
		Geom::OrthoRectInt bound{pos, 3, 3};

		if(chamberFrame.placementValid(bound)){
			chamberFrame.insert(testFactory->genChamberTile(bound));
		}
	});

	Core::input->registerMouseBind(Ctrl::MOUSE_BUTTON_3, Ctrl::Act_Press, []{
		auto transed = (Core::input->getCursorPos() - Core::renderer->getSize().div(2)).div(Core::camera->getScale()) + Core::camera->getPosition();

		Draw::Line::line(transed, {0, 0});
		const auto pos = chamberFrame.getNearby(transed);

		chamberFrame.erase(pos, true);

	});


	Core::input->registerKeyBind(Ctrl::KEY_F1, Ctrl::Act_Press, [&]{
		const OS::File tgt{R"(D:\projects\GameEngine\properties\resource\tiles.png)"};

		const auto pixmap = Game::ChamberUtil::saveToPixmap(chamberFrame);

		pixmap.write(tgt, true);
	});


	Core::input->registerKeyBind(Ctrl::KEY_F, Ctrl::Act_Continuous, []{
		static ext::Timer<> timer{};

		timer.run(12, OS::updateDeltaTick(), []{
			Core::audio->play(Assets::Sounds::laser5);
			Geom::RectBox box{};
			box.setSize(180, 12);
			box.offset = box.sizeVec2;
			box.offset.mul(-0.5f);

			for(int i = -3; i <= 3; ++i){
				const auto ptr = Game::EntityManage::obtain<Game::Bullet>();
				ptr->trait = &Game::Content::basicBulletType;
				ptr->trans.vec.set(Core::camera->getPosition());
				ptr->trans.rot = (Core::renderer->getSize() * 0.5f).angleTo(Core::input->getCursorPos());

				ptr->trans.vec.add(Geom::Vec2{}.setPolar(ptr->trans.rot + 90, 80.0f * i));

				ptr->velo.vec.set(320, 0).rotate(ptr->trans.rot);
				Game::EntityManage::add(ptr);
				ptr->hitBox.init(box);
				ptr->physicsBody.inertialMass = 100;
				ptr->damage.materialDamage.fullDamage = 100;
				ptr->activate();
			}
		});
	});

	Core::input->registerMouseBind(
		Ctrl::MOUSE_BUTTON_2, Ctrl::Act_Press,
		Ctrl::Mode_Shift
	  , []{
			Game::EntityManage::realEntities.quadTree->intersectPoint(
				Core::camera->getScreenToWorld(Core::renderer->getNormalized(Core::input->getCursorPos())),
				[](decltype(Game::EntityManage::realEntities)::ValueType* entity){
					entity->controller->selected = !entity->controller->selected;
					Game::core->overlayManager->registerSelected(
						std::dynamic_pointer_cast<Game::RealityEntity>(std::move(entity->obtainSharedSelf())));
				});
		});
}

void genRandomEntities(){
	// Math::Rand rand = Math::globalRand;
	// for(int i = 0; i < 300; ++i) {
	// 	Geom::RectBox box{};
	// 	box.setSize(rand.random(40, 1200), rand.random(40, 1200));
	// 	box.rotation = rand.random(360);
	// 	box.offset = box.sizeVec2;
	// 	box.offset.mul(-0.5f);
	//
	// 	const auto ptr = Game::EntityManage::obtain<Game::SpaceCraft>();
	// 	ptr->trans.rot  = rand.random(360.0f);
	// 	ptr->trans.pos.set(rand.range(20000), rand.range(20000));
	// 	Game::EntityManage::add(ptr);
	// 	ptr->hitBox.init(box);
	// 	ptr->setHealth(500);
	// 	ptr->physicsBody.inertialMass = rand.random(0.5f, 1.5f) * box.sizeVec2.length();
	// 	ptr->velocity.set(1, 0).rotate(rand.random(360));
	// 	ptr->activate();
	//
	// 	ptr->init();
	// 	ptr->setTurretType(&Game::Content::baseTurret);
	// }

	const auto ptr = Game::EntityManage::obtain<Game::SpaceCraft>();
	ptr->trans.vec.set(0, 0);
	Game::EntityManage::add(ptr);
	Game::read(OS::File{Assets::assetsDir.subFile(R"(hitbox\pester.hitbox)")}, ptr->hitBox);
	ptr->velo.vec.set(0, 0);
	ptr->setHealth(10000);
	ptr->init(Game::Content::Builtin::test);
	ptr->activate();
}

/**
 * @brief
 * @tparam T TypeName
 * @tparam write true -> write | false -> read
 */
template <typename T, bool write>
struct JsonEachIO{
	// using FieldHint = ext::reflect::Field<void>;
	using ClassField =  ext::reflect::ClassField<T>;
	using PassType = typename ext::ConstConditional<write, T&>::type;

	template <std::size_t I>
	constexpr static void with(PassType val, ext::json::JsonValue& root){
		using Field = typename ClassField::template FieldAt<I>;
		if constexpr (Field::enableSrl){
			//Chain invoke
			root.append(Field::getName, ext::json::getJsonOf(val.*Field::fieldPtr));
		}
	}
};

int main(){
	OS::File fi{R"(D:\projects\GameEngine\properties\resource\test.json)"};


	// ext::json::Json json{fi.readString()};
	// TestT test{};
	// ext::json::getValueTo(test, json.getData());

	ext::json::JsonValue root{};

	TestT test{{{10, 15}, 90}, false, Colors::PALE_GREEN, {12,45}, {-12, -545}, {0, 123123, 1512, 12}};
	root = ext::json::getJsonOf(test);

	// auto str = std::format("{:f}", root);
	auto str = std::format("{}", root);

	std::println("{}", str);

	fi.writeString(str);
}

/*
int main(const int argc, char* argv[]){
	return 0;
	//Init
	::Test::init(argc, argv);

	::Test::assetsLoad();

	::Test::setupAudioTest();

	if(true){
		Game::core->overlayManager->activate();
		Game::core->hitBoxEditor->deactivate();
	} else{
		Game::core->hitBoxEditor->activate();
		Game::core->overlayManager->deactivate();
	}

	std::unordered_map<Geom::Point2, Game::ChamberTile> tiles{};

	for(int x = 0; x < 100; ++x){
		for(int y = 0; y < 100; ++y){
			tiles.try_emplace(Geom::Point2{x, y}, Game::ChamberFactory::genEmptyTile({x, y}));
		}
	}

	for(int i = 0; i < 128; ++i){
		tiles.insert_or_assign({(i % 12) * 3, (i / 12) * 2}, testFactory->genChamberTile({(i % 12) * 3, (i / 12) * 2, 3, 2}));

		tiles.insert_or_assign({i % 12 + 36, i / 12 + 36}, testFactory->genChamberTile({i % 12 + 36, i / 12 + 36, 1, 1}));
	}
	//

	chamberFrame.build(std::move(tiles) | std::ranges::views::values);

	chamberFrame.insert(testFactory->genChamberTile({66, 6, 12, 4}));

	// chamberFrame.erase({-12, -6});
	chamberFrame.erase({11, 6}, true);

	chamberFrame.erase({11, 3}, true);

	::Core::renderer->getListener().on<Event::Draw_Overlay>([](const auto& e){
		Graphic::Batch::flush();
		Core::uiRoot->drawCursor();
		Graphic::Batch::flush();
	});

	::Core::renderer->getListener().on<Event::Draw_After>([&]([[maybe_unused]] const auto& e){
		Game::core->effectManager->render();
		Graphic::Batch::flush();
	});

	::Core::renderer->getListener().on<Event::Draw_After>([]([[maybe_unused]] const auto& e){
		Game::core->drawBeneathUI(e.renderer);
		Graphic::Batch::flush();
	});


	// UI Test
	setupUITest();

	setupCtrl();

	// genRandomEntities();

	GL::MultiSampleFrameBuffer multiSample{Core::renderer->getWidth(), Core::renderer->getHeight()};
	GL::FrameBuffer frameBuffer{Core::renderer->getWidth(), Core::renderer->getHeight()};

	GL::MultiSampleFrameBuffer worldFrameBuffer{Core::renderer->getWidth(), Core::renderer->getHeight(), 4, 3};
	GL::FrameBuffer acceptBuffer1{Core::renderer->getWidth(), Core::renderer->getHeight(), 3};

	Game::CombinePostProcessor merger{
			Assets::PostProcessors::blurX_Far.get(), Assets::PostProcessors::blurY_Far.get(),
			Assets::Shaders::merge
		};

	Core::renderer->registerSynchronizedResizableObject(&multiSample);
	Core::renderer->registerSynchronizedResizableObject(&frameBuffer);
	Core::renderer->registerSynchronizedResizableObject(&worldFrameBuffer);
	Core::renderer->registerSynchronizedResizableObject(&acceptBuffer1);

	Game::EntityManage::init();
	Game::EntityManage::realEntities.resizeTree({-50000, -50000, 100000, 100000});

	float depth = 2;

	Core::input->registerKeyBind(Ctrl::KEY_PAGE_DOWN, Ctrl::Act_Press, [&depth](){
		depth++;
	});

	Core::input->registerKeyBind(Ctrl::KEY_PAGE_UP, Ctrl::Act_Press, [&depth](){
		depth--;
	});

	Core::renderer->getListener().on<Event::Draw_Post>([&](const auto& event){
		Core::Renderer& renderer = *event.renderer;
		renderer.frameBegin(&acceptBuffer1);

		acceptBuffer1.clearColor(Graphic::Colors::BLACK);

		GL::enable(GL::Test::DEPTH);
		GL::setDepthFunc(GL::Func::GEQUAL);
		GL::setDepthMask(true);

		Game::EntityManage::drawables.setViewport(Core::camera->getViewport().getPorjectedBound());
		Game::EntityManage::render();

		Game::core->effectManager->render();

		// auto* region = Core::assetsManager->getAtlas().find("base-pester");
		// //auto* region1 = Core::assetsManager->getAtlas().find("test-collapser");

		Graphic::Batch::flush<BatchWorld>();

		GL::setDepthMask(false);
		GL::disable(GL::Test::DEPTH);
		// GL::setDepthMask(false);

		GL::Blendings::Normal.apply();
		renderer.frameEnd(merger);
	});

	Core::renderer->getListener().on<Event::Draw_Prepare>([&](const auto& event){
		Core::Renderer& renderer = *event.renderer;
		renderer.frameBegin(&acceptBuffer1);

		acceptBuffer1.clearColor(Graphic::Colors::BLACK);

		GL::enable(GL::Test::DEPTH);
		GL::setDepthFunc(GL::Func::GEQUAL);
		GL::setDepthMask(true);

		Game::EntityManage::drawables.setViewport(Core::camera->getViewport().getPorjectedBound());
		Game::EntityManage::render();

		Game::core->effectManager->render();

		// auto* region = Core::assetsManager->getAtlas().find("base-pester");
		// //auto* region1 = Core::assetsManager->getAtlas().find("test-collapser");

		Graphic::Batch::flush<BatchWorld>();

		GL::setDepthMask(false);
		GL::disable(GL::Test::DEPTH);
		// GL::setDepthMask(false);

		GL::Blendings::Normal.apply();
		renderer.frameEnd(merger);
	});



	Core::renderer->getListener().on<Event::Draw_After>([&](const Event::Draw_After& event){
		event.renderer->frameBegin(&event.renderer->effectBuffer);

		Draw::Line::setLineStroke(5);
		Draw::color(Colors::GRAY);
		Game::EntityManage::realEntities.quadTree->each([](decltype(Game::EntityManage::realEntities)::TreeType* t){
			Draw::Line::rectOrtho(t->getBoundary());
		});

		Game::EntityManage::renderDebug();

		event.renderer->frameEnd(Assets::PostProcessors::bloom.get());
	});


	Core::renderer->getListener().on<Event::Draw_After>([&](const auto& e){
		Graphic::Mesh::meshBegin(Assets::Meshes::coords);
		Graphic::Mesh::meshEnd(true);

		e.renderer->frameBegin(&frameBuffer);
		e.renderer->frameBegin(&multiSample);

		auto [valid, invalid] = ext::partBy(chamberFrame.getData() | std::ranges::views::filter([](const Game::ChamberTile& tile){
			return Core::camera->viewportRect().overlap(tile.getChamberRealRegion());
		}), &Game::ChamberTile::valid);

		{
			[[maybe_unused]] auto guard = Draw::genColorGuard();
			e.renderer->frameBegin(&e.renderer->effectBuffer);
			Draw::color(Colors::DARK_GRAY);

			for(Game::ChamberTile& tile : valid){
				Draw::rectOrtho(Draw::globalState.defaultTexture, tile.getTileBound());
			}

			Assets::Shaders::outline_orthoArgs.set(4, Core::renderer->getSize().inverse());
			e.renderer->frameEnd(Assets::Shaders::outline_ortho);
			Assets::Shaders::outline_orthoArgs.setDef();

			Draw::color(Colors::GRAY, 0.45f);
			Draw::Line::setLineStroke(2.0f);
			for(Game::ChamberTile& tile : valid){
				Draw::Line::rectOrtho(tile.getTileBound());
			}

			Draw::color(Colors::LIGHT_GRAY, 0.85f);
			for(Game::ChamberTile& tile : valid | std::ranges::views::filter(&Game::ChamberTile::ownsChamber)){
				Draw::Line::rectOrtho(tile.getChamberRealRegion());
			}
		}


		{
			[[maybe_unused]] auto guard = Draw::genColorGuard();

			Draw::color(Colors::WHITE, 0.8f);
			Draw::mixColor(Colors::BLACK.createLerp(Colors::RED_DUSK, 0.3f));

			Assets::Shaders::slideLineShaderDrawArgs.set(30.0f, 45.0f, Colors::CLEAR, 1.0f);
			Graphic::Batch::beginShader(Assets::Shaders::sildeLines, true);

			for(Game::ChamberTile& tile : invalid)Draw::rectOrtho(Draw::getDefaultTexture(), tile.getTileBound());

			Graphic::Batch::endShader(true);

			Draw::color(Colors::PALE_GREEN);
			Draw::mixColor();

			auto transed = (Core::input->getCursorPos() - Core::renderer->getSize().div(2)).div(Core::camera->getScale()) + Core::camera->getPosition();

			Draw::Line::line(transed, {0, 0});
			const auto pos = chamberFrame.getNearby(transed);
			Geom::OrthoRectInt bound{pos, 3, 3};

			if(const auto finded = chamberFrame.find(pos)){
				Draw::rectOrtho(Draw::getDefaultTexture(), finded->getTileBound());
			}

			if(chamberFrame.placementValid(bound)){
				Draw::color(Colors::PALE_GREEN);
			}else{
				Draw::color(Colors::RED_DUSK);
			}
			Draw::Line::rectOrtho(bound.as<float>().scl(Game::TileSize, Game::TileSize));

			Assets::Shaders::slideLineShaderDrawArgs.setDef();
		}


		Graphic::Batch::blend();
		//
		const auto cameraPos = Core::camera->screenCenter();
		//
		Graphic::Mesh::meshBegin(Core::batchGroup.batchOverlay->getMesh());
		Draw::color();

		Draw::Line::setLineStroke(4);
		// Draw::Line::poly(cameraPos.getX(), cameraPos.getY(), 64, 160, 0, Math::clamp(fmod(OS::updateTime() / 5.0f, 1.0f)),
		// 		   Colors::SKY, Colors::ROYAL, Colors::SKY, Colors::WHITE, Colors::ROYAL, Colors::SKY
		// );

		Graphic::Draw::Line::setLineStroke(2.0f);

		Graphic::Draw::Line::setLineStroke(2.0f);
		Graphic::Draw::color(Graphic::Colors::YELLOW, 0.25f);
		chamberFrame.getQuadTree().each([](decltype(chamberFrame)::TreeType* t){
			Draw::Line::rectOrtho(t->getBoundary());
		});

		Draw::color();

		{
			static Geom::Matrix3D mat{};

			Graphic::Batch::beginPorj(mat.setOrthogonal(Core::renderer->getSize()));
			Draw::color();

			Draw::Line::square(Core::renderer->getCenterX(), Core::renderer->getCenterY(), 50, 45);

			Graphic::Batch::endPorj();
		}

		Graphic::Batch::flush();
		Graphic::Mesh::meshEnd(Core::batchGroup.batchOverlay->getMesh());

		e.renderer->frameEnd(Assets::PostProcessors::blendMulti.get());
		e.renderer->frameEnd(Assets::PostProcessors::bloom.get());
	});

	OS::activateHander();

	while(!Core::platform->shouldExit()){
		OS::update();

		Core::renderer->draw();

		Core::platform->pollEvents();
		OS::pollWindowEvent();
	}

	//Application Exit
	OS::deactivateHander();

	Game::EntityManage::clear();

	Assets::dispose();
	Core::dispose();

	return 0;
}
*/
