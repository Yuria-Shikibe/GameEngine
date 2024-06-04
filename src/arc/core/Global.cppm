//
// Created by Matrix on 2024/5/10.
//

export module Core:Global;

export import Core.Audio;
export import Assets.Manager;
export import Assets.Bundle;
export import Core.Settings;

export import UI.Root;
export import Core.BatchGroup;
export import Core.Input;
export import Core.Camera;
export import Core.Log;
export import Core.Renderer;
export import Core.Platform;
export import Core.Platform.Current;

export import OS.FileTree;
export import OS.Ctrl;

export namespace Core{
	inline Input input{};

	//TODO is ptr necessary?
	inline Camera2D* camera = nullptr;

	inline BatchGroup batchGroup{};

	inline Renderer* renderer = nullptr;

	inline OS::FileTree rootFileTree{};

	//TODO main components... maybe more

	/* 0.00% */
	inline Audio* audio = nullptr;

	inline Assets::Manager* assetsManager = nullptr;

	inline Settings* settings = nullptr;

	inline UI::Root* uiRoot = nullptr;

	inline Assets::Bundle bundle{};

	inline Log* log = nullptr;

	inline std::unique_ptr<PlatformHandle> platform{};

	inline Ctrl::GlobalFocus focus{
		{camera, camera},

	};
}