module;

export module Assets.Graphic;

import Assets.Directories;

import GL.Buffer.DataBuffer;
import GL.VertexArray;
import Font;
import Font.FrequentCharCode;
import Font.UnicodeRefParser;
import Font.GlyphArrangement;
import OS.FileTree;
import GL.Mesh;
import GL.Shader;
import GL.Uniform;
import GL.Texture.Texture2D;
import GL.Texture.TextureRegion;
import GL.Texture.TextureRegion;

import Geom.Vector2D;

import Graphic.Color;
import Graphic.Pixmap;
import Graphic.PostProcessor.MultiSampleBliter;
import Graphic.PostProcessor.ShaderProcessor;
import Graphic.PostProcessor.BloomProcessor;
import Graphic.PostProcessor.P4Processor;
import Graphic.PostProcessor.PipeProcessor;
import Graphic.PostProcessor;

import GL.Shader.Manager;
export import GL.Shader.UniformWrapper;
import GL.Constants;

import OS.File;

import std;
import Math;

namespace Core{
	export class Camera2D;
}


/**
 * \brief Inbuilt Resources
 *
 */
export namespace Assets{
	inline OS::FileTree textureTree;

	namespace Shaders {
		inline GL::ShaderSource* stdPost = nullptr;
		inline GL::ShaderSource* texPost = nullptr;
		inline GL::ShaderSource* screenSpace = nullptr;

		/** @brief [Screen Size, Camera Pos]*/
		inline GL::UniformArgsWrapper coordAxisArgs{static_cast<Core::Camera2D*>(nullptr)};
		inline GL::ShaderSource* coordAxis = nullptr;
		inline GL::ShaderSource* filter = nullptr;

		inline GL::ShaderSource* threshold_light = nullptr;
		inline GL::ShaderSource* gaussian = nullptr;
		inline GL::ShaderSource* gaussian_world = nullptr;
		inline GL::ShaderSource* bloom = nullptr;
		inline GL::ShaderSource* blit = nullptr;

		/** @brief [width, spacing, color, mulScale] */
		inline GL::UniformArgsWrapper slideLineShaderDrawArgs
			{25.0f, -1.0f, Graphic::Colors::GRAY.createLerp(Graphic::Colors::LIGHT_GRAY, 0.5f), 1.0f};

		/** @brief [screen scale, offset, timeScale] */
		inline GL::UniformArgsWrapper slideLineShaderScaleArgs{1.0f, Geom::SNAN2};
		inline GL::UniformArgsWrapper slideLineShaderAngle{45.0f};
		inline GL::ShaderSource* sildeLines = nullptr;

		inline GL::ShaderSource* world = nullptr;
		inline GL::ShaderSource* merge = nullptr;

		inline GL::ShaderSource* mask = nullptr;
		inline GL::ShaderSource* worldBloom = nullptr;

		inline GL::ShaderSource* frostedGlass = nullptr;

		inline GL::UniformArgsWrapper outlineArgs{4.0f, 0.0f, Geom::norBaseVec2<float>};
		inline GL::ShaderSource* outline_ortho = nullptr;
		inline GL::ShaderSource* outline_sobel = nullptr;


		void loadPrimitive();

		void load(GL::ShaderManager* manager);
	}

	namespace PostProcessors {
		std::unique_ptr<Graphic::PostProcessor>
			multiToBasic{nullptr},
			blendMulti{nullptr},
			blurX{nullptr},
			blurY{nullptr},
			blurX_World{nullptr},
			blurY_World{nullptr},
			blend{nullptr},
			frostedGlass{nullptr},
			frostedGlassTest{nullptr}
		;

		std::unique_ptr<Graphic::PingPongProcessor>
			blur_Far,
			frostedGlassBlur
		;


		std::unique_ptr<Graphic::BloomProcessor> bloom{nullptr};

		void loadPrimitive();

		void load();
	}

	namespace Textures {
		inline const GL::Texture2D* whiteTex = nullptr;
		inline GL::TextureRegion whiteRegion{};
		Graphic::Pixmap error{};

		void load() {
			whiteTex = new GL::Texture2D{Dir::texture.find("white.png")};
			error = Graphic::Pixmap{Dir::texture.find("error.png")};
			whiteRegion = GL::TextureRegion{whiteTex};
			whiteRegion.shrinkEdge(15.0f);
		}

		void dispose() {
			delete whiteTex;
		}
	}

	namespace Fonts {
		constexpr int DefFlag = 1L << 2;

		OS::File unicodeRefDir;
		OS::File cacheDir;
		// Font::FontsManager

		//const std::vector<Math::Section<Font::CharCode>> targetChars {{' ' + 1, '~'}};
		const std::vector<Font::CharCode> targetChars = std::ranges::views::iota(' ' + 1, '~' + 1) | std::ranges::to<std::vector<Font::CharCode>>();// {{' ' + 1, '~'}};
		std::vector<Font::CharCode> targetChars_withChinese = std::vector{targetChars};

		Font::FontFlags
			*sourceHan_SC_SB{nullptr},

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

			telegrama =
				 loader->registerFont(new Font::FontFlags{Dir::font.subFile("telegrama.otf"),  targetChars, DefFlag, 120});

			loader->load();

			Font::initParser(telegrama);
		}

		void load(Font::FontManager* loader) { // NOLINT(*-non-const-parameter
			targetChars_withChinese.append_range(Font::genRefTable(unicodeRefDir.find("zh_cn.txt")));

			sourceHan_SC_SB =
				loader->registerFont(new Font::FontFlags{Dir::font.subFile("SourceHanSerifSC-SemiBold.otf" ),  targetChars_withChinese});
			sourceHan_SC_SB->setDefErrorFallback(::Assets::Textures::error);

			consola_Regular =
				loader->registerFont(new Font::FontFlags{Dir::font.subFile("consola.ttf" ),  targetChars});
			consola_Italic =
			 	loader->registerFont(new Font::FontFlags{Dir::font.subFile("consolai.ttf"),  targetChars});
			consola_Bold =
			 	loader->registerFont(new Font::FontFlags{Dir::font.subFile("consolab.ttf"),  targetChars});
			consola_Bold_Italic =
			 	loader->registerFont(new Font::FontFlags{Dir::font.subFile("consolaz.ttf"),  targetChars});

			times_Regular =
				loader->registerFont(new Font::FontFlags{Dir::font.subFile("times.ttf" ),  targetChars});
			times_Italic =
				loader->registerFont(new Font::FontFlags{Dir::font.subFile("timesi.ttf"),  targetChars});
			times_Bold =
				loader->registerFont(new Font::FontFlags{Dir::font.subFile("timesbd.ttf"),  targetChars});
			times_Bold_Italic =
				loader->registerFont(new Font::FontFlags{Dir::font.subFile("timesbi.ttf"),  targetChars});

			josefinSans_Regular =
			 	loader->registerFont(new Font::FontFlags{Dir::font.subFile("josefinSans-ES-Regular.ttf"),  targetChars});
			josefinSans_Bold =
			 	loader->registerFont(new Font::FontFlags{Dir::font.subFile("josefinSans-ES-Bold.ttf"),  targetChars});

			josefinSans_Regular_Large =
			 	loader->registerFont(new Font::FontFlags{Dir::font.subFile("josefinSans-ES-Regular.ttf"),  targetChars, DefFlag, 90});
			josefinSans_Bold_Large =
			 	loader->registerFont(new Font::FontFlags{Dir::font.subFile("josefinSans-ES-Bold.ttf"),  targetChars, DefFlag, 90});

			telegrama =
			 	loader->registerFont(new Font::FontFlags{Dir::font.subFile("telegrama.otf"),  targetChars});

			telegrama->fallback = sourceHan_SC_SB;


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
			Font::registerParserableFont("srch", sourceHan_SC_SB);


			telegrama->setDefErrorFallback(::Assets::Textures::error);
			sourceHan_SC_SB->setDefErrorFallback(::Assets::Textures::error);

			josefinSans_Bold_Large->setDefErrorFallback(::Assets::Textures::error);
			josefinSans_Regular_Large->setDefErrorFallback(::Assets::Textures::error);

		}
	}

	namespace Meshes {
		std::unique_ptr<GL::Mesh> raw{nullptr}
		;

		void loadPrimitive() {
			raw.reset(new GL::Mesh{[]( GL::Mesh& mesh) {
				mesh.bind();
				mesh.vertexBuffer.setData({
					-1.0f, -1.0f, 0.0f, 0.0f,
					 1.0f, -1.0f, 1.0f, 0.0f,
					 1.0f,  1.0f, 1.0f, 1.0f,
					-1.0f,  1.0f, 0.0f, 1.0f
				}, GL_STATIC_DRAW);

				mesh.indexBuffer.setDataRaw(GL::ELEMENTS_STRIP_STD.data(), GL::ELEMENTS_QUAD_STRIP_LENGTH, GL_STATIC_DRAW);

				GL::AttributeLayout& layout = mesh.vertexArray.getLayout();
				layout.addFloat(2);
				layout.addFloat(2);
				mesh.applyLayout();

				mesh.unbind();
			}});
		}

		void load() {

		}

		void dispose() {
			raw.reset(nullptr);
		}
	}

	inline void loadBasic();

	inline void loadAfter() {
		Meshes::load();
		PostProcessors::load();
	}

	inline void dispose() {
		Textures::dispose();
		Meshes::dispose();
	}
}

