export module Assets;

import Font.FreeType;
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

using namespace GL;

/**
 * \brief Inbuilt Resources
 *
 */
export namespace Assets{
//	namespace Shaders{
//		inline std::unique_ptr<Shader> texPost;
//	}


	inline OS::File assetsDir;
	inline OS::File shaderDir;
	inline OS::File textureDir;
	inline OS::File fontDir;
	inline OS::File screenshotDir;

	namespace Shaders {
		inline Shader* stdPost = nullptr;
		inline Shader* texPost = nullptr;
		inline Shader* screenSpace = nullptr;

		void load() {
			texPost = new Shader{ shaderDir, "tex-std" };
			texPost->setUniformer([]([[maybe_unused]] const Shader& shader) {});

			stdPost = new Shader{ shaderDir, "std" };
			stdPost->setUniformer([]([[maybe_unused]] const Shader& shader) {
				// GL::uniformColor(0, Graphic::Colors::WHITE);
			});

			screenSpace = new Shader(shaderDir, "screenspace");
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
		OS::File cache;

		void load() {
			cache = fontDir.subFile("cache");
			if(!cache.exist())cache.createDir();
			Font::FT::loadLib();

			Font::FT::load({' ' + 1, 0b0111'1110}, fontDir.subFile("consola.ttf"), cache);
		}

		void dispose() {

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
	}

	inline void dispose() {
		Shaders::dispose();
		Textures::dispose();
	}
}

