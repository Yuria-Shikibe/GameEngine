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

	Geom::RectBox box1{};
	Geom::RectBox box2{};

	box1.setSize(100, 200);

	box2.setSize(20, 500);
	box2.rotation = 60.0f;

	box1.offset      = { -50, -100 };
	box1.rotation    = 30;
	box1.originPoint = { 100, 100 };
	box1.update();

	Game::EntityManage::init();

	Core::input->registerKeyBind(Ctrl::KEY_SPACE, Ctrl::Act_Repeat, [] {
		Geom::RectBox box{};
		box.setSize(20, 500);
		box.rotation = 60.0f;

		const auto ptr = Game::EntityManage::obtain<Game::SpaceCraft>();
		Game::EntityManage::add(ptr);
		ptr->hitBox = box;
		// std::cout << Game::EntityManage::entities.idMap.size();
	});

	Core::renderer->getListener().on<Event::Draw_Post>([&](const Event::Draw_Post& e) {
		Game::EntityManage::drawables.setViewport(Core::camera->getViewport().getPorjectedBound());
		Game::EntityManage::render();
	}); {
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
			Draw::poly(cameraPos.getX(), cameraPos.getY(), 64, 160, 0, Math::clamp(fmod(OS::globalTime() / 5.0f, 1.0f)),
			           { &Colors::SKY, &Colors::ROYAL, &Colors::SKY, &Colors::WHITE, &Colors::ROYAL, &Colors::SKY }
			);

			box2.originPoint.set(140, 140).mul(Math::sin(OS::globalTime()) + 0.75f)
			    .add(Math::sin(OS::globalTime() * 2.0f) * 60, Math::cos(OS::globalTime() * 2.0f) * 60);
			box2.update();

			Draw::setLineStroke(1);
			if(box1.overlapRough(box2)) {
				Draw::color(Graphic::Colors::CORAL);
			}
			Draw::rectLine(box1.maxOrthoBound);
			Draw::rectLine(box2.maxOrthoBound);

			Draw::color(Colors::BLUE_SKY);
			if(box1.overlapExact(box2)) {
				Draw::color(Graphic::Colors::CORAL);
			}

			Draw::quad(Draw::defaultTexture, box1.v0, box1.v1, box1.v2, box1.v3);
			Draw::quad(Draw::defaultTexture, box2.v0, box2.v1, box2.v2, box2.v3);

			Draw::color(); {
				mat.setOrthogonal(0.0f, 0.0f, static_cast<float>(Core::renderer->getWidth()),
				                  static_cast<float>(Core::renderer->getHeight()));

				Core::batch->beginProjection(mat);
				Draw::color();

				ss.str("");
				ss << "${font#tele}${scl#[0.7]}(" << std::fixed << std::setprecision(2) << cameraPos.getX() << ", " <<
						cameraPos.getY() << ") | " << std::to_string(OS::getFPS());
				ss << "\n\nEntity count: " << Game::EntityManage::entities.idMap.size();
				ss << "\nDraw count: " << std::ranges::count_if(Game::EntityManage::drawables.idMap | std::ranges::views::values, [](const decltype(Game::EntityManage::drawables.idMap)::value_type::second_type& i) {
					return i->isInScreen();
				});


				currentCoordText = ss.str();
				*currentCoord    = currentCoordText;
				Font::glyphParser->parse(coordCenter, *currentCoord);

				box2.rotation += OS::deltaTick();

				coordCenter->offset.set(Core::renderer->getCenterX(), Core::renderer->getCenterY()).add(45, 45);
				coordCenter->setAlign(Align::Mode::bottom_left);
				coordCenter->render();

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
