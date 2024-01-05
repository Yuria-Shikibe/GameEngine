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
	class DrawMap : public EntityMap<DrawableEntity>{
	protected:
		Geom::Shape::OrthoRectFloat viewPort{};

	public:
		using EntityMap::idMap;

		void render() override {
			for(const auto& entity : idMap | std::ranges::views::values) {
				entity->calculateInScreen(viewPort);
				if(entity->isInScreen())entity->draw();
			}
		}

		void setViewport(const Geom::Shape::OrthoRectFloat& view) {
			viewPort = view;
		}
	};
}
