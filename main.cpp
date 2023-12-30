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

using namespace std;
using namespace Graphic;
using namespace Draw;
using namespace GL;
using Geom::Vector2D;

std::string currentCoordText{" "};
std::string_view* currentCoord;

void setupUITest(){
	const auto HUD = new UI::Table{};

	Core::uiRoot->root->add(HUD).fillParent().setAlign(Align::center);

	HUD->relativeLayoutFormat = false;
	HUD->zerolizeMargin();
	HUD->setDrawer(UI::emptyDrawer.get());

	{

		auto& cell = HUD->add(new UI::Label{});
		cell.setAlign(Align::top_left).setSizeScale(0.25f, 0.2f);
		cell.marginRight = cell.marginBottom = 10;
		cell.item->color = Colors::RED;
		cell.item->color.mul(0.6f);
		cell.clearRelativeMove();
		cell.as<UI::Label>().setText("test 1231231231");
		cell.as<UI::Label>().setDynamic(true);

		currentCoord = &cell.as<UI::Label>().getView();

		cell.item->getInputListener().on<UI::MouseActionPress>([item = cell.item](auto& e) {
			if(e.buttonID == Ctrl::LMB)item->color.lerp(Colors::BLUE, 0.1f);
			if(e.buttonID == Ctrl::RMB) {
				item->color = Colors::RED;
				item->color.mul(0.6f);
			}
		});

		HUD->add(new UI::Table{})
			.setAlign(Align::Mode::top_left)
			.setSizeScale(0.4f, 0.08f)
			.setSrcScale(0.25f, 0.0f)
			.setMargin(10, 0, 0, 0);
		HUD->add(new UI::Table{})
			.setAlign(Align::Mode::top_left)
			.setSizeScale(0.1f, 0.6f)
			.setSrcScale(0.0f, 0.2f)
			.setMargin(0, 0, 10, 10);

		{
			auto& cell2 = HUD->add(new UI::Table{});
			cell2.item->color = Colors::GREEN;
			cell2.setAlign(Align::Mode::bottom_left).setSizeScale(0.25f, 0.2f).setMargin(0, 10, 0, 10);
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
			.setMargin(10, 0, 0, 10);
	}

	{
		auto pane = new UI::ScrollPane{};

		auto rt = new UI::Table{};
		rt->setSize(400, 900);
		rt->setFillparentX();
		pane->setItem(rt);

		HUD->add(pane).setAlign(Align::Mode::top_right).setSizeScale(0.225f, 0.25f).setMargin(10, 0, 0, 10);

		auto t = new UI::Table{};
		t->color = Colors::RED;
		t->name = "testT";
		rt->add(t);
		// rt->add(new UI::Elem);

		rt->lineFeed();
		rt->add(new UI::Elem{});
		rt->add(new UI::Elem{});
	}

	{
		HUD->add(new UI::Table{})
		   .setAlign(Align::top_right)
		   .setSizeScale(0.185f, 0.575f).setSrcScale(0.0f, 0.25f)
		   .setMargin(10, 0, 10, 0);
		//
		HUD->add(new UI::Table{})
			.setAlign(Align::top_right)
			.setSizeScale(0.225f - 0.185f, 0.575f).setSrcScale(0.185f, 0.25f)
			.setMargin(10, 10, 10, 0);
		//
		HUD->add(new UI::Table{})
			.setAlign(Align::Mode::bottom_right)
			.setSizeScale(0.3f, 0.15f)
			.setMargin(10, 0, 10, 0);
	}

	/*{

		auto t = new UI::Table{};

		t->setTouchbility(UI::TouchbilityFlags::childrenOnly);

		t->defaultCellLayout.expandY();

		auto& cellTable = t->add(new UI::Table{});

		auto& inner = cellTable.as<UI::Table>().add(new UI::Elem{}).fillParentX().wrapY().setAlign(Align::Mode::top_left);
		inner.marginLeft = inner.marginRight = inner.marginBottom = inner.marginTop = 10;

		inner.item->name = "test";

		t->endRow();

		t->add(new UI::Elem{});
		t->add(new UI::Elem{});

		t->setFillparentX(true);

		t->iterateAll([](UI::Elem* elem) {
			elem->setSize(400, 500);
		});

		inner.item->setHeight(200.0f);

		auto& cellp = Core::uiRoot->root->add(t).setAlign(Align::Mode::bottom_right).setSizeScale(0.225f, 0.33f).clearRelativeMove();
		cellp.marginLeft = cellp.marginRight = cellp.marginBottom = cellp.marginTop = 50;
	}*/
}

void init(const int argc, char* argv[]) {
	//TODO move these into application loader
	//Register Cmd
	OS::args.reserve(argc);
	for(int i = 0; i < argc; ++i)OS::args.emplace_back(argv[0]);

	stbi::setFlipVertically_load(true);
	stbi::setFlipVertically_write(true);

	Core::initCore();

	OS::loadListeners(Core::mainWindow);
	//
	Assets::loadBasic();
	//
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
		Core::uiRoot = new UI::Root{};
		Core::renderer = new Core::Renderer{static_cast<unsigned>(w), static_cast<unsigned>(h)};

		Core::uiRoot->resize(w, h);
		Core::camera->resize(w, h);

		Core::renderer->registerSynchronizedResizableObject(Core::camera);
		Core::renderer->registerSynchronizedResizableObject(Core::uiRoot);

		Ctrl::registerCommands(Core::input);
		OS::registerListener(Core::uiRoot);
	});

	Graphic::Draw::defTexture(&Assets::Textures::whiteRegion);
	Graphic::Draw::texture();
	Graphic::Draw::rawMesh = Assets::Meshes::raw;
	Graphic::Draw::blitter = Assets::Shaders::blit;
}

int main(const int argc, char* argv[]) {
	//Init
	::init(argc, argv);
	Core::assetsManager->getEventTrigger().on<Assets::AssetsLoadPull>([](const auto& event) {
		Assets::TexturePackPage* uiPage = event.manager->getAtlas().registerPage("ui", Assets::texCacheDir);
		uiPage->forcePack = true;
		Assets::textureDir.subFile("ui").forAllSubs([&uiPage](OS::File&& file) {
			uiPage->pushRequest(file);
		});
	});

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

		for (auto& texture : event.manager->getAtlas().getPage("ui").textures) {
			texture->setScale(GL_LINEAR, GL_LINEAR);
		}

		Graphic::Draw::defTexture(&Assets::Textures::whiteRegion);
		Graphic::Draw::texture();

		UI::Styles::load(event.manager->getAtlas());
	});

	//Majority Load
	Core::loadAssets();

	// UI Test
	setupUITest();

	Core::input->registerKeyBind(Ctrl::KEY_K, Ctrl::Act_Repeat, [] {
		Core::audio->play(Assets::Sounds::laser5);
	});

	//Draw Test
	// auto tex = Core::assetsManager->getAtlas().find("test-pester-full");
	//
	// const GL::Texture2D bottomLeftTex{ Assets::textureDir.subFile("ui").find("bottom-left.png") };
	// const GL::Texture2D texture{ Assets::textureDir.find("yyz.png") };
	//
	GL::MultiSampleFrameBuffer multiSample{ Core::renderer->getWidth(), Core::renderer->getHeight() };
	GL::FrameBuffer frameBuffer{ Core::renderer->getWidth(), Core::renderer->getHeight() };

	Core::renderer->registerSynchronizedResizableObject(&multiSample);
	Core::renderer->registerSynchronizedResizableObject(&frameBuffer);

	std::stringstream ss{};
	Geom::Matrix3D mat{};
	const auto coordCenter = Font::obtainLayoutPtr();

	Core::renderer->getListener().on<Event::Draw_Post>([&]([[maybe_unused]] const Event::Draw_Post& e){
		Draw::meshBegin(Assets::Meshes::coords);
		Draw::meshEnd(true);
		//
		e.renderer->frameBegin(&frameBuffer);
		e.renderer->frameBegin(&multiSample);
		//
		const auto cameraPos = Core::camera->screenCenter();
		//
		Draw::meshBegin(Core::batch->getMesh());

		//
		Draw::color();

		// Draw::rect(tex, 200, 500, -45);
		//
		// layout->render();


		mat.setOrthogonal(0.0f, 0.0f, static_cast<float>(Core::renderer->getWidth()), static_cast<float>(Core::renderer->getHeight()));

		Core::batch->beginProjection(mat);

		ss.str("");
		ss << "${font#tele}${scl#[0.52]}(" << std::fixed << std::setprecision(2) << cameraPos.getX() << ", " << cameraPos.getY() << ") | " << std::to_string(OS::getFPS());

		currentCoordText = ss.str();
		*currentCoord = currentCoordText;
		Font::glyphParser->parse(coordCenter, *currentCoord);

		coordCenter->offset.set(Core::renderer->getCenterX(), Core::renderer->getCenterY()).add(155, 35);
		coordCenter->setAlign(Align::Mode::bottom_left);
		coordCenter->render();

		Draw::setLineStroke(4);
		Draw::lineSquare(Core::renderer->getCenterX(), Core::renderer->getCenterY(), 50, 45);

		Core::batch->endProjection();




		Draw::setLineStroke(3);
		Draw::color(Colors::BLUE, Colors::SKY, 0.745f);

		Draw::setLineStroke(5);
		Draw::poly(cameraPos.getX(), cameraPos.getY(), 64, 160, 0, Math::clamp(fmod(OS::globalTime() / 5.0f, 1.0f)),
			{ &Colors::SKY, &Colors::ROYAL, &Colors::SKY, &Colors::WHITE, &Colors::ROYAL, &Colors::SKY }
		);

		Draw::flush();
		Draw::meshEnd(Core::batch->getMesh(), false);
		e.renderer->frameEnd(Assets::PostProcessors::blendMulti);
		e.renderer->frameEnd(Assets::PostProcessors::bloom);
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
