import <iostream>;
import <GLFW/glfw3.h>;
//import <print>;
import <functional>;

import Platform;
import File;
import Concepts;
import Container.Pool;
import Event;
import StackTrace;

import Graphic.Draw;
import Graphic.RendererImpl;

import Math;
import Geom.Vector2D;

import OS;
import OS.ApplicationListenerSetter;

import Core;
import Core.Batch.Batch_Sprite;

import Assets;
import Graphic.Color;


import GL.Buffer.MultiSampleFrameBuffer;
import GL.Buffer.IndexBuffer;
import GL.VertexArray;
import GL.Mesh;
import GL.Blending;
import Event;

using namespace std;
using namespace Graphic;
using namespace Draw;
using namespace GL;

int main(){
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

	Graphic::Draw::defTexture(*Assets::Textures::whiteRegion);
	Graphic::Draw::texture();

	const GL::Texture2D texture{ Assets::textureDir.find("yyz.png") };

	GL::MultiSampleFrameBuffer frameBuffer{ Core::renderer->getWidth(), Core::renderer->getHeight() };

	Core::renderer->registerSynchronizedObject(&frameBuffer);

	using Graphic::Color;

	Event::generalUpdateEvents.on<Event::Draw_Post>([&]([[maybe_unused]] const Event::Draw_Post& d){

	    Core::renderer->frameBegin(frameBuffer);


	    float offset = 200.0f;

	    for (int il = -8; il <= 8; ++il) {
	        const auto i = abs(static_cast<float>(il));

	        Color c1{ Math::absin(i * OS::globalTime() * 1.5f, 24 / i,0.6f) + 0.45f, Math::absin(i * OS::globalTime() * 1.5f + 0.33f * 2, 16 / i,0.6f) + 0.45f, Math::absin(i * OS::globalTime() * 1.5f + 0.66f * 1, 24 / i,0.6f) + 0.45f , 0.3f + Math::absin(OS::globalTime() * i * 1.0f, 5, 0.7f) };
	        Color c2{ Math::absin(i * OS::globalTime() * 2.5f, 32 / i,0.6f) + 0.45f, Math::absin(i * OS::globalTime() * 2.5f + 0.33f * 3, 24 / i,0.6f) + 0.45f, Math::absin(i * OS::globalTime() * 2.5f + 0.66f * 2, 16 / i,0.6f) + 0.45f , 0.3f + Math::absin(OS::globalTime() * i * 2.0f, 5, 0.7f) };
	        Color c3{ Math::absin(i * OS::globalTime() * 3.0f, 24 / i,0.6f) + 0.45f, Math::absin(i * OS::globalTime() * 3.0f + 0.33f * 0, 32 / i,0.6f) + 0.45f, Math::absin(i * OS::globalTime() * 3.0f + 0.66f * 3, 24 / i,0.6f) + 0.45f , 0.3f + Math::absin(OS::globalTime() * i * 3.0f, 5, 0.7f) };
	        Color c4{ Math::absin(i * OS::globalTime() * 2.0f, 16 / i,0.6f) + 0.45f, Math::absin(i * OS::globalTime() * 2.0f + 0.33f * 1, 24 / i,0.6f) + 0.45f, Math::absin(i * OS::globalTime() * 2.0f + 0.66f * 0, 32 / i,0.6f) + 0.45f , 0.3f + Math::absin(OS::globalTime() * i * 4.0f, 5, 0.7f) };

	        Draw::vert(texture,
	            (i - 0.5f) * offset, (i - 0.5f) * offset, c1, Colors::CLEAR,
	            (i - 0.5f) * offset, (i + 0.5f) * offset, c2, Colors::CLEAR,
	            (i + 0.5f) * offset, (i + 0.5f) * offset, c3, Colors::CLEAR,
	            (i + 0.5f) * offset, (i - 0.5f) * offset, c4, Colors::CLEAR
	        );
	    }

	    Draw::blend(GL::Blendings::ADDICTIVE);

	    offset += 12;

	    for (int il = -8; il <= 8; ++il) {
	        const auto i = abs(static_cast<float>(il));

	        Color c4{ Math::absin(i * OS::globalTime() * 1.5f, 24 / i,0.6f) + 0.45f, Math::absin(i * OS::globalTime() * 1.5f + 0.33f * 2, 16 / i,0.6f) + 0.45f, Math::absin(i * OS::globalTime() * 1.5f + 0.66f * 1, 24 / i,0.6f) + 0.45f , 0.1f + Math::absin(i * OS::globalTime() * i * 1.0f, 5, 0.3f) };
	        Color c3{ Math::absin(i * OS::globalTime() * 2.5f, 32 / i,0.6f) + 0.45f, Math::absin(i * OS::globalTime() * 2.5f + 0.33f * 3, 24 / i,0.6f) + 0.45f, Math::absin(i * OS::globalTime() * 2.5f + 0.66f * 2, 16 / i,0.6f) + 0.45f , 0.1f + Math::absin(i * OS::globalTime() * i * 2.0f, 5, 0.3f) };
	        Color c2{ Math::absin(i * OS::globalTime() * 3.0f, 24 / i,0.6f) + 0.45f, Math::absin(i * OS::globalTime() * 3.0f + 0.33f * 0, 32 / i,0.6f) + 0.45f, Math::absin(i * OS::globalTime() * 3.0f + 0.66f * 3, 24 / i,0.6f) + 0.45f , 0.1f + Math::absin(i * OS::globalTime() * i * 3.0f, 5, 0.3f) };
	        Color c1{ Math::absin(i * OS::globalTime() * 2.0f, 16 / i,0.6f) + 0.45f, Math::absin(i * OS::globalTime() * 2.0f + 0.33f * 1, 24 / i,0.6f) + 0.45f, Math::absin(i * OS::globalTime() * 2.0f + 0.66f * 0, 32 / i,0.6f) + 0.45f , 0.1f + Math::absin(i * OS::globalTime() * i * 4.0f, 5, 0.3f) };

	        Draw::vert(texture,
	            (i - 0.5f) * offset, (i - 0.5f) * offset, c1, Colors::CLEAR,
	            (i - 0.5f) * offset, (i + 0.5f) * offset, c2, Colors::CLEAR,
	            (i + 0.5f) * offset, (i + 0.5f) * offset, c3, Colors::CLEAR,
	            (i + 0.5f) * offset, (i - 0.5f) * offset, c4, Colors::CLEAR
	        );
	    }

	    Geom::Vector2D c = Core::camera->screenCenter();

	    Draw::stroke(5);
	    Draw::poly(c.getX(), c.getY(), 64, 160, 0, Math::clamp(fmod(OS::globalTime() / 5.0f, 1.051f)),
	        { Colors::SKY, Colors::ROYAL, Colors::SKY, Colors::WHITE, Colors::ROYAL, Colors::SKY }
	    );

	    Draw::blend();

		Draw::color(Colors::WHITE);
		Draw::stroke(5);

	    Draw::lineAngleCenter(c.getX(), c.getY(), 45, 50);
	    Draw::lineAngleCenter(c.getX(), c.getY(), -45, 50);

	    Draw::flush();

	    Core::renderer->frameEnd();
	});

	while (OS::continueLoop(Core::window)){
		/*Main Loop*/

		OS::update();

		Core::camera->setOrtho(static_cast<float>(Core::renderer->getWidth()), static_cast<float>(Core::renderer->getHeight()));

		Core::renderer->draw();

		OS::poll(Core::window);
	}

	Assets::dispose();

	Core::dispose();
}