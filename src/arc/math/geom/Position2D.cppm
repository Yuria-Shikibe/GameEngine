module ;

export module Geom.Position;

import <complex>;

export namespace Geom {
	class Position2D
	{
	public:
		virtual ~Position2D() = default;

		[[nodiscard]] virtual float getX() const = 0;
		[[nodiscard]] virtual float getY() const = 0;

		virtual void setX(float x) = 0;
		virtual void setY(float y) = 0;

		[[nodiscard]] float dst2(const Position2D& other) const {
			const float dx = getX() - other.getX();
			const float dy = getY() - other.getY();

			return dx * dx + dy * dy;
		}

		[[nodiscard]] float dst(const Position2D& other) const{
			return std::sqrt(dst2(other));
		}

		[[nodiscard]] bool within(const Position2D& other, const float dst) const{
			return dst2(other) < dst * dst;
		}

		[[nodiscard]] virtual bool isPosNaN() const{
			return !(isnan(getX()) || isnan(getY()));
		}
	};
}