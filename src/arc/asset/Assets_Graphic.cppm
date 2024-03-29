module;

export module Assets.Graphic;

import GL.Mesh.RenderableMesh;
import GL.Buffer.DataBuffer;
import GL.VertexArray;
import Font;
import Font.GlyphArrangement;
import OS.FileTree;
import GL.Shader;
import GL.Uniform;
import GL.Texture.Texture2D;
import GL.Texture.TextureRegion;
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
import GL.Constants;

import OS.File;

import std;

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
	inline OS::File soundDir;
	inline OS::File screenshotDir;

	inline OS::FileTree textureTree;

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

		inline Shader* world = nullptr;
		inline Shader* merge = nullptr;

		inline Shader* worldBloom = nullptr;

		void loadPrevious();

		void load(GL::ShaderManager* manager);
	}

	namespace PostProcessors {
		std::unique_ptr<Graphic::PostProcessor>
			multiToBasic{nullptr},
			blendMulti{nullptr},
			blurX{nullptr},
			blurY{nullptr},
			blurX_Far{nullptr},
			blurY_Far{nullptr},
			blend{nullptr};

		std::unique_ptr<Graphic::BloomProcessor> bloom{nullptr};

		void load() {
			multiToBasic.reset(new Graphic::MultiSampleBliter{});

			blurX.reset(new Graphic::ShaderProcessor{Assets::Shaders::gaussian, [](const Shader& shader) {
				shader.setVec2("direction", Geom::Vec2{1.2f, 0});
			}});

			blurY.reset(new Graphic::ShaderProcessor{Assets::Shaders::gaussian, [](const Shader& shader) {
				shader.setVec2("direction", Geom::Vec2{0, 1.2f});
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

			blend.reset(new Graphic::ShaderProcessor{Shaders::blit});
			// Graphic::P4Processor processor{&blurX, &blurY};
			blendMulti.reset(new Graphic::PipeProcessor{multiToBasic.get(), blend.get()});
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

		void loadPreivous(Font::FontManager* loader) { // NOLINT(*-non-const-parameter)
			loader->quickInit = true;
			cacheDir = fontDir.subFile("cache-load");
			if(!cacheDir.exist())cacheDir.createDirQuiet();
			
			loader->rootCacheDir = cacheDir;
			
			telegrama =
				 loader->registerFont(new Font::FontFlags{fontDir.subFile("telegrama.otf"),  targetChars, DefFlag, 120});

			loader->load();

			Font::initParser(telegrama);
		}

		void load(Font::FontManager* loader) { // NOLINT(*-non-const-parameter

			cacheDir = fontDir.subFile("cache");
			if(!cacheDir.exist())cacheDir.createDirQuiet();

			loader->rootCacheDir = cacheDir;
			consola_Regular =
				loader->registerFont(new Font::FontFlags{fontDir.subFile("consola.ttf" ),  targetChars});
			consola_Italic =
			 	loader->registerFont(new Font::FontFlags{fontDir.subFile("consolai.ttf"),  targetChars});
			consola_Bold =
			 	loader->registerFont(new Font::FontFlags{fontDir.subFile("consolab.ttf"),  targetChars});
			consola_Bold_Italic =
			 	loader->registerFont(new Font::FontFlags{fontDir.subFile("consolaz.ttf"),  targetChars});

			times_Regular =
				loader->registerFont(new Font::FontFlags{fontDir.subFile("times.ttf" ),  targetChars});
			times_Italic =
				loader->registerFont(new Font::FontFlags{fontDir.subFile("timesi.ttf"),  targetChars});
			times_Bold =
				loader->registerFont(new Font::FontFlags{fontDir.subFile("timesbd.ttf"),  targetChars});
			times_Bold_Italic =
				loader->registerFont(new Font::FontFlags{fontDir.subFile("timesbi.ttf"),  targetChars});

			josefinSans_Regular =
			 	loader->registerFont(new Font::FontFlags{fontDir.subFile("josefinSans-ES-Regular.ttf"),  targetChars});
			josefinSans_Bold =
			 	loader->registerFont(new Font::FontFlags{fontDir.subFile("josefinSans-ES-Bold.ttf"),  targetChars});

			josefinSans_Regular_Large =
			 	loader->registerFont(new Font::FontFlags{fontDir.subFile("josefinSans-ES-Regular.ttf"),  targetChars, DefFlag, 90});
			josefinSans_Bold_Large =
			 	loader->registerFont(new Font::FontFlags{fontDir.subFile("josefinSans-ES-Bold.ttf"),  targetChars, DefFlag, 90});

			telegrama =
			 	loader->registerFont(new Font::FontFlags{fontDir.subFile("telegrama.otf"),  targetChars});


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
			raw = new GL::Mesh{[]( GL::Mesh& mesh) {
				mesh.bind();
				mesh.getVertexBuffer().setData({
					-1.0f, -1.0f, 0.0f, 0.0f,
					 1.0f, -1.0f, 1.0f, 0.0f,
					 1.0f,  1.0f, 1.0f, 1.0f,
					-1.0f,  1.0f, 0.0f, 1.0f
				}, GL_STATIC_DRAW);

				mesh.getIndexBuffer().setDataRaw(GL::ELEMENTS_STRIP_STD.data(), GL::ELEMENTS_QUAD_STRIP_LENGTH, GL_STATIC_DRAW);

				AttributeLayout& layout = mesh.getVertexArray().getLayout();
				layout.addFloat(2);
				layout.addFloat(2);
				mesh.applyLayout();

				mesh.unbind();
			}};
		}

		void load() {
			coords = new GL::RenderableMesh<GL_TRIANGLE_FAN>(Assets::Shaders::coordAxis, [](GL::RenderableMesh<GL_TRIANGLE_FAN>& mesh) {
				mesh.bind();
				mesh.getVertexBuffer().setData({
					-1.0f, -1.0f,
					 1.0f, -1.0f,
					 1.0f,  1.0f,
					-1.0f,  1.0f
				}, GL_STATIC_DRAW);
				mesh.getIndexBuffer().setDataRaw(GL::ELEMENTS_STRIP_STD.data(), GL::ELEMENTS_QUAD_STRIP_LENGTH, GL_STATIC_DRAW);

				AttributeLayout& layout = mesh.getVertexArray().getLayout();
				layout.addFloat(2);
				mesh.applyLayout();;

				mesh.unbind();
			});
		}

		void dispose() {
			delete raw;
			delete coords;
		}
	}

	inline void loadBasic();

	inline void loadAfter() {
		Meshes::load();
	}

	inline void dispose() {
		Textures::dispose();
		Meshes::dispose();
	}
}

