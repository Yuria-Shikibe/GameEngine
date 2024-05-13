export module UI.Scene;

export import UI.Table;

import std;
import Core.Input;

export namespace UI{
	constexpr std::string_view Menu_Main = "main-menu";
	constexpr std::string_view Menu_Game = "main-game";
	constexpr std::string_view Menu_Settings = "main-settings";

	class Scene : public UI::Table{
	public:
		std::unique_ptr<Core::Input> uiInput{std::make_unique<Core::Input>()};

		[[nodiscard]] std::string_view getSceneName() const {
			return name;
		}

		virtual void build(){
		}

		bool getRootVisiable() const;

		void update(const float delta) override{
			if(visiable)Table::update(delta);
		}
	};
}
