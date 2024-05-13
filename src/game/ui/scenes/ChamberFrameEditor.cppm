//
// Created by Matrix on 2024/5/10.
//

export module Game.Scene.ChamberFrameEditor;

export import UI.Scene;
export import Game.Chamber;
export import Game.Chamber.Frame;

import std;

export namespace Game::UI::Scenes{
	template <typename E>
	class ChamberFrameEditor : public ::UI::Scene{
		using ValueType = E;
		using Tile = ChamberTile<ValueType>;
		Table sideSections{};
		// Table
	};
}

