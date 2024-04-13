export module UI.Scene;

export import UI.Table;

import std;

export namespace UI{
	class Scene : public UI::Table{
	public:
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
