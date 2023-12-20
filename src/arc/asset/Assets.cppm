module;

export module Assets;

import GL.Mesh.RenderableMesh;
import GL.Buffer.IndexBuffer;
import GL.Buffer.VertexBuffer;
import GL.VertexArray;
import Font;
import GlyphArrangement;
import OS.FileTree;
export import GL.Shader;
export import GL.Uniform;
export import GL.Texture.Texture2D;
export import GL.Texture.TextureRegion;
import GL.Texture.TextureRegionRect;

import Geom.Vector2D;

import Graphic.Color;
import Graphic.PostProcessor.MultiSampleBliter;
import Graphic.PostProcessor.ShaderProcessor;
import Graphic.PostProcessor.BloomProcessor;
import Graphic.PostProcessor.P4Processor;
import Graphic.PostProcessor.PipeProcessor;
import Graphic.PostProcessor;

import GL.Shader.Manager;

import File;
import Core;
import OS;
import <future>;
import <iostream>;
import <memory>;
import <span>;
import <unordered_map>;

using namespace GL;

/**
 * \brief Inbuilt Resources
 *
 */
export namespace Assets{
//	namespace Shaders{
//		inline std::unique_ptr<Shader> texPost;
//	}
	typedef unsigned long CharCode;

	inline OS::File assetsDir;
	inline OS::File shaderDir;
	inline OS::File textureDir;
	inline OS::File texCacheDir;
	inline OS::File fontDir;
	inline OS::File screenshotDir;

	namespace Shaders {
		inline Shader* stdPost = nullptr;
		inline Shader* texPost = nullptr;
		inline Shader* screenSpace = nullptr;
		inline Shader* coordAxis = nullptr;
		inline Shader* filter = nullptr;

		inline Shader* threshold_light = nullptr;
		inline Shader* gaussian = nullptr;
		inline Shader* bloom = nullptr;
		inline Shader* blit = nullptr;
		
		inline Shader* sildeLines = nullptr;

		void loadPrevious() {
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

		void load(GL::ShaderManager* manager) { // NOLINT(*-non-const-parameter)
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
	}

	namespace PostProcessors {
		Graphic::PostProcessor
			*multiToBasic{nullptr},
			*blendMulti{nullptr}
		;

		Graphic::ShaderProcessor
			*blurX{nullptr},
			*blurY{nullptr},
			*blend{nullptr};

		Graphic::BloomProcessor* bloom{nullptr};

		void load() {
			multiToBasic = new Graphic::MultiSampleBliter{};

			blurX = new Graphic::ShaderProcessor{Assets::Shaders::gaussian, [](const Shader& shader) {
				shader.setVec2("direction", Geom::Vector2D{1.12f, 0});
			}};

			blurY = new Graphic::ShaderProcessor{Assets::Shaders::gaussian, [](const Shader& shader) {
				shader.setVec2("direction", Geom::Vector2D{0, 1.12f});
			}};

			bloom = new Graphic::BloomProcessor{blurX, blurY, Shaders::bloom, Shaders::threshold_light};

			blend = new Graphic::ShaderProcessor{Shaders::blit};
			// Graphic::P4Processor processor{&blurX, &blurY};
			blendMulti = new Graphic::PipeProcessor{Assets::PostProcessors::multiToBasic, blend};
		}

		void dispose() {
			delete multiToBasic;

			delete blurX;
			delete blurY;
			delete blend;
			delete bloom;
			delete blendMulti;
		}
	}

	namespace Textures {
		inline const Texture2D* whiteTex = nullptr;
		inline TextureRegionRect whiteRegion{};

		void load() {
			whiteTex = new Texture2D{textureDir.find("white.png")};
			whiteRegion = TextureRegionRect{whiteTex};
		}

		void dispose() {
			delete whiteTex;
		}
	}

	namespace Fonts {
		constexpr int DefFlag = 1L << 2;

		OS::File cacheDir;
		// Font::FontsManager

		const std::vector<CharCode> targetChars {' ' + 1, '~'};

		const Font::FontFlags
			*consola_Regular{nullptr},
			*consola_Italic{nullptr},
			*consola_Bold{nullptr},
			*consola_Bold_Italic{nullptr},

			*times_Regular{nullptr},
			*times_Italic{nullptr},
			*times_Bold{nullptr},
			*times_Bold_Italic{nullptr},

			*josefinSans_Regular{nullptr},
			*josefinSans_Bold{nullptr},

			*josefinSans_Regular_Large{nullptr},
			*josefinSans_Bold_Large{nullptr},

			*telegrama{nullptr}
		;

		void loadPreivous(Font::FontLoader* loader) { // NOLINT(*-non-const-parameter)
			loader->quickInit = true;
			cacheDir = fontDir.subFile("cache-load");
			if(!cacheDir.exist())cacheDir.createDirQuiet();
			
			loader->rootCacheDir = cacheDir;
			
			telegrama =
				 loader->registerFont(new Font::FontFlags{fontDir.subFile("telegrama.otf"), cacheDir,  targetChars, DefFlag, 120});

			loader->load();

			Font::initParser(telegrama);
		}

		void load(Font::FontLoader* loader) { // NOLINT(*-non-const-parameter

			cacheDir = fontDir.subFile("cache");
			if(!cacheDir.exist())cacheDir.createDirQuiet();

			loader->rootCacheDir = cacheDir;
			consola_Regular =
				loader->registerFont(new Font::FontFlags{fontDir.subFile("consola.ttf" ), cacheDir,  targetChars});
			consola_Italic =
			 	loader->registerFont(new Font::FontFlags{fontDir.subFile("consolai.ttf"), cacheDir,  targetChars});
			consola_Bold =
			 	loader->registerFont(new Font::FontFlags{fontDir.subFile("consolab.ttf"), cacheDir,  targetChars});
			consola_Bold_Italic =
			 	loader->registerFont(new Font::FontFlags{fontDir.subFile("consolaz.ttf"), cacheDir,  targetChars});

			times_Regular =
				loader->registerFont(new Font::FontFlags{fontDir.subFile("times.ttf" ), cacheDir,  targetChars});
			times_Italic =
				loader->registerFont(new Font::FontFlags{fontDir.subFile("timesi.ttf"), cacheDir,  targetChars});
			times_Bold =
				loader->registerFont(new Font::FontFlags{fontDir.subFile("timesbd.ttf"), cacheDir,  targetChars});
			times_Bold_Italic =
				loader->registerFont(new Font::FontFlags{fontDir.subFile("timesbi.ttf"), cacheDir,  targetChars});

			josefinSans_Regular =
			 	loader->registerFont(new Font::FontFlags{fontDir.subFile("josefinSans-ES-Regular.ttf"), cacheDir,  targetChars});
			josefinSans_Bold =
			 	loader->registerFont(new Font::FontFlags{fontDir.subFile("josefinSans-ES-Bold.ttf"), cacheDir,  targetChars});

			josefinSans_Regular_Large =
			 	loader->registerFont(new Font::FontFlags{fontDir.subFile("josefinSans-ES-Regular.ttf"), cacheDir,  targetChars, DefFlag, 90});
			josefinSans_Bold_Large =
			 	loader->registerFont(new Font::FontFlags{fontDir.subFile("josefinSans-ES-Bold.ttf"), cacheDir,  targetChars, DefFlag, 90});

			telegrama =
			 	loader->registerFont(new Font::FontFlags{fontDir.subFile("telegrama.otf"), cacheDir,  targetChars});


			Font::registerParserableFont("tms-R" , times_Regular);
			Font::registerParserableFont("tms-B" , times_Bold);
			Font::registerParserableFont("tms-I" , times_Italic);
			Font::registerParserableFont("tms-BI", times_Bold_Italic);

			Font::registerParserableFont("csl-R" , consola_Regular);
			Font::registerParserableFont("csl-B" , consola_Bold);
			Font::registerParserableFont("csl-I" , consola_Italic);
			Font::registerParserableFont("csl-BI", consola_Bold_Italic);

			Font::registerParserableFont("jfs-B", josefinSans_Bold);
			Font::registerParserableFont("jfs-R", josefinSans_Regular);

			Font::registerParserableFont("jfsL-B", josefinSans_Bold_Large);
			Font::registerParserableFont("jfsL-R", josefinSans_Regular_Large);

			Font::registerParserableFont("tele", telegrama);
		}
	}

	namespace Meshes {
		GL::Mesh
			*raw{nullptr},
			*coords{nullptr}
		;

		void loadPrevious() {
			raw = new Mesh{[](const Mesh& mesh) {
				mesh.getVertexBuffer().bind();
				mesh.getVertexBuffer().setData({
					-1.0f, -1.0f, 0.0f, 0.0f,
					 1.0f, -1.0f, 1.0f, 0.0f,
					 1.0f,  1.0f, 1.0f, 1.0f,
					-1.0f,  1.0f, 0.0f, 1.0f
				}, GL_STATIC_DRAW);

				mesh.getIndexBuffer().bind();
				mesh.getIndexBuffer().setDataRaw(GL::IndexBuffer::ELEMENTS_STRIP_STD.data(), GL::IndexBuffer::ELEMENTS_QUAD_STRIP_LENGTH, GL_STATIC_DRAW);

				AttributeLayout& layout = mesh.getVertexArray().getLayout();
				layout.addFloat(2);
				layout.addFloat(2);
				mesh.getVertexArray().active();

				mesh.unbind();
			}};
		}

		void load() {
			coords = new GL::RenderableMesh(Assets::Shaders::coordAxis, [](const GL::RenderableMesh& mesh) {
				mesh.getVertexBuffer().bind();
				mesh.getVertexBuffer().setData({
					-1.0f, -1.0f,
					 1.0f, -1.0f,
					 1.0f,  1.0f,
					-1.0f,  1.0f
				}, GL_STATIC_DRAW);
				mesh.getIndexBuffer().bind();
				mesh.getIndexBuffer().setDataRaw(GL::IndexBuffer::ELEMENTS_STRIP_STD.data(), GL::IndexBuffer::ELEMENTS_QUAD_STRIP_LENGTH, GL_STATIC_DRAW);

				AttributeLayout& layout = mesh.getVertexArray().getLayout();
				layout.addFloat(2);
				mesh.getVertexArray().active();

				mesh.unbind();
			});
		}

		void dispose() {
			delete raw;
			delete coords;
		}
	}

	inline void loadBasic() {
		OS::FileTree& mainTree = *Core::rootFileTree;

		assetsDir = mainTree.findDir("assets");
		shaderDir = mainTree.find("shader");
		textureDir = mainTree.find("texture");
		fontDir = mainTree.find("fonts");

		texCacheDir = assetsDir.subFile("tex-cache");
		texCacheDir.createDirQuiet();

		screenshotDir = mainTree.find("screenshots"); //TODO move this to other places, it doesn't belong to assets!

		Font::loadLib();

		//TODO uses this if showing text during load is needed.
		//Fonts::loadPreivous();

		Shaders::loadPrevious();
		Meshes::loadPrevious();
		Textures::load();
		PostProcessors::load();
	}

	inline void loadAfter() {
		Meshes::load();
	}

	inline void dispose() {
		Textures::dispose();
		Meshes::dispose();
		PostProcessors::dispose();
	}
}

