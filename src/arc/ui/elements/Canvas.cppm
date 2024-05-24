//
// Created by Matrix on 2024/5/24.
//

export module UI.Canvas;

import UI.Elem;
import ext.Concepts;

export namespace UI{

	template <Concepts::Invokable<void(Elem&)> Drawer>
	class Canvas : public Elem{
		Drawer drawer;

	public:
		[[nodiscard]] explicit Canvas(const Drawer& drawer)
			: drawer{drawer}{}

		void drawContent() const override{
			drawer(*this);
		}
	};

	template <Concepts::Invokable<void(Elem&)> Drawer>
	Canvas(Drawer) -> Canvas<Drawer>;
}
