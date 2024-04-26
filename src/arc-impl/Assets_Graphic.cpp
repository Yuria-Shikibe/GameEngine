module Assets.Graphic;

import Core;
import OS;
import std;

void Assets::Shaders::loadPrevious() {
	blit = new Shader(shaderDir, "blit");
	blit->setUniformer([](const Shader& shader) {
		shader.setTexture2D("texture0", 0);
		shader.setTexture2D("texture1", 1);
		shader.setTexture2D("texture2", 2);
	});

	sildeLines = new Shader(shaderDir, {{ShaderType::frag, "slide-line"}, {ShaderType::vert, "screenspace"}});
	sildeLines->setUniformer([](const Shader& shader) {
		shader.setTexture2D("u_texture", 0);
		shader.setFloat("time", OS::globalTick() * slideLineShaderScaleArgs.get<2>());
		shader.setFloat("width", slideLineShaderDrawArgs.get<0>());
		shader.setFloat("spacing", slideLineShaderDrawArgs.get<1>());
		shader.setColor("mulColor", slideLineShaderDrawArgs.get<2, true>());
		shader.setFloat("mulSub", slideLineShaderDrawArgs.get<3>());
		shader.setVec2("scale", slideLineShaderScaleArgs.get<0>());
		shader.setVec2("offset", slideLineShaderScaleArgs.get<1>());
	});

	threshold_light = new Shader(shaderDir, {{ShaderType::frag, "threshold"}, {ShaderType::vert, "blit"}});
	threshold_light->setUniformer([](const Shader& shader) {
		shader.setTexture2D("u_texture", 0);
	});

	gaussian = new Shader(shaderDir, "gaussian-blur");

	//TODO dynamic apply
	gaussian->setUniformer([](const Shader& shader) {
		shader.setTexture2D("u_texture");

		if(Core::batchGroup.batchOverlay->getProjection() == &Core::camera->getWorldToScreen()){
			shader.setVec2("size", Core::renderer->getSize().div(Core::batchGroup.batchOverlay->getProjection()->getOrthoScale()));
		}else{
			shader.setVec2("size", Core::renderer->getSize());
		}

	});

	bloom = new Shader(shaderDir, {{ShaderType::frag, "bloom"}, {ShaderType::vert, "blit"}});
	bloom->setUniformer([](const Shader& shader) {
		shader.setTexture2D("texture0", 0);
		shader.setTexture2D("texture1", 1);
	});

	screenSpace = new Shader(shaderDir, "screenspace");

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
	texPost = manager->registerShader(shaderDir, "tex-std");
	texPost->setUniformer([]([[maybe_unused]] const Shader& shader) {});

	stdPost = manager->registerShader(shaderDir, "std");
	stdPost->setUniformer([]([[maybe_unused]] const Shader& shader) {
		// GL::uniformColor(0, Graphic::Colors::WHITE);
	});

	coordAxis = manager->registerShader(shaderDir, "coordinate-axis");
	coordAxis->setUniformer([]([[maybe_unused]] const Shader& shader) {
		shader.setFloat("width", 3.0f);
		shader.setFloat("spacing", 100);
		shader.setFloat("scale",  Core::camera->getScale());
		shader.setVec2("screenSize", Core::renderer->getDrawWidth(), Core::renderer->getDrawHeight());
		shader.setVec2("cameraPos", Core::camera->screenCenter());
	});

	filter = manager->registerShader(shaderDir, "filter");
	filter->setUniformer([](const Shader& shader) {
		shader.setTexture2D("tex");
	});

	world = manager->registerShader(shaderDir, "screenspace-world");

	merge = manager->registerShader(shaderDir.subFile("world"), "merge");
	merge->setUniformer([](const Shader& shader) {
		shader.setTexture2D("texBase", 0);
		shader.setTexture2D("texNormal", 1);
		shader.setTexture2D("texLight", 2);
		shader.setTexture2D("bloom", 3);
		// shader.setVec2("screenSizeInv", 1.0f / Core::renderer->getDrawWidth(), 1.0f / Core::renderer->getDrawHeight());
	});

	worldBloom = manager->registerShader(new Shader{shaderDir.subFile("world"), {{ShaderType::frag, "bloom-world"}, {ShaderType::vert, "blit"}}});
	mask = manager->registerShader(new Shader{shaderDir.subFile("post-process"), {{ShaderType::frag, "mask"}, {ShaderType::vert, "blit"}}});
	mask->setUniformer([](const Shader& shader) {
		shader.setTexture2D("texture0", 0);
		shader.setTexture2D("texture1", 1);
		shader.setTexture2D("texture2", 2);
		shader.setFloat("clamp", 1.0f);
	});

	outline_ortho = manager->registerShader(new Shader{shaderDir, {{ShaderType::frag, "outline-ortho"}, {ShaderType::vert, "blit"}}});
	outline_ortho->setUniformer([](const Shader& shader) {
		shader.setTexture2D("texture", 0);
		shader.setVec2("scaleInv", outline_orthoArgs.get<1>());
		shader.setFloat("stepLength", outline_orthoArgs.get<0>());
	});

	manager->registerShader(screenSpace);
	manager->registerShader(threshold_light);
	manager->registerShader(gaussian);
	manager->registerShader(bloom);
	manager->registerShader(blit);
	manager->registerShader(sildeLines);
}

void Assets::PostProcessors::load(){
	multiToBasic.reset(new Graphic::MultiSampleBliter{});

	blurX.reset(new Graphic::ShaderProcessor{Assets::Shaders::gaussian, [](const Shader& shader) {
		shader.setVec2("direction", Geom::Vec2{1.15f, 0});
	}});

	blurY.reset(new Graphic::ShaderProcessor{Assets::Shaders::gaussian, [](const Shader& shader) {
		shader.setVec2("direction", Geom::Vec2{0, 1.15f});
	}});

	blurX_Far.reset(new Graphic::ShaderProcessor{Assets::Shaders::gaussian, [](const Shader& shader) {
		shader.setVec2("direction", Geom::Vec2{2.0f, 0});
	}});

	blurY_Far.reset(new Graphic::ShaderProcessor{Assets::Shaders::gaussian, [](const Shader& shader) {
		shader.setVec2("direction", Geom::Vec2{0, 2.f});
	}});

	bloom.reset(
		new Graphic::BloomProcessor{blurX.get(), blurY.get(), Shaders::bloom, Shaders::threshold_light}
	);

	blur_Far.reset(
		new Graphic::PingPongProcessor{blurX.get(), blurY.get(), 4}
	);

	// blur_Far->setScale(0.75f);

	blend.reset(new Graphic::ShaderProcessor{Shaders::blit});
	// Graphic::P4Processor processor{&blurX, &blurY};
	blendMulti.reset(new Graphic::PipeProcessor{multiToBasic.get(), blend.get()});
}

void Assets::loadBasic() {
	OS::FileTree& mainTree = *Core::rootFileTree;

	assetsDir  = mainTree.findDir("assets");
	shaderDir  = mainTree.findDir("shader");
	textureDir = mainTree.findDir("texture");
	fontDir    = mainTree.findDir("fonts");
	soundDir   = mainTree.findDir("sounds");

	texCacheDir = assetsDir.subFile("tex-cache");
	texCacheDir.createDirQuiet();

	screenshotDir = mainTree.findDir("screenshots"); //TODO move this to other places, it doesn't belong to assets!

	textureTree.reDirect(textureDir);
	textureTree.buildFileTree(true);
	textureTree.mapSubFiles([](const OS::File& file){
		return file.stem();
	});

	Fonts::unicodeRefDir = assetsDir.subFile("unicode-ref");
	if(!Fonts::unicodeRefDir.exist()){
		Fonts::unicodeRefDir.createDirQuiet();
	}

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
	Shaders::loadPrevious();
	Meshes::loadPrevious();
	Textures::load();
	PostProcessors::load();
}
