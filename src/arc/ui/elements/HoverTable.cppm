//
// Created by Matrix on 2024/3/30.
//

export module UI.HoverTable;

export import UI.Table;

export namespace UI{
	class HoverTable : public Table{
	public:
		bool isIgnoreLayout() const override{
			return true;
		}
	};
}