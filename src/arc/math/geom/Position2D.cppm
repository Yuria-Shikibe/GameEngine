module ;

export module Geom.Position;

import <complex>;

export namespace Geom {
	class Position  // NOLINT(cppcoreguidelines-special-member-functions)
	{
	public:
		virtual ~Position() = default;

		[[nodiscard]] virtual float getX() const = 0;
		[[nodiscard]] virtual float getY() const = 0;

		virtual void setX(float x) = 0;
		virtual void setY(float y) = 0;

		[[nodiscard]] float dst2(const Position& other) const {
			const float dx = getX() - other.getX();
			const float dy = getY() - other.getY();

			return dx * dx + dy * dy;
		}

		[[nodiscard]] float dst(const Position& other) const{
			return std::sqrt(dst2(other));
		}

		[[nodiscard]] bool within(const Position& other, const float dst) const{
			return dst2(other) < dst * dst;
		}

		[[nodiscard]] bool invalid() const{
			return !(isnan(getX()) || isnan(getY()));
		}
	};
}