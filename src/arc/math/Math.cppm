export module Math;

import std;

import SinTable;
import Concepts;
import RuntimeException;

export namespace Math {
	constexpr int SIGNS[2]     = { -1, 1 };
	constexpr int ZERO_ONE[2]  = { 0, 1 };
	constexpr bool BOOLEANS[2] = { true, false };

	constexpr float FLOAT_ROUNDING_ERROR = 0.000001f;
	constexpr float PI                   = std::numbers::pi_v<float>;
	// ReSharper disable once CppInconsistentNaming
	constexpr float pi        = PI, HALF_PI = PI / 2.0f;
	constexpr double PI_EXACT = std::numbers::pi_v<double>; //3.14159265358979323846;
	constexpr float PI2       = PI * 2.0f;
	constexpr float E         = 2.7182818f;
	constexpr float SQRT2     = std::numbers::sqrt2_v<float>;
	constexpr float SQRT3     = std::numbers::sqrt3_v<float>;
	/** multiply by this to convert from radians to degrees */
	constexpr float RADIANS_TO_DEGREES = 180.0f / PI;
	constexpr float RAD_DEG            = RADIANS_TO_DEGREES;
	/** multiply by this to convert from degrees to radians */
	constexpr float DEGREES_TO_RADIANS = PI / 180.0f;
	constexpr float DEG_RAD            = DEGREES_TO_RADIANS;
	constexpr double DOUBLE_DEG_RAD    = 0.017453292519943295;
	constexpr double DOUBLE_RAD_DEG    = 57.29577951308232;

	constexpr unsigned int SIN_BITS  = 16;
	constexpr unsigned int SIN_MASK  = ~(-1 << SIN_BITS);
	constexpr unsigned int SIN_COUNT = SIN_MASK + 1;

	constexpr float RAD_FULL          = PI * 2;
	constexpr float DEG_FULL          = 360.0f;
	constexpr float RAD_TO_INDEX      = SIN_COUNT / RAD_FULL;
	constexpr float DEG_TO_INDEX      = SIN_COUNT / DEG_FULL;
	constexpr int BIG_ENOUGH_INT      = 16 * 1024;
	constexpr double BIG_ENOUGH_FLOOR = BIG_ENOUGH_INT;
	constexpr double CEIL             = 0.9999999;
	constexpr double BIG_ENOUGH_ROUND = static_cast<double>(BIG_ENOUGH_INT) + 0.5f;

	constexpr auto sinTable = genTable<SIN_MASK, DEG_TO_INDEX>();

	using Progress = float;

	[[nodiscard]] std::string printSinTable() {
		const SinTable<SIN_COUNT, SIN_MASK, DEG_TO_INDEX> table{};

		std::stringstream ss;
		ss << '{';
		for(const float sin : table.table) {
			std::string s = std::to_string(sin);
			ss << s;
			ss << "f,";
		}
		ss << "};";

		return ss.str();
	}

	/** Returns the sine in radians from a lookup table. */
	constexpr float sin(const float radians) {
		return sinTable[static_cast<int>(radians * RAD_TO_INDEX) & SIN_MASK];
	}

	constexpr float sin(const float radians, const float scl, const float mag) {
		return sin(radians / scl) * mag;
	}

	/** Returns the cosine in radians from a lookup table. */
	constexpr float cos(const float radians) {
		return sinTable[static_cast<int>((radians + PI / 2) * RAD_TO_INDEX) & SIN_MASK];
	}

	/** Returns the sine in radians from a lookup table. */
	constexpr float sinDeg(const float degrees) {
		return sinTable[static_cast<int>(degrees * DEG_TO_INDEX) & SIN_MASK];
	}

	/** Returns the cosine in radians from a lookup table. */
	constexpr float cosDeg(const float degrees) {
		return sinTable[static_cast<int>((degrees + 90) * DEG_TO_INDEX) & SIN_MASK];
	}

	constexpr float absin(const float in, const float scl, const float mag) {
		return (sin(in, scl * 2.0f, mag) + mag) / 2.0f;
	}

	constexpr float tan(const float radians, const float scl, const float mag) {
		return sin(radians / scl) / cos(radians / scl) * mag;
	}

	constexpr float cos(const float radians, const float scl, const float mag) {
		return cos(radians / scl) * mag;
	}

	float angleExact(const float x, const float y) {
		float result = std::atan2(y, x) * RAD_DEG;
		if(result < 0) result += DEG_FULL;
		return result;
	}

	/** Wraps the given angle to the range [-PI, PI]
	 * @param a the angle in radians
	 * @return the given angle wrapped to the range [-PI, PI] */
	float wrapAngleAroundZero(const float a) {
		if(a >= 0) {
			float rotation = std::fmod(a, PI2);
			if(rotation > PI) rotation -= PI2;
			return rotation;
		}

		float rotation = std::fmod(-a, PI2);
		if(rotation > PI) rotation -= PI2;
		return -rotation;
	}

	/** A variant on atan that does not tolerate infinite inputs for speed reasons, and because infinite inputs
	 * should never occur where this is used (only in {@link atan2(float, float)@endlink}).
	 * @param i any finite float
	 * @return an output from the inverse tangent function, from PI/-2.0 to PI/2.0 inclusive
	 * */
	float atn(const double i) {
		// We use double precision internally, because some constants need double precision.
		const double n = std::abs(i);
		// c uses the "equally-good" formulation that permits n to be from 0 to almost infinity.
		const double c = (n - 1.0) / (n + 1.0);
		// The approximation needs 6 odd powers of c.
		const double c2 = c * c, c3 = c * c2, c5 = c3 * c2, c7 = c5 * c2, c9 = c7 * c2, c11 = c9 * c2;
		return static_cast<float>
		(std::copysign(PI_EXACT * 0.25
		               + (0.99997726 * c - 0.33262347 * c3 + 0.19354346 * c5 - 0.11643287 * c7 + 0.05265332 * c9 -
		                  0.0117212 * c11), i));
	}

	/** Close approximation of the frequently-used trigonometric method atan2, with higher precision than libGDX's atan2
	 * approximation. Average error is 1.057E-6 radians; maximum error is 1.922E-6. Takes y and x (in that unusual order) as
	 * floats, and returns the angle from the origin to that point in radians. It is about 4 times faster than
	 * {@link atan2(double, double)@endlink} (roughly 15 ns instead of roughly 60 ns for Math, on Java 8 HotSpot). <br>
	 * Credit for this goes to the 1955 research study "Approximations for Digital Computers," by RAND Corporation. This is sheet
	 * 11's algorithm, which is the fourth-fastest and fourth-least precise. The algorithms on sheets 8-10 are faster, but only by
	 * a very small degree, and are considerably less precise. That study provides an atan method, and that cleanly
	 * translates to atan2().
	 * @param y y-component of the point to find the angle towards; note the parameter order is unusual by convention
	 * @param x x-component of the point to find the angle towards; note the parameter order is unusual by convention
	 * @return the angle to the given point, in radians as a float; ranges from -PI to PI */
	float atan2(float x, const float y) {
		float n = y / x;

		if(std::isnan(n)) {
			n = y == x ? 1.0f : -1.0f; // if both y and x are infinite, n would be NaN
		} else if(std::isinf(n)) {
			x = 0.0f; // if n is infinite, y is infinitely larger than x.
		}

		if(x > 0.0f) {
			return atn(n);
		}
		if(x < 0.0f) {
			return y >= 0 ? atn(n) + PI : atn(n) - PI;
		}
		if(y > 0.0f) {
			return x + HALF_PI;
		}
		if(y < 0.0f) {
			return x - HALF_PI;
		}

		return x + y; // returns 0 for 0,0 or NaN if either y or x is NaN
	}

	[[nodiscard]] float angle(const float x, const float y) {
		float result = atan2(x, y) * RAD_DEG;
		if(result < 0) result += DEG_FULL;
		return result;
	}

	[[nodiscard]] float angleRad(const float x, const float y) {
		float result = atan2(x, y);
		if(result < 0) result += PI2;
		return result;
	}

	constexpr int digits(const int n) {
		return n < 100000
			       ? n < 100
				         ? n < 10
					           ? 1
					           : 2
				         : n < 1000
					           ? 3
					           : n < 10000
						             ? 4
						             : 5
			       : n < 10000000
				         ? n < 1000000
					           ? 6
					           : 7
				         : n < 100000000
					           ? 8
					           : n < 1000000000
						             ? 9
						             : 10;
	}

	int digits(const long n) {
		return n == 0 ? 1 : static_cast<int>(std::log10(n) + 1);
	}

	constexpr float sqr(const float x) {
		return x * x;
	}

	constexpr float map(const float value, const float fromA, const float toa, const float fromB, const float tob) {
		return fromB + (value - fromA) * (tob - fromB) / (toa - fromA);
	}

	constexpr /** Map value from [0, 1].*/
	float map(const float value, const float from, const float to) {
		return map(value, 0, 1, from, to);
	}

	/**Returns -1 if f<0, 1 otherwise.*/
	constexpr float sign(const float f) {
		return f == 0.0f ? 0.0f : f < 0.0f ? -1.0f : 1.0f;
	}

	template <Concepts::Number T>
	constexpr T sign(const T f) requires Concepts::Signed<T> {
		return f == 0 ? 0 : f < 0 ? -1 : 1;
	}

	template <Concepts::Number T>
	constexpr T sign(const T f) requires Concepts::NonNegative<T> {
		return f == 0 ? 0 : 1;
	}

	/** Returns 1 if true, -1 if false. */
	constexpr int sign(const bool b) {
		return b ? 1 : -1;
	}

	/**Converts a bool to an integer: 1 if true, 0, if false.*/
	constexpr int num(const bool b) {
		return b ? 1 : 0;
	}

	float pow_float(const float a, const float b) {
		return std::pow(a, b);
	}

	int pow_int(const int a, const int b) { //TODO wtf
		return static_cast<int>(std::ceil(std::pow(a, b)));
	}

	template <unsigned Exponent, typename T>
	constexpr T powIntegral(const T val) {
		if constexpr(Exponent == 0) {
			return 1;
		}else if constexpr (Exponent % 2 == 0) {
			return powIntegral<Exponent / 2, T>(val) * powIntegral<Exponent / 2, T>(val);
		}else {
			return val * powIntegral<(Exponent - 1) / 2, T>(val) * powIntegral<(Exponent - 1) / 2, T>(val);
		}
	}

	/** Returns the next power of two. Returns the specified value if the value is already a power of two. */
	constexpr int nextPowerOfTwo(int value) {
		if(value == 0) return 1;
		value--;
		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;
		return value + 1;
	}

	constexpr bool isPowerOfTwo(const int value) {
		return value != 0 && (value & value - 1) == 0;
	}

	template <Concepts::Number T>
	constexpr T clamp(const T v, const T min, const T max) {
#ifdef _DEBUG
		if(min > max) {
			throw ext::IllegalArguments{"Min Greater Than Max: " + std::to_string(min) + " : " + std::to_string(max)};
		}
#endif

		if(v > max) return max;
		if(v < min) return min;

		return v;
	}

	template <Concepts::Number T>
	constexpr T abs(const T v) {
		if constexpr(std::is_unsigned_v<T>) {
			return v;
		} else {
			return std::abs(v);
		}
	}

	template <Concepts::Number T>
	constexpr T max(const T v1, const T v2) {
		return v1 > v2 ? v1 : v2;
	}

	template <Concepts::Number T>
	constexpr T min(const T v1, const T v2) {
		return v1 < v2 ? v1 : v2;
	}

	/** Clamps to [0, 1]. */
	constexpr float clamp(const float value) {
		return clamp(value, 0.0f, 1.0f);
	}

	constexpr float maxZero(const float val) {
		return ::Math::max(val, 0.0f);
	}

	/** Approaches a value at linear speed. */
	constexpr float approach(const float from, const float to, const float speed) {
		return from + clamp(to - from, -speed, speed);
	}

	template <typename T>
	T& lerp(const T& fromValue, const T& toValue, const T& progress) {
		return fromValue + (toValue - fromValue) * progress;
	}

	template <Concepts::Number T>
	constexpr float lerp(const T fromValue, const T toValue, const T progress) {
		return fromValue + (toValue - fromValue) * progress;
	}

	/** Linearly interpolates between fromValue to toValue on progress position. */
	constexpr float lerp(const float fromValue, const float toValue, const float progress) {
		return fromValue + (toValue - fromValue) * progress;
	}


	/**
	 * Linearly interpolates between two angles in radians. Takes into account that angles wrap at two pi and always takes the
	 * direction with the smallest delta angle.
	 * @param fromRadians start angle in radians
	 * @param toRadians target angle in radians
	 * @param progress interpolation value in the range [0, 1]
	 * @return the interpolated angle in the range [0, PI2]
	 */
	float slerpRad(const float fromRadians, const float toRadians, const float progress) {
		const float delta = std::fmod(toRadians - fromRadians + PI2 + PI, PI2) - PI;
		return std::fmod(fromRadians + delta * progress + PI2, PI2);
	}

	/**
	 * Linearly interpolates between two angles in degrees. Takes into account that angles wrap at 360 degrees and always takes
	 * the direction with the smallest delta angle.
	 * @param fromDegrees start angle in degrees
	 * @param toDegrees target angle in degrees
	 * @param progress interpolation value in the range [0, 1]
	 * @return the interpolated angle in the range [0, 360[
	 */
	float slerp(const float fromDegrees, const float toDegrees, const float progress) {
		const float delta = std::fmod(toDegrees - fromDegrees + DEG_FULL + 180.0f, DEG_FULL) - 180.0f;
		return std::fmod(fromDegrees + delta * progress + DEG_FULL, DEG_FULL);
	}

	/**
	 * Returns the largest integer less than or equal to the specified float. This method will only properly floor floats from
	 * -(2^14) to (Float.MAX_VALUE - 2^14).
	 */
	constexpr int floor(const float value) {
		return static_cast<int>(value + BIG_ENOUGH_FLOOR) - BIG_ENOUGH_INT;
	}

	/**
	 * Returns the largest integer less than or equal to the specified float. This method will only properly floor floats that are
	 * positive. Note this method simply casts the float to int.
	 */
	inline int floorPositive(const float value) {
		return static_cast<int>(value);
	}

	/**
	 * Returns the smallest integer greater than or equal to the specified float. This method will only properly ceil floats from
	 * -(2^14) to (Float.MAX_VALUE - 2^14).
	 */
	constexpr int ceil(const float value) {
		return BIG_ENOUGH_INT - static_cast<int>(BIG_ENOUGH_FLOOR - value);
	}

	/**
	 * Returns the smallest integer greater than or equal to the specified float. This method will only properly ceil floats that
	 * are positive.
	 */
	inline int ceilPositive(const float value) {
		return static_cast<int>(value + CEIL);
	}

	/**
	 * Returns the closest integer to the specified float. This method will only properly round floats from -(2^14) to
	 * (Float.MAX_VALUE - 2^14).
	 */
	template <typename T>
		requires std::is_integral_v<T>
	constexpr T round(const float value) {
		return static_cast<T>(value + BIG_ENOUGH_ROUND) - BIG_ENOUGH_INT;
	}

	constexpr int round(const int value, const int step) {
		return value / step * step;
	}

	constexpr float round(const float value, const float step) {
		return static_cast<float>(static_cast<int>(value / step)) * step;
	}

	constexpr int round(const float value, const int step) {
		return static_cast<int>(value / static_cast<float>(step)) * step;
	}

	/** Returns the closest integer to the specified float. This method will only properly round floats that are positive. */
	int roundPositive(const float value) {
		return std::lround(value + 0.5f);
	}

	/**
	 * Returns true if the value is zero.
	 * @param value N/A
	 * @param tolerance represent an upper bound below which the value is considered zero.
	 */
	inline bool zero(const float value, const float tolerance = FLOAT_ROUNDING_ERROR) {
		return abs(value) <= tolerance;
	}

	/**
	 * Returns true if a is nearly equal to b. The function uses the default floating error tolerance.
	 * @param a the first value.
	 * @param b the second value.
	 */
	inline bool equal(const float a, const float b) {
		return abs(a - b) <= FLOAT_ROUNDING_ERROR;
	}

	/**
	 * Returns true if a is nearly equal to b.
	 * @param a the first value.
	 * @param b the second value.
	 * @param tolerance represent an upper bound below which the two values are considered equal.
	 */
	inline bool equal(const float a, const float b, const float tolerance) {
		return abs(a - b) <= tolerance;
	}

	// /** @return the logarithm of value with base a */
	// float log(float a, float value) {
	//     return (float)(log(value) / log(a));
	//     log
	// }
	//
	// /** @return the logarithm of value with base 2 */
	// float log2(const float value) {
	//     return log(2, value);
	// }
	//
	// /** @return base-2 logarithm of the specified integer */
	// int log2(int value) {
	//     return value == 0 ? 0 : 31 - Integer.numberOfLeadingZeros(value);
	// }

	/** Mod function that works properly for negative numbers. */
	inline int mod(const int x, const int n) {
		return (x % n + n) % n;
	}

	template <Concepts::Number T>
	T mod(const T x, const T n) {
		if constexpr(std::is_floating_point_v<T>) {
			return std::fmod(x, n);
		} else {
			return x % n;
		}
	}

	/**
	 * @return a sampled value based on position in an array of float values.
	 * @param values toLerp
	 * @param time [0, 1]
	 */
	template <typename T>
	float sample(const std::span<T>& values, float time) {
		time             = clamp(time);
		const auto sizeF = static_cast<float>(values.size() - 1ull);

		const float pos = time * sizeF;

		const auto cur  = static_cast<unsigned long long>(min(time * sizeF, sizeF));
		const auto next = min(cur + 1ULL, values.size() - 1ULL);
		const float mod = pos - static_cast<float>(cur);
		return ::Math::lerp<T>(values[cur], values[next], mod);
	}

	template <>
	float sample<float>(const std::span<float>& values, float time) {
		time             = clamp(time);
		const auto sizeF = static_cast<float>(values.size() - 1ull);

		const float pos = time * sizeF;

		const auto cur  = static_cast<unsigned long long>(min(time * sizeF, sizeF));
		const auto next = min(cur + 1ULL, values.size() - 1ULL);
		const float mod = pos - static_cast<float>(cur);
		return Math::lerp(values[cur], values[next], mod);
	}

	/** @return the input 0-1 value scaled to 0-1-0. */
	constexpr float slope(const float fin) {
		return 1.0f - abs(fin - 0.5f) * 2.0f;
	}

	/**Converts a 0-1 value to 0-1 when it is in [offset, 1].*/
	constexpr float curve(const float f, const float offset) {
		if(f < offset) {
			return 0.0f;
		}
		return (f - offset) / (1.0f - offset);
	}

	/**Converts a 0-1 value to 0-1 when it is in [offset, to].*/
	constexpr float curve(const float f, const float from, const float to) {
		if(f < from) {
			return 0.0f;
		}
		if(f > to) {
			return 1.0f;
		}
		return (f - from) / (to - from);
	}

	/** Transforms a 0-1 value to a value with a 0.5 plateau in the middle. When margin = 0.5, this method doesn't do anything. */
	inline float curveMargin(const float f, const float marginLeft, const float marginRight) {
		if(f < marginLeft) return f / marginLeft * 0.5f;
		if(f > 1.0f - marginRight) return (f - 1.0f + marginRight) / marginRight * 0.5f + 0.5f;
		return 0.5f;
	}

	/** Transforms a 0-1 value to a value with a 0.5 plateau in the middle. When margin = 0.5, this method doesn't do anything. */
	inline float curveMargin(const float f, const float margin) {
		return curveMargin(f, margin, margin);
	}

	inline float len(const float x, const float y) {
		return std::sqrt(x * x + y * y);
	}

	inline float len2(const float x, const float y) {
		return x * x + y * y;
	}

	inline float dot(const float x1, const float y1, const float x2, const float y2) {
		return x1 * x2 + y1 * y2;
	}

	inline float dst(const float x1, const float y1) {
		return std::sqrt(x1 * x1 + y1 * y1);
	}

	inline float dst2(const float x1, const float y1) {
		return x1 * x1 + y1 * y1;
	}

	inline float dst(const float x1, const float y1, const float x2, const float y2) {
		const float xd = x2 - x1;
		const float yd = y2 - y1;
		return std::sqrt(xd * xd + yd * yd);
	}

	inline float dst2(const float x1, const float y1, const float x2, const float y2) {
		const float xd = x2 - x1;
		const float yd = y2 - y1;
		return xd * xd + yd * yd;
	}

	/** Manhattan distance. */
	inline float dst_mht(const float x1, const float y1, const float x2, const float y2) {
		return abs(x1 - x2) + abs(y1 - y2);
	}


	/** @return whether dst(x1, y1, x2, y2) < dst */
	inline bool within(const float x1, const float y1, const float x2, const float y2, const float dst) {
		return dst2(x1, y1, x2, y2) < dst * dst;
	}

	/** @return whether dst(x, y, 0, 0) < dst */
	inline bool within(const float x1, const float y1, const float dst) {
		return x1 * x1 + y1 * y1 < dst * dst;
	}

	/**
	 * \return a corret dst value safe for unsigned numbers, can be negative if params are signed.
	 */
	template <Concepts::Number T>
	T constexpr safeDst(const T a, const T b) {
		if constexpr(std::is_signed_v<T>) {
			return a - b;
		} else {
			return a > b ? a - b : b - a;
		}
	}

	namespace Angle{
		float getAngleInPi2(float a){
			a = Math::mod(a, DEG_FULL);
			if(a < 0)a += DEG_FULL;
			return a;
		}

		float forwardDistance(const float angle1, const float angle2){
			return Math::abs(angle1 - angle2);
		}

		float backwardDistance(const float angle1, const float angle2){
			return DEG_FULL - Math::abs(angle1 - angle2);
		}


		float angleDst(float a, float b){
			a = getAngleInPi2(a);
			b = getAngleInPi2(b);
			return Math::min((a - b) < 0 ? a - b + DEG_FULL : a - b, (b - a) < 0 ? b - a + DEG_FULL : b - a);
		}

		float angleDstSign(float a, float b){
			a = getAngleInPi2(a);
			b = getAngleInPi2(b);

			float dst = -(a - b);

			if(abs(dst) > 180){
				dst *= -1;
			}

			return sign(dst);
		}

		float moveToward(float angle, float to, const float speed){
			if(Math::abs(angleDst(angle, to)) < speed) return to;
			angle = getAngleInPi2(angle);
			to = getAngleInPi2(to);

			if((angle > to) == (backwardDistance(angle, to) > forwardDistance(angle, to))){
				angle -= speed;
			}else{
				angle += speed;
			}

			return angle;
		}

		inline bool within(const float a, const float b, const float margin){
			return angleDst(a, b) <= margin;
		}

		inline float clampRange(const float angle, const float dest, const float range){
			const float dst = angleDst(angle, dest);
			return dst <= range ? angle : moveToward(angle, dest, dst - range);
		}
	}
}
