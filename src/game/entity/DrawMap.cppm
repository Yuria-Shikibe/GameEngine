module;

export module Game.Entity.DrawMap;

import Game.Entity.EntityMap;
import Game.Entity.Drawable;

import std;

export namespace Game{
	class DrawMap : public EntityMap<DrawableEntity>{
	protected:
		Geom::OrthoRectFloat viewPort{};

	public:
		using EntityMap::idMap;

		void updateInScreen(auto& tree){

		}

		void render(){
			auto view = idMap | std::ranges::views::values;

			std::for_each(std::execution::unseq, view.begin(), view.end(), [this](decltype(idMap)::value_type::second_type& v) {
				v->calculateInScreen(viewPort);
			});

			for(const auto& entity : view) {
				if(entity->isInScreen())entity->draw();
			}
		}

		void renderDebug(){
			auto view = idMap | std::ranges::views::values;

			for(const auto& entity : view) {
				if(entity->isInScreen())entity->drawDebug();
			}
		}

		[[nodiscard]] const Geom::OrthoRectFloat& getViewPort() const{ return viewPort; }

		void setViewport(const Geom::OrthoRectFloat& view) {
			viewPort = view;
		}
	};
}
