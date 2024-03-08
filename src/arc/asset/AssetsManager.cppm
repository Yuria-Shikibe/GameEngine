module;

export module Assets.Manager;

import Core.Renderer;
import Assets.Loader;
import Assets.TexturePacker;
import Assets.Sound;
import Assets.SoundLoader;
import Graphic.TextureAtlas;
import GL.Shader.Manager;
import Font;
import Event;

import <memory>;
import <GLFW/glfw3.h>;



export
/**
 * \brief All Assets Should be able to access from here!
 */
namespace Assets{
	class Manager;

	struct AssetsLoadPull final : Event::EventType {
		Manager* const manager;
		[[nodiscard]] explicit AssetsLoadPull(Manager* const manager) : manager(manager) {}
	};

	struct AssetsLoadBegin final : Event::EventType {
		Manager* const manager;
		[[nodiscard]] explicit AssetsLoadBegin(Manager* const manager) : manager(manager) {}
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
			Event::indexOf<AssetsLoadPull>(),
			Event::indexOf<AssetsLoadBegin>(),
			Event::indexOf<AssetsLoadPost>(),
			Event::indexOf<AssetsLoadEnd>()
		};

		std::unique_ptr<Font::FontManager> tempFontLoader{std::make_unique<Font::FontManager>()};

	public:
		void pullRequest();

		void load_Visible(unsigned int width, unsigned int height, GLFWwindow* window, Core::Renderer* renderer);

		void load_Quiet(bool forceGet = true);

		void loadPost();

		void loadEnd();

		[[nodiscard]] GL::ShaderManager& getShaders();

		[[nodiscard]] Graphic::TextureAtlas& getAtlas();

		[[nodiscard]] Font::FontCache& getonts() const;

		[[nodiscard]] Assets::AssetsLoader& getLoader();

		[[nodiscard]] Event::EventManager& getEventTrigger();

		[[nodiscard]] Assets::SoundLoader& getSoundLoader();
	};
}

