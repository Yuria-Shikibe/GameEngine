//
// Created by Matrix on 2024/5/14.
//

export module Assets.MiscLoader;

import ext.Async;
import Assets.Loader;
import std;

export namespace Assets{
	struct MiscTask{
		bool postLoad{};
		std::function<void()> task{};
	};

	struct MiscLoader : public ext::ProgressTask<void, Assets::AssetsTaskHandler>{

	};
}
