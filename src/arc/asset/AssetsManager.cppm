export module Assets.Manager;

import Core.Renderer;
import Assets.Loader;
import Assets.TexturePacker;
import Assets.SoundLoader;
import Graphic.TextureAtlas;
import GL.Shader.Manager;
import Font;
import Event;

import std;

export
/**
 * \brief All Assets Should be able to access from here!
 */
namespace Assets{
	class Manager;

	struct AssetsLoadInit final : Event::EventType {
		Manager* const manager;
		[[nodiscard]] explicit AssetsLoadInit(Manager* const manager) : manager(manager) {}
	};

	struct AssetsLoadPull final : Event::EventType {
		Manager* const manager;
		[[nodiscard]] explicit AssetsLoadPull(Manager* const manager) : manager(manager) {}
	};

	struct AssetsLoadPost final : Event::EventType {
		Manager* const manager;
		[[nodiscard]] explicit AssetsLoadPost(Manager* const manager) : manager(manager) {}
	};

	struct AssetsLoadEnd final : Event::EventType {
		Manager* const manager;
		[[nodiscard]] explicit AssetsLoadEnd(Manager* const manager) : manager(manager) {}
	};

	class Manager {
	protected:
		GL::ShaderManager shaders{};
		Graphic::TextureAtlas atlas{};
		Font::FontManager fonts{};

		Assets::SoundLoader soundLoader{};
		Assets::AssetsLoader loader{};

		Event::EventManager loadEvents{
			Event::indexOf<AssetsLoadInit>(),
			Event::indexOf<AssetsLoadPull>(),
			Event::indexOf<AssetsLoadPost>(),
			Event::indexOf<AssetsLoadEnd>()
		};

		std::unique_ptr<Font::FontManager> tempFontLoader{std::make_unique<Font::FontManager>()};

	public:
		void pullRequest();

		void load_Visible(const int width, const int height, Core::Renderer* renderer);

		void load_Quiet(bool forceGet = true);

		void loadPost();

		void loadEnd();

		[[nodiscard]] GL::ShaderManager& getShaders() {
			return shaders;
		}

		[[nodiscard]] Graphic::TextureAtlas& getAtlas() {
			return atlas;
		}

		[[nodiscard]] Font::FontAtlas& getFonts() const{
			return *fonts.atlas;
		}

		[[nodiscard]] Font::FontManager& getFontsManager() {
			return fonts;
		}

		[[nodiscard]] Font::FontManager& getFontsManager_Load() const{
			return *this->tempFontLoader;
		}

		[[nodiscard]] Assets::AssetsLoader& getLoader() {
			return loader;
		}

		[[nodiscard]] Event::EventManager& getEventTrigger() {
			return loadEvents;
		}

		[[nodiscard]] Assets::SoundLoader& getSoundLoader() {
			return soundLoader;
		}
	};
}

