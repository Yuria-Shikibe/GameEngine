module;

export module Assets.SoundLoader;

import Async;
import OS.File;
import Assets.Loader;
import RuntimeException;
import <irrKlang.h>;

using namespace irrklang;

export namespace Assets {
	class SoundLoader : public ext::ProgressTask<void, Assets::AssetsTaskHandler> {
	protected:
		ISoundEngine* engine{nullptr};

		std::vector<ISoundSource*> toLoad{};

	public:
		SoundLoader(const SoundLoader& other) = delete;

		SoundLoader(SoundLoader&& other) noexcept = delete;

		SoundLoader& operator=(const SoundLoader& other) = delete;

		SoundLoader& operator=(SoundLoader&& other) noexcept = delete;

		[[nodiscard]] SoundLoader()           = default;

		void setEngine(ISoundEngine* engine) {
			this->engine = engine;
		}

		[[nodiscard]] explicit SoundLoader(ISoundEngine* const engine)
			: engine(engine) {
		}

		[[nodiscard]] ISoundSource* pullRequest(const OS::File& file){
			const auto data = engine->getSoundSource(file.absolutePath().string().data());
			if(!data) {
				throw ext::RuntimeException{"Failed to load sound file: " + file.filename()};
			}

			toLoad.push_back(data);
			return data;
		}

		void load() {
			for(const auto& ss : toLoad) {
				const auto instance = engine->play2D(ss, false, true, true);
				instance->setVolume(0.0f);
				instance->setIsPaused(false);
				instance->drop();

				// instance->stop();
				addProgress_onePart(toLoad.size());
			}

			setDone();
		}

		[[nodiscard]] std::future<void> launch(const std::launch policy) override {
			return std::async(policy, &SoundLoader::load, this);
		}

		[[nodiscard]] std::future<void> launch() override {
			return launch(std::launch::async);
		}

		[[nodiscard]] std::string_view getTaskName() const override {
			return "Loading Sound Sources";
		}
	};
}
