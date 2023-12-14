module;

export module Graphic.Viewport.Viewport_OrthoRect;

export import Graphic.Viewport;

import Concepts;
import Geom.Shape.Rect_Orthogonal;

using Geom::Shape::OrthoRectFloat;

export namespace Graphic {
	class Viewport_OrthoRect : public Viewport<>{
		OrthoRectFloat bound{};

	public:
		[[nodiscard]] Viewport_OrthoRect() = default;

		void apply(Geom::Matrix3D& projection) const override {
			projection.setOrthogonal(bound.getSrcX(), bound.getSrcY(), bound.getWidth(), bound.getHeight());
		}

		void setOrtho(const float srcX, const float srcY, float width, float height) override {
			bound.set(srcX, srcY, width, height);
		}

		[[nodiscard]] Geom::Vector2D& getCenter() override {
			return this->position;
		}

		void setOrtho(const float width, const float height) override {
			bound.set(
				position.x - width * 0.5f, position.y - height * 0.5f,
				width, height
			);
		}
	};
}
