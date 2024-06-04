//
// Created by Matrix on 2024/5/24.
//

export module UI.Canvas;

import UI.Elem;
import ext.Concepts;
import std;

export namespace UI{

	template <Concepts::Invokable<void(const Elem&)> Drawer>
	class Canvas final : public Elem{
		Drawer drawer;

	public:
		using DrawerType = Drawer;
		[[nodiscard]] explicit Canvas(Drawer&& drawer)
			: drawer{std::forward<Drawer>(drawer)}{}

		void drawContent() const override{
			drawer(*this);
		}
	};

	template <Concepts::Invokable<void(const Elem&)> Drawer>
	Canvas(Drawer) -> Canvas<Drawer>;

	template <Concepts::Invokable<void(const Elem&)> Drawer>
	std::unique_ptr<Elem> makeCanvas(Drawer&& drawer){
		return std::make_unique<Canvas<std::decay_t<Drawer>>>(std::forward<Drawer>(drawer));
	}
}
