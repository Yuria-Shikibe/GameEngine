//
// Created by Matrix on 2024/5/23.
//

export module Game.UI.ChamberWorkshop;

export import UI.Table;
export import UI.Screen;
export import Game.Chamber;
export import Game.Chamber.Frame;

export namespace Game::UI{
	template <typename T>
	class ChamberWorkshop : public ::UI::Table{
		Game::ChamberFrame<T> frame{};

	public:
		using EntityType = T;

		[[nodiscard]] ChamberWorkshop() = default;


	};
}
