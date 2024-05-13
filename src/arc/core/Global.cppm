//
// Created by Matrix on 2024/5/10.
//

export module Core:Global;

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
export import Core.Platform;
export import Core.Platform.Current;

export import OS.FileTree;
export import OS.Ctrl;

export namespace Core{
	/* Almost Done */
	inline Input input{};
	/* Basically Done */
	inline Camera2D* camera = nullptr;
	/* Basically Done */
	inline BatchGroup batchGroup{};
	/* Basically Done */
	inline Renderer* renderer = nullptr;
	/* 88.00% */
	inline OS::FileTree rootFileTree{};

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
	inline Assets::Bundle bundle{};
	/* 30.00% */
	inline Log* log = nullptr;

	inline std::unique_ptr<PlatformHandle> platform{};

	inline Ctrl::GlobalFocus focus{
		{camera, camera},

	};
}