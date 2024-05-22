//
// Created by Matrix on 2024/5/13.
//

export module Game.Scene.MainMenu;

export import UI.Scene;

import Assets.Ctrl;
import UI.ScrollPane;
import UI.Button;
import UI.Label;
import UI.ControlBindTable;

import UI.Dialog.CtrlBind;
import UI.Screen;
import std;

export namespace Game::Scenes{
	class MainMenu : public ::UI::Scene{
		UI::Screen* screen{};
	public:
		[[nodiscard]] MainMenu(){}

		void build() override;

		void drawContent() const override;
	};
}
