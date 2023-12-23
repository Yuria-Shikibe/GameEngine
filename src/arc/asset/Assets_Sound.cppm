module;

export module Assets.Sound;

import <irrKlang.h>;
import Assets.SoundLoader;
import File;

using namespace irrklang;

export namespace Assets::Sounds{
	using SoundSource = ISoundSource*;

	SoundSource
		flak,
		largeExplosion
	;

	void load(const OS::File& dir, Assets::SoundLoader& loader) {
		flak = loader.pullRequest(dir.subFile("flak.mp3"));
		largeExplosion = loader.pullRequest(dir.subFile("largeExplosion.ogg"));
	}
}
