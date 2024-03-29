export module Geom.Shape;

import std;
import Geom.Vector2D;
import Math;



export namespace Geom::Shape{

	template <class T, typename dataT>
	class Shape{
	protected:
		~Shape() = default;
		Shape() = default;

	public:
		virtual std::vector<Vec2>& vertices(std::vector<Vec2>& collector) const = 0;

		[[nodiscard]] virtual bool containsPos_edgeExclusive(const Vec2& v) const = 0;
		[[nodiscard]] virtual bool containsPos_edgeInclusive(const Vec2& v) const = 0;
		[[nodiscard]] virtual bool containsPos(const Vec2& v)const {
			return containsPos_edgeExclusive(v);
		}

		[[nodiscard]] virtual bool contains(const T& other) const = 0;
		[[nodiscard]] virtual bool overlap(const T& other) const = 0;
		[[nodiscard]] virtual bool dissociated(const T& other)const {
			return !this->overlap(other);
		}

		virtual dataT maxDiagonalSqLen() const = 0;
		virtual dataT maxDiagonalLen()const { return std::sqrt(maxDiagonalSqLen()); }

		Shape(const Shape& other) = default;

		Shape(Shape&& other) = default;

		Shape& operator=(const Shape& other) = default;

		Shape& operator=(Shape&& other) = default;
	};
}
