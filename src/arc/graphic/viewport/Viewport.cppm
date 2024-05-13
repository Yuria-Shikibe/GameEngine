module;

export module Graphic.Viewport;

import ext.Concepts;

//TODO Mat4D support!
export import Geom.Matrix3D;
export import Geom.Vector3D;
export import Geom.Vector2D;
export import Geom.Rect_Orthogonal;

using Geom::OrthoRectFloat;

export namespace Graphic {
	template <typename DimensionVec = Geom::Vec2, typename DimensionMat = Geom::Matrix3D>
		requires Concepts::DefConstructable<DimensionVec> && Concepts::DefConstructable<DimensionMat>
	class Viewport {
	protected:
		DimensionVec position{};

	public:
		[[nodiscard]] Viewport() = default;

		virtual ~Viewport() = default;

		virtual void setPosition(const DimensionVec& vec) {
			position.set(vec);
		}

		[[nodiscard]] DimensionVec& getPosition() {
			return position;
		}

		[[nodiscard]] virtual DimensionVec& getCenter() = 0;

		virtual void modify(DimensionMat& projection) const = 0;

		virtual void setOrtho(float srcX, float srcY, float width, float height) = 0;

		virtual void setOrtho(float width, float height) = 0;

		virtual OrthoRectFloat& getPorjectedBound() = 0;
	};
}
