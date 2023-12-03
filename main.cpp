#include <iomanip>

import <iostream>;
import <GLFW/glfw3.h>;
import <glad/glad.h>;
//import <print>;
import <functional>;
import <sstream>;

import Platform;
import File;
import Concepts;
import Container.Pool;
import Event;
import StackTrace;

import Graphic.Draw;
import Graphic.Pixmap;
import Graphic.RendererImpl;

import Graphic.PostProcessor.BloomProcessor;
import Graphic.PostProcessor.ShaderProcessor;
import Graphic.PostProcessor.PipeProcessor;
import Graphic.PostProcessor.P4Processor;



import Math;
import Geom.Vector2D;
import Geom.Shape.Rect_Orthogonal;

import OS;
import OS.ApplicationListenerSetter;

import Core;
import Ctrl.ControlCommands;
import Ctrl.Constants;
import Core.Batch.Batch_Sprite;

import Assets;
import Graphic.Color;

import Image;

import GL.Buffer.MultiSampleFrameBuffer;
import GL.Buffer.FrameBuffer;
import GL.Buffer.IndexBuffer;
import GL.VertexArray;
import GL.Mesh;
import GL.Texture.TextureRegionRect;
import GL.Blending;
import Event;
import GlyphArrangement;

using namespace std;
using namespace Graphic;
using namespace Draw;
using namespace GL;

bool screenShotRequest = false;

void init() {
	stbi::setFlipVertically_load(true);
	stbi::setFlipVertically_write(true);
	//TODO move these into application loader
	Core::initCore();

	OS::loadListeners(Core::window);

	Assets::load();

	Core::initCore_Post([] {
		Core::batch = new Core::SpriteBatch([](const Core::SpriteBatch& self) {
			auto* const shader = Assets::Shaders::screenSpace;

			shader->setUniformer([&self](const GL::Shader& s){
				s.setTexture2D("u_texture", *self.getTexture());
				s.setMat3("view", *self.getProjection());
			});

			return shader;
		});

		Core::batch->setProjection(&Core::camera->worldToScreen);

		int w, h;

		glfwGetWindowSize(Core::window, &w, &h);

		Core::renderer = new Graphic::RendererImpl{w, h};

		Ctrl::registerCommands(Core::input);

		Core::input->registerMouseBind(Ctrl::LMB, Ctrl::Act_DoubleClick, [] {
			Geom::Vector2D pos = Core::input->getMousePos();
			pos.div(Core::renderer->getWidth(), Core::renderer->getHeight()).scl(2.0f).sub(1.0f, 1.0f).scl(1.0f, -1.0f);
			pos *= Core::camera->screenToWorld;
			Core::camera->position.set(pos);
		});
	});

	{//TODO pack this into a class like screen shot manager
		Core::input->registerKeyBind(Ctrl::KEY_F1, Ctrl::Act_Press, []() mutable  {
			screenShotRequest = true;
		});

		Event::generalUpdateEvents.on<Event::Draw_After>([](const Event::Draw_After& a){
			if(screenShotRequest) {
				const Graphic::Pixmap pixmap{Core::renderer->getWidth(), Core::renderer->getHeight(), Core::renderer->defaultFrameBuffer->readPixelsRaw()};
				//
				auto&& f = Assets::screenshotDir.subFile("lastShot.png");
				//
				pixmap.write(f, true);

				screenShotRequest = false;
			}
		});
	}

	Graphic::Draw::defTexture(*Assets::Textures::whiteRegion);
	Graphic::Draw::texture();
	Graphic::Draw::rawMesh = Assets::Meshes::raw;
}

int main(){
	init();

	const GL::Texture2D texture{ Assets::textureDir.find("yyz.png") };

	GL::MultiSampleFrameBuffer multiSample{ Core::renderer->getWidth(), Core::renderer->getHeight() };
	GL::FrameBuffer frameBuffer{ Core::renderer->getWidth(), Core::renderer->getHeight() };

	Core::renderer->registerSynchronizedObject(&multiSample);
	Core::renderer->registerSynchronizedObject(&frameBuffer);



	auto&& file = Assets::assetsDir.subFile("test.txt");

	const auto layout = Font::glyphParser->parse(file.readString());
	layout->setAlign(Font::TypeSettingAlign::bottom_left);
	layout->move(80, 30);

	const auto coordCenter = std::make_shared<Font::GlyphLayout>();

	Graphic::ShaderProcessor blurX{Assets::Shaders::gaussian, [](const Shader& shader) {
		shader.setVec2("direction", {2, 0});
	}};

	Graphic::ShaderProcessor blurY{Assets::Shaders::gaussian, [](const Shader& shader) {
		shader.setVec2("direction", {0, 2});
	}};


	Graphic::BloomProcessor processor{&blurX, &blurY, Assets::Shaders::bloom};

	Event::generalUpdateEvents.on<Event::Draw_Post>([&]([[maybe_unused]] const Event::Draw_Post& d){
		Draw::meshBegin(Assets::Meshes::coords);
		Draw::meshEnd(true);

		Draw::meshBegin(Core::batch->getMesh());

	    Core::renderer->frameBegin(frameBuffer);
	    Core::renderer->frameBegin(multiSample);
	    // auto data = Assets::Fonts::manager->obtain(Assets::Fonts::telegrama);
		const Geom::Vector2D c = Core::camera->screenCenter();

		// Core::batch->getMesh()->vertexArray->bind();
		// glFlush();
		// Assets::Meshes::coords->unbind();

		Draw::stroke(3);
		Draw::color(Colors::WHITE);

		Draw::lineAngleCenter(c.getX(), c.getY(), 135.0f, 50.0f);

		Draw::lineAngleCenter(c.getX(), c.getY(), 45, 50);

		layout->render();

		std::stringstream ss{};


		ss << "${font#tele}${scl#[0.52]}(" << std::fixed << std::setprecision(2) << c.getX() << ", " << c.getY() << ")";

		Font::glyphParser->parse(coordCenter.get(), ss.str());

		coordCenter->offset.set(c).add(155, 35);
		coordCenter->setAlign(Font::TypeSettingAlign::bottom_left);
		coordCenter->render();

		Draw::stroke(3);
		Draw::color(Colors::RED);

		Draw::rect_line(layout->bound, true, layout->offset);

		Draw::stroke(5);
		Draw::poly(c.getX(), c.getY(), 64, 160, 0, Math::clamp(fmod(OS::globalTime() / 5.0f, 1.0f)),
		    { Colors::SKY, Colors::ROYAL, Colors::SKY, Colors::WHITE, Colors::ROYAL, Colors::SKY }
		);

	    Core::renderer->frameEnd(Assets::PostProcessors::multiToBasic);
	    Core::renderer->frameEnd(&processor);
		Draw::meshEnd(Core::batch->getMesh());
	});

	while (OS::continueLoop(Core::window)){
		/*Main Loop*/

		OS::update();

		Core::camera->setOrtho(static_cast<float>(Core::renderer->getWidth()), static_cast<float>(Core::renderer->getHeight()));

		Core::renderer->draw();

		// Draw::blit()

		OS::poll(Core::window);
	}

	Assets::dispose();

	Core::dispose();
}