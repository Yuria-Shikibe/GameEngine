module;

export module Geom.Point2D;

import Concepts;
import <cmath>;
import Geom.Vector2D;
import Geom.Matrix3D;

export namespace Geom {
	template<Concepts::Number T>
	struct Point2D {
		T x{};
		T y{};

		[[nodiscard]] constexpr Point2D() = default;

		[[nodiscard]] constexpr Point2D(const T x, const T y)
			: x(x),
			y(y) {
		}

		Point2D& operator+=(const Point2D& tgt) {
			x += tgt.x;
			y += tgt.y;
			return *this;
		}

		Point2D operator+(const Point2D& tgt) {
			return {x + tgt.x, y + tgt.y};
		}

		Point2D& operator-=(const Point2D& tgt) {
			x -= tgt.x;
			y -= tgt.y;
			return *this;
		}

		Point2D& operator-(const Point2D& tgt) {
			return {x - tgt.x, y - tgt.y};
		}

		Point2D& operator*=(const Point2D& tgt) {
			x *= tgt.x;
			y *= tgt.y;
			return *this;
		}

		Point2D operator*(const Point2D& tgt) {
			return {x * tgt.x, y * tgt.y};
		}

		Point2D& operator/=(const Point2D& tgt) {
			x /= tgt.x;
			y /= tgt.y;
			return *this;
		}

		Point2D& operator/(const Point2D& tgt) {
			return {x / tgt.x, y / tgt.y};
		}

		Point2D& set(const T ox, const T oy) {
			this->x = ox;
			this->y = oy;

			return *this;
		}

		template<typename N1, typename N2>
		Point2D& set(const N1 ox, const N2 oy) {
			this->x = static_cast<T>(ox);
			this->y = static_cast<T>(oy);

			return *this;
		}

		Point2D& set(const T val) {
			return set(val, val);
		}

		Point2D& set(const Point2D& tgt) {
			return set(tgt.x, tgt.y);
		}

		Point2D& add(const T ox, const T oy) {
			x += ox;
			y += oy;

			return *this;
		}

		Point2D& add(const Point2D& other) {
			return add(other.x, other.y);
		}

		Point2D& sub(const T ox, const T oy) {
			x -= ox;
			y -= oy;

			return *this;
		}

		Point2D& sub(const Point2D& other) {
			return sub(other.x, other.y);
		}

		Point2D& mul(const T ox, const T oy) {
			x *= ox;
			y *= oy;

			return *this;
		}

		Point2D& mul(const T val) {
			return mul(val, val);
		}

		Point2D& mul(const Point2D& other) {
			return mul(other.x, other.y);
		}

		Point2D& setZero() {
			return set(static_cast<T>(0), static_cast<T>(0));
		}

		Point2D& div(const T ox, const T oy) {
			x /= ox;
			y /= oy;

			return *this;
		}

		Point2D& div(const T val) {
			return div(val, val);
		}

		Point2D& div(const Point2D& other) {
			return div(other.x, other.y);
		}

		[[nodiscard]] T getX() const{
			return x;
		}

		[[nodiscard]] T getY() const{
			return y;
		}

		void setX(const T ox){
			this->x = ox;
		}

		void setY(const T oy){
			this->y = oy;
		}

		Vector2D toVec() {
			return {static_cast<float>(x), static_cast<float>(y)};
		}

		friend bool operator==(const Point2D& lhs, const Point2D& rhs) {
			return lhs.x == rhs.x
			       && lhs.y == rhs.y;
		}

		friend bool operator!=(const Point2D& lhs, const Point2D& rhs) {
			return !(lhs == rhs);
		}

		Point2D& rotateRad(const float rad) {
			//  Matrix Multi
			//  cos rad		-sin rad	x    crx   -sry
			//	sin rad		 cos rad	y	 srx	cry
			const float cosRad = Math::cos(rad);
			const float sinRad = Math::sin(rad);

			return set(cosRad * x - sinRad * y, sinRad * x + cosRad * y);
		}

		Point2D& rotate(const float cos, const float sin) {
			return set(cos * x - sin * y, sin * x + cos * y);
		}

		[[nodiscard]] Point2D<float> copyRotate(const float cos, const float sin) const {
			return {cos * x - sin * y, sin * x + cos * y};
		}

		Point2D& rotate(const float degree) {
			return rotateRad(degree * Math::DEGREES_TO_RADIANS);
		}

		T dot(const Point2D& o) const {
			return x* o.x + y * o.y;
		}

		Point2D& rotateRT(){
			return set(y, -x);
		}

		operator Geom::Vector2D(){
			return Geom::Vector2D{x, y};
		}
	};

	using PointF2D = Point2D<float>;

	PointF2D& operator *=(PointF2D& vec2, const Matrix3D& mat) {
		return vec2.set(vec2.x * mat.val[0] + vec2.y * mat.val[3] + mat.val[6], vec2.x * mat.val[1] + vec2.y * mat.val[4] + mat.val[7]);
	}

	constexpr PointF2D pAxis_X = PointF2D{1, 0};
	constexpr PointF2D pAxis_Y = PointF2D{0, 1};
}
