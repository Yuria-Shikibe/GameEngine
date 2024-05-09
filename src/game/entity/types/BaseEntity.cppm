module;

export module Game.Entity.BaseEntity;

export import Game.Entity;
export import Game.Entity.Drawable;
export import Game.Entity.Healthed;
export import Game.Entity.Factional;
export import Game.Entity.PosedEntity;

import Geom.Position;

export namespace Game {
	class BaseEntity : public DrawableEntity, public PosedEntity, public Healthed, public Factional {
	public:
		[[nodiscard]] BaseEntity() noexcept = default;

		virtual void init(){

		}
	};
}
