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
import Graphic.PostProcessor;

import File;
import Core;
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

		void load() {
			texPost = new Shader{ shaderDir, "tex-std" };
			texPost->setUniformer([]([[maybe_unused]] const Shader& shader) {});

			stdPost = new Shader{ shaderDir, "std" };
			stdPost->setUniformer([]([[maybe_unused]] const Shader& shader) {
				// GL::uniformColor(0, Graphic::Colors::WHITE);
			});

			screenSpace = new Shader(shaderDir, "screenspace");

			coordAxis = new Shader(shaderDir, "coordinate-axis");
			coordAxis->setUniformer([]([[maybe_unused]] const Shader& shader) {
				shader.setFloat("width", 3.0f);
				shader.setFloat("spacing", 100);
				shader.setFloat("scale", Core::camera->scale);
				shader.setVec2("screenSize", Core::renderer->getWidth(), Core::renderer->getHeight());
				shader.setVec2("cameraPos", Core::camera->position);
			});

			filter = new Shader(shaderDir, "filter");
			filter->setUniformer([](const Shader& shader) {
				shader.setTexture2D("tex");
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

			blit = new Shader(shaderDir, "blit");
			blit->setUniformer([](const Shader& shader) {
				shader.setTexture2D("texture", 0);
			});
		}

		void dispose() {
			delete texPost;
			delete stdPost;
			delete screenSpace;
			delete coordAxis;
			delete filter;
			delete gaussian;
			delete bloom;
			delete blit;
		}
	}

	namespace PostProcessors {
		Graphic::PostProcessor
			*multiToBasic{nullptr}

		;

		void load() {
			multiToBasic = new Graphic::MultiSampleBliter{};
		}

		void dispose() {
			delete multiToBasic;
		}
	}

	namespace Textures {
		inline Texture2D* whiteTex = nullptr;
		inline TextureRegion* whiteRegion = nullptr;

		void load() {
			whiteTex = new Texture2D{textureDir.find("white.png")};
			whiteRegion = new TextureRegionRect{whiteTex};
		}

		void dispose() {
			delete whiteTex;
			delete whiteRegion;
		}
	}

	namespace Fonts {
		OS::File cacheDir;
		// Font::FontsManager

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

		const Font::FontsManager* manager{nullptr};


		void load() {
			constexpr int DefFlag = 1L << 2;

			cacheDir = fontDir.subFile("cache");
			if(!cacheDir.exist())cacheDir.createDirQuiet();
			// Font::FT::loadLib();
;
			const std::vector<CharCode> targetChars {' ' + 1, '~'};

			Font::rootCacheDir = cacheDir;
			consola_Regular =
				Font::registerFont(new Font::FontFlags{fontDir.subFile("consola.ttf" ), cacheDir,  targetChars});
			consola_Italic =
			 	Font::registerFont(new Font::FontFlags{fontDir.subFile("consolai.ttf"), cacheDir,  targetChars});
			consola_Bold =
			 	Font::registerFont(new Font::FontFlags{fontDir.subFile("consolab.ttf"), cacheDir,  targetChars});
			consola_Bold_Italic =
			 	Font::registerFont(new Font::FontFlags{fontDir.subFile("consolaz.ttf"), cacheDir,  targetChars});

			times_Regular =
				Font::registerFont(new Font::FontFlags{fontDir.subFile("times.ttf" ), cacheDir,  targetChars});
			times_Italic =
				Font::registerFont(new Font::FontFlags{fontDir.subFile("timesi.ttf"), cacheDir,  targetChars});
			times_Bold =
				Font::registerFont(new Font::FontFlags{fontDir.subFile("timesbd.ttf"), cacheDir,  targetChars});
			times_Bold_Italic =
				Font::registerFont(new Font::FontFlags{fontDir.subFile("timesbi.ttf"), cacheDir,  targetChars});

			josefinSans_Regular =
			 	Font::registerFont(new Font::FontFlags{fontDir.subFile("josefinSans-ES-Regular.ttf"), cacheDir,  targetChars});
			josefinSans_Bold =
			 	Font::registerFont(new Font::FontFlags{fontDir.subFile("josefinSans-ES-Bold.ttf"), cacheDir,  targetChars});

			josefinSans_Regular_Large =
			 	Font::registerFont(new Font::FontFlags{fontDir.subFile("josefinSans-ES-Regular.ttf"), cacheDir,  targetChars, DefFlag, 90});
			josefinSans_Bold_Large =
			 	Font::registerFont(new Font::FontFlags{fontDir.subFile("josefinSans-ES-Bold.ttf"), cacheDir,  targetChars, DefFlag, 90});

			telegrama =
			 	Font::registerFont(new Font::FontFlags{fontDir.subFile("telegrama.otf"), cacheDir,  targetChars});

			Font::load();

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

			manager = Font::manager.get();

			Font::loadParser(consola_Regular);
		}

		void dispose() {

		}
	}

	namespace Meshes {
		GL::Mesh
			*raw{nullptr},
			*coords{nullptr}
		;

		void load() {
			raw = new Mesh{[](const Mesh& mesh) {
				mesh.getIndexBuffer().bind();
				mesh.getIndexBuffer().setDataRaw(GL::IndexBuffer::ELEMENTS_STD.data(), GL::IndexBuffer::ELEMENTS_QUAD_LENGTH, GL_STATIC_DRAW);
				mesh.getVertexBuffer().bind();
				mesh.getVertexBuffer().setData({
					-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f
				}, GL_STATIC_DRAW);

				AttributeLayout& layout = mesh.getVertexArray().getLayout();
				layout.addFloat(2);
				layout.addFloat(2);
				mesh.getVertexArray().active();
			}};

			coords = new GL::RenderableMesh(Assets::Shaders::coordAxis, [](const GL::RenderableMesh& mesh) {
				mesh.getIndexBuffer().bind();
				mesh.getIndexBuffer().setDataRaw(GL::IndexBuffer::ELEMENTS_STD.data(), GL::IndexBuffer::ELEMENTS_QUAD_LENGTH, GL_STATIC_DRAW);
				mesh.getVertexBuffer().bind();
				mesh.getVertexBuffer().setData({
					-1.0f, -1.0f,
					-1.0f,  1.0f,
					 1.0f,  1.0f,
					 1.0f, -1.0f
				}, GL_STATIC_DRAW);

				AttributeLayout& layout = mesh.getVertexArray().getLayout();
				layout.addFloat(2);
				mesh.getVertexArray().active();
			});
		}

		void dispose() {
			delete raw;
			delete coords;
		}
	}

	inline void load() {
		OS::FileTree& mainTree = *Core::rootFileTree;

		assetsDir = mainTree.findDir("assets");
		shaderDir = mainTree.find("shader");
		textureDir = mainTree.find("texture");
		fontDir = mainTree.find("fonts");

		screenshotDir = mainTree.find("screenshots"); //TODO move this to other places, it doesn't belong to assets!

		Shaders::load();
		Textures::load();
		Fonts::load();
		Meshes::load();
		PostProcessors::load();
	}

	inline void dispose() {
		Shaders::dispose();
		Textures::dispose();
		Fonts::dispose();
		Meshes::dispose();
		PostProcessors::dispose();
	}
}

