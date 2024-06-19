//
// Created by Matrix on 2024/6/10.
//

export module Assets.Load.MainLoader;

import Assets.Load.Core;

import Assets.Load.Misc;
import Assets.Load.FontLoader;
import Assets.Load.TexturePacker;
import Assets.Load.ShaderLoader;
import Assets.Load.SoundLoader;
import Assets.TexturePage;

import ext.Event;

export namespace Core{
	class Renderer;
}


export namespace Assets::Load{
	class MainLoader;

	template <Phase phase>
	struct LoadEvent final : ext::EventType{
		static constexpr auto EventPhase = phase;
		MainLoader& loader;

		[[nodiscard]] explicit LoadEvent(MainLoader* loader)
			: loader{*loader}{}
	};

	class MainLoader : public LoadManager{
		QuickInitFontLoader tempFontLoader{};
		Assets::TexturePage tempPage{"font-load", 2};

		void pull_builtin();

		void post_builtin();

	public:
		[[nodiscard]] explicit MainLoader(const std::string& taskName)
			: LoadManager{taskName}{}

		MiscTaskManager<void()> miscs{};

		FontLoader fontLoader{};
		TexturePacker texturePacker{};
		ShaderLoader shaderLoader{};
		SoundLoader soundLoader{};

		[[nodiscard]] MainLoader() : MainLoader{"Main Loader"}{
			phaseArriveEventManager.on<Phase::init>([this]{
				eventManager.emplace_fire<LoadEvent<Phase::init>>(this);
			});

			phaseArriveEventManager.on<Phase::pull>([this]{
				eventManager.emplace_fire<LoadEvent<Phase::pull>>(this);
			});

			phaseArriveEventManager.on<Phase::load>([this]{
				eventManager.emplace_fire<LoadEvent<Phase::load>>(this);
			});

			phaseArriveEventManager.on<Phase::post_load>([this]{
				eventManager.emplace_fire<LoadEvent<Phase::post_load>>(this);
			});

			phaseArriveEventManager.on<Phase::end>([this]{
				eventManager.emplace_fire<LoadEvent<Phase::end>>(this);
			});

			phaseArriveEventManager.on<Phase::clear>([this]{
				eventManager.emplace_fire<LoadEvent<Phase::clear>>(this);
			});
		}

		ext::EventManager eventManager{
			ext::typeIndexOf<LoadEvent<Phase::init>>(),
			ext::typeIndexOf<LoadEvent<Phase::pull>>(),
			ext::typeIndexOf<LoadEvent<Phase::load>>(),
			ext::typeIndexOf<LoadEvent<Phase::post_load>>(),
			ext::typeIndexOf<LoadEvent<Phase::end>>(),
			ext::typeIndexOf<LoadEvent<Phase::clear>>(),
		};

		void loadWith(Core::Renderer* renderer);
	};
}
