module;

export module Game.Entity.DrawMap;

import Game.Entity.EntityMap;
import Game.Entity.Drawable;

import std;

export namespace Game{
	class DrawMap : public EntityMap<DrawableEntity>{
	protected:
		Geom::Shape::OrthoRectFloat viewPort{};

	public:
		using EntityMap::idMap;

		void render(){
			auto view = idMap | std::ranges::views::values;

			std::for_each(std::execution::par_unseq, view.begin(), view.end(), [this](decltype(idMap)::value_type::second_type& v) {
				v->calculateInScreen(viewPort);
			});

			for(const auto& entity : view) {
				if(entity->isInScreen())entity->draw();
			}
		}

		void setViewport(const Geom::Shape::OrthoRectFloat& view) {
			viewPort = view;
		}
	};
}
