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
	namespace Load{
		class FontLoader;
		class QuickInitFontLoader;
		class ShaderLoader;
	}


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

		void load(Load::ShaderLoader& manager);
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

		Font::FontFace
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

		void loadPreivous(Load::QuickInitFontLoader& loader);

		void pull(Load::FontLoader& loader);
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

