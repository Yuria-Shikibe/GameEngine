module;

export module Game.Entity.Drawable;

import Geom.Shape.Rect_Orthogonal;
import Game.Entity;

export namespace Game {
	class Drawable : public Entity{
	protected:
		/**
		 * \brief When entity exit screen, part of its calculation can be roughly.
		 */
		bool inScreen{true};

	public:
		~Drawable() override = default;

		[[nodiscard]] virtual const Geom::Shape::OrthoRectFloat& getDrawBound() const = 0;

		[[nodiscard]] bool isInScreen() const {
			return inScreen;
		}

		virtual void calculateInScreen(Geom::Shape::OrthoRectFloat& viewport) {
			inScreen = true;
		}

		virtual void draw() const = 0;

		virtual void drawDebug() const = 0;
	};

	class DrawableBounded : public Drawable {
		Geom::Shape::OrthoRectFloat expectedDrawRegion{};

		[[nodiscard]] const Geom::Shape::OrthoRectFloat& getDrawBound() const override {
			return expectedDrawRegion;
		}

		void calculateInScreen(Geom::Shape::OrthoRectFloat& viewport) override {
			inScreen = viewport.overlap(expectedDrawRegion);
		}
	};
}
