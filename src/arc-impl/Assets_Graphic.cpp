module;

module Assets.Graphic;

import Core;
void Assets::Shaders::loadPrevious() {
	blit = new Shader(shaderDir, "blit");
	blit->setUniformer([](const Shader& shader) {
		shader.setTexture2D("texture", 0);
	});

	sildeLines = new Shader(shaderDir, {{ShaderType::frag, "slide-line"}, {ShaderType::vert, "screenspace"}});
	sildeLines->setUniformer([](const Shader& shader) {
		shader.setTexture2D("u_texture", 0);
		shader.setFloat("time", OS::globalTick());
	});

	threshold_light = new Shader(shaderDir, {{ShaderType::frag, "threshold"}, {ShaderType::vert, "blit"}});
	threshold_light->setUniformer([](const Shader& shader) {
		shader.setTexture2D("u_texture");
	});

	gaussian = new Shader(shaderDir, "gaussian-blur");
	gaussian->setUniformer([](const Shader& shader) {
		shader.setTexture2D("u_texture");
		shader.setVec2("size", Core::renderer->getWidth(), Core::renderer->getHeight());
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
	texPost = manager->registerShader(new Shader{ shaderDir, "tex-std" });
	texPost->setUniformer([]([[maybe_unused]] const Shader& shader) {});

	stdPost = manager->registerShader(new Shader{ shaderDir, "std" });
	stdPost->setUniformer([]([[maybe_unused]] const Shader& shader) {
		// GL::uniformColor(0, Graphic::Colors::WHITE);
	});

	coordAxis = manager->registerShader(new Shader(shaderDir, "coordinate-axis"));
	coordAxis->setUniformer([]([[maybe_unused]] const Shader& shader) {
		shader.setFloat("width", 3.0f);
		shader.setFloat("spacing", 100);
		shader.setFloat("scale", Core::camera->getScale());
		shader.setVec2("screenSize", Core::renderer->getWidth(), Core::renderer->getHeight());
		shader.setVec2("cameraPos", Core::camera->screenCenter());
	});

	filter = manager->registerShader(new Shader(shaderDir, "filter"));
	filter->setUniformer([](const Shader& shader) {
		shader.setTexture2D("tex");
	});


	manager->registerShader(screenSpace);
	manager->registerShader(threshold_light);
	manager->registerShader(gaussian);
	manager->registerShader(bloom);
	manager->registerShader(blit);
	manager->registerShader(sildeLines);
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
