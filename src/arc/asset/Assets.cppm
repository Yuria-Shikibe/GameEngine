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
import Graphic.Color;
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
		}

		void dispose() {
			delete texPost;
			delete stdPost;
			delete screenSpace;
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

			*josefinSans_Regular{nullptr},
			*josefinSans_Bold{nullptr},

			*telegrama{nullptr}
		;

		const Font::FontsManager* manager{nullptr};


		void load() {
			cacheDir = fontDir.subFile("cache");
			if(!cacheDir.exist())cacheDir.createDirQuiet();
			// Font::FT::loadLib();
;
			const std::vector<CharCode> targetChars {' ' + 1, '~'};

			Font::rootCacheDir = cacheDir;
			// consola_Regular =
			// 	Font::registerFont(new Font::FontFlags{fontDir.subFile("consola.ttf" ), cacheDir,  targetChars});
			// consola_Italic =
			//  	Font::registerFont(new Font::FontFlags{fontDir.subFile("consolai.ttf"), cacheDir,  targetChars});
			// consola_Bold =
			//  	Font::registerFont(new Font::FontFlags{fontDir.subFile("consolab.ttf"), cacheDir,  targetChars});
			// consola_Bold_Italic =
			//  	Font::registerFont(new Font::FontFlags{fontDir.subFile("consolaz.ttf"), cacheDir,  targetChars});
			//
			// josefinSans_Regular =
			//  	Font::registerFont(new Font::FontFlags{fontDir.subFile("josefinSans-ES-Regular.ttf"), cacheDir,  targetChars});
			// josefinSans_Bold =
			//  	Font::registerFont(new Font::FontFlags{fontDir.subFile("josefinSans-ES-Bold.ttf"), cacheDir,  targetChars});

			telegrama =
			 	Font::registerFont(new Font::FontFlags{fontDir.subFile("telegrama.otf"), cacheDir,  targetChars});

			Font::load();

			Font::registerParserableFont(telegrama);

			manager = Font::manager.get();

			Font::loadParser(telegrama);
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
					-1.0f, -1.0f,
					-1.0f,  1.0f,
					 1.0f,  1.0f,
					 1.0f, -1.0f
				}, GL_STATIC_DRAW);

				AttributeLayout& layout = mesh.getVertexArray().getLayout();
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

			auto t = coords;

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
	}

	inline void dispose() {
		Shaders::dispose();
		Textures::dispose();
		Fonts::dispose();
	}
}

