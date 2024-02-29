module;

export module Assets.Sound;

import <irrKlang.h>;
import Assets.SoundLoader;
import OS.File;

using namespace irrklang;

export namespace Assets::Sounds{
	using SoundSource = ISoundSource*;

	SoundSource
		flak,
		largeExplosion,
		laser5
	;

	void load(const OS::File& dir, Assets::SoundLoader& loader) {
		flak = loader.pullRequest(dir.subFile("flak.mp3"));
		largeExplosion = loader.pullRequest(dir.subFile("largeExplosion.ogg"));
		laser5 = loader.pullRequest(dir.subFile("laser5.ogg"));
	}
}
