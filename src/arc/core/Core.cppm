module;

#include <GLFW/glfw3.h>

export module Core;

import std;

import Geom.Rect_Orthogonal;

export import :Util;
export import :Global;

export namespace Core {
	constexpr std::string_view title = APPLICATION_NAME;

	bool maximizeWinOnInit = true;

	std::vector<std::function<void()>> destructors{};
	//TODO using unique ptr?

	//TODO maybe some objects for task management, if necessary
	//TODO Async Impl...

	void initPlatform(const int argc = 0, char* argv[] = nullptr){
		initCurrentPlatform(platform, APPLICATION_NAME, argc, argv);
	}

	void initMainWindow();

	void initFileSystem();

	void initCore(const std::function<void()>& initializer = nullptr);

	/**
	* \brief Invoke this after assets load!
	**/
	void initCore_Post(const std::function<void()>& initializer = nullptr);

	void loadAssets();

	void dispose();
}
