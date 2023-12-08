#include <iomanip>

import <iostream>;
import <GLFW/glfw3.h>;
import <glad/glad.h>;
import <functional>;
import <sstream>;
import <unordered_set>;

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
import Math.StripPacker2D;
import Geom.Vector2D;
import Geom.Matrix3D;
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
import GL.Texture.TextureNineRegion;
import GL.Blending;
import Event;
import GlyphArrangement;
import Graphic.TexturePacker;

using namespace std;
using namespace Graphic;
using namespace Draw;
using namespace GL;



bool screenShotRequest = false;
bool bloomTest = true;

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

		Core::batch->setProjection(&Core::camera->getWorldToScreen());

		int w, h;

		glfwGetWindowSize(Core::window, &w, &h);

		Core::renderer = new Graphic::RendererImpl{w, h};

		Ctrl::registerCommands(Core::input);

		Core::input->registerMouseBind(Ctrl::LMB, Ctrl::Act_DoubleClick, [] {
			Geom::Vector2D pos = Core::input->getMousePos();
			pos.div(Core::renderer->getWidth(), Core::renderer->getHeight()).scl(2.0f).sub(1.0f, 1.0f).scl(1.0f, -1.0f);
			pos *= Core::camera->getScreenToWorld();
			Core::camera->setPosition(pos);
		});
	});

	{//TODO pack this into a class like screen shot manager
		Core::input->registerKeyBind(Ctrl::KEY_F1, Ctrl::Act_Press, []() mutable  {
			screenShotRequest = true;
		});

		Core::input->registerKeyBind(Ctrl::KEY_F2, Ctrl::Act_Press, []() mutable  {
			bloomTest = !bloomTest;
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
	Graphic::Draw::blitter = Assets::Shaders::blit;
}

int main(int argc, char* argv[]){
	OS::args.reserve(argc);

	for(int i = 0; i < argc; ++i) {
		OS::args.emplace_back(argv[0]);
	}

	::init();

	Graphic::TexturePackPage page{Assets::texCacheDir, "test"};

	Assets::textureDir.subFile("test").forAllSubs([&page](OS::File&& file) {
		page.push(file);
	});

	page.load();

	const GL::Texture2D bottomLeftTex{ Assets::textureDir.subFile("ui").find("bottom-left.png") };

	const GL::Texture2D texture{ Assets::textureDir.find("yyz.png") };

	GL::MultiSampleFrameBuffer multiSample{ Core::renderer->getWidth(), Core::renderer->getHeight() };
	GL::FrameBuffer frameBuffer{ Core::renderer->getWidth(), Core::renderer->getHeight() };

	GL::TextureNineRegion uiTest{&bottomLeftTex, {0, 0, 256, 256}, {64, 64, 32, 64}};


	Core::renderer->registerSynchronizedObject(&multiSample);
	Core::renderer->registerSynchronizedObject(&frameBuffer);


	auto&& file = Assets::assetsDir.subFile("test.txt");
	const auto coordCenter = std::make_shared<Font::GlyphLayout>();
	const auto layout = std::make_shared<Font::GlyphLayout>();
	layout->maxWidth = 720;
	Font::glyphParser->parse(layout.get(), file.readString());
	layout->setAlign(Font::TypeSettingAlign::bottom_left);
	layout->move(80, 30);

	Graphic::ShaderProcessor blurX{Assets::Shaders::gaussian, [](const Shader& shader) {
		shader.setVec2("direction", {1.32f, 0});
	}};

	Graphic::ShaderProcessor blurY{Assets::Shaders::gaussian, [](const Shader& shader) {
		shader.setVec2("direction", {0, 1.32f});
	}};

	Graphic::BloomProcessor bloom{&blurX, &blurY, Assets::Shaders::bloom, Assets::Shaders::threshold_light};

	Graphic::ShaderProcessor blend{Draw::blitter};
	// Graphic::P4Processor processor{&blurX, &blurY};
	Graphic::PipeProcessor multiBlend{};
	multiBlend << Assets::PostProcessors::multiToBasic << &blend;

	Event::generalUpdateEvents.on<Event::Draw_Post>([&]([[maybe_unused]] const Event::Draw_Post& d){
		Draw::meshBegin(Assets::Meshes::coords);
		Draw::meshEnd(true);

	    if(bloomTest)Core::renderer->frameBegin(frameBuffer);
	    Core::renderer->frameBegin(multiSample);

		const Geom::Vector2D c = Core::camera->screenCenter();
		Draw::meshBegin(Core::batch->getMesh());

		Draw::stroke(3);
		Draw::color(Colors::WHITE);

		Draw::lineAngleCenter(c.getX(), c.getY(), 135.0f, 50.0f);

		Draw::lineAngleCenter(c.getX(), c.getY(), 45, 50);

		//const auto& t = page.find("pester-full")->textureRegion;
		//Draw::rect(t, 200, 500, -45);

		layout->render();

		std::stringstream ss{};

		Geom::Matrix3D mat{};
		mat.setOrthogonal(0.0f, 0.0f, static_cast<float>(Core::renderer->getWidth()), static_cast<float>(Core::renderer->getHeight()));

		Core::batch->beginProjection(mat);

		uiTest.render_RelativeExter(100, 100, 500, 800);

		Core::batch->endProjection();


		ss << "${font#tele}${scl#[0.52]}(" << std::fixed << std::setprecision(2) << c.getX() << ", " << c.getY() << ")";

		Font::glyphParser->parse(coordCenter.get(), ss.str());

		coordCenter->offset.set(c).add(155, 35);
		coordCenter->setAlign(Font::TypeSettingAlign::bottom_left);
		coordCenter->render();

		Draw::stroke(3);
		Draw::color(Colors::BLUE, Colors::SKY, 0.745f);

		Draw::rect_line(layout->bound, true, layout->offset);

		Draw::stroke(5);
		Draw::poly(c.getX(), c.getY(), 64, 160, 0, Math::clamp(fmod(OS::globalTime() / 5.0f, 1.0f)),
		    { Colors::SKY, Colors::ROYAL, Colors::SKY, Colors::WHITE, Colors::ROYAL, Colors::SKY }
		);

		Draw::flush();
		Draw::meshEnd(Core::batch->getMesh());
	    Core::renderer->frameEnd(&multiBlend);
	    if(bloomTest)Core::renderer->frameEnd(&bloom);
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