//
// Created by Matrix on 2023/11/23.
//

module ;

export module Core.Audio;

import <irrKlang.h>;
import RuntimeException;
import Concepts;
import OS.File;
import <memory>;


using namespace irrklang;

export namespace Core{

	struct DropableDeleter {
		void operator()(IVirtualRefCounted* toDrop) const {
			if(toDrop)toDrop->drop();
		}
	};

	class Audio{
	public:
		DropableDeleter deleter{};
		ISoundEngine* engine{nullptr};

		const vec3df listenerDirection{0,0,1};
		vec3df listenerPos{0,0,0};

		vec3df tempVec3{0,0,1};

		using SoundSource = ISoundSource*;
		using SoundInstance = std::unique_ptr<ISound, DropableDeleter>;

		[[nodiscard]] SoundSource loadFile_delayed(const OS::File& file) const {
			return engine->getSoundSource(file.absolutePath().string().data());
		}

		void setListenerPosition(const float x, const float y, const float z = 0.0f) {
			listenerPos.set(x, y, z);
			engine->setListenerPosition(listenerPos, listenerDirection);
		}

		void playAt(const SoundSource source, const float x, const float y, const float z) {
			tempVec3.set(x, y, z);
			engine->play3D(source, tempVec3);
		}

		void play(const SoundSource source) const {
			engine->play2D(source);
		}

		SoundInstance play(const SoundSource source, const float x, const float y, const float z, const bool track = false, const bool loop = false, const bool enableEffect = false) {
			tempVec3.set(x, y, z);
			return std::unique_ptr<ISound, DropableDeleter>{engine->play3D(source, tempVec3, loop, false, track, enableEffect), deleter};
		}

		SoundInstance play(ISoundSource* source, const vec3df& pos,
				  const bool playLooped,
			   const bool enableSoundEffects) const {
			const auto sound = engine->play3D(source, pos, playLooped, false, true, enableSoundEffects);
			return std::unique_ptr<ISound, DropableDeleter>{sound, deleter};
		}

		template <Concepts::Invokable<void(ISound*)> Func>
		void play(ISoundSource* source, const vec3df& pos,
		          const bool playLooped,
			   const bool enableSoundEffects, Func&& modifier) {
			auto sound = engine->play3D(source, pos, playLooped, false, true, enableSoundEffects);
			modifier(sound);
			sound->drop();
		}

		template <Concepts::Invokable<void(ISound*)> Func>
		void play(ISoundSource* source, const vec3df& pos,
			   const bool enableSoundEffects, Func&& modifier) {
			auto sound = engine->play3D(source, pos, false, false, true, enableSoundEffects);
			modifier(sound);
			sound->drop();
		}

		[[nodiscard]] Audio() {
			engine = createIrrKlangDevice(
				// ESOD_AUTO_DETECT, ESEO_MULTI_THREADED | ESEO_LOAD_PLUGINS | ESEO_USE_3D_BUFFERS | ESEO_PRINT_DEBUG_INFO_TO_DEBUGGER | ESEO_PRINT_DEBUG_INFO_TO_STDOUT
			);

			if (!engine)throw ext::RuntimeException{"Failed To Initialize irrklang!"};
		}

		~Audio() {
			engine->stopAllSounds();

			engine->drop();
		}
	};
}