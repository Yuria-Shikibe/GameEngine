module Assets.Graphic;

import Core;
import OS;
import std;

using namespace GL;
using namespace Graphic;

void Assets::Shaders::loadPrimitive() {
	blit = new Shader(Dir::shader, "blit");
	blit->setUniformer(+[](const Shader& shader) {
		shader.setTexture2D("texture0", 0);
		// shader.setTexture2D("texture1", 1);
		// shader.setTexture2D("texture2", 2);
	});

	sildeLines = new Shader(Dir::shader, {{ShaderType::frag, "slide-line"}, {ShaderType::vert, "screenspace"}});
	sildeLines->setUniformer(+[](const Shader& shader) {
		shader.setTexture2D("texture", 0);
		const float stroke = slideLineShaderDrawArgs.get<0>();
		const float spacing = slideLineShaderDrawArgs.get<1>();

		shader.setFloat("width", slideLineShaderDrawArgs.get<0>());
		shader.setFloat("spacing", spacing >= 0.0f ? spacing : stroke * 2.0f);
		shader.setColor("mulColor", slideLineShaderDrawArgs.get<2, true>());
		shader.setFloat("mulSub", slideLineShaderDrawArgs.get<3>());

		shader.setVec2("scale", ~Core::renderer->getSize());
		shader.setFloat("time", OS::globalTick() * slideLineShaderScaleArgs.get<0>());

		shader.setFloat("angle", slideLineShaderAngle.get<0>() + 45.0f);
	});

	threshold_light = new Shader(Dir::shader, {{ShaderType::frag, "threshold"}, {ShaderType::vert, "blit"}});
	threshold_light->setUniformer(+[](const Shader& shader) {
		shader.setTexture2D("u_texture", 0);
	});

	gaussian = new Shader(Dir::shader, "gaussian-blur");

	//TODO dynamic apply
	gaussian->setUniformer(+[](const Shader& shader) {
		shader.setTexture2D("texture");

		//TODO size adapter
		shader.setVec2("size", ~Core::renderer->getSize());
	});

	bloom = new Shader(Dir::shader, {{ShaderType::frag, "bloom"}, {ShaderType::vert, "blit"}});
	bloom->setUniformer(+[](const Shader& shader) {
		shader.setTexture2D("texture0", 0);
		shader.setTexture2D("texture1", 1);
	});

	screenSpace = new Shader(Dir::shader, "screenspace");

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
	texPost->setUniformer([]([[maybe_unused]] const Shader& shader) {});

	stdPost = manager->registerShader(Dir::shader, "std");
	stdPost->setUniformer([]([[maybe_unused]] const Shader& shader) {
		// GL::uniformColor(0, Graphic::Colors::WHITE);
	});

	gaussian_world = manager->registerShader(Dir::shader, "gaussian-blur");
	gaussian_world->setUniformer(+[](const Shader& shader) {
		shader.setTexture2D("texture");
		shader.setVec2("size", ~Core::renderer->getSize() * Core::camera->getScale());
		// shader.setVec2("size", Core::renderer->getSize());

	});

	coordAxis = manager->registerShader(Dir::shader, "coordinate-axis");
	coordAxis->setUniformer([](const Shader& shader) {
		shader.setFloat("width", 3.0f);
		shader.setFloat("spacing", 100);
		shader.setFloat("scale",  Core::camera->getScale());
		shader.setVec2("screenSize", Core::renderer->getDrawWidth(), Core::renderer->getDrawHeight());
		shader.setVec2("cameraPos", Core::camera->screenCenter());
	});

	filter = manager->registerShader(Dir::shader, "filter");
	filter->setUniformer([](const Shader& shader) {
		shader.setTexture2D("tex");
	});

	world = manager->registerShader(Dir::shader, "screenspace-world");

	merge = manager->registerShader(Dir::shader.subFile("world"), "merge");
	merge->setUniformer([](const Shader& shader) {
		shader.setTexture2D("texBase", 0);
		shader.setTexture2D("texNormal", 1);
		shader.setTexture2D("texLight", 2);
		shader.setTexture2D("bloom", 3);
		// shader.setVec2("screenSizeInv", 1.0f / Core::renderer->getDrawWidth(), 1.0f / Core::renderer->getDrawHeight());
	});

	worldBloom = manager->registerShader(new Shader{Dir::shader.subFile("world"), {{ShaderType::frag, "bloom-world"}, {ShaderType::vert, "blit"}}});
	mask = manager->registerShader(new Shader{Dir::shader.subFile("post-process"), {{ShaderType::frag, "mask"}, {ShaderType::vert, "blit"}}});
	mask->setUniformer([](const Shader& shader) {
		shader.setTexture2D("srcTex", 0);
		shader.setTexture2D("dstTex", 1);
		shader.setTexture2D("maskTex", 2);
	});

	frostedGlass = manager->registerShader(new Shader{Dir::shader.subFile("post-process"), {{ShaderType::frag, "frosted-glass"}, {ShaderType::vert, "blit"}}});
	frostedGlass->setUniformer([](const Shader& shader) {
		shader.setTexture2D("texture", 0);
		shader.setVec2("norStep", ~Core::renderer->getSize() * 1.0f);
	});

	outline_ortho = manager->registerShader(new Shader{Dir::shader, {{ShaderType::frag, "outline-ortho"}, {ShaderType::vert, "blit"}}});
	outline_ortho->setUniformer([](const Shader& shader) {
		shader.setTexture2D("texture", 0);
		shader.setVec2("scaleInv", outlineArgs.get<2>());
		shader.setFloat("stepLength", outlineArgs.get<0>());
		shader.setFloat("rot", outlineArgs.get<1>());
	});

	outline_sobel = manager->registerShader(new Shader{Dir::shader, {{ShaderType::frag, "outline-sobel"}, {ShaderType::vert, "blit"}}});
	outline_sobel->setUniformer([](const Shader& shader) {
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

	blurX.reset(new Graphic::ShaderProcessor{Assets::Shaders::gaussian, [](const Shader& shader) {
		shader.setVec2("direction", Geom::Vec2{1.15f, 0});
	}});
	blurX->setTargetState(GL::State::BLEND, false);


	blurY.reset(new Graphic::ShaderProcessor{Assets::Shaders::gaussian, [](const Shader& shader) {
		shader.setVec2("direction", Geom::Vec2{0, 1.15f});
	}});
	blurY->setTargetState(GL::State::BLEND, false);


	bloom.reset(
		new Graphic::BloomProcessor{blurX.get(), blurY.get(), Shaders::bloom, Shaders::threshold_light}
	);
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

	blurX_World.reset(new Graphic::ShaderProcessor{Assets::Shaders::gaussian_world, [](const Shader& shader) {
		shader.setVec2("direction", Geom::Vec2{1.80f, 0});
	}});
	blurX_World->setTargetState(GL::State::BLEND, false);

	blurY_World.reset(new Graphic::ShaderProcessor{Assets::Shaders::gaussian_world, [](const Shader& shader) {
		shader.setVec2("direction", Geom::Vec2{0, 1.80f});
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
