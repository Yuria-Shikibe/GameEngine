#include <glad/glad.h>

#include "src/application_head.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../src/code-gen/ReflectData_Builtin.hpp"
#include "src/ext/ReflectionUtil.hpp"

import std;

import Assets.LoaderRenderer;

import UI.Align;

import Core.Platform;
import OS.File;
import ext.Concepts;
import ext.Container.ObjectPool;
import ext.Event;
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
import Math.Algo.StripPacker2D;
import Geom.Vector2D;
import Geom.Matrix3D;
import Geom.Rect_Orthogonal;

import OS;
import OS.ApplicationListenerSetter;

import Core;
import Core.MainLoopManager;

import Core.Audio;
import Assets.Manager;
import Assets.Effects;
import Assets.Sound;
import Assets.Bundle;
import Assets.Ctrl;
import Core.Settings;
import Core.Log;

import UI.Root;
import Core.Batch;
import Core.Input;
import Core.Camera;
import Core.Renderer;

import OS.Ctrl.ControlCommands;
import OS.Ctrl.Bind.Constants;
import Core.Batch.Batch_Sprite;

import Assets.Directories;
import Assets.Graphic;
import Graphic.Color;

import GL.Buffer.MultiSampleFrameBuffer;
import GL.Buffer.FrameBuffer;
import GL.Texture.TextureRegion;
import GL.Texture.TextureNineRegion;
import GL.Blending;
import ext.Event;
import Font.GlyphArrangement;
import ext.Timer;

import Assets.TexturePacker;
import Assets.Loader;
import Assets.Manager;

//TODO 模块分区
import UI.Elem;
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
import UI.InputArea;

import Geom.Shape.RectBox;
import Geom.QuadTree;

import Test;
import Game.Core;
import Game.Entity.EntityManager;
import Game.Entity.RealityEntity;
import Game.Entity.SpaceCraft;
import Game.Entity.Bullet;
import Game.Entity.Collision;
import Game.Entity.Turret;
import Game.Entity.Controller.AI;
import Game.Entity.Controller.Player;

import Game.Content.Type.BasicBulletType;
import Game.Content.Type.Turret.BasicTurretType;
import Game.Content.Builtin.SpaceCrafts;
import Game.Content.Builtin.Turrets;

import Game.Graphic.CombinePostProcessor;
import Game.Graphic.Draw;

import Game.Chamber.Frame;
import Game.Chamber.Util;
import Game.Chamber.FrameTrans;

import Game.Scene.MainMenu;

import ext.Encoding;
import ext.TreeStructure;
import ext.algorithm;
import ext.Heterogeneous;
import ext.Json;
import ext.Base64;
import ext.StringParse;
import ext.StaticReflection;
import ext.ReflectData;

import ext.json.io;
import ext.SpecIOSummary;

using namespace Graphic;
using namespace GL;
using Geom::Vec2;

import SideTemp;
bool drawDebug{false};

import Assets.Load.State;
import Assets.Load.Core;
import Assets.Load.Misc;

import Assets.Load.TexturePacker;
import Assets.TexturePage;

import Math.Algo.TopologicalSort;

template <auto what>
struct Print{
	void operator ()() const{
		std::println("{}", what);
		std::cout.flush();
	}
};



int main(){
	// ext::StringMap<int> map{};

	Assets::TexturePage mainPage{"main"};
	Assets::TexturePage lightPage{"light"};

	OS::File testTextureDir{R"(D:\projects\GameEngine\properties\resource\assets\texture\main)"};


	using namespace Assets::Load;
	//
	LoadManager loadManager{};
	TexturePacker packer{};

	packer.setCacheDir(OS::File{R"(D:\projects\GameEngine\properties\resource\cache)"});

	testTextureDir.subFile("base").forAllSubs([&](auto&& file){
		mainPage.pullRequest<Assets::FileImportData>(file.stem(), file);
	});

	testTextureDir.subFile("light").forAllSubs([&](auto&& file){
		auto name = file.stem();
		auto pos = name.find_first_of('.');
		name = name.substr(0, pos);

		lightPage.pullRequest<Assets::FileImportData>(name, file);
	});

	lightPage.dependency = &mainPage;
	packer.pushPage(&mainPage);
	packer.pushPage(&lightPage);
	loadManager.registerTask(packer);

	loadManager.launch();

	while(!loadManager.isFinished()){
		loadManager.processRequests();
	}

	loadManager.requestDone();
	// page.p


	// Math::TestT _1{nullptr, 1};
	// Math::TestT _2{nullptr, 2};
	// Math::TestT _3{nullptr, 3};
	// Math::TestT _4{nullptr, 4};
	// Math::TestT _5{nullptr, 5};
	//
	// std::vector<Math::TestT*> arr{
	// 	&_1,
	// 	&_2,
	// 	&_3,
	// 	&_4,
	// 	&_5,
	// };
	//
	// arr[0]->dependencyTarget = arr[4];
	// arr[4]->dependencyTarget = arr[2];
	// arr[2]->dependencyTarget = arr[3];
	// arr[3]->dependencyTarget = arr[1];
	// Math::sort_topological<std::less>(arr, &Math::TestT::dependencyTarget);
	// for (auto value : arr){
	// 	std::println("{}", value->val);
	// }

	// MiscTaskManager miscTaskManager1{};
	// MiscTaskManager miscTaskManager2{};
	//
	// miscTaskManager1.push(Phase::init, Print<1>{});
	// miscTaskManager1.push(Phase::pull, Print<2>{});
	// miscTaskManager1.push(Phase::load, Print<3>{});
	// miscTaskManager1.push(Phase::load, Print<4>{});
	// miscTaskManager1.push(Phase::post_load, Print<5>{});
	// miscTaskManager1.push(Phase::end, Print<6>{});
	// miscTaskManager1.push(Phase::clear, Print<7>{});
	//
	// miscTaskManager2.push(Phase::init, Print<11>{});
	// miscTaskManager2.push(Phase::pull, Print<12>{});
	// miscTaskManager2.push(Phase::load, Print<13>{});
	// miscTaskManager2.push(Phase::load, Print<14>{});
	// miscTaskManager2.push(Phase::post_load, Print<15>{});
	// miscTaskManager2.push(Phase::end, Print<16>{});
	// miscTaskManager2.push(Phase::clear, Print<17>{});
	// // std::ranges::find_if()
	// auto& t =miscTaskManager1.eventManager;
	// auto i = std::to_underlying(Phase::end);
	//
	// miscTaskManager1.eventManager.on(Phase::end, []{
	// 	std::println("Manager 1 End Call");
	// });
	//
	// loadManager.registerTask(miscTaskManager1);
	// loadManager.registerTask(miscTaskManager2);
	//
	// loadManager.launch();
	//
	// loadManager.requestDone();
}

//TODO temp global static mut should be remove


void setupUITest(){
	const auto HUD = new UI::Table{};

	Core::uiRoot->currentScene->transferElem(HUD).fillParent().setAlign(Align::Layout::center);

	HUD->setLayoutByRelative(false);
	HUD->setBorderZero();
	HUD->setDrawer(&UI::emptyDrawer);

	HUD->add<UI::Table>([](UI::Table& label){
		   label.add<UI::ScrollPane>([](UI::ScrollPane& pane){
			   pane.setItem<UI::Label>([](UI::Label& label){
				   label.setText([]{
					   std::ostringstream sstream{};
					   sstream << "$<scl#[0.55]>(" << std::fixed << std::setprecision(2) << Core::camera->
						   getPosition().x << ", " <<
						   Core::camera->getPosition().y << ") | " << std::to_string(OS::getFPS());
					   sstream << "\n\nEntity count: " << Game::EntityManage::entities.idMap.size();
					   sstream << "\nDraw count: " << std::ranges::count_if(
						   Game::EntityManage::drawables.idMap | std::ranges::views::values, std::identity{},
						   &decltype(Game::EntityManage::drawables)::ValueType::isInScreen);

					   sstream << "\nDrawCount: " << GL::getDrawCallCount();
					   return std::move(sstream).str();
				   });

				   label.setEmptyDrawer();
				   label.setFillparentX();
				   label.setWrap(false);
			   });
			   pane.setEmptyDrawer();
			   // pane.setItem<UI::Elem>([](UI::Elem& area){
			   // 	area.setWidth(1000);
			   // 	area.setFillparentY();
			   // });
		   });
	   })
	   .setAlign(Align::Layout::top_left).setSizeScale(0.25f, 0.2f)
	   .setMargin(0, 10, 0, 10);


	HUD->add<UI::Table>([](UI::Table& table){
		   table.add<UI::ProgressBar>([](UI::ProgressBar& bar){
			   bar.progressGetter = []{
				   if(Game::core->playerController){
					   return Game::core->playerController->getOwner()->getHealthRatio();
				   }
				   return 0.0f;
			   };

			   bar.PointCheck = 12;

			   bar.setEmptyDrawer();

			   bar.setTooltipBuilder({
					   .followTarget = UI::TooltipFollowTarget::parent,
					   .followTargetAlign = Align::Layout::bottom_center,
					   .tooltipSrcAlign = Align::Layout::top_center,
					   .builder = [&bar](UI::Table& hint){
						   // hint.setMinimumSize({600, 300});
						   hint.setCellAlignMode(Align::Layout::top_left);
						   hint.add<UI::Label>([&bar](UI::Label& label){
							   label.setWrap();
							   label.setText([&bar]{
								   return std::format(
									   "$<c#GRAY>Hit Point: $<c#LIGHT_GRAY>{:.2f}$<scl#[0.75]>$<c#PALE_GREEN>%",
									   bar.getDrawProgress() * 100.0f);
							   });
							   label.getGlyphLayout()->setSCale(0.55f);
							   label.setEmptyDrawer();
							   label.getBorder().expand(2.0f);
						   }).expand().endLine();
					   }
				   });
		   });

	   })
	   .setAlign(Align::Layout::top_left)
	   .setSizeScale(0.4f, 0.08f)
	   .setSrcScale(0.25f, 0.0f)
	   .setMargin(10, 0, 0, 0);


	HUD->add<UI::Table>([](UI::Table& table){
		   table.add<UI::ScrollPane>([](UI::ScrollPane& pane){
			   pane.setEmptyDrawer();
			   pane.setFillparent();
			   pane.setItem<UI::Table>([](UI::Table& menu){
				   menu.setEmptyDrawer();
				   menu.add<UI::Button>([](UI::Button& button){
					   button.add<UI::Label>([](UI::Label& label){
						   label.setTextAlign(Align::Layout::center);
						   label.setEmptyDrawer();
						   label.setTextScl(0.65f);
						   label.setFillparentX();
						   label.setWrap();

						   label.setText(label.getBundleEntry("sync-camera"));
					   }).setHeight(60);
					   button.setCall([]{
						   Game::core->cameraLock = !Game::core->cameraLock;
					   });
					   button.setActivatedChecker([](auto&){
						   return Game::core->cameraLock;
					   });
				   }).wrapY().endLine().setPad({.bottom = 10.0f});

				   menu.add<UI::Button>([](UI::Button& button){
					   button.add<UI::Label>([](UI::Label& label){
						   label.setTextAlign(Align::Layout::center);
						   label.setEmptyDrawer();
						   label.setTextScl(0.65f);
						   label.setFillparentX();
						   label.setWrap();

						   label.setText(label.getBundleEntry("draw-debug"));
					   }).setHeight(60);
					   button.setCall([]{
						   drawDebug = !drawDebug;
					   });
					   button.setActivatedChecker([](auto&){
						   return drawDebug;
					   });
				   }).wrapY().endLine();
			   });
		   });
	   })
	   .setAlign(Align::Layout::top_left)
	   .setSizeScale(0.15f, 0.6f)
	   .setSrcScale(0.0f, 0.2f)
	   .setMargin(0, 0, 10, 10);
}

void setupCtrl(){
	Assets::Ctrl::gameGroup = Ctrl::OperationGroup{
			Assets::Ctrl::gameGroup.getName(), {
				GameCtrl::moveLeft,
				GameCtrl::moveRight,
				GameCtrl::moveForward,
				GameCtrl::moveBack,

				GameCtrl::shoot_prs,
				GameCtrl::shoot_rls,
				GameCtrl::moveTrans_prs,
				GameCtrl::moveTrans_rls,
			}
		};
	//TODO unsafe
	Assets::Ctrl::gameGroup.loadInstruction(Core::bundle);
	Assets::Ctrl::gameGroup.targetGroup = &Game::core->gameBinds;
	Assets::Ctrl::gameGroup.applyToTarget();
}

void setupBaseDraw(){
	::Core::renderer->getListener().on<ext::Draw_Overlay>([](const auto& e){
		Core::uiRoot->drawCursor();
	});

	::Core::renderer->getListener().on<ext::Draw_After>([]([[maybe_unused]] const auto& e){
		Game::core->drawBeneathUI(e.renderer);
		// Graphic::Draw::Overlay::getBatch().flush();
	});
}

int main_(const int argc, char* argv[]){
	//Init
	::Test::init(argc, argv);

	::Test::assetsLoad();
	::Test::setupUITest_Old();

	::Test::setupAudioTest();

	Core::audio->engine->setSoundVolume(0.5f);

	::Test::chamberFrame = std::make_unique<Game::ChamberGridTrans<Game::SpaceCraft>>();

	if(true){
		Game::core->overlayManager->activate();
		Game::core->hitBoxEditor->deactivate();
	} else{
		Game::core->hitBoxEditor->activate();
		Game::core->overlayManager->deactivate();
	}


	::Test::genRandomEntities();

	// UI Test
	setupUITest();

	setupCtrl();

	setupBaseDraw();


	::Test::chamberFrame = std::make_unique<Game::ChamberGridTrans<Game::SpaceCraft>>();
	::Test::loadChamberTest();

	Core::uiRoot->registerScene<Game::Scenes::MainMenu>(UI::Menu_Main);

	Core::uiRoot->switchScene(UI::Menu_Main);



	GL::MultiSampleFrameBuffer multiSample{Core::renderer->getWidth(), Core::renderer->getHeight()};
	GL::FrameBuffer frameBuffer{Core::renderer->getWidth(), Core::renderer->getHeight()};

	GL::MultiSampleFrameBuffer worldFrameBuffer{Core::renderer->getWidth(), Core::renderer->getHeight(), 16, 4, true};
	GL::FrameBuffer acceptBuffer1{Core::renderer->getWidth(), Core::renderer->getHeight(), 4, true};

	Game::CombinePostProcessor merger{
			Assets::PostProcessors::blurX_World.get(), Assets::PostProcessors::blurY_World.get(),
			Assets::Shaders::merge
		};
	merger.blur.setScale(0.5f);
	merger.blur.setProcessTimes(3);
	merger.setTargetState(GL::State::BLEND, true);

	Core::renderer->registerSynchronizedResizableObject(&multiSample);
	Core::renderer->registerSynchronizedResizableObject(&frameBuffer);
	Core::renderer->registerSynchronizedResizableObject(&worldFrameBuffer);
	Core::renderer->registerSynchronizedResizableObject(&acceptBuffer1);

	// multiSample.getRenderBuffer().reset();

	Game::EntityManage::init();
	Game::EntityManage::resizeTree({-50000, -50000, 100000, 100000});
	//

	// GL::Blendings::Disable.apply(worldFrameBuffer.getID());


	Core::renderer->getListener().on<ext::Draw_After>([&](const ext::Draw_After& event){
		if(!drawDebug) return;
		// event.renderer->effectBuffer.bind();
		event.renderer->frameBegin(&frameBuffer);

		Draw::Overlay::Line::setLineStroke(5);
		Draw::Overlay::color(Colors::GRAY);
		Game::EntityManage::mainTree.each([](const decltype(Game::EntityManage::mainTree)& t){
			Draw::Overlay::Line::rectOrtho(t.getBoundary());
		});

		Game::EntityManage::renderDebug();

		event.renderer->frameEnd(Assets::PostProcessors::bloom.get());
	});

	// glDepthRangef(Graphic::Draw::DepthNear, Graphic::Draw::DepthFar);
	Core::renderer->getListener().on<ext::Draw_Prepare>([&](const auto& event){
		GL::Blendings::Normal.apply();

		Graphic::Mesh::meshBegin();
		Graphic::Mesh::meshEnd(true, Assets::Shaders::coordAxis);

		// acceptBuffer1.getColorAttachments().at(3)->setFilter(GL::nearest, GL::nearest);

		Core::Renderer& renderer = *event.renderer;

		GL::enable(GL::Test::DEPTH);
		GL::setDepthMask(true);
		renderer.frameBegin(acceptBuffer1);
		renderer.frameBegin(worldFrameBuffer);

		// acceptBuffer1.clearColor(Graphic::Colors::BLACK, 2);
		// acceptBuffer1.clearColor(Graphic::Colors::WHITE, 1);
		acceptBuffer1.clearColor(Graphic::Colors::WHITE, 3);

		GL::setDepthFunc(GL::Func::GEQUAL);

		// GL::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		Game::EntityManage::drawables.setViewport(Core::camera->getViewport());
		Game::EntityManage::render();

		Game::core->effectManager->render(Core::camera->getViewport());

		Graphic::Draw::World::getBatch().flush();

		GL::disable(GL::Test::DEPTH);
		GL::setDepthMask(false);

		// GL::Blendings::Disable.apply();
		renderer.frameEnd(Assets::PostProcessors::multiToBasic.get());
		renderer.frameEnd(merger);
		// GL::Blendings::Normal.apply();
	});

	Core::renderer->getListener().on<ext::Draw_After>([&](const auto& e){
		if(!drawDebug) return;
		e.renderer->frameBegin(&frameBuffer);
		e.renderer->frameBegin(&multiSample);

		Graphic::Draw::Overlay::getBatch().flush();
		Graphic::Draw::Overlay::getBatch().switchBlending();

		{
			Core::BatchGuard_Proj _{Graphic::Draw::Overlay::getBatch()};
			_->setOrthogonal(Core::renderer->getSize());

			auto [x, y] = Core::renderer->getSize().scl(0.5f);

			using Graphic::Draw::Overlay;

			Overlay::color(Graphic::Colors::PALE_GREEN);
			Overlay::Line::setLineStroke(4.f);
			Overlay::Line::square(x, y, 50, 45);
			Overlay::Line::poly(x, y, 64, 160, 0, Math::clamp(fmod(OS::updateTime().count() / 5.0f, 1.0f)),
				Colors::SKY.copy().setA(0.55f), Colors::ROYAL.copy().setA(0.55f),
				Colors::SKY.copy().setA(0.55f), Colors::WHITE.copy().setA(0.55f),
				Colors::ROYAL.copy().setA(0.55f), Colors::SKY.copy().setA(0.55f)
			);
		}

		e.renderer->frameEnd(Assets::PostProcessors::blendMulti.get());
		e.renderer->frameEnd(Assets::PostProcessors::bloom.get());
	});

	OS::activateHander();

	ext::Timer<1> timer{};
	while(!Core::platform->shouldExit()){
		Core::renderer->draw();

		Core::loopManager->update();

		Core::audio->setListenerPosition(Core::camera->getPosition().x, Core::camera->getPosition().y);

		// std::println("{}", OS::getFPS());

		Core::platform->pollEvents();

		OS::pollWindowEvent();

		GL::resetDrawCallCount();

		Core::renderer->drawOverlay();

		// timer.run(30.f, OS::deltaTick(), []{
		// 	std::println("{}", GL::getDrawCallCount());
		// 	std::cout.flush();
		// });
		Core::renderer->blit();

		Core::loopManager->updateTaskEnd();
	}

	//Application Exit
	OS::deactivateHander();

	Game::EntityManage::clear();

	Assets::dispose();
	Core::dispose();

	return 0;
}
