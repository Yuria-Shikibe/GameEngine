module;

/**
 * \brief Keep This Module Imported in .cpp if possible!
 */
export module Core;

import <string>;
import <functional>;
import <glad/glad.h>;
import <GLFW/glfw3.h>;

import Geom.Shape.Rect_Orthogonal;

import Core.Audio;
import Assets.Manager;
import Assets.Bundle;
import Core.Settings;
import Core.Log;

import UI.Root;
import Core.Batch;
import Core.Input;
import Core.Camera;
import Core.Renderer;
import OS.FileTree;
import GL;

export namespace Core {
	const std::string title = APPLICATION_NAME;

	inline GLFWwindow* mainWindow      = nullptr;
	inline GLFWmonitor* mainMonitor    = nullptr;
	inline GLFWmonitor* currentMonitor = nullptr;

	inline Geom::Shape::OrthoRectInt lastScreenBound{};
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
	inline Batch* overlayBatch = nullptr;

	inline Batch* worldBatch = nullptr;
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
