#include <Windows.h>

#include "src/application_head.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

import std;

import Assets.LoaderRenderer;

import UI.Align;

import Platform;
import OS.File;
import Concepts;
import Container.Pool;
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
import Geom.Shape.Rect_Orthogonal;

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
import Interval;

import Assets.TexturePacker;
import Assets.Loader;
import Assets.Manager;

//TODO 模块分区
import UI.Elem;
import UI.Root;
import UI.Table;
import UI.Label;
import UI.ScrollPane;
import UI.ElemDrawer;
import UI.Styles;
import UI.Button;
import UI.SliderBar;


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

using namespace std;
using namespace Graphic;
using namespace GL;
using Geom::Vec2;

std::stringstream sstream{};

void setupUITest() {
	const auto HUD = new UI::Table{};

	Core::uiRoot->root->transferElem(HUD).fillParent().setAlign(Align::center);

	HUD->relativeLayoutFormat = false;
	HUD->setBorderZero();
	HUD->setDrawer(UI::Styles::empty); {
		HUD->add<UI::Label>([](UI::Label& label){
			label.setText([]{
				auto str = sstream.str();
				sstream.str("");
				return str;
			});
			label.color.mul(0.6f);
			label.getInputListener().on<UI::MouseActionPress>([&label](const auto& e){
				switch(e.buttonID){
					case Ctrl::RMB :{
					   label.color = Colors::RED;
					   label.color.mul(0.6f);
					   break;
					}
					default : label.color.lerp(Colors::BLUE, 0.1f);
				}
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
						button.setCall([i](){
							std::cout << i << std::endl;
						});
					});
				}

			}).fillParent();

			table.add<UI::Table>([](UI::Table& t){
				t.setEmptyDrawer();
				t.add<UI::SliderBar>([](UI::SliderBar& s){
					s.setClampedOnHori();
				}).fillParent().lineFeed();
				t.add<UI::SliderBar>();
				t.add<UI::SliderBar>().setSrcScale(0.5f, 0.0f, false).lineFeed();
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

		{
			auto& cell2 = HUD->add<UI::Table>([](UI::Table& table){
				table.add<UI::Elem>();
				table.lineFeed();
				table.add<UI::Elem>();
				table.add<UI::Elem>();
			})
			.setAlign(Align::Mode::bottom_left)
			.setSizeScale(0.25f, 0.2f)
			.setMargin(0, 10, 10, 10);

			Core::input->registerKeyBind(Ctrl::KEY_F1, Ctrl::Act_Continuous, [&]{
				cell2.setSizeScale(cell2.getHoriScale() + 0.05f, 0.2f);
			});
		}

		HUD->transferElem(new UI::Table{})
		   .setAlign(Align::Mode::bottom_left)
		   .setSizeScale(0.075f, 0.2f)
		   .setSrcScale(0.25f, 0.0f)
		   .setMargin(10, 0, 10, 10);
	} {
		auto pane = new UI::ScrollPane{};

		pane->setItem<UI::Table>([](UI::Table& rt){
			rt.setSize(400, 900);
			rt.setFillparentX();

			rt.add<UI::ScrollPane>([](UI::ScrollPane& pane){
				pane.setItem<UI::Table>([](UI::Table& paneT){
					paneT.setHeight(600);
					paneT.setFillparentX();

					paneT.add<UI::Elem>();
					paneT.lineFeed();
					paneT.add<UI::Elem>();
					paneT.add<UI::Elem>();
				});
			});
			// rt->add(new UI::Elem);
			rt.lineFeed();
			rt.transferElem(new UI::Elem{});
			rt.transferElem(new UI::Elem{});
		});

		HUD->transferElem(pane).setAlign(Align::Mode::top_right).setSizeScale(0.225f, 0.25f).setMargin(10, 0, 0, 10);
	} {
		HUD->transferElem(new UI::Table{})
		   .setAlign(Align::top_right)
		   .setSizeScale(0.185f, 0.575f).setSrcScale(0.0f, 0.25f)
		   .setMargin(10, 0, 10, 0);
		//
		HUD->transferElem(new UI::Table{})
		   .setAlign(Align::top_right)
		   .setSizeScale(0.225f - 0.185f, 0.45f).setSrcScale(0.185f, 0.25f)
		   .setMargin(10, 10, 10, 0);
		//
		HUD->transferElem(new UI::Table{})
		   .setAlign(Align::Mode::bottom_right)
		   .setSizeScale(0.3f, 0.15f)
		   .setMargin(10, 0, 10, 0);
	}
}

void setupCtrl(){
	// Core::input->registerMouseBind(Ctrl::MOUSE_BUTTON_1, Ctrl::Act_Press, [] {
	// 	const auto pos = Game::core->overlayManager->getMouseInWorld();
	// 	auto* effect = Game::core->effectManager->suspend();
	// 	effect->setDrawer(&Assets::Effects::CircleDrawer)->set(pos, 0, Graphic::Colors::SKY);
	// });

	Core::input->registerKeyBind(Ctrl::KEY_F, Ctrl::Act_Continuous, [] {
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
				ptr->trans.rot = (Core::renderer->getSize() * 0.5f).angleTo(Core::input->getMousePos());

				ptr->trans.vec.add(Geom::Vec2{}.setPolar(ptr->trans.rot + 90, 80 * i));

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
		, [] {
		Game::EntityManage::realEntities.quadTree->intersectPoint(Core::camera->getScreenToWorld(Core::renderer->getNormalized(Core::input->getMousePos())),
		[](decltype(Game::EntityManage::realEntities)::ValueType* entity) {
			entity->controller->selected = !entity->controller->selected;
			Game::core->overlayManager->registerSelected(std::dynamic_pointer_cast<Game::RealityEntity>(std::move(entity->obtainSharedSelf())));
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
	Game::read(OS::File{R"(D:\projects\GameEngine\properties\resource\assets\hitbox\pester.hitbox)"}, ptr->hitBox);
	ptr->velo.vec.set(0, 0);
	ptr->setHealth(10000);
	ptr->init(Game::Content::Builtin::test);
	ptr->activate();
}

int main(const int argc, char* argv[]) {
	//Init
	::Test::init(argc, argv);

	::Test::assetsLoad();

	::Test::setupAudioTest();

	if(true){
		Game::core->overlayManager->activate();
		Game::core->hitBoxEditor->deactivate();
	}else{
		Game::core->hitBoxEditor->activate();
		Game::core->overlayManager->deactivate();
	}

	// Math::Rand rand{};
	// for(int i = 0; i < 20; ++i){
	// 	std::cout << rand.random(10.0f, 30.0f) << std::endl;
	// }


	::Core::renderer->getListener().on<Event::Draw_Overlay>([]([[maybe_unused]] const auto& e){
		Graphic::Batch::flush();
		Game::core->drawAboveUI(e.renderer);
		Graphic::Batch::flush();
	});

	::Core::renderer->getListener().on<Event::Draw_After>([]([[maybe_unused]] const auto& e){
		/*Game::core->effectManager->render();*/
		Graphic::Batch::flush();
	});

	::Core::renderer->getListener().on<Event::Draw_After>([]([[maybe_unused]] const auto& e){
		Game::core->drawBeneathUI(e.renderer);
		Graphic::Batch::flush();
	});


	// UI Test
	setupUITest();

	setupCtrl();

	genRandomEntities();

	GL::MultiSampleFrameBuffer multiSample{ Core::renderer->getWidth(), Core::renderer->getHeight() };
	GL::FrameBuffer frameBuffer{ Core::renderer->getWidth(), Core::renderer->getHeight() };

	GL::MultiSampleFrameBuffer worldFrameBuffer{ Core::renderer->getWidth(), Core::renderer->getHeight(), 4, 3};
	GL::FrameBuffer acceptBuffer1{ Core::renderer->getWidth(), Core::renderer->getHeight(), 3};

	Game::CombinePostProcessor merger{
		Assets::PostProcessors::blurX_Far.get(), Assets::PostProcessors::blurY_Far.get(),
		Assets::Shaders::merge
	};

	Core::renderer->registerSynchronizedResizableObject(&multiSample);
	Core::renderer->registerSynchronizedResizableObject(&frameBuffer);
	Core::renderer->registerSynchronizedResizableObject(&worldFrameBuffer);
	Core::renderer->registerSynchronizedResizableObject(&acceptBuffer1);

	const auto coordCenter = Font::obtainLayoutPtr();

	Game::EntityManage::init();
	Game::EntityManage::realEntities.resizeTree({-50000, -50000, 100000, 100000});

	float depth = 2;

	Core::input->registerKeyBind(Ctrl::KEY_PAGE_DOWN, Ctrl::Act_Press, [&depth](){
		depth++;
	});

	Core::input->registerKeyBind(Ctrl::KEY_PAGE_UP, Ctrl::Act_Press, [&depth](){
		depth--;
	});

	Core::renderer->getListener().on<Event::Draw_Post>([&](const auto& event) {
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



	Core::renderer->getListener().on<Event::Draw_After>([&](const auto& event) {
		event.renderer->frameBegin(&frameBuffer);

		Draw::Line::setLineStroke(5);
		Draw::color(Colors::GRAY);
		Game::EntityManage::realEntities.quadTree->each([](decltype(Game::EntityManage::realEntities)::TreeType* t) {
			Draw::Line::rectOrtho(t->getBoundary());
		});

		Game::EntityManage::renderDebug();

		event.renderer->frameEnd(Assets::PostProcessors::bloom.get());
	});


	Core::renderer->getListener().on<Event::Draw_After>([&](const auto& e) {
			Graphic::Mesh::meshBegin(Assets::Meshes::coords);
			Graphic::Mesh::meshEnd(true);

			e.renderer->frameBegin(&frameBuffer);
			e.renderer->frameBegin(&multiSample);

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

			Draw::color();

			{
				static Geom::Matrix3D mat{};

				Graphic::Batch::beginPorj(mat.setOrthogonal(Core::renderer->getSize()));
				Draw::color();

				sstream << "${font#tele}${scl#[0.55]}(" << std::fixed << std::setprecision(2) << cameraPos.getX() << ", " <<
						cameraPos.getY() << ") | " << std::to_string(OS::getFPS());
				sstream << "\n\nEntity count: " << Game::EntityManage::entities.idMap.size();
				sstream << "\nDraw count: " << std::ranges::count_if(Game::EntityManage::drawables.idMap | std::ranges::views::values, [](const decltype(Game::EntityManage::drawables.idMap)::value_type::second_type& i) {
					return i->isInScreen();
				});

				// Font::glyphParser->parse(coordCenter, R"()");
				//
				// coordCenter->offset.set(Core::renderer->getCenterX(), Core::renderer->getCenterY()).add(45, 45);
				// coordCenter->setAlign(Align::Mode::bottom_left);
				// coordCenter->render();

				Draw::Line::square(Core::renderer->getCenterX(), Core::renderer->getCenterY(), 50, 45);

				Graphic::Batch::endPorj();
			}

			Graphic::Batch::flush();
			Graphic::Mesh::meshEnd(Core::batchGroup.batchOverlay->getMesh());

			e.renderer->frameEnd(Assets::PostProcessors::blendMulti.get());
			e.renderer->frameEnd(Assets::PostProcessors::bloom.get());
		});

	OS::setupMainLoop();

	while(OS::shouldContinueLoop(Core::mainWindow)) {
		OS::update();

		Core::renderer->draw();

		OS::pollWindowEvent(Core::mainWindow);
	}

	//Application Exit
	OS::terminateMainLoop();

	Game::EntityManage::clear();

	Assets::dispose();
	Core::dispose();
}