//
// Created by Matrix on 2023/11/18.
//

module ;

export module Geom.Vector2D;

import std;

import Geom.Vector3D;
import Math;
import Math.Bit;
import Concepts;

export namespace Geom{
	template <Concepts::Number T>
	struct Vector2D
	{
		T x{0};
		T y{0};

		[[nodiscard]] constexpr Vector2D(const T x, const T y) : x(x), y(y) {
		}

		[[nodiscard]] constexpr Vector2D() = default;

		using PassType = Concepts::ParamPassType<Vector2D, sizeof(T) * 2>;

		[[nodiscard]] constexpr Vector2D operator+(const PassType tgt) const {
			return {x + tgt.x, y + tgt.y};
		}

		[[nodiscard]] constexpr Vector2D operator-(const PassType tgt) const {
			return {x - tgt.x, y - tgt.y};
		}

		[[nodiscard]] constexpr Vector2D operator*(const PassType tgt) const {
			return {x * tgt.x, y * tgt.y};
		}

		[[nodiscard]] constexpr Vector2D operator*(const T val) const {
			return {x * val, y * val};
		}

		[[nodiscard]] constexpr Vector2D operator-() const {
			return {- x, - y};
		}

		[[nodiscard]] constexpr Vector2D operator/(const T val) const {
			return {x / val, y / val};
		}

		[[nodiscard]] constexpr Vector2D operator/(const PassType tgt) const {
			return {x / tgt.x, y / tgt.y};
		}

		[[nodiscard]] constexpr Vector2D operator%(const PassType tgt) const {
			return {Math::mod(x, tgt.x), Math::mod(y, tgt.y)};
		}

		constexpr Vector2D& operator+=(const PassType tgt) {
			return this->add(tgt);
		}

		constexpr Vector2D& operator+=(const T tgt) {
			return this->add(tgt);
		}

		constexpr Vector2D& operator-=(const PassType tgt) {
			return this->sub(tgt);
		}

		constexpr Vector2D& operator*=(const PassType tgt) {
			return this->mul(tgt);
		}

		constexpr Vector2D& operator*=(const T val) {
			return this->scl(val);
		}

		constexpr Vector2D& operator/=(const PassType tgt) {
			return this->sub(tgt);
		}

		constexpr Vector2D& operator/=(const T tgt) {
			return this->div(tgt, tgt);
		}

		constexpr Vector2D& operator%=(const PassType tgt) {
			return this->mod(tgt.x, tgt.y);
		}

		constexpr Vector2D& operator%=(const T tgt) {
			return this->mod(tgt, tgt);
		}

		constexpr Vector2D& mod(const T ox, const T oy) {
			x = Math::mod(x, ox);
			y = Math::mod(y, oy);
			return *this;
		}

		constexpr Vector2D& mod(const T val) {
			return this->mod(val, val);
		}

		constexpr Vector2D& mod(const PassType other) {
			return this->mod(other.x, other.y);
		}

		constexpr [[nodiscard]] Vector2D copy() const {
			return Vector2D{ x, y };
		}

		constexpr [[nodiscard]] Vector2D copy(){
			return Vector2D{ x, y };
		}

		constexpr Vector2D& setZero() {
			return this->set(static_cast<T>(0), static_cast<T>(0));
		}

		constexpr Vector2D& setNaN() requires std::is_floating_point_v<T> {
			return set(std::numeric_limits<float>::signaling_NaN(), std::numeric_limits<float>::signaling_NaN());
		}

		constexpr Vector2D& set(const T ox, const T oy) {
			this->x = ox;
			this->y = oy;

			return *this;
		}

		constexpr Vector2D& set(const T val) {
			return this->set(val, val);
		}

		constexpr Vector2D& set(const PassType tgt) {
			return this->set(tgt.x, tgt.y);
		}

		constexpr Vector2D& add(const T ox, const T oy) {
			x += ox;
			y += oy;

			return *this;
		}

		constexpr Vector2D& add(const T val) {
			x += val;
			y += val;

			return *this;
		}

		constexpr Vector2D& add(const PassType other) {
			return this->add(other.x, other.y);
		}

		constexpr Vector2D& add(const PassType other, const T scale) {
			return this->add(other.x * scale, other.y * scale);
		}

		constexpr Vector2D& sub(const T ox, const T oy) {
			x -= ox;
			y -= oy;

			return *this;
		}

		constexpr Vector2D& sub(const PassType other) {
			return this->sub(other.x, other.y);
		}

		constexpr Vector2D& sub(const PassType other, const T scale) {
			return this->sub(other.x * scale, other.y * scale);
		}

		constexpr Vector2D& mul(const T ox, const T oy) {
			x *= ox;
			y *= oy;

			return *this;
		}

		constexpr Vector2D& mul(const T val) {
			return this->mul(val, val);
		}

		constexpr Vector2D& inv() requires Concepts::Signed<T> {
			x = -x;
			y = -y;
			return *this;
		}

		constexpr Vector2D& mul(const PassType other) {
			return this->mul(other.x, other.y);
		}

		constexpr Vector2D& div(const T ox, const T oy) {
			x /= ox;
			y /= oy;

			return *this;
		}

		constexpr Vector2D& div(const T val) {
			return this->div(val, val);
		}

		constexpr Vector2D& div(const PassType other) {
			return this->div(other.x, other.y);
		}

		[[nodiscard]] constexpr T getX() const{
			return x;
		}

		[[nodiscard]] constexpr T getY() const{
			return y;
		}

		constexpr Vector2D& setX(const T ox){
			this->x = ox;
			return *this;
		}

		constexpr Vector2D& setY(const T oy){
			this->y = oy;
			return *this;
		}

		[[nodiscard]] constexpr T dst2(const PassType other) const {
			T dx = Math::safeDst(x, other.x);
			T dy = Math::safeDst(y, other.y);

			return dx * dx + dy * dy;
		}

		[[nodiscard]] constexpr float dst(const PassType other) const{
			return std::sqrt(static_cast<float>(this->dst2(other)));
		}

		[[nodiscard]] constexpr float dst(const float tx, const float ty) const{
			return Math::dst(x, y, tx, ty);
		}

		[[nodiscard]] constexpr float dst2(const float tx, const float ty) const{
			return Math::dst2(x, y, tx, ty);
		}

		[[nodiscard]] constexpr bool within(const PassType other, const T dst) const{
			return this->dst2(other) < dst * dst;
		}

		[[nodiscard]] bool isNaN() const{
			if constexpr(std::is_floating_point_v<T>) {
				return std::isnan(x) || std::isnan(y);
			} else {
				return false;
			}
		}

		[[nodiscard]] bool isInf() const{
			return std::isinf(x) || std::isinf(y);
		}

		[[nodiscard]] float length() const {
			return std::sqrt(static_cast<float>(length2()));
		}

		[[nodiscard]] constexpr T length2() const {
			return x * x + y * y;
		}

		[[nodiscard]] float angle() const {
			float angle = Math::atan2(static_cast<float>(x), static_cast<float>(y)) * Math::RADIANS_TO_DEGREES;
			if (angle < 0) angle += 360;
			return angle;
		}

		[[nodiscard]] float angle(const PassType reference) const {
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

		constexpr Vector2D& rotate(const float cos, const float sin) {
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

		Vector2D& lerp(const PassType tgt, const float alpha) {
			return this->set(Math::lerp(x, tgt.x, alpha), Math::lerp(y, tgt.y, alpha));
		}

		Vector2D& approach(const PassType target, const float alpha) {
			float dx = x - target.x, dy = y - target.y;
			const float alpha2 = alpha * alpha;

			if (const float len2 = dx * dx + dy * dy; len2 > alpha2) {
				const float scl = std::sqrt(alpha2 / len2);
				dx *= scl;
				dy *= scl;

				return sub(dx, dy);
			}

			return this->set(target);
		}

		[[nodiscard]] static constexpr Vector2D byPolar(const float angDeg, const float length) {
			return {length * Math::cos(angDeg * Math::DEGREES_TO_RADIANS), length * Math::sin(angDeg * Math::DEGREES_TO_RADIANS)};
		}

		[[nodiscard]] static constexpr Vector2D byPolarRad(const float angRad, const float length) {
			return {length * Math::cos(angRad), length * Math::sin(angRad)};
		}

		constexpr Vector2D& setPolar(const float angDeg, const float length) {
			return set(length * Math::cos(angDeg * Math::DEGREES_TO_RADIANS), length * Math::sin(angDeg * Math::DEGREES_TO_RADIANS));
		}

		constexpr Vector2D& setPolar(const float angDeg) {
			return setPolar(angDeg, length());
		}

		[[nodiscard]] constexpr T dot(const PassType tgt) const {
			return x * tgt.x + y * tgt.y;
		}

		[[nodiscard]] constexpr T cross(const PassType tgt) const {
			return x * tgt.y - y * tgt.x;
		}

		[[nodiscard]] constexpr Vector2D cross(T val){
			return {-y * val, x * val};
		}

		constexpr Vector2D& project_normalized(const PassType tgt) {
			float scl = this->dot(tgt);

			return this->set(tgt).mul(scl / tgt.length2());
		}

		constexpr Vector2D& project(const PassType tgt) {
			const float scl = this->dot(tgt);

			return this->set(tgt.x * scl, tgt.y * scl);
		}

		[[nodiscard]] constexpr float projLen2(const PassType axis) const {
			const float dot = this->dot(axis);
			return dot * dot / axis.length2();
		}

		[[nodiscard]] float projLen(const PassType axis) const {
			return std::sqrt(this->projLen2(axis));
		}

		friend void swap(PassType lhs, PassType rhs) noexcept {
			const float x1 = lhs.x;
			const float y1 = lhs.y;

			lhs.x = rhs.x;
			lhs.y = rhs.y;
			rhs.x = x1;
			rhs.y = y1;
		}

		[[nodiscard]] friend bool operator==(const PassType lhs, const PassType rhs) {
			return lhs.x == rhs.x && lhs.y == rhs.y;
		}

		[[nodiscard]] friend bool operator!=(const PassType lhs, const PassType rhs) {
			return lhs.x != rhs.x || lhs.y != rhs.y;
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

		constexpr Vector2D& clampX(const T min, const T max) {
			x = Math::clamp(x, min, max);
			return *this;
		}

		constexpr Vector2D& clampY(const T min, const T max) {
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

		constexpr Vector2D& scl(const T val) {
			return this->scl(val, val);
		}

		constexpr Vector2D& scl(const T ox, const T oy) {
			x *= ox;
			y *= oy;
			return *this;
		}

		Vector2D& clamp(const T min, const T max) {
			const T len2 = length2();
			if (len2 == 0) return *this;  // NOLINT(clang-diagnostic-float-equal)
			if (const T max2 = max * max; len2 > max2) return this->scl(std::sqrt(max2 / len2));
			if (const T min2 = min * min; len2 < min2) return this->scl(std::sqrt(min2 / len2));
			return *this;
		}

		Vector2D& clampMin(const T min) {
			const T len2 = length2();
			if (len2 == 0) return *this;  // NOLINT(clang-diagnostic-float-equal)
			if (const T min2 = min * min; len2 < min2) return this->scl(std::sqrt(min2 / len2));
			return *this;
		}

		Vector2D& clampMax(const T max) {
			const T len2 = length2();
			if (len2 == 0) return *this;  // NOLINT(clang-diagnostic-float-equal)
			if (const T max2 = max * max; len2 > max2) return this->scl(std::sqrt(max2 / len2));
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
		 * \brief clockwise rotation
		 */
		constexpr Vector2D& rotateRT_clockwise() requires std::is_signed_v<T> {
			return this->set(-y, x);
		}

		/**
		 * \brief clockwise rotation
		 */
		constexpr Vector2D& rotateRT() requires std::is_signed_v<T> {
			return rotateRT_clockwise();
		}

		/**
		 * \brief counterclockwise rotation
		 */
		constexpr Vector2D& rotateRT_counterclockwise() requires std::is_signed_v<T> {
			return this->set(y, -x);
		}

		[[nodiscard]] constexpr bool isZero() const {
			return length2() == 0.0f;
		}

		[[nodiscard]] constexpr bool isZero(const float margin) const {
			return length2() < margin;
		}

		[[nodiscard]] constexpr Vector2D sign() const {
			return {Math::sign(x), Math::sign(y)};
		}

		auto constexpr operator<=>(const PassType v) const {
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

		friend std::ostream& operator<<(std::ostream& os, const PassType obj){
			return os << '(' << std::to_string(obj.x) << ", " << std::to_string(obj.y) << ')';
		}
	};

	using Vec2 = Vector2D<float>;
	using Point2 = Vector2D<int>;
	using Point2U = Vector2D<unsigned int>;
	using Point2S = Vector2D<short>;
	using Point2US = Vector2D<unsigned short>;

	constexpr Vec2 ZERO{ 0, 0 };
	constexpr Point2U ZERO_U{ 0u, 0u };
	constexpr Vec2 QNAN2{ std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN() };
	constexpr Vec2 SNAN2{ std::numeric_limits<float>::signaling_NaN(), std::numeric_limits<float>::signaling_NaN() };
	constexpr Vec2 X2{ 1, 0 };
	constexpr Vec2 Y2{ 0, 1 };
}

export
	template<>
	struct std::hash<Geom::Vec2>{
		size_t operator()(const Geom::Vec2& v) const noexcept {
			return *reinterpret_cast<const size_t*>(&v.x);
		}
	};

export
	template<>
	struct std::hash<Geom::Point2>{
		size_t operator()(const Geom::Point2& v) const noexcept {
			return *reinterpret_cast<const size_t*>(&v.x);
		}
	};

export
	template<>
	struct std::hash<Geom::Point2U>{
		size_t operator()(const Geom::Point2U& v) const noexcept {
			return *reinterpret_cast<const size_t*>(&v.x);
		}
	};


