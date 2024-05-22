export module UI.Scene;

export import UI.Table;

import std;
import OS.Ctrl.Bind;

export namespace UI{
	constexpr std::string_view Menu_Main = "main-menu";
	constexpr std::string_view Menu_Game = "main-game";
	constexpr std::string_view Menu_Settings = "main-settings";
	constexpr std::string_view In_Game = "in-game";

	class Scene : public UI::Table{
	public:
		[[nodiscard]] Scene() = default;

		[[nodiscard]] explicit Scene(std::string&& name){
			this->name = std::move(name);
		}

		std::unique_ptr<OS::InputBindGroup> uiInput{};

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
