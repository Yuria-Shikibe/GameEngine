module;

export module Game.BaseEntity;

import Game.Entity;
import Game.Entity.Drawable;
import Game.Entity.Factional;

import Geom.Position;

export namespace Game {
	class BaseEntity
			: public Geom::Position, public Drawable {
	public:
		void update(float deltaTick) override;

		void activate() override;

		void deactivate() override;

		[[nodiscard]] float getX() const override;

		[[nodiscard]] float getY() const override;

		void setX(float x) override;

		void setY(float y) override;

		[[nodiscard]] const Geom::Shape::OrthoRectFloat& getDrawBound() const override;

		void draw() const override;
	};
}
