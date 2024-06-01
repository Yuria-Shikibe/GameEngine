module;

export module Game.Entity.DrawMap;

import Game.Entity.EntityMap;
import Game.Entity.Drawable;
import Geom.Rect_Orthogonal;

import std;

export namespace Game{
	class DrawMap : public EntityMap<DrawableEntity>{
	protected:
		Geom::OrthoRectFloat viewPort{};

	public:
		std::vector<DrawableEntity*> nextToDraw{};
		using EntityMap::idMap;

		void updateInScreen(auto& tree){

		}

		void render(){
			for(const auto& entity : nextToDraw) {
				entity->draw();
			}

			// for(const auto& entity : nextToDraw) {
			// 	entity->setInScreen(false);
			// }
			nextToDraw.clear();
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
