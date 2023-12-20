import <iomanip>;

import <iostream>;
import <GLFW/glfw3.h>;
import <glad/glad.h>;
import <functional>;
import <sstream>;
import <unordered_set>;

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
import Assets.Manager;

import TimeMark;

import UI.Root;
import UI.Label;
import UI.ScrollPane;

using namespace std;
using namespace Graphic;
using namespace Draw;
using namespace GL;
using Geom::Vector2D;


void init(const int argc, char* argv[]) {
	//TODO move these into application loader
	//Register Cmd
	OS::args.reserve(argc);
	for(int i = 0; i < argc; ++i)OS::args.emplace_back(argv[0]);

	stbi::setFlipVertically_load(true);
	stbi::setFlipVertically_write(true);

	Core::initCore();

	OS::loadListeners(Core::mainWindow);

	Assets::loadBasic();

	OS::setApplicationIcon(Core::mainWindow, stbi::obtain_GLFWimage(Assets::assetsDir.subFile("icon.png")).get());

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

		glfwGetWindowSize(Core::mainWindow, &w, &h);
		Core::renderer = new Graphic::RendererImpl{static_cast<unsigned>(w), static_cast<unsigned>(h)};

		Ctrl::registerCommands(Core::input);

		Core::renderer->registerSynchronizedResizableObject(Core::camera);
		Core::camera->resize(w, h);

		Core::uiRoot = new UI::Root{};
		Core::uiRoot->root->setRoot(Core::uiRoot);
		Core::uiRoot->resize(w, h);
		Core::renderer->registerSynchronizedResizableObject(Core::uiRoot);
		OS::registerListener(Core::uiRoot);
	});

	Graphic::Draw::defTexture(&Assets::Textures::whiteRegion);
	Graphic::Draw::texture();
	Graphic::Draw::rawMesh = Assets::Meshes::raw;
	Graphic::Draw::blitter = Assets::Shaders::blit;

	Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadPull>([](const auto& event) {
		Assets::TexturePackPage* testPage = event.manager->getAtlas().registerPage("test", Assets::texCacheDir);
		Assets::textureDir.subFile("test").forAllSubs([&testPage](OS::File&& file) {
			testPage->pushRequest(file);
		});

		testPage->pushRequest(Assets::textureDir.find("white.png"));
	});

	Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadEnd>([](const auto& event) {

		Assets::Textures::whiteRegion = *event.manager->getAtlas().find("test-white");

		Assets::Textures::whiteRegion.shrinkEdge(4.0f);

		Graphic::Draw::defTexture(&Assets::Textures::whiteRegion);
		Graphic::Draw::texture();
	});


	//Majority Load
	Core::loadAssets();
}

int main(const int argc, char* argv[]) {
	//Init
	::init(argc, argv);

	//UI Test
	auto& cell = Core::uiRoot->root->add(new UI::Label{});
	cell.setAlign(Align::Mode::top_left).setSizeScale(0.35f, 0.2f);
	cell.marginLeft = cell.marginRight = cell.marginBottom = cell.marginTop = 10;
	cell.item->color = Colors::RED;
	cell.item->color.mul(0.6f);
	cell.clearRelativeMove();
	cell.as<UI::Label>().setText("test 1231231231");
	cell.as<UI::Label>().setDynamic(true);

	auto& view = cell.as<UI::Label>().getView();

	cell.item->getInputListener().on<UI::MouseActionPress>([item = cell.item](auto& e) {
		if(e.buttonID == Ctrl::LMB)item->color.lerp(Colors::BLUE, 0.1f);
		if(e.buttonID == Ctrl::RMB) {
			item->color = Colors::RED;
			item->color.mul(0.6f);
		}
	});

	auto& cell2 = Core::uiRoot->root->add(new UI::Table{});
	cell2.item->color = Colors::GREEN;
	cell2.setAlign(Align::Mode::bottom_left).setSizeScale(0.3f, 0.3f);
	cell2.clearRelativeMove();

	UI::Table& table = cell2.as<UI::Table>();

	table.add(new UI::Elem{});

	table.endRow();

	table.add(new UI::Elem{});
	table.add(new UI::Elem{});


	auto& cellp = Core::uiRoot->root->add(new UI::ScrollPane{}).setAlign(Align::Mode::bottom_right).setSizeScale(0.225f, 0.33f).clearRelativeMove();
	cellp.marginLeft = cellp.marginRight = cellp.marginBottom = cellp.marginTop = 50;

	//Draw Test
	auto tex = Core::assetsManager->getAtlas().find("test-pester-full");

	const GL::Texture2D bottomLeftTex{ Assets::textureDir.subFile("ui").find("bottom-left.png") };
	const GL::Texture2D texture{ Assets::textureDir.find("yyz.png") };

	GL::MultiSampleFrameBuffer multiSample{ Core::renderer->getWidth(), Core::renderer->getHeight() };
	GL::FrameBuffer frameBuffer{ Core::renderer->getWidth(), Core::renderer->getHeight() };

	GL::TextureNineRegion uiTest{&bottomLeftTex, {0, 0, 256, 256}, {64, 64, 32, 64}};

	Core::renderer->registerSynchronizedResizableObject(&multiSample);
	Core::renderer->registerSynchronizedResizableObject(&frameBuffer);

	auto&& file = Assets::assetsDir.subFile("test.txt");
	const auto coordCenter = Font::obtainLayoutPtr();
	std::stringstream ss{};

	const auto layout = Font::obtainLayoutPtr();
	layout->maxWidth = 720;

	Font::glyphParser->parse(layout, file.readString());
	layout->setAlign(Align::Mode::bottom_left);
	layout->move(80, 30);

	std::string str;

	Event::generalUpdateEvents.on<Event::Draw_Post>([&]([[maybe_unused]] const Event::Draw_Post& d){
		Draw::meshBegin(Assets::Meshes::coords);
		Draw::meshEnd(true);
		//
		Core::renderer->frameBegin(&frameBuffer);
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

		Draw::rect(tex, 200, 500, -45);

		layout->render();



		Geom::Matrix3D mat{};
		mat.setOrthogonal(0.0f, 0.0f, static_cast<float>(Core::renderer->getWidth()), static_cast<float>(Core::renderer->getHeight()));

		Core::batch->beginProjection(mat);

		uiTest.render_RelativeExter(100, 100, 500, 800);

		Core::batch->endProjection();


		ss.str("");
		ss << "${font#tele}${scl#[0.52]}(" << std::fixed << std::setprecision(2) << center.getX() << ", " << center.getY() << " | " << std::to_string(OS::getFPS()) << ")";
		str = ss.str();
		view = std::string_view{str};
		Font::glyphParser->parse(coordCenter, view);

		coordCenter->offset.set(center).add(155, 35);

		coordCenter->setAlign(Align::Mode::bottom_left);
		coordCenter->render();

		Draw::rectLine(layout->bound, true, layout->offset);
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
		Core::renderer->frameEnd(Assets::PostProcessors::bloom);
	});

	OS::setupLoop();

	while (OS::continueLoop(Core::mainWindow)){
		OS::update();

		Core::renderer->draw();

		OS::poll(Core::mainWindow);
	}

	//Application Exit
	OS::terminateLoop();

	Assets::dispose();
	Core::dispose();
}
