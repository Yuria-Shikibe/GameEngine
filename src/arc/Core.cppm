module;

#include <GLFW/glfw3.h>

export module Core;

import std;

import Geom.Shape.Rect_Orthogonal;

export import Core.Audio;
export import Assets.Manager;
export import Assets.Bundle;
export import Core.Settings;
export import Core.Log;

export import UI.Root;
export import Core.BatchGroup;
export import Core.Input;
export import Core.Camera;
export import Core.Renderer;

import OS.FileTree;

export namespace Core {
	const std::string title = APPLICATION_NAME;

	inline GLFWwindow* mainWindow      = nullptr;
	inline GLFWmonitor* mainMonitor    = nullptr;
	inline GLFWmonitor* currentMonitor = nullptr;

	inline Geom::OrthoRectInt lastScreenBound{};
	inline bool maximizeWinOnInit = true;
	inline bool maximized = true;
	inline bool windowized = false;

	void setScreenBound(GLFWwindow* win = mainWindow);

	inline const GLFWvidmode* mainScreenMode = nullptr;
	//TODO using unique ptr?

	/* Almost Done */
	inline Input* input = nullptr;
	/* Basically Done */
	inline Camera2D* camera = nullptr;
	/* Basically Done */
	inline BatchGroup batchGroup{};
	/* Basically Done */
	inline Renderer* renderer = nullptr;
	/* 88.00% */
	inline OS::FileTree* rootFileTree = nullptr;

	//TODO main components... maybe more

	/* 0.00% */
	inline Audio* audio = nullptr;
	/* 90.00% */
	inline Assets::Manager* assetsManager = nullptr;
	/* 0.00% */
	inline Settings* settings = nullptr;
	/* 3.00% */
	inline UI::Root* uiRoot = nullptr;
	/* 0.00% */
	inline Assets::Bundle* bundle = nullptr;
	/* 30.00% */
	inline Log* log = nullptr;

	//TODO maybe some objects for task management, if necessary
	//TODO Async Impl...

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
