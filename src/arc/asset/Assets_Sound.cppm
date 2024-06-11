export module Assets.Sound;

export import <irrKlang.h>;
import Assets.Load.SoundLoader;
import OS.File;

using namespace irrklang;

export namespace Assets::Sounds{
	using SoundSource = ISoundSource*;

	SoundSource
		uiClick,
		uiClick2,

		flak,
		largeExplosion,
		laser5
	;

	void load(const OS::File& dir, Load::SoundLoader& loader) {
		flak = loader.pullRequest(dir.subFile("flak.mp3"));
		uiClick = loader.pullRequest(dir.subFile("ui-click.ogg"));
		uiClick2 = loader.pullRequest(dir.subFile("ui-click-2.ogg"));
		uiClick->setDefaultVolume(0.055f);
		uiClick2->setDefaultVolume(0.065f);


		largeExplosion = loader.pullRequest(dir.subFile("largeExplosion.ogg"));
		laser5 = loader.pullRequest(dir.subFile("laser5.ogg"));
	}
}
