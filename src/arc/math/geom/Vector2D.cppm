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

		using PassType = Concepts::ParamPassType<Vector2D, sizeof(T) * 2>;

		[[nodiscard]] constexpr Vector2D operator+(const PassType tgt) const noexcept {
			return {x + tgt.x, y + tgt.y};
		}

		[[nodiscard]] constexpr Vector2D operator-(const PassType tgt) const noexcept {
			return {x - tgt.x, y - tgt.y};
		}

		[[nodiscard]] constexpr Vector2D operator*(const PassType tgt) const noexcept {
			return {x * tgt.x, y * tgt.y};
		}

		[[nodiscard]] constexpr Vector2D operator*(const T val) const noexcept {
			return {x * val, y * val};
		}

		[[nodiscard]] constexpr Vector2D operator-() const noexcept {
			return {- x, - y};
		}

		[[nodiscard]] constexpr Vector2D operator/(const T val) const noexcept {
			return {x / val, y / val};
		}

		[[nodiscard]] constexpr Vector2D operator/(const PassType tgt) const noexcept {
			return {x / tgt.x, y / tgt.y};
		}

		[[nodiscard]] constexpr Vector2D operator%(const PassType tgt) const noexcept {
			return {Math::mod(x, tgt.x), Math::mod(y, tgt.y)};
		}

		constexpr Vector2D& operator+=(const PassType tgt) noexcept {
			return this->add(tgt);
		}

		constexpr Vector2D operator~() noexcept {
			if constexpr (std::is_floating_point_v<T>){
				return {static_cast<T>(1) / x, static_cast<T>(1) / y};
			}else{
				return {~x, ~y};
			}
		}

		constexpr Vector2D& inverse() noexcept{
			return this->set(this->operator~());
		}

		constexpr Vector2D& operator+=(const T tgt) noexcept {
			return this->add(tgt);
		}

		constexpr Vector2D& operator-=(const PassType tgt) noexcept {
			return this->sub(tgt);
		}

		constexpr Vector2D& operator*=(const PassType tgt) noexcept {
			return this->mul(tgt);
		}

		constexpr Vector2D& operator*=(const T val) noexcept {
			return this->scl(val);
		}

		constexpr Vector2D& operator/=(const PassType tgt) noexcept {
			return this->sub(tgt);
		}

		constexpr Vector2D& operator/=(const T tgt) noexcept {
			return this->div(tgt, tgt);
		}

		constexpr Vector2D& operator%=(const PassType tgt) noexcept {
			return this->mod(tgt.x, tgt.y);
		}

		constexpr Vector2D& operator%=(const T tgt) noexcept {
			return this->mod(tgt, tgt);
		}

		constexpr Vector2D& mod(const T ox, const T oy) noexcept {
			x = Math::mod(x, ox);
			y = Math::mod(y, oy);
			return *this;
		}

		constexpr Vector2D& mod(const T val) noexcept {
			return this->mod(val, val);
		}

		constexpr Vector2D& mod(const PassType other) noexcept {
			return this->mod(other.x, other.y);
		}

		[[nodiscard]] constexpr Vector2D copy() const noexcept {
			return Vector2D{ x, y };
		}

		constexpr Vector2D& setZero() noexcept {
			return this->set(static_cast<T>(0), static_cast<T>(0));
		}

		constexpr Vector2D& setNaN() noexcept requires std::is_floating_point_v<T> {
			return set(std::numeric_limits<float>::signaling_NaN(), std::numeric_limits<float>::signaling_NaN());
		}

		constexpr friend std::size_t hash_value(const Vector2D& obj) requires requires{sizeof(T) <= 4;} {
			return obj.hash_value();
		}

		constexpr std::size_t hash_value() const requires requires{sizeof(T) <= 4;}{
			const std::size_t l = std::bit_cast<unsigned>(x);
			const std::size_t r = std::bit_cast<unsigned>(y);
			return l << 32 | r;
		}

		constexpr Vector2D& set(const T ox, const T oy) noexcept {
			this->x = ox;
			this->y = oy;

			return *this;
		}

		constexpr Vector2D& set(const T val) noexcept {
			return this->set(val, val);
		}

		constexpr Vector2D& set(const PassType tgt) noexcept {
			return this->set(tgt.x, tgt.y);
		}

		constexpr Vector2D& add(const T ox, const T oy) noexcept {
			x += ox;
			y += oy;

			return *this;
		}

		constexpr Vector2D& add(const T val) noexcept {
			x += val;
			y += val;

			return *this;
		}

		constexpr Vector2D& add(const PassType other) noexcept {
			return this->add(other.x, other.y);
		}

		constexpr Vector2D& mulAdd(const PassType other, const T scale) noexcept {
			return this->add(other.x * scale, other.y * scale);
		}

		constexpr Vector2D& sub(const T ox, const T oy) noexcept {
			x -= ox;
			y -= oy;

			return *this;
		}

		constexpr Vector2D& sub(const PassType other) noexcept {
			return this->sub(other.x, other.y);
		}

		constexpr Vector2D& sub(const PassType other, const T scale) noexcept {
			return this->sub(other.x * scale, other.y * scale);
		}

		constexpr Vector2D& mul(const T ox, const T oy) noexcept {
			x *= ox;
			y *= oy;

			return *this;
		}

		constexpr Vector2D& mul(const T val) noexcept {
			return this->mul(val, val);
		}

		constexpr Vector2D& inv() noexcept requires Concepts::Signed<T> {
			x = -x;
			y = -y;
			return *this;
		}

		constexpr Vector2D& mul(const PassType other) noexcept {
			return this->mul(other.x, other.y);
		}

		constexpr Vector2D& div(const T ox, const T oy) noexcept {
			x /= ox;
			y /= oy;

			return *this;
		}

		constexpr Vector2D& div(const T val) noexcept {
			return this->div(val, val);
		}

		constexpr Vector2D& div(const PassType other) noexcept {
			return this->div(other.x, other.y);
		}

		[[nodiscard]] constexpr T getX() const noexcept{
			return x;
		}

		[[nodiscard]] constexpr T getY() const noexcept{
			return y;
		}

		constexpr Vector2D& setX(const T ox) noexcept{
			this->x = ox;
			return *this;
		}

		constexpr Vector2D& setY(const T oy) noexcept{
			this->y = oy;
			return *this;
		}

		[[nodiscard]] constexpr T dst2(const PassType other) const noexcept {
			T dx = Math::safeDst(x, other.x);
			T dy = Math::safeDst(y, other.y);

			return dx * dx + dy * dy;
		}

		[[nodiscard]] constexpr float dst(const PassType other) const noexcept{
			return std::sqrt(static_cast<float>(this->dst2(other)));
		}

		[[nodiscard]] constexpr float dst(const float tx, const float ty) const noexcept{
			return Math::dst(x, y, tx, ty);
		}

		[[nodiscard]] constexpr float dst2(const float tx, const float ty) const noexcept{
			return Math::dst2(x, y, tx, ty);
		}

		[[nodiscard]] constexpr bool within(const PassType other, const T dst) const noexcept{
			return this->dst2(other) < dst * dst;
		}

		[[nodiscard]] bool isNaN() const noexcept{
			if constexpr(std::is_floating_point_v<T>) {
				return std::isnan(x) || std::isnan(y);
			} else {
				return false;
			}
		}

		[[nodiscard]] bool isInf() const noexcept{
			return std::isinf(x) || std::isinf(y);
		}

		[[nodiscard]] float length() const noexcept {
			return std::sqrt(static_cast<float>(length2()));
		}

		[[nodiscard]] constexpr T length2() const noexcept {
			return x * x + y * y;
		}

		[[nodiscard]] float angle() const noexcept {
			float angle = Math::atan2(static_cast<float>(x), static_cast<float>(y)) * Math::RADIANS_TO_DEGREES;
			return angle;
		}

		[[nodiscard]] float angleTo(const PassType reference) const noexcept {
			return (reference - *this).angle();
		}

		[[nodiscard]] float angleBetween(const PassType reference) const noexcept {
			return Math::atan2(this->cross(reference), this->dot(reference)) * Math::RADIANS_TO_DEGREES;
		}

		Vector2D& normalize() noexcept {
			return div(length());
		}

		Vector2D& normalizeToBase() noexcept{
			x = Math::sign<T>(x);
			y = Math::sign<T>(y);

			return *this;
		}

		Vector2D& rotateRad(const float rad) noexcept{
			//  Matrix Multi
			//  cos rad		-sin rad	x    crx   -sry
			//	sin rad		 cos rad	y	 srx	cry
			return rotate(Math::cos(rad), Math::sin(rad));
		}

		constexpr Vector2D& rotate(const float cos, const float sin) noexcept{
			if constexpr(std::is_floating_point_v<T>) {
				return this->set(cos * x - sin * y, sin * x + cos * y);
			}else {
				return this->set(
					static_cast<T>(cos * static_cast<float>(x) - sin * static_cast<float>(y)),
					static_cast<T>(sin * static_cast<float>(x) + cos * static_cast<float>(y))
				);
			}
		}

		Vector2D& rotate(const float degree) noexcept{
			return rotateRad(degree * Math::DEGREES_TO_RADIANS);
		}

		Vector2D& lerp(const PassType tgt, const float alpha) noexcept{
			return this->set(Math::lerp(x, tgt.x, alpha), Math::lerp(y, tgt.y, alpha));
		}

		Vector2D& approach(const PassType target, const float alpha) noexcept{
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

		[[nodiscard]] static constexpr Vector2D byPolar(const float angDeg, const float length) noexcept{
			return {length * Math::cos(angDeg * Math::DEGREES_TO_RADIANS), length * Math::sin(angDeg * Math::DEGREES_TO_RADIANS)};
		}

		[[nodiscard]] static constexpr Vector2D byPolarRad(const float angRad, const float length) noexcept{
			return {length * Math::cos(angRad), length * Math::sin(angRad)};
		}

		constexpr Vector2D& setPolar(const float angDeg, const float length) noexcept{
			return set(length * Math::cos(angDeg * Math::DEGREES_TO_RADIANS), length * Math::sin(angDeg * Math::DEGREES_TO_RADIANS));
		}

		constexpr Vector2D& setPolar(const float angDeg) noexcept{
			return setPolar(angDeg, length());
		}

		[[nodiscard]] constexpr T dot(const PassType tgt) const noexcept{
			return x * tgt.x + y * tgt.y;
		}

		[[nodiscard]] constexpr T cross(const PassType tgt) const noexcept{
			return x * tgt.y - y * tgt.x;
		}

		[[nodiscard]] constexpr Vector2D cross(T val) const noexcept{
			return {-y * val, x * val};
		}

		constexpr Vector2D& project_normalized(const PassType tgt) noexcept{
			float scl = this->dot(tgt);

			return this->set(tgt).mul(scl / tgt.length2());
		}

		constexpr Vector2D& project(const PassType tgt) noexcept {
			const float scl = this->dot(tgt);

			return this->set(tgt.x * scl, tgt.y * scl);
		}

		[[nodiscard]] constexpr float projLen2(const PassType axis) const noexcept{
			const float dot = this->dot(axis);
			return dot * dot / axis.length2();
		}

		[[nodiscard]] float projLen(const PassType axis) const noexcept{
			return std::sqrt(this->projLen2(axis));
		}

		friend void swap(Vector2D& lhs, Vector2D& rhs) noexcept {
			const float x1 = lhs.x;
			const float y1 = lhs.y;

			lhs.x = rhs.x;
			lhs.y = rhs.y;
			rhs.x = x1;
			rhs.y = y1;
		}

		[[nodiscard]] friend constexpr bool operator==(const Vector2D& lhs, const Vector2D& rhs) noexcept {
			return lhs.x == rhs.x && lhs.y == rhs.y;
		}

		[[nodiscard]] friend constexpr bool operator!=(const Vector2D& lhs, const Vector2D& rhs) noexcept {
			return lhs.x != rhs.x || lhs.y != rhs.y;
		}

		Vector2D& clampLength(const T min, const T max) noexcept {
			if (const float len2 = length2(); len2 >= max * max) {
				return this->limit(max);
			}

			else if (len2 <= min * min) {
				return this->setLength(min);
			}

			return *this;
		}

		constexpr Vector2D& clampX(const T min, const T max) noexcept {
			x = Math::clamp(x, min, max);
			return *this;
		}

		constexpr Vector2D& clampY(const T min, const T max) noexcept {
			y = Math::clamp(y, min, max);
			return *this;
		}

		constexpr Vector2D& minX(const T min) noexcept {
			x = Math::min(x, min);
			return *this;
		}

		constexpr Vector2D& minY(const T min) noexcept {
			y = Math::min(y, min);
			return *this;
		}

		constexpr Vector2D& maxX(const T max) noexcept {
			x = Math::max(x, max);
			return *this;
		}

		constexpr Vector2D& maxY(const T max) noexcept {
			y = Math::max(y, max);
			return *this;
		}

		constexpr Vector2D& clampNormalized() noexcept requires std::floating_point<T>{
			return clampX(0, 1).clampY(0, 1);
		}


		Vector2D& toAbs() noexcept {
			if constexpr(!std::is_unsigned_v<T>) {
				x = Math::abs(x);
				y = Math::abs(y);
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

		Vector2D& limit(const T xAbs, const T yAbs) noexcept {
			this->limitX(xAbs);
			this->limitY(yAbs);
			return *this;
		}

		Vector2D& limit(const T limit) noexcept {
			return this->limit2(limit * limit);
		}

		Vector2D& limit2(const T limit2) noexcept {
			if (const float len2 = length2(); len2 > limit2) {
				return this->scl(std::sqrt(static_cast<float>(limit2) / static_cast<float>(len2)));
			}

			return *this;
		}

		constexpr Vector2D& scl(const T val) noexcept {
			return this->scl(val, val);
		}

		constexpr Vector2D& scl(const T ox, const T oy) noexcept {
			x *= ox;
			y *= oy;
			return *this;
		}

		Vector2D& clamp(const T min, const T max) noexcept {
			const T len2 = length2();
			if (len2 == 0) [[unlikely]] return *this;  // NOLINT(clang-diagnostic-float-equal)
			if (const T max2 = max * max; len2 > max2) return this->scl(std::sqrt(max2 / len2));
			if (const T min2 = min * min; len2 < min2) return this->scl(std::sqrt(min2 / len2));
			return *this;
		}

		Vector2D& clampMin(const T min) noexcept {
			const T len2 = length2();
			if (len2 == 0) [[unlikely]] return *this;  // NOLINT(clang-diagnostic-float-equal)
			if (const T min2 = min * min; len2 < min2) [[likely]] return this->scl(std::sqrt(min2 / len2));
			return *this;
		}

		Vector2D& clampMax(const T max) noexcept {
			const T len2 = length2();
			if (len2 == 0) [[unlikely]] return *this;  // NOLINT(clang-diagnostic-float-equal)
			if (const T max2 = max * max; len2 > max2) [[likely]] return this->scl(std::sqrt(max2 / len2));
			return *this;
		}

		Vector2D& setLength(const T len) noexcept {
			return this->setLength2(len * len);
		}

		Vector2D& setLength2(const T len2) noexcept {
			const float oldLen2 = length2();
			return oldLen2 == 0 || oldLen2 == len2 ? *this : this->scl(std::sqrt(len2 / oldLen2));  // NOLINT(clang-diagnostic-float-equal)
		}

		[[nodiscard]] float angleRad() const noexcept{
			return Math::atan2(x, y);
		}

		[[nodiscard]] float angleExact() const noexcept{
			return std::atan2(y, x) * Math::DEGREES_TO_RADIANS;
		}

		/**
		 * \brief clockwise rotation
		 */
		constexpr Vector2D& rotateRT_clockwise() noexcept requires std::is_signed_v<T> {
			return this->set(-y, x);
		}

		/**
		 * \brief clockwise rotation
		 */
		constexpr Vector2D& rotateRT() noexcept requires std::is_signed_v<T> {
			return rotateRT_clockwise();
		}

		/**
		 * \brief counterclockwise rotation
		 */
		constexpr Vector2D& rotateRT_counterclockwise() noexcept requires std::is_signed_v<T> {
			return this->set(y, -x);
		}

		Vector2D& roundBy(const Vector2D::PassType other) noexcept requires std::is_floating_point_v<T>{
			x = Math::round<T>(x, other.x);
			y = Math::round<T>(y, other.y);

			return *this;
		}

		Vector2D& roundBy(const T val) noexcept requires std::is_floating_point_v<T>{
			x = Math::round<T>(x, val);
			y = Math::round<T>(y, val);

			return *this;
		}

		template <typename N>
		Vector2D<N> roundBy(const typename Vector2D<N>::PassType other) noexcept{
			Vector2D<N> tgt = as<N>();
			tgt.x = Math::round<N>(static_cast<N>(x), other.x);
			tgt.y = Math::round<N>(static_cast<N>(y), other.y);

			return tgt;
		}

		template <typename N>
		Vector2D<N> roundBy(const N val) noexcept{
			Vector2D<N> tgt = as<N>();
			tgt.x = Math::round<N>(static_cast<N>(x), val);
			tgt.y = Math::round<N>(static_cast<N>(y), val);

			return tgt;
		}

		template <std::integral N>
		Vector2D<N> trac() noexcept{
			return Vector2D<N>{Math::trac<N>(x), Math::trac<N>(y)};
		}

		[[nodiscard]] constexpr bool isZero() const noexcept {
			return length2() == static_cast<T>(0);
		}

		[[nodiscard]] constexpr bool isZero(const T margin) const noexcept{
			return length2() < margin;
		}

		[[nodiscard]] constexpr Vector2D sign() const noexcept{
			return {Math::sign(x), Math::sign(y)};
		}

		/*auto constexpr operator<=>(const PassType v) const noexcept {
			T len = length2();
			T lenO = v.length2();

			if(len > lenO) {
				return std::weak_ordering::greater;
			}

			if(len < lenO) {
				return std::weak_ordering::less;
			}

			return std::weak_ordering::equivalent;
		}*/

		template <Concepts::Number TN>
		[[nodiscard]] constexpr Vector2D<TN> as() const noexcept{
			return Vector2D<TN>{static_cast<TN>(x), static_cast<TN>(y)};
		}

		friend std::ostream& operator<<(std::ostream& os, const PassType obj) {
			return os << '(' << std::to_string(obj.x) << ", " << std::to_string(obj.y) << ')';
		}


	};

	using Vec2 = Vector2D<float>;
	using Point2 = Vector2D<int>;
	using Point2U = Vector2D<unsigned int>;
	using Point2S = Vector2D<short>;
	using Point2US = Vector2D<unsigned short>;

	template <typename N>
	constexpr Vector2D<N> norBaseVec2{1, 1};

	template <typename N>
	constexpr Vector2D<N> zeroVec2{0, 0};

	template <typename N>
	constexpr Vector2D<N> norXVec2{1, 0};

	template <typename N>
	constexpr Vector2D<N> norYVec2{0, 1};

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
			return *reinterpret_cast<const size_t*>(&v);
		}
	};

export
	template<>
	struct std::hash<Geom::Point2>{
		size_t operator()(const Geom::Point2& v) const noexcept {
			return *reinterpret_cast<const size_t*>(&v);
		}
	};

export
	template<>
	struct std::hash<Geom::Point2U>{
		size_t operator()(const Geom::Point2U& v) const noexcept {
			return *reinterpret_cast<const size_t*>(&v);
		}
	};


// template <template <typename T> typename V, typename T>
// 	requires std::convertible_to<V<T>, Geom::Vector2D<T>>
// struct formatter_base{
// 	constexpr auto parse(std::format_parse_context& context) const{
// 		return context.begin();
// 	}
//
// 	auto format(const V<T>& p, auto& context) const{
// 		return std::format_to(context.out(), "({}, {})", p.x, p.y);
// 	}
// };
//
//
// export template <>
// struct std::formatter<Geom::Vec2> : formatter_base<Geom::Vector2D, float>{
// 	using formatter_base::parse;
// 	using formatter_base::format;
// };


