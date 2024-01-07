#include "src/application_head.h"

import <iomanip>;

import <functional>;
import <iostream>;
import <cmath>;
import <numeric>;
#include <ranges>
import <sstream>;
import <unordered_set>;
import <glad/glad.h>;
import <GLFW/glfw3.h>;

import Assets.LoaderRenderer;

import Align;

import Platform;
import File;
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

import Async;

import OS;
import OS.ApplicationListenerSetter;

import Core;

import Core.Audio;
import Assets.Manager;
import Assets.Sound;
import Assets.Bundle;
import Core.Settings;
import Core.Log;

import UI.Root;
import Core.Batch;
import Core.Input;
import Core.Camera;
import Core.Renderer;
import OS.FileTree;

import Ctrl.ControlCommands;
import Ctrl.Constants;
import Core.Batch.Batch_Sprite;

import Assets.Graphic;
import Graphic.Color;

import Image;

import GL.Buffer.MultiSampleFrameBuffer;
import GL.Buffer.FrameBuffer;
import GL.Buffer.IndexBuffer;
import GL.VertexArray;
import GL.Mesh;
import GL.Texture.TextureRegionRect;
import GL.Texture.TextureNineRegion;
import GL.Blending;
import Event;
import GlyphArrangement;

import GL.Shader.Manager;

import Assets.TexturePacker;
import Assets.Loader;
import Assets.Manager;

import TimeMark;

import UI.Elem;
import UI.Root;
import UI.Table;
import UI.Label;
import UI.ScrollPane;
import UI.ElemDrawer;

import UI.Styles;
import Geom.Shape.RectBox;

import Test;
import Game.Core;
import Game.Entity.EntityManager;
import Game.Entity.SpaceCraft;

using namespace std;
using namespace Graphic;
using namespace Draw;
using namespace GL;
using Geom::Vec2;

std::string currentCoordText{ " " };
std::string_view* currentCoord = new std::string_view;

void setupUITest() {
	const auto HUD = new UI::Table{};

	Core::uiRoot->root->add(HUD).fillParent().setAlign(Align::center);

	HUD->relativeLayoutFormat = false;
	HUD->setMarginZero();
	HUD->setDrawer(UI::emptyDrawer.get()); {
		HUD->add<UI::Label>([](UI::Label& label) {
			   currentCoord = &label.getView();
			   label.color  = Colors::RED;
			   label.color.mul(0.6f);
			   label.setDynamic(true);
			   label.getInputListener().on<UI::MouseActionPress>([&label](const auto& e) {
				   switch(e.buttonID) {
					   case Ctrl::RMB : {
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


		HUD->add(new UI::Table{})
		   .setAlign(Align::Mode::top_left)
		   .setSizeScale(0.4f, 0.08f)
		   .setSrcScale(0.25f, 0.0f)
		   .setMargin(10, 0, 0, 0);
		HUD->add(new UI::Table{})
		   .setAlign(Align::Mode::top_left)
		   .setSizeScale(0.1f, 0.6f)
		   .setSrcScale(0.0f, 0.2f)
		   .setMargin(0, 0, 10, 10); {
			auto& cell2       = HUD->add(new UI::Table{});
			cell2.item->color = Colors::GREEN;
			cell2.setAlign(Align::Mode::bottom_left).setSizeScale(0.25f, 0.2f).setMargin(0, 10, 10, 10);
			cell2.clearRelativeMove();

			UI::Table& table = cell2.as<UI::Table>();

			table.add(new UI::Elem{});

			table.lineFeed();

			table.add(new UI::Elem{});
			table.add(new UI::Elem{});
		}

		HUD->add(new UI::Table{})
		   .setAlign(Align::Mode::bottom_left)
		   .setSizeScale(0.075f, 0.2f)
		   .setSrcScale(0.25f, 0.0f)
		   .setMargin(10, 0, 10, 10);
	} {
		auto pane = new UI::ScrollPane{};

		auto rt = new UI::Table{};
		rt->setSize(400, 900);
		rt->setFillparentX();
		pane->setItem(rt);

		HUD->add(pane).setAlign(Align::Mode::top_right).setSizeScale(0.225f, 0.25f).setMargin(10, 0, 0, 10);

		auto t   = new UI::Table{};
		t->color = Colors::RED;
		t->name  = "testT";
		rt->add(t);
		// rt->add(new UI::Elem);

		rt->lineFeed();
		rt->add(new UI::Elem{});
		rt->add(new UI::Elem{});
	} {
		HUD->add(new UI::Table{})
		   .setAlign(Align::top_right)
		   .setSizeScale(0.185f, 0.575f).setSrcScale(0.0f, 0.25f)
		   .setMargin(10, 0, 10, 0);
		//
		HUD->add(new UI::Table{})
		   .setAlign(Align::top_right)
		   .setSizeScale(0.225f - 0.185f, 0.45f).setSrcScale(0.185f, 0.25f)
		   .setMargin(10, 10, 10, 0);
		//
		HUD->add(new UI::Table{})
		   .setAlign(Align::Mode::bottom_right)
		   .setSizeScale(0.3f, 0.15f)
		   .setMargin(10, 0, 10, 0);
	}
}


int main(const int argc, char* argv[]) {
	//Init
	::Test::init(argc, argv);

	::Test::assetsLoad();

	::Test::setupAudioTest();

	std::unique_ptr<Game::Core> gameCore = std::make_unique<Game::Core>();

	OS::registerListener(gameCore.get());

	// UI Test
	setupUITest();

	GL::MultiSampleFrameBuffer multiSample{ Core::renderer->getWidth(), Core::renderer->getHeight() };
	GL::FrameBuffer frameBuffer{ Core::renderer->getWidth(), Core::renderer->getHeight() };

	Core::renderer->registerSynchronizedResizableObject(&multiSample);
	Core::renderer->registerSynchronizedResizableObject(&frameBuffer);

	std::stringstream ss{};
	Geom::Matrix3D mat{};
	const auto coordCenter = Font::obtainLayoutPtr();

	Game::EntityManage::init();
	Game::EntityManage::realEntities.resizeTree({-50000, -50000, 100000, 100000});

	Core::input->registerKeyBind(Ctrl::KEY_F, Ctrl::Act_Press, [] {
		Geom::RectBox box{};
		box.setSize(75, 15);
		box.offset = box.edgeLength;
		box.offset.mul(-0.5f);

		const auto ptr = Game::EntityManage::obtain<Game::SpaceCraft>();
		ptr->position.set(Core::camera->getPosition().x,  + Core::camera->getPosition().y);
		ptr->rotation =
			Core::input->getMousePos()
				.sub(Core::renderer->getDrawWidth() * 0.5f, Core::renderer->getDrawHeight() * 0.5f)
				.angle();

		ptr->velocity.set(180, 0).rotate(ptr->rotation);
		Game::EntityManage::add(ptr);
		ptr->hitBox = box;
		ptr->inertialMass = 100;
		ptr->activate();
	});

	Core::input->registerKeyBind(Ctrl::KEY_I, Ctrl::Act_Press, [] {
		int i =  Game::EntityManage::entities.idMap.at(0).use_count();
		std::cout << i << std::endl;
	});

	Core::input->registerKeyBind(Ctrl::KEY_F, Ctrl::Act_Repeat, [] {
		Geom::RectBox box{};
		box.setSize(50, 15);
		box.offset = box.edgeLength;
		box.offset.mul(-0.5f);

		const auto ptr = Game::EntityManage::obtain<Game::SpaceCraft>();
		ptr->position.set(Core::camera->getPosition().x,  + Core::camera->getPosition().y);
		ptr->rotation =
			Core::input->getMousePos()
				.sub(Core::renderer->getDrawWidth() * 0.5f, Core::renderer->getDrawHeight() * 0.5f)
				.angle();

		ptr->velocity.set(120, 0).rotate(ptr->rotation);
		Game::EntityManage::add(ptr);
		ptr->hitBox = box;
		ptr->activate();
	});

	{
		Math::Rand rand = Math::globalRand;
		for(int i = 0; i < 800; ++i) {
			Geom::RectBox box{};
			box.setSize(rand.random(360), rand.random(360));
			box.rotation = rand.random(360);
			box.offset = box.edgeLength;
			box.offset.mul(-0.5f);

			const auto ptr = Game::EntityManage::obtain<Game::SpaceCraft>();
			ptr->rotation  = rand.random(360.0f);
			ptr->position.set(rand.range(40000), rand.range(40000));
			Game::EntityManage::add(ptr);
			ptr->hitBox = box;
			ptr->velocity.set(1, 0).rotate(rand.random(360));
			ptr->activate();
		}

		Geom::RectBox box{};
		box.setSize(20, 100);
		box.rotation = 0;
		box.offset = box.edgeLength;
		box.offset.mul(-0.5f);

		const auto ptr = Game::EntityManage::obtain<Game::SpaceCraft>();
		ptr->position.set(1000, 100);
		Game::EntityManage::add(ptr);
		ptr->hitBox = box;
		ptr->velocity.set(0, 0);
		ptr->activate();

	}

	Core::input->registerKeyBind(Ctrl::KEY_P, Ctrl::Act_Press, [] {
		OS::setPause(!OS::isPaused());
	});

	Core::renderer->getListener().on<Event::Draw_Post>([&](const Event::Draw_Post& e) {
		e.renderer->frameBegin(&frameBuffer);
		Game::EntityManage::drawables.setViewport(Core::camera->getViewport().getPorjectedBound());
		Game::EntityManage::render();

		Draw::setLineStroke(5);
		Draw::color(Colors::GRAY);
		Game::EntityManage::realEntities.quadTree->each([](Geom::QuadTreeF<Game::RealityEntity>* t) {
			Draw::rectLine(t->getBoundary());
		});
		e.renderer->frameEnd(Assets::PostProcessors::bloom);
	});

	{
		Core::renderer->getListener().on<Event::Draw_Post>([&]([[maybe_unused]] const Event::Draw_Post& e) {
			// e.renderer->frameBegin(&multiSample);
			Draw::meshBegin(Assets::Meshes::coords);
			Draw::meshEnd(true);
			// e.renderer->frameEnd(Assets::PostProcessors::blendMulti);
			//
			e.renderer->frameBegin(&frameBuffer);
			e.renderer->frameBegin(&multiSample);
			//
			const auto cameraPos = Core::camera->screenCenter();
			//
			Draw::meshBegin(Core::batch->getMesh());
			Draw::color();

			Draw::setLineStroke(3);
			Draw::color(Colors::BLUE_SKY);

			Draw::setLineStroke(5);
			Draw::poly(cameraPos.getX(), cameraPos.getY(), 64, 160, 0, Math::clamp(fmod(OS::updateTime() / 5.0f, 1.0f)),
					   { &Colors::SKY, &Colors::ROYAL, &Colors::SKY, &Colors::WHITE, &Colors::ROYAL, &Colors::SKY }
			);

			Draw::color(); {
				mat.setOrthogonal(0.0f, 0.0f, static_cast<float>(Core::renderer->getWidth()),
								  static_cast<float>(Core::renderer->getHeight()));

				Core::batch->beginProjection(mat);
				Draw::color();

				ss.str("");
				ss << "${font#tele}${scl#[0.55]}(" << std::fixed << std::setprecision(2) << cameraPos.getX() << ", " <<
						cameraPos.getY() << ") | " << std::to_string(OS::getFPS());
				ss << "\n\nEntity count: " << Game::EntityManage::entities.idMap.size();
				ss << "\nDraw count: " << std::ranges::count_if(Game::EntityManage::drawables.idMap | std::ranges::views::values, [](const decltype(Game::EntityManage::drawables.idMap)::value_type::second_type& i) {
					return i->isInScreen();
				});
				ss << "\nTotal hit: " << Game::totalHit;


				currentCoordText = ss.str();
				*currentCoord    = currentCoordText;
				// Font::glyphParser->parse(coordCenter, *currentCoord);
				//
				// coordCenter->offset.set(Core::renderer->getCenterX(), Core::renderer->getCenterY()).add(45, 45);
				// coordCenter->setAlign(Align::Mode::bottom_left);
				// coordCenter->render();

				Draw::setLineStroke(4);
				Draw::lineSquare(Core::renderer->getCenterX(), Core::renderer->getCenterY(), 50, 45);

				Core::batch->endProjection();
			}

			Draw::flush();
			Draw::meshEnd(Core::batch->getMesh(), false);

			e.renderer->frameEnd(Assets::PostProcessors::blendMulti);
			e.renderer->frameEnd(Assets::PostProcessors::bloom);
		});
	}

	OS::setupLoop();

	while(OS::continueLoop(Core::mainWindow)) {
		OS::update();

		Core::renderer->draw();

		OS::poll(Core::mainWindow);
	}

	//Application Exit
	OS::terminateLoop();

	Game::EntityManage::clear();

	Assets::dispose();
	Core::dispose();
}