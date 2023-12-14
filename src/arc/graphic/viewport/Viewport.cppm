module;

export module Graphic.Viewport;

import Concepts;

//TODO Mat4D support!
export import Geom.Matrix3D;
export import Geom.Vector3D;
export import Geom.Vector2D;

export namespace Graphic {
	template <typename DimensionVec = Geom::Vector2D, typename DimensionMat = Geom::Matrix3D>
		requires Concepts::HasDefConstructor<DimensionVec> && Concepts::HasDefConstructor<DimensionMat>
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

		virtual void apply(DimensionMat& projection) const = 0;

		virtual void setOrtho(float srcX, float srcY, float width, float height) = 0;

		virtual void setOrtho(float width, float height) = 0;
	};
}
