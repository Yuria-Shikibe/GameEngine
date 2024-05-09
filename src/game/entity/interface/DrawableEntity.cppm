module;

export module Game.Entity.Drawable;

import Geom.Rect_Orthogonal;
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

		[[nodiscard]] virtual const Geom::OrthoRectFloat& getDrawBound() const{
			static constexpr Geom::OrthoRectFloat EmptyBound = {};
			return EmptyBound;
		}

		[[nodiscard]] bool isInScreen() const {
			return inScreen;
		}

		void setInScreen(const bool val){
			inScreen = val;
		}

		virtual void calculateInScreen(const Geom::OrthoRectFloat& viewport) {
			inScreen = getDrawBound().overlap(viewport);
		}

		virtual void draw() const = 0;

		virtual void drawDebug() const = 0;
	};

	class DrawableBounded : public DrawableEntity {
	public:
		Geom::OrthoRectFloat expectedDrawRegion{};

		[[nodiscard]] const Geom::OrthoRectFloat& getDrawBound() const override {
			return expectedDrawRegion;
		}

		void calculateInScreen(const Geom::OrthoRectFloat& viewport) override {
			inScreen = viewport.overlap(expectedDrawRegion);
		}
	};
}
