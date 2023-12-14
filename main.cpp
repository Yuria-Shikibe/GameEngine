#include <iomanip>

import <iostream>;
import <GLFW/glfw3.h>;
import <glad/glad.h>;
import <functional>;
import <sstream>;
import <unordered_set>;

import Assets.LoaderRenderer;

import Platform;
import File;
import Concepts;
import Container.Pool;
import Event;
import StackTrace;

import Graphic.Draw;
import Graphic.Pixmap;
import Graphic.RendererImpl;
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

import GL.Shader.Manager;

import Assets.TexturePacker;
import Assets.Loader;

import TimeMark;

using namespace std;
using namespace Graphic;
using namespace Draw;
using namespace GL;
using Geom::Vector2D;


bool bloomTest = true;

void init() {
	stbi::setFlipVertically_load(true);
	stbi::setFlipVertically_write(true);
	//TODO move these into application loader
	Core::initCore();

	OS::loadListeners(Core::window);

	Assets::loadBasic();

	OS::setApplicationIcon(Core::window, stbi::obtain_GLFWimage(Assets::assetsDir.subFile("icon.png")));

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
		Core::renderer = new Graphic::RendererImpl{static_cast<unsigned>(w), static_cast<unsigned>(h)};

		Ctrl::registerCommands(Core::input);

		Core::renderer->registerSynchronizedResizableObject(Core::camera);
		Core::camera->resize(w, h);
	});

	Graphic::Draw::defTexture(Assets::Textures::whiteRegion);
	Graphic::Draw::texture();
	Graphic::Draw::rawMesh = Assets::Meshes::raw;
	Graphic::Draw::blitter = Assets::Shaders::blit;

	Core::batch->setupBlending();
}

int main(const int argc, char* argv[]) {
	//Register Cmd
	OS::args.reserve(argc);
	for(int i = 0; i < argc; ++i)OS::args.emplace_back(argv[0]);

	//Init
	::init();

	//Test
	Graphic::TextureAtlas atlas{};
	Assets::TexturePackPage* testPage = atlas.registerPage(Assets::TexturePackPage{Assets::texCacheDir, "test"});

	atlas.setContextPage("test");
	Assets::textureDir.subFile("test").forAllSubs([&atlas](OS::File&& file) {
		atlas.load(file);
	});

	Assets::AssetsLoader loader{};

	ShaderManager shaderManager{};
	Font::FontLoader fontLoader{};


	{
		Assets::Shaders::load(&shaderManager);

		Font::FontLoader tempFontLoader{};
		tempFontLoader.quickInit = true;
		Assets::Fonts::loadPreivous(&tempFontLoader);
		Assets::Fonts::load(&fontLoader);

		auto chores = ext::create<Assets::AssetsTaskHandler>([]{

		});

		loader.push(testPage);
		loader.push(&shaderManager);
		loader.push(&fontLoader);
		// loader.push(&chores);

		loader.begin();
		// loader.forceGet();

		auto loadRenderer = Assets::LoaderRenderer{Core::renderer->getWidth(), Core::renderer->getHeight(), &loader};
		while (true){
			if(loader.finished()) {
				if(loadRenderer.lastProgress > 0.999f) {
					break;
				}
			}else{
				loader.processRequests();
			}

			loadRenderer.draw();

			OS::poll(Core::window);


		}

		atlas.flush();
		Assets::loadAfter();

		Font::glyphParser->context.defaultFont = Assets::Fonts::telegrama;
		Font::defaultManager = fontLoader.manager.get();
	}

	const auto& tex = atlas.getContextPage()->findPackData("pester-full")->textureRegion;

		const GL::Texture2D bottomLeftTex{ Assets::textureDir.subFile("ui").find("bottom-left.png") };
		const GL::Texture2D texture{ Assets::textureDir.find("yyz.png") };

		GL::MultiSampleFrameBuffer multiSample{ Core::renderer->getWidth(), Core::renderer->getHeight() };
		GL::FrameBuffer frameBuffer{ Core::renderer->getWidth(), Core::renderer->getHeight() };

		GL::TextureNineRegion uiTest{&bottomLeftTex, {0, 0, 256, 256}, {64, 64, 32, 64}};

		Core::renderer->registerSynchronizedResizableObject(&multiSample);
		Core::renderer->registerSynchronizedResizableObject(&frameBuffer);

		auto&& file = Assets::assetsDir.subFile("test.txt");
		const auto coordCenter = std::make_shared<Font::GlyphLayout>();
		std::stringstream ss{};

		const auto layout = std::make_shared<Font::GlyphLayout>();
		layout->maxWidth = 720;

		Font::glyphParser->parse(layout, file.readString());
		layout->setAlign(Font::TypeSettingAlign::bottom_left);
		layout->move(80, 30);

		Event::generalUpdateEvents.on<Event::Draw_Post>([&]([[maybe_unused]] const Event::Draw_Post& d){
			Draw::meshBegin(Assets::Meshes::coords);
			Draw::meshEnd(true);
			//
			if(bloomTest)Core::renderer->frameBegin(&frameBuffer);
			Core::renderer->frameBegin(&multiSample);
			//
			const auto center = Core::camera->screenCenter();
			//
			Draw::meshBegin(Core::batch->getMesh());
			//
			Draw::setLineStroke(3);
			Draw::color(Colors::WHITE);

			Draw::lineAngleCenter(center.getX(), center.getY(), 135.0f, 50.0f);

			Draw::lineAngleCenter(center.getX(), center.getY(), 45, 50);
			//
			Draw::color();

			if(atlas.getContextPage()->finished()) {
				const auto texTest = atlas.find("test-pester");

				Draw::rect(texTest, 200, 500, -45);
			}else {

				Draw::setLineStroke(10);
				Draw::color(Colors::GRAY);
				Draw::lineAngleCenter(100, 100, 0, 800);
				Draw::color(Colors::SKY);
				Draw::lineAngleCenter(100, 100, 0, 800 * atlas.getContextPage()->getProgress());
			}

			layout->render();

			ss.str("");

			Geom::Matrix3D mat{};
			mat.setOrthogonal(0.0f, 0.0f, static_cast<float>(Core::renderer->getWidth()), static_cast<float>(Core::renderer->getHeight()));

			Core::batch->beginProjection(mat);

			uiTest.render_RelativeExter(100, 100, 500, 800);

			Core::batch->endProjection();


			ss << "${font#tele}${scl#[0.52]}(" << std::fixed << std::setprecision(2) << center.getX() << ", " << center.getY() << " | " << std::to_string(OS::getFPS()) << ")";

			Font::glyphParser->parse(coordCenter, ss.str());

			coordCenter->offset.set(center).add(155, 35);

			coordCenter->setAlign(Font::TypeSettingAlign::bottom_left);
			coordCenter->render();

			Draw::rect_line(layout->bound, true, layout->offset);
			//
			Draw::setLineStroke(3);
			Draw::color(Colors::BLUE, Colors::SKY, 0.745f);

			Draw::setLineStroke(5);
			Draw::poly(center.getX(), center.getY(), 64, 160, 0, Math::clamp(fmod(OS::globalTime() / 5.0f, 1.0f)),
				{ Colors::SKY, Colors::ROYAL, Colors::SKY, Colors::WHITE, Colors::ROYAL, Colors::SKY }
			);

			Draw::flush();
			Draw::meshEnd(Core::batch->getMesh(), false);
			Core::renderer->frameEnd(Assets::PostProcessors::blendMulti);
			if(bloomTest)Core::renderer->frameEnd(Assets::PostProcessors::bloom);
		});

	OS::setupLoop();

	while (OS::continueLoop(Core::window)){
		OS::update();

		Core::renderer->draw();

		OS::poll(Core::window);
	}

	//Application Exit
	OS::terminateLoop();

	Assets::dispose();

	Core::dispose();
}
