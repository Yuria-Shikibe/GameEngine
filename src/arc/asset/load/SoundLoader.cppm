module;

# include <irrKlang.h>

export module Assets.Load.SoundLoader;

import Assets.Load.Core;
import std;
import OS.File;
import ext.RuntimeException;

export namespace Assets::Load{
	class SoundLoader : public LoadTask {
	protected:
		irrklang::ISoundEngine* engine{nullptr};

		std::vector<irrklang::ISoundSource*> toLoad{};

	public:
		SoundLoader(const SoundLoader& other) = delete;

		SoundLoader(SoundLoader&& other) noexcept = delete;

		SoundLoader& operator=(const SoundLoader& other) = delete;

		SoundLoader& operator=(SoundLoader&& other) noexcept = delete;

		[[nodiscard]] SoundLoader() = default;

		void setEngine(irrklang::ISoundEngine* engine) {
			this->engine = engine;
		}

		[[nodiscard]] explicit SoundLoader(irrklang::ISoundEngine* const engine)
			: engine(engine) {
		}

		[[nodiscard]] irrklang::ISoundSource* pullRequest(const OS::File& file){
			const auto data = engine->getSoundSource(file.absolutePath().string().data());
			if(!data || !file.exist()) {
				throw ext::RuntimeException{"Failed to load sound file: " + file.filename()};
			}

			toLoad.push_back(data);
			return data;
		}

		bool load() {
			if(!engine){
				throw ext::NullPointerException{"Missing Sound Engine"};
			}

			for(const auto& ss : toLoad) {
				const auto instance = engine->play2D(ss, false, true, true);
				instance->setVolume(0.0f);
				instance->setIsPaused(false);
				instance->drop();
			}

			done();
			handler.join();
			return finished;
		}

		[[nodiscard]] std::future<bool> launch(const std::launch policy) override {
			return std::async(policy, &SoundLoader::load, this);
		}
	};
}


