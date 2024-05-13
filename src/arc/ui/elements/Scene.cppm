export module UI.Scene;

export import UI.Table;

import std;
import Core.Input;

export namespace UI{
	class Scene : public UI::Table{
	public:
		std::unique_ptr<Core::Input> uiInput{std::make_unique<Core::Input>()};

		[[nodiscard]] std::string_view getSceneName() const {
			return name;
		}

		bool getRootVisiable() const;

		void update(const float delta) override{
			visiable = getRootVisiable();

			if(visiable)Table::update(delta);
		}
	};
}
