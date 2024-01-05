//
// Created by Matrix on 2023/11/18.
//

module ;

export module Geom.Vector2D;

import <complex>;
import <algorithm>;
import <type_traits>;

export import Geom.Position;
export import Geom.Vector3D;
export import Math;
export import Math.Bit;

import Concepts;

export namespace Geom{

	template <Concepts::Number T>
	struct Vector2D
	{
		T x = 0, y = 0;

		constexpr Vector2D(const T x, const T y) : x(x), y(y) {
		}

		constexpr Vector2D() : Vector2D(0, 0) {

		}

		Vector2D operator+(const Vector2D& tgt) {
			return this->copy().add(tgt);
		}

		Vector2D operator-(const Vector2D& tgt) {
			return this->copy().sub(tgt);
		}

		Vector2D operator*(const Vector2D& tgt) {
			return this->copy().mul(tgt);
		}

		Vector2D operator/(const Vector2D& tgt) {
			return this->copy().sub(tgt);
		}

		Vector2D operator%(const Vector2D& tgt) {
			return this->copy().mod(tgt.x, tgt.y);
		}

		Vector2D& operator+=(const Vector2D& tgt) {
			return this->add(tgt);
		}

		Vector2D& operator-=(const Vector2D& tgt) {
			return this->sub(tgt);
		}

		Vector2D& operator*=(const Vector2D& tgt) {
			return this->mul(tgt);
		}

		Vector2D& operator*=(const T val) {
			return this->scl(val);
		}

		Vector2D& operator/=(const Vector2D& tgt) {
			return this->sub(tgt);
		}

		Vector2D& operator/=(const T tgt) {
			return this->div(tgt, tgt);
		}

		Vector2D& operator%=(const Vector2D& tgt) {
			return this->mod(tgt.x, tgt.y);
		}

		Vector2D& operator%=(const T tgt) {
			return this->mod(tgt, tgt);
		}

		Vector2D& mod(const T ox, const T oy) {
			x = Math::mod(x, ox);
			y = Math::mod(y, oy);
			return *this;
		}

		Vector2D& mod(const T val) {
			return this->mod(val, val);
		}

		Vector2D& mod(const Vector2D& other) {
			return this->mod(other.x, other.y);
		}

		[[nodiscard]] Vector2D copy() const {
			return Vector2D{ x, y };
		}

		Vector2D& setZero() {
			return this->set(static_cast<T>(0), static_cast<T>(0));
		}

		Vector2D& setNaN() requires std::is_floating_point_v<T> {
			return set(NAN, NAN);
		}

		Vector2D& set(const T ox, const T oy) {
			this->x = ox;
			this->y = oy;

			return *this;
		}

		Vector2D& set(const T val) {
			return this->set(val, val);
		}

		Vector2D& set(const Vector2D& tgt) {
			return this->set(tgt.x, tgt.y);
		}

		Vector2D& add(const T ox, const T oy) {
			x += ox;
			y += oy;

			return *this;
		}

		Vector2D& add(const Vector2D& other) {
			return this->add(other.x, other.y);
		}

		Vector2D& sub(const T ox, const T oy) {
			x -= ox;
			y -= oy;

			return *this;
		}

		Vector2D& sub(const Vector2D& other) {
			return this->sub(other.x, other.y);
		}

		Vector2D& mul(const T ox, const T oy) {
			x *= ox;
			y *= oy;

			return *this;
		}

		Vector2D& mul(const T val) {
			return this->mul(val, val);
		}

		Vector2D& mul(const Vector2D& other) {
			return this->mul(other.x, other.y);
		}

		Vector2D& div(const T ox, const T oy) {
			x /= ox;
			y /= oy;

			return *this;
		}

		Vector2D& div(const T val) {
			return this->div(val, val);
		}

		Vector2D& div(const Vector2D& other) {
			return this->div(other.x, other.y);
		}

		[[nodiscard]] T getX() const{
			return x;
		}

		[[nodiscard]] T getY() const{
			return y;
		}

		Vector2D& setX(const T ox){
			this->x = ox;
			return *this;
		}

		Vector2D& setY(const T oy){
			this->y = oy;
			return *this;
		}

		[[nodiscard]] T dst2(const Vector2D& other) const {
			T dx = Math::safeDst(x, other.x);
			T dy = Math::safeDst(y, other.y);

			return dx * dx + dy * dy;
		}

		[[nodiscard]] float dst(const Vector2D& other) const{
			return std::sqrtf(static_cast<float>(this->dst2(other)));
		}

		[[nodiscard]] bool within(const Vector2D& other, const T dst) const{
			return this->dst2(other) < dst * dst;
		}

		[[nodiscard]] bool isNaN() const{
			if constexpr(std::is_floating_point_v<T>) {
				return std::isnan(x) || std::isnan(y);
			} else {
				return false;
			}
		}

		[[nodiscard]] float length() const {
			return std::sqrt(static_cast<float>(length2()));
		}

		[[nodiscard]] T length2() const {
			return x * x + y * y;
		}

		[[nodiscard]] float angle() const {
			float angle = Math::atan2(static_cast<float>(x), static_cast<float>(y)) * Math::RADIANS_TO_DEGREES;
			if (angle < 0) angle += 360;
			return angle;
		}

		[[nodiscard]] float angle(const Vector2D& reference) const {
			return Math::atan2(cross(reference), dot(reference)) * Math::RADIANS_TO_DEGREES;
		}

		Vector2D& normalize() {
			return div(length());
		}

		Vector2D& rotateRad(const float rad) {
			//  Matrix Multi
			//  cos rad		-sin rad	x    crx   -sry
			//	sin rad		 cos rad	y	 srx	cry
			return rotate(Math::cos(rad), Math::sin(rad));
		}

		Vector2D& rotate(const float cos, const float sin) {
			if constexpr(std::is_floating_point_v<T>) {
				return this->set(cos * x - sin * y, sin * x + cos * y);
			}else {
				return this->set(
					static_cast<T>(cos * static_cast<float>(x) - sin * static_cast<float>(y)),
					static_cast<T>(sin * static_cast<float>(x) + cos * static_cast<float>(y))
				);
			}
		}

		Vector2D& rotate(const float degree) {
			return rotateRad(degree * Math::DEGREES_TO_RADIANS);
		}

		Vector2D& lerp(const Vector2D& tgt, const float alpha) {
			return this->set(Math::lerp(x, tgt.x, alpha), Math::lerp(y, tgt.y, alpha));
		}

		Vector2D& approach(const Vector2D& target, const float alpha) {
			float dx = x - target.x, dy = y - target.y;
			const float alpha2 = alpha * alpha;

			if (const float len2 = dx * dx + dy * dy; len2 > alpha2) {
				const float scl = sqrt(alpha2 / len2);
				dx *= scl;
				dy *= scl;

				return sub(dx, dy);
			}

			return set(target);
		}

		Vector2D& setPolar(const float angDeg, const float length) {
			return set(length * Math::cos(angDeg * Math::DEGREES_TO_RADIANS), length * Math::sin(angDeg * Math::DEGREES_TO_RADIANS));
		}

		Vector2D& setPolar(const float angDeg) {
			return setPolar(angDeg, length());
		}

		[[nodiscard]] float dot(const Vector2D& tgt) const {
			return x * tgt.x + y * tgt.y;
		}

		[[nodiscard]] float cross(const Vector2D& tgt) const {
			return x * tgt.y - y * tgt.x;
		}

		Vector2D& project(const Vector2D& tgt) {
			float scl = this->dot(tgt) / this->length();

			return this->set(tgt).mul(scl / tgt.length2());
		}

		friend void swap(Vector2D& lhs, Vector2D& rhs) noexcept {
			const float x1 = lhs.x;
			const float y1 = lhs.y;

			lhs.x = rhs.x;
			lhs.y = rhs.y;
			rhs.x = x1;
			rhs.y = y1;
		}

		friend bool operator==(const Vector2D& lhs, const Vector2D& rhs) {
			return lhs.x == rhs.x && lhs.y == rhs.y;
		}

		friend bool operator!=(const Vector2D& lhs, const Vector2D& rhs) {
			return lhs.x != rhs.x || lhs.y != rhs.y;
		}

		[[nodiscard]] Vector3D expandTo3D(const T z) const {
			return Vector3D{ x, y, z };
		}

		[[nodiscard]] Vector3D expandTo3D() const {
			return expandTo3D(0);
		}

		Vector2D& clampLength(const T min, const T max) {
			if (const float len2 = length2(); len2 >= max * max) {
				return this->limit(max);
			}

			else if (len2 <= min * min) {
				return this->setLength(min);
			}

			return *this;
		}

		Vector2D& clampX(const T min, const T max) {
			x = Math::clamp(x, min, max);
			return *this;
		}

		Vector2D& clampY(const T min, const T max) {
			y = Math::clamp(y, min, max);
			return *this;
		}

		Vector2D& abs() {
			if constexpr(!std::is_unsigned_v<T>) {
				x = std::abs(x);
				y = std::abs(y);
			}

			return *this;
		}

		Vector2D& limitY(const T yAbs) {
			const float yMax = Math::min(Math::abs(y), yAbs);
			y = std::copysign(yMax, y);
			return *this;
		}

		Vector2D& limitX(const T xAbs) {
			const float xMax = Math::min(Math::abs(x), xAbs);
			x = std::copysign(xMax, x);
			return *this;
		}

		Vector2D& limit(const T xAbs, const T yAbs) {
			this->limitX(xAbs);
			this->limitY(yAbs);
			return *this;
		}

		Vector2D& limit(const T limit) {
			return this->limit2(limit * limit);
		}

		Vector2D& limit2(const T limit2) {
			if (const float len2 = length2(); len2 > limit2) {
				return this->scl(std::sqrt(static_cast<float>(limit2) / static_cast<float>(len2)));
			}

			return *this;
		}

		Vector2D& scl(const T val) {
			return this->scl(val, val);
		}

		Vector2D& scl(const T ox, const T oy) {
			x *= ox;
			y *= oy;
			return *this;
		}

		Vector2D& clamp(const T min, const T max) {
			const float len2 = length2();
			if (len2 == 0) return *this;  // NOLINT(clang-diagnostic-float-equal)
			if (const float max2 = max * max; len2 > max2) return scl(std::sqrt(max2 / len2));
			if (const float min2 = min * min; len2 < min2) return scl(std::sqrt(min2 / len2));
			return *this;
		}

		Vector2D& setLength(const T len) {
			return this->setLength2(len * len);
		}

		Vector2D& setLength2(const T len2) {
			const float oldLen2 = length2();
			return oldLen2 == 0 || oldLen2 == len2 ? *this : this->scl(std::sqrt(len2 / oldLen2));  // NOLINT(clang-diagnostic-float-equal)
		}

		[[nodiscard]] float angleRad() const {
			return Math::atan2(y, x);
		}

		[[nodiscard]] float angleExact() const {
			return std::atan2(y, x) * Math::DEGREES_TO_RADIANS;
		}

		/**
		 * \brief
		 * \return
		 */
		Vector2D& rotateRT() requires std::is_signed_v<T> {
			return this->set(y, -x);
		}

		auto operator<=>(const Vector2D& v) const {
			T len = length2();
			T lenO = v.length2();

			if(len > lenO) {
				return std::strong_ordering::greater;
			}

			if(len < lenO) {
				return std::strong_ordering::less;
			}

			return std::strong_ordering::equivalent;
		}

		friend std::ostream& operator<<(std::ostream& os, const Vector2D& obj){
			return os << '(' << std::to_string(obj.x) << ", " << std::to_string(obj.y) << ')';
		}
	};

	using Vec2 = Vector2D<float>;
	using Point = Vector2D<int>;
	using PointU = Vector2D<unsigned int>;
	using PointS = Vector2D<short>;
	using PointUS = Vector2D<unsigned short>;

	constexpr Vec2 ZERO{ 0, 0 };
	constexpr Vec2 X{ 1, 0 };
	constexpr Vec2 Y{ 0, 1 };
}

export {
	template<>
	struct std::hash<Geom::Vec2>{
		size_t operator()(const Geom::Vec2& v) const noexcept {
			return *reinterpret_cast<const size_t*>(&v.x);
		}
	};

	template<>
	struct std::hash<Geom::Point>{
		size_t operator()(const Geom::Point& v) const noexcept {
			return *reinterpret_cast<const size_t*>(&v.x);
		}
	};

	template<>
	struct std::hash<Geom::PointU>{
		size_t operator()(const Geom::PointU& v) const noexcept {
			return *reinterpret_cast<const size_t*>(&v.x);
		}
	};
}

