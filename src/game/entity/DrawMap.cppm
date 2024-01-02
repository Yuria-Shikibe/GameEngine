module;

export module Game.Entity.DrawMap;

import Game.Entity.EntityMap;
import Game.Entity.Drawable;
import Concepts;

import <algorithm>;
import <execution>;
import <memory>;
import <ranges>;
import <unordered_map>;

export namespace Game{
	template <Concepts::Derived<Drawable> T>
	class DrawMap : public EntityMap<T> {
		using EntityMap<T>::idMap;

		Geom::Shape::OrthoRectFloat viewPort{};

		void render() override {
			auto range = idMap | std::ranges::views::values;
			std::for_each(std::execution::par_unseq, range.begin(), range.end(), [this](std::shared_ptr<T>& t) {
				if(!t->isSleeping())t->calculateInScreen(viewPort);

				if(t->isInScreen())t->draw();
			});
		}
	};
}
