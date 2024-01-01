module;

export module Game.Entity.Drawable;

import Geom.Shape.Rect_Orthogonal;
import Game.Entity;

export namespace Game {
	class Drawable : Entity {
	public:
		~Drawable() override = default;

		[[nodiscard]] virtual const Geom::Shape::OrthoRectFloat& getDrawBound() const = 0;

		virtual void draw() const = 0;
	};

	class DrawableBounded : public Drawable {
		Geom::Shape::OrthoRectFloat expectedDrawRegion{};

		[[nodiscard]] const Geom::Shape::OrthoRectFloat& getDrawBound() const override {
			return expectedDrawRegion;
		}
	};
}
