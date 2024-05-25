module Assets.Graphic;

import Core;
import OS;
import std;
import Geom.Vector3D;
import Math.Rand;

using namespace GL;
using namespace Graphic;

void Assets::Shaders::loadPrimitive() {
	blit = new ShaderSource(Dir::shader, "blit");
	blit->setUniformer(+[](const ShaderProgram& shader) {
		shader.setTexture2D("texture0", 0);
		// shader.setTexture2D("texture1", 1);
		// shader.setTexture2D("texture2", 2);
	});

	sildeLines = new ShaderSource(Dir::shader, {{ShaderType::frag, "slide-line"}, {ShaderType::vert, "screenspace"}});
	sildeLines->setUniformer(+[](const ShaderProgram& shader) {
		shader.setTexture2D("texture", 0);
		const float stroke = slideLineShaderDrawArgs.get<0>();
		const float spacing = slideLineShaderDrawArgs.get<1>();

		shader.setFloat("width", slideLineShaderDrawArgs.get<0>());
		shader.setFloat("spacing", spacing >= 0.0f ? spacing : stroke * 2.0f);
		shader.setColor("mulColor", slideLineShaderDrawArgs.get<2, true>());
		shader.setFloat("mulSub", slideLineShaderDrawArgs.get<3>());

		if(auto vec = slideLineShaderScaleArgs.get<1>(); vec.isNaN()){
			shader.setVec2("scale", ~Core::renderer->getSize());
		}else{
			shader.setVec2("scale", ~vec);
		}
		shader.setFloat("time", OS::globalTick() * slideLineShaderScaleArgs.get<0>());

		shader.setFloat("angle", slideLineShaderAngle.get<0>() + 45.0f);
	});

	threshold_light = new ShaderSource(Dir::shader, {{ShaderType::frag, "threshold"}, {ShaderType::vert, "blit"}});
	threshold_light->setUniformer(+[](const ShaderProgram& shader) {
		shader.setTexture2D("u_texture", 0);
	});

	gaussian = new ShaderSource(Dir::shader, "gaussian-blur");

	//TODO dynamic apply
	gaussian->setUniformer(+[](const ShaderProgram& shader) {
		shader.setTexture2D("texture");

		//TODO size adapter
		shader.setVec2("size", ~Core::renderer->getSize());
	});

	bloom = new ShaderSource(Dir::shader, {{ShaderType::frag, "bloom"}, {ShaderType::vert, "blit"}});
	bloom->setUniformer(+[](const ShaderProgram& shader) {
		shader.setTexture2D("texture0", 0);
		shader.setTexture2D("texture1", 1);
	});

	screenSpace = new ShaderSource(Dir::shader, "screenspace");

	blit->readSource();
	blit->compile();

	sildeLines->readSource();
	sildeLines->compile();

	threshold_light->readSource();
	threshold_light->compile();

	gaussian->readSource();
	gaussian->compile();

	bloom->readSource();
	bloom->compile();

	screenSpace->readSource();
	screenSpace->compile();
}

void Assets::Shaders::load(GL::ShaderManager* manager) { // NOLINT(*-non-const-parameter)
	texPost = manager->registerShader(Dir::shader, "tex-std");
	texPost->setUniformer([]([[maybe_unused]] const ShaderProgram& shader) {});

	stdPost = manager->registerShader(Dir::shader, "std");
	stdPost->setUniformer([]([[maybe_unused]] const ShaderProgram& shader) {
		// GL::uniformColor(0, Graphic::Colors::WHITE);
	});

	gaussian_world = manager->registerShader(Dir::shader, "gaussian-blur");
	gaussian_world->setUniformer(+[](const ShaderProgram& shader) {
		shader.setTexture2D("texture");
		shader.setVec2("size", ~Core::renderer->getSize() * Core::camera->getScale());
		// shader.setVec2("size", Core::renderer->getSize());

	});

	coordAxis = manager->registerShader(Dir::shader, "coordinate-axis");
	coordAxis->setUniformer([](const ShaderProgram& shader) {
		shader.setFloat("width", 3.0f);
		shader.setFloat("spacing", 100);
		auto* camera = coordAxisArgs.get<0>() ? coordAxisArgs.get<0>() : Core::camera;
		shader.setFloat("scale",  camera->getScale());
		shader.setVec2("screenSize", camera->getScreenSize());
		shader.setVec2("cameraPos", camera->getViewportCenter());
	});

	filter = manager->registerShader(Dir::shader, "filter");
	filter->setUniformer([](const ShaderProgram& shader) {
		shader.setTexture2D("tex");
	});

	world = manager->registerShader(Dir::shader, "screenspace-world");

	merge = manager->registerShader(new ShaderSource{Dir::shader.subFile("world"),{{ShaderType::frag, "merge"}, {ShaderType::vert, "blit"}}});
	merge->setUniformer([](const ShaderProgram& shader) {
		shader.setTexture2D("texBase", 0);
		shader.setTexture2D("texNormal", 1);
		shader.setTexture2D("texLight", 2);
		shader.setTexture2D("texData", 3);
		shader.setTexture2D("texBloom", 4);

		constexpr auto roundNear = 12;
		constexpr auto roundMid = 8;
		constexpr auto roundFar = 4;
		constexpr unsigned Size = roundNear + roundMid + roundFar;

		const float cameraScale = Core::camera->getScale();
		const unsigned kernalSize = Math::clamp(static_cast<unsigned>(Size * cameraScale), 8u, Size);
		const auto scale = ~Core::renderer->getSize() * cameraScale;
		shader.setVec2("scale", scale);

		static constexpr std::array<Geom::Vector2D<float>, Size> smp = []() constexpr {
			std::array<Geom::Vector2D<float>, Size> arr{};

			for(int i = 0; i < roundNear; ++i){
				arr[0 + i].setPolar(Math::DEG_FULL / roundNear * static_cast<float>(i), 0.2270270270f);
			}

			for(int i = 0; i < roundMid; ++i){
				arr[roundNear + i].setPolar(Math::DEG_FULL / roundMid * static_cast<float>(i), 0.53062162162f);
			}

			for(int i = 0; i < roundFar; ++i){
				arr[roundMid + roundNear + i].setPolar(Math::DEG_FULL / roundFar * static_cast<float>(i) + Math::DEG_FULL / roundFar / 2.f, 0.83062162162f);
			}

			return arr;
		}();

		shader.setVec2Arr("kernal[0]", smp.data(), Size);
		shader.setUint("kernalSize", Size);


		// shader.setVec2("screenSizeInv", 1.0f / Core::renderer->getDrawWidth(), 1.0f / Core::renderer->getDrawHeight());
	});

	worldBloom = manager->registerShader(new ShaderSource{Dir::shader.subFile("world"), {{ShaderType::frag, "bloom-world"}, {ShaderType::vert, "blit"}}});
	mask = manager->registerShader(new ShaderSource{Dir::shader.subFile("post-process"), {{ShaderType::frag, "mask"}, {ShaderType::vert, "blit"}}});
	mask->setUniformer([](const ShaderProgram& shader) {
		shader.setTexture2D("srcTex", 0);
		shader.setTexture2D("dstTex", 1);
		shader.setTexture2D("maskTex", 2);
	});

	frostedGlass = manager->registerShader(new ShaderSource{Dir::shader.subFile("post-process"), {{ShaderType::frag, "frosted-glass"}, {ShaderType::vert, "blit"}}});
	frostedGlass->setUniformer([](const ShaderProgram& shader) {
		shader.setTexture2D("texture", 0);
		shader.setVec2("norStep", ~Core::renderer->getSize() * 1.0f);
	});

	outline_ortho = manager->registerShader(new ShaderSource{Dir::shader, {{ShaderType::frag, "outline-ortho"}, {ShaderType::vert, "blit"}}});
	outline_ortho->setUniformer([](const ShaderProgram& shader) {
		shader.setTexture2D("texture", 0);
		shader.setVec2("scaleInv", outlineArgs.get<2>());
		shader.setFloat("stepLength", outlineArgs.get<0>());
		shader.setFloat("rot", outlineArgs.get<1>());
	});

	outline_sobel = manager->registerShader(new ShaderSource{Dir::shader, {{ShaderType::frag, "outline-sobel"}, {ShaderType::vert, "blit"}}});
	outline_sobel->setUniformer([](const ShaderProgram& shader) {
		shader.setTexture2D("texture", 0);
		shader.setVec2("scaleInv", outlineArgs.get<2>());
		shader.setFloat("stepLength", outlineArgs.get<0>());
	});

	manager->registerShader(screenSpace);
	manager->registerShader(threshold_light);
	manager->registerShader(gaussian);
	manager->registerShader(bloom);
	manager->registerShader(blit);
	manager->registerShader(sildeLines);
}

void Assets::PostProcessors::loadPrimitive(){
	multiToBasic.reset(new Graphic::MultiSampleBliter{});

	blurX.reset(new Graphic::ShaderProcessor{Assets::Shaders::gaussian, [](const ShaderProgram& shader) {
		shader.setVec2("direction", Geom::Vec2{1.15f, 0});
	}});
	blurX->setTargetState(GL::State::BLEND, false);


	blurY.reset(new Graphic::ShaderProcessor{Assets::Shaders::gaussian, [](const ShaderProgram& shader) {
		shader.setVec2("direction", Geom::Vec2{0, 1.15f});
	}});
	blurY->setTargetState(GL::State::BLEND, false);


	bloom.reset(
		new Graphic::BloomProcessor{blurX.get(), blurY.get(), Shaders::bloom, Shaders::threshold_light}
	);
	bloom->setIntensity(0.95f, 1.15f);

	// bloom->blur.setTargetState(GL::State::BLEND, false);


	// bloom->intensity_blo += 0.35f;
	// bloom->intensity_ori -= 0.15f;

	blur_Far.reset(
		new Graphic::PingPongProcessor{blurX.get(), blurY.get(), 6}
	);

	blur_Far->setScale(0.5f);

	blend.reset(new Graphic::ShaderProcessor{Shaders::blit});
	// Graphic::P4Processor processor{&blurX, &blurY};
	blendMulti.reset(new Graphic::PipeProcessor{multiToBasic.get(), blend.get()});
}

void Assets::PostProcessors::load(){
	frostedGlass.reset(new Graphic::ShaderProcessor{Shaders::frostedGlass});
	frostedGlass->setTargetState(GL::State::BLEND, false);

	frostedGlassBlur.reset(new Graphic::PingPongProcessor{frostedGlass.get(), frostedGlass.get(), 2});
	frostedGlassBlur->setScale(0.5f);
	frostedGlassBlur->setTargetState(GL::State::BLEND, false);

	bloom->blur.ping2pong = bloom->blur.pong2ping = frostedGlass.get();
	bloom->blur.processTimes = 2;
	bloom->scale = 0.5f;
	bloom->blur.setScale(0.5f);

	blurX_World.reset(new Graphic::ShaderProcessor{Assets::Shaders::gaussian_world, [](const ShaderProgram& shader) {
		shader.setVec2("direction", Geom::Vec2{1.45f, 0});
	}});
	blurX_World->setTargetState(GL::State::BLEND, false);

	blurY_World.reset(new Graphic::ShaderProcessor{Assets::Shaders::gaussian_world, [](const ShaderProgram& shader) {
		shader.setVec2("direction", Geom::Vec2{0, 1.45f});
	}});
	blurY_World->setTargetState(GL::State::BLEND, false);

}

void Assets::loadBasic() {
	OS::FileTree& mainTree = Core::rootFileTree;
	
	textureTree.reDirect(Dir::texture);
	textureTree.buildFileTree(true);
	textureTree.mapSubFiles([](const OS::File& file){
		return file.stem();
	});

	Fonts::unicodeRefDir = Dir::assets.subFile("unicode-ref");
	Dir::patch(Fonts::unicodeRefDir);

	// auto vec = textureTree.getFlatView() | std::ranges::views::keys | std::ranges::to<std::vector<std::string>>();
	// std::ranges::sort(vec);
	//
	// for (auto& string : vec){
	// 	std::cout << textureTree.flatFind(string) << std::endl;
	// }

	Font::loadLib();
	//
	// //TODO uses this if showing text during load is needed.
	// Fonts::loadPreivous();
	//
	Shaders::loadPrimitive();
	Meshes::loadPrimitive();
	Textures::load();
	PostProcessors::loadPrimitive();
}
