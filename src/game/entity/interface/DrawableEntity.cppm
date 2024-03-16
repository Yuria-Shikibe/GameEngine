module;

export module Game.Entity.Drawable;

import Geom.Shape.Rect_Orthogonal;
import Game.Entity;

export namespace Game {
	class DrawableEntity : public Entity{
	protected:
		/**
		 * \brief When entity exit screen, part of its calculation can be roughly.
		 */
		bool inScreen{true};

	public:
		~DrawableEntity() override = default;

		[[nodiscard]] virtual const Geom::Shape::OrthoRectFloat& getDrawBound() const{
			static constexpr Geom::Shape::OrthoRectFloat EmptyBound = {};
			return EmptyBound;
		}

		[[nodiscard]] bool isInScreen() const {
			return inScreen;
		}

		void setInScreen(bool val){
			inScreen = val;
		}

		virtual void calculateInScreen(Geom::Shape::OrthoRectFloat& viewport) {
			inScreen = getDrawBound().overlap(viewport);
		}

		virtual void draw() const = 0;

		virtual void drawDebug() const = 0;
	};

	class DrawableBounded : public DrawableEntity {
	public:
		Geom::Shape::OrthoRectFloat expectedDrawRegion{};

		[[nodiscard]] const Geom::Shape::OrthoRectFloat& getDrawBound() const override {
			return expectedDrawRegion;
		}

		void calculateInScreen(Geom::Shape::OrthoRectFloat& viewport) override {
			inScreen = viewport.overlap(expectedDrawRegion);
		}
	};
}
