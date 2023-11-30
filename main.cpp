import <iostream>;
import <GLFW/glfw3.h>;
import <glad/glad.h>;
//import <print>;
import <functional>;

import Platform;
import File;
import Concepts;
import Container.Pool;
import Event;
import StackTrace;

import Graphic.Draw;
import Graphic.Pixmap;
import Graphic.RendererImpl;

import Math;
import Geom.Vector2D;
import Geom.Shape.Rect_Orthogonal;

import OS;
import OS.ApplicationListenerSetter;

import Core;
import Core.Batch.Batch_Sprite;

import Assets;
import Graphic.Color;

import Image;

import GL.Buffer.MultiSampleFrameBuffer;
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

int main(){
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

		int w, h;

		glfwGetWindowSize(Core::window, &w, &h);

		Core::renderer = new Graphic::RendererImpl{w, h};
	});

	{//TODO pack this into a class like screen shot manager
		Core::input->registerKeyBind(false, GLFW_KEY_F1, GLFW_PRESS, [](int key)mutable  {
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

	const GL::Texture2D texture{ Assets::textureDir.find("yyz.png") };

	GL::MultiSampleFrameBuffer frameBuffer{ Core::renderer->getWidth(), Core::renderer->getHeight() };
	Core::renderer->registerSynchronizedObject(&frameBuffer);

	auto&& file = Assets::assetsDir.subFile("test.txt");


	const Font::GlyphLayout layout{Font::parser->parse(file.readString())};


	Event::generalUpdateEvents.on<Event::Draw_Post>([&]([[maybe_unused]] const Event::Draw_Post& d){
		Draw::meshBegin(Core::batch->getMesh());
	    Core::renderer->frameBegin(frameBuffer);

	    // auto data = Assets::Fonts::manager->obtain(Assets::Fonts::telegrama);
		const Geom::Vector2D c = Core::camera->screenCenter();

		// Core::batch->getMesh()->vertexArray->bind();
		// glFlush();
		// Assets::Meshes::coords->unbind();

		Draw::stroke(3);
		Draw::color(Colors::WHITE);

		Draw::lineAngleCenter(c.getX(), c.getY(), 135.0f, 50.0f);

		Draw::lineAngleCenter(c.getX(), c.getY(), 45, 50);

		Draw::meshBegin(Assets::Meshes::coords);
		Draw::meshEnd(true);

		layout.render();
	 //
		 Draw::stroke(5);
		 Draw::poly(c.getX(), c.getY(), 64, 160, 0, Math::clamp(fmod(OS::globalTime() / 5.0f, 1.051f)),
		     { Colors::SKY, Colors::ROYAL, Colors::SKY, Colors::WHITE, Colors::ROYAL, Colors::SKY }
		 );


	    Core::renderer->frameEnd();
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