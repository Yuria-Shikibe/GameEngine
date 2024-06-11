module;

#include <GLFW/glfw3.h>

export module Core;

import std;

import Geom.Rect_Orthogonal;

export import :Util;
export import :Global;

namespace Core{
	class MainLoopManager;
}

export namespace Core {

	constexpr std::string_view title = APPLICATION_NAME;

	inline std::vector<std::function<void()>> destructors{};
	//TODO using unique ptr?

	//TODO maybe some objects for task management, if necessary
	//TODO Async Impl...

	void initPlatform(const int argc = 0, char* argv[] = nullptr);

	void initMainWindow(bool maximizeWinOnInit = true);

	void initFileSystem();

	void initCore_Kernal(const std::function<void()>& initializer = nullptr);

	/**
	* \brief Invoke this after assets load!
	**/
	void initCore_Interactive(const std::function<void()>& initializer = nullptr);

	/** @brief Load assets to memory*/
	void beginLoadAssets();

	/** @brief Export assets to manager */
	void endLoadAssets();

	void dispose();

	MainLoopManager* getLoopManager();
}
