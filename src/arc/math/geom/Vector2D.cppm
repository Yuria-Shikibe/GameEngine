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


export namespace Geom{

	struct Vector2D : Position
	{

		~Vector2D() override = default;

		float x = 0, y = 0;

		Vector2D(const float x, const float y) : x(x), y(y) {
		}

		Vector2D() : Vector2D(0, 0) {

		}

		explicit Vector2D(const size_t hash) {
			set(Math::unpackX_Float(hash), Math::unpackY_Float(hash));
		}

		Vector2D& operator+=(const Vector2D& tgt) {
			return add(tgt);
		}

		Vector2D& operator-=(const Vector2D& tgt) {
			return sub(tgt);
		}

		Vector2D& operator*=(const Vector2D& tgt) {
			return mul(tgt);
		}

		Vector2D& operator*=(const float val) {
			return scl(val);
		}

		Vector2D& operator/=(const Vector2D& tgt) {
			return sub(tgt);
		}

		Vector2D& operator/=(const float tgt) {
			return div(tgt, tgt);
		}

		Vector2D& operator%=(const Vector2D& tgt) {
			return mod(tgt.x, tgt.y);
		}

		Vector2D& operator%(const float tgt) {
			return mod(tgt, tgt);
		}

		Vector2D& mod(const float ox, const float oy) {
			x = fmod(x, ox);
			y = fmod(y, oy);
			return *this;
		}

		Vector2D& mod(const float val) {
			return mod(val, val);
		}

		Vector2D& mod(const Vector2D& other) {
			return mod(other.x, other.y);
		}

		Vector2D& map(void(*transformer)(Vector2D&)) {
			transformer(*this);
			return *this;
		}

		template <typename T>
		T& map(T& (*transformer)(Vector2D&)) {
			return transformer(*this);
		}

		template <typename T>
		T map(T(*transformer)(Vector2D&)) {
			return transformer(*this);
		}

		[[nodiscard]] Vector2D copy() const {
			return Vector2D{ x, y };
		}

		Vector2D& setZero() {
			return set(0.0f, 0.0f);
		}

		Vector2D& setNaN() {
			return set(NAN, NAN);
		}

		Vector2D& set(const float ox, const float oy) {
			this->x = ox;
			this->y = oy;

			return *this;
		}

		Vector2D& set(const float val) {
			return set(val, val);
		}

		Vector2D& set(const Vector2D& tgt) {
			return set(tgt.x, tgt.y);
		}

		Vector2D& add(const float ox, const float oy) {
			x += ox;
			y += oy;

			return *this;
		}

		Vector2D& add(const Vector2D& other) {
			return add(other.x, other.y);
		}

		Vector2D& sub(const float ox, const float oy) {
			x -= ox;
			y -= oy;

			return *this;
		}

		Vector2D& sub(const Vector2D& other) {
			return sub(other.x, other.y);
		}

		Vector2D& mul(const float ox, const float oy) {
			x *= ox;
			y *= oy;

			return *this;
		}

		Vector2D& mul(const float val) {
			return mul(val, val);
		}

		Vector2D& mul(const Vector2D& other) {
			return mul(other.x, other.y);
		}

		Vector2D& div(const float ox, const float oy) {
			x /= ox;
			y /= oy;

			return *this;
		}

		Vector2D& div(const float val) {
			return div(val, val);
		}

		Vector2D& div(const Vector2D& other) {
			return div(other.x, other.y);
		}

		[[nodiscard]] float getX() const override{
			return x;
		}

		[[nodiscard]] float getY() const override{
			return y;
		}

		void setX(const float ox) override{
			this->x = ox;
		}

		void setY(const float oy) override{
			this->y = oy;
		}

		[[nodiscard]] float length() const {
			return std::sqrt(length2());
		}

		[[nodiscard]] float length2() const {
			return x * x + y * y;
		}

		[[nodiscard]] float angle() const {
			float angle = Math::atan2(x, y) * Math::RADIANS_TO_DEGREES;
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
			const float cosRad = Math::cos(rad);
			const float sinRad = Math::sin(rad);

			return set(cosRad * x - sinRad * y, sinRad * x + cosRad * y);
		}

		Vector2D& rotate(const float degree) {
			return rotateRad(degree * Math::DEGREES_TO_RADIANS);
		}

		Vector2D& lerp(const Vector2D& tgt, const float alpha) {
			return set(x + (tgt.x - x) * alpha, y + (tgt.y - y) * alpha);
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
			return set(tgt).mul(dot(tgt) / length() / tgt.length2());
		}

		Vector2D& operator()(const float ox, const float oy) {
			return set(ox, oy);
		}

		[[nodiscard]] size_t hashCode() const {
			return Math::pack(x, y);
		}

		static size_t hash_code(const Vector2D& tgt) {
			return tgt.hashCode();
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

		[[nodiscard]] Vector3D expandTo3D(const float z) const {
			return Vector3D{ x, y, z };
		}

		[[nodiscard]] Vector3D expandTo3D() const {
			return expandTo3D(0);
		}

		Vector2D& clampLength(const float min, const float max) {
			if (const float len2 = length2(); len2 >= max * max) {
				return limit(max);
			}
			else if (len2 <= min * min) {
				return setLength(min);
			}

			return *this;
		}

		Vector2D& clampX(const float min, const float max) {
			x = std::clamp(x, min, max);
			return *this;
		}

		Vector2D& clampY(const float min, const float max) {
			y = std::clamp(y, min, max);
			return *this;
		}

		Vector2D& abs() {
			x = std::abs(x);
			y = std::abs(y);
			return *this;
		}

		Vector2D& limitY(const float yAbs) {
			const float yMax = std::fminf(std::abs(y), yAbs);
			y = std::copysign(yMax, y);
			return *this;
		}

		Vector2D& limitX(const float xAbs) {
			const float xMax = std::fminf(std::abs(x), xAbs);
			x = std::copysign(xMax, x);
			return *this;
		}

		Vector2D& limit(const float xAbs, const float yAbs) {
			limitX(xAbs);
			limitY(yAbs);
			return *this;
		}

		Vector2D& limit(const float limit) {
			return limit2(limit * limit);
		}

		Vector2D& limit2(const float limit2) {
			if (const float len2 = length2(); len2 > limit2) {
				return scl(sqrt(limit2 / len2));
			}
			return *this;
		}

		Vector2D& scl(const float val) {
			return scl(val, val);
		}

		Vector2D& scl(const float ox, const float oy) {
			x *= ox;
			y *= oy;
			return *this;
		}

		Vector2D& clamp(const float min, const float max) {
			const float len2 = length2();
			if (len2 == 0) return *this;  // NOLINT(clang-diagnostic-float-equal)
			if (const float max2 = max * max; len2 > max2) return scl(sqrt(max2 / len2));
			if (const float min2 = min * min; len2 < min2) return scl(sqrt(min2 / len2));
			return *this;
		}

		Vector2D& setLength(const float len) {
			return setLength2(len * len);
		}

		Vector2D& setLength2(const float len2) {
			const float oldLen2 = length2();
			return oldLen2 == 0 || oldLen2 == len2 ? *this : scl(sqrt(len2 / oldLen2));  // NOLINT(clang-diagnostic-float-equal)
		}

		[[nodiscard]] float angleRad() const {
			return Math::atan2(y, x);
		}

		[[nodiscard]] float angleExact() const {
			return Math::atan2(y, x) * Math::DEGREES_TO_RADIANS;
		}



		/**
		 * \brief
		 * \return
		 */
		Vector2D& rotateRT(){
			return set(y, -x);
		}

		friend bool operator<(const Vector2D& lhs, const Vector2D& rhs) {
			if (lhs.length2() < rhs.length2()) return true;
			return false;
		}

		friend bool operator<=(const Vector2D& lhs, const Vector2D& rhs) {
			return rhs >= lhs;
		}

		friend bool operator>(const Vector2D& lhs, const Vector2D& rhs) {
			return rhs < lhs;
		}

		friend bool operator>=(const Vector2D& lhs, const Vector2D& rhs) {
			return !(lhs < rhs);
		}

		friend std::ostream& operator<<(std::ostream& os, const Vector2D& obj){
			return os << '(' << obj.x << ", " << obj.y << ')';
		}
	};

	const Vector2D ZERO{ 0, 0 };
	const Vector2D X{ 1, 0 };
	const Vector2D Y{ 0, 1 };
}

export {
	template<>
	struct std::hash<Geom::Vector2D>{
		size_t operator()(const Geom::Vector2D& v) const noexcept {
			return *reinterpret_cast<const size_t*>(&v.x);
		}
	};
}

