module;

export module Graphic.Viewport.Viewport_OrthoRect;

export import Graphic.Viewport;

import ext.Concepts;

export namespace Graphic {
	class Viewport_OrthoRect : public Viewport<>{
		Geom::OrthoRectFloat bound{};

	public:
		[[nodiscard]] Viewport_OrthoRect() = default;

		void modify(Geom::Matrix3D& projection) const override {
			projection.setOrthogonal(bound.getSrcX(), bound.getSrcY(), bound.getWidth(), bound.getHeight());
		}

		void setOrtho(const float srcX, const float srcY, const float width, const float height) override {
			bound.set(srcX, srcY, width, height);
		}

		[[nodiscard]] Geom::Vec2& getCenter() override {
			return this->position;
		}

		void setOrtho(const float width, const float height) override {
			bound.set(
				position.x - width * 0.5f, position.y - height * 0.5f,
				width, height
			);
		}

		Geom::OrthoRectFloat& getPorjectedBound() override {
			return bound;
		}
	};
}
