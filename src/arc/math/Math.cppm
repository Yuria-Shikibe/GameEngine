module ;

export module Math;

import <memory>;
import <vector>;
import <algorithm>;
import <cmath>;
#include <numbers>

export namespace Math{
	constexpr int SIGNS[2] = { -1, 1 };
	constexpr int ZERO_ONE[2] = { 0, 1 };
	constexpr bool BOOLEANS[2] = { true, false };
	constexpr float FLOAT_ROUNDING_ERROR = 0.000001f;
	constexpr float PI = std::numbers::pi_v<float>;
	// ReSharper disable once CppInconsistentNaming
	constexpr float pi        = PI, HALF_PI = PI / 2.0f;
	constexpr double PI_EXACT = 3.14159265358979323846;
	constexpr float PI2       = PI * 2.0f;
	constexpr float E         = 2.7182818f;
	constexpr float SQRT2     = std::numbers::sqrt2_v<float>;
	constexpr float SQRT3     = std::numbers::sqrt3_v<float>;
	/** multiply by this to convert from radians to degrees */
	constexpr float RADIANS_TO_DEGREES = 180.0f / std::numbers::pi_v<float>;
	constexpr float RAD_DEG = RADIANS_TO_DEGREES;
	/** multiply by this to convert from degrees to radians */
	constexpr float DEGREES_TO_RADIANS = PI / 180;
	constexpr float DEG_RAD = DEGREES_TO_RADIANS;
	constexpr double DOUBLE_DEG_RAD = 0.017453292519943295;
	constexpr double DOUBLE_RAD_DEG = 57.29577951308232;

	constexpr unsigned int SIN_BITS = 16;
	constexpr unsigned int SIN_MASK = ~(-1 << SIN_BITS);
	constexpr unsigned int SIN_COUNT = SIN_MASK + 1;

	constexpr float RAD_FULL = PI * 2;
	constexpr float DEG_FULL = 360.0f;
	constexpr float RAD_TO_INDEX = SIN_COUNT / RAD_FULL;
	constexpr float DEG_TO_INDEX = SIN_COUNT / DEG_FULL;
	constexpr int BIG_ENOUGH_INT = 16 * 1024;
	constexpr double BIG_ENOUGH_FLOOR = BIG_ENOUGH_INT;
	constexpr double CEIL = 0.9999999;
	constexpr double BIG_ENOUGH_ROUND = static_cast<double>(BIG_ENOUGH_INT) + 0.5f;

	inline const std::unique_ptr<const float[]> sinTable = []{
		std::unique_ptr<float[]> table(new float[SIN_COUNT]);

		for (unsigned int i = 0; i < SIN_COUNT; i++) {
			table[i] = std::sinf((static_cast<float>(i) + 0.5f) / SIN_COUNT * RAD_FULL);
		}

		for (unsigned int i = 0; i < 360; i += 90){
			const auto fi = static_cast<float>(i);
			table[static_cast<unsigned int>(fi * DEG_TO_INDEX) & SIN_MASK] = std::sinf(fi * DEGREES_TO_RADIANS);
		}

		table[0                                                       ] =  0.0f;
		table[static_cast<unsigned int>( 90 * DEG_TO_INDEX) & SIN_MASK] =  1.0f;
		table[static_cast<unsigned int>(180 * DEG_TO_INDEX) & SIN_MASK] =  0.0f;
		table[static_cast<unsigned int>(270 * DEG_TO_INDEX) & SIN_MASK] = -1.0f;

		return table;
	}();

	// Rand rand = new Rand();

	/** Returns the sine in radians from a lookup table. */
	float sin(const float radians) {
		return sinTable[static_cast<int>(radians * RAD_TO_INDEX) & SIN_MASK];
	}

	float sin(const float radians, const float scl, const float mag) {
		return sin(radians / scl) * mag;
	}

	/** Returns the cosine in radians from a lookup table. */
	float cos(const float radians) {
		return sinTable[static_cast<int>((radians + PI / 2) * RAD_TO_INDEX) & SIN_MASK];
	}

	/** Returns the sine in radians from a lookup table. */
	float sinDeg(const float degrees) {
		return sinTable[static_cast<int>(degrees * DEG_TO_INDEX) & SIN_MASK];
	}

	/** Returns the cosine in radians from a lookup table. */
	float cosDeg(const float degrees) {
		return sinTable[static_cast<int>((degrees + 90) * DEG_TO_INDEX) & SIN_MASK];
	}

	float absin(const float in, const float scl, const float mag) {
		return (sin(in, scl * 2.0f, mag) + mag) / 2.0f;
	}

	float tan(const float radians, const float scl, const float mag) {
		return sin(radians / scl) / cos(radians / scl) * mag;
	}

	float cos(const float radians, const float scl, const float mag) {
		return cos(radians / scl) * mag;
	}

	float angleExact(const float x, const float y) {
		float result = atan2(y, x) * RAD_DEG;
		if (result < 0) result += DEG_FULL;
		return result;
	}

	/** Wraps the given angle to the range [-PI, PI]
	 * @param a the angle in radians
	 * @return the given angle wrapped to the range [-PI, PI] */
	float wrapAngleAroundZero(const float a) {
		if (a >= 0) {
			float rotation = fmod(a, PI2);
			if (rotation > PI) rotation -= PI2;
			return rotation;
		}

		float rotation = fmod(-a, PI2);
		if (rotation > PI) rotation -= PI2;
		return -rotation;
	}

	/** A variant on atan that does not tolerate infinite inputs for speed reasons, and because infinite inputs
	 * should never occur where this is used (only in {@link atan2(float, float)@endlink}).
	 * @param i any finite float
	 * @return an output from the inverse tangent function, from PI/-2.0 to PI/2.0 inclusive
	 * */
	float atn(const double i) {
		// We use double precision internally, because some constants need double precision.
		const double n = abs(i);
		// c uses the "equally-good" formulation that permits n to be from 0 to almost infinity.
		const double c = (n - 1.0) / (n + 1.0);
		// The approximation needs 6 odd powers of c.
		const double c2 = c * c, c3 = c * c2, c5 = c3 * c2, c7 = c5 * c2, c9 = c7 * c2, c11 = c9 * c2;
		return static_cast<float>
		(std::copysign(PI_EXACT * 0.25
		               + (0.99997726 * c - 0.33262347 * c3 + 0.19354346 * c5 - 0.11643287 * c7 + 0.05265332 * c9 - 0.0117212 * c11), i));
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
	float atan2(const float x, const float y) {
		const float n = y / x;

//		 if (n != n) {
//		     n = (y == x ? 1.0f : -1.0f); // if both y and x are infinite, n would be NaN
//		 }
//		 else if (n - n != n - n) {
//		     x = 0.0f; // if n is infinite, y is infinitely larger than x.
//		 }

		if (x > 0.0f) {
			return atn(n);
		}
		if (x < 0.0f) {
			return y >= 0 ? atn(n) + PI : atn(n) - PI;
		}
		if (y > 0.0f) {
			return x + HALF_PI;
		}
		if (y < 0.0f) {
			return x - HALF_PI;
		}

		return x + y; // returns 0 for 0,0 or NaN if either y or x is NaN
	}

	float angle(const float x, const float y) {
		float result = atan2(x, y) * RAD_DEG;
		if (result < 0) result += DEG_FULL;
		return result;
	}

	int digits(const int n) {
		return n < 100000 ? n < 100 ? n < 10 ? 1 : 2 : n < 1000 ? 3 : n < 10000 ? 4 : 5 : n < 10000000 ? n < 1000000 ? 6 : 7 : n < 100000000 ? 8 : n < 1000000000 ? 9 : 10;
	}

	int digits(const long n) {
		return n == 0 ? 1 : static_cast<int>(log10(n) + 1);
	}

	float sqr(const float x) {
		return x * x;
	}

	float map(const float value, const float fromA, const float toa, const float fromB, const float tob) {
		return fromB + (value - fromA) * (tob - fromB) / (toa - fromA);
	}

	/** Map value from [0, 1].*/
	float map(const float value, const float from, const float to) {
		return map(value, 0, 1, from, to);
	}

	/**Returns -1 if f<0, 1 otherwise.*/
	int sign(const float f) {
		return f < 0 ? -1 : 1;
	}

	/** Returns 1 if true, -1 if false. */
	int sign(const bool b) {
		return b ? 1 : -1;
	}

	/**Converts a bool to an integer: 1 if true, 0, if false.*/
	int num(const bool b) {
		return b ? 1 : 0;
	}

	float pow_float(const float a, const float b) {
		return pow(a, b);
	}

	int pow_int(const int a, const int b) {
		return static_cast<int>(ceil(pow(a, b)));
	}

	/** Returns the next power of two. Returns the specified value if the value is already a power of two. */
	int nextPowerOfTwo(int value) {
		if (value == 0) return 1;
		value--;
		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;
		return value + 1;
	}

	bool isPowerOfTwo(const int value) {
		return value != 0 && (value & value - 1) == 0;
	}

	using std::max;
	using std::min;


	//Seriously, using std::clamp
	inline int clamp(const int value, const int minv, const int maxv) {
		return max(min(value, maxv), minv);
	}

	inline long clamp(const long value, const long minv, const long maxv) {
		return max(min(value, maxv), minv);
	}

	inline float clamp(const float value, const float minv, const float maxv) {
		return max(min(value, maxv), minv);
	}

	/** Clamps to [0, 1]. */
	inline float clamp(const float value) {
		return clamp(value, 0.0f, 1.0f);
	}

	inline double clamp(const double value, const double minv, const double maxv) {
		return max(min(value, maxv), minv);
	}

	inline float maxZero(const float val) {
		return max(val, 0.0f);
	}

	/** Approaches a value at linear speed. */
	inline float approach(const float from, const float to, const float speed) {
		return from + clamp(to - from, -speed, speed);
	}

	/** Linearly interpolates between fromValue to toValue on progress position. */
	inline float lerp(const float fromValue, const float toValue, const float progress) {
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
		const float delta = fmod(toRadians - fromRadians + PI2 + PI, PI2) - PI;
		return fmod(fromRadians + delta * progress + PI2, PI2);
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
		const float delta = fmod(toDegrees - fromDegrees + DEG_FULL + 180.0f, DEG_FULL) - 180.0f;
		return fmod(fromDegrees + delta * progress + DEG_FULL, DEG_FULL);
	}

	/**
	 * Returns the largest integer less than or equal to the specified float. This method will only properly floor floats from
	 * -(2^14) to (Float.MAX_VALUE - 2^14).
	 */
	inline int floor(const float value) {
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
	inline int ceil(const float value) {
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
	inline int round(const float value) {
		return static_cast<int>(value + BIG_ENOUGH_ROUND) - BIG_ENOUGH_INT;
	}

	inline int round(const int value, const int step) {
		return value / step * step;
	}

	inline float round(const float value, const float step) {
		return static_cast<float>(static_cast<int>(value / step)) * step;
	}

	inline int round(const float value, const int step) {
		return static_cast<int>(value / static_cast<float>(step)) * step;
	}

	/** Returns the closest integer to the specified float. This method will only properly round floats that are positive. */
	inline int roundPositive(const float value) {
		return lround(value + 0.5f);
	}

	/** Returns true if the value is zero (using the default tolerance as upper bound) */
	inline bool zero(const float value) {
		return abs(value) <= FLOAT_ROUNDING_ERROR;
	}

	/** Returns true if the value is zero (using the default tolerance as upper bound) */
	inline bool zero(const double value) {
		return abs(value) <= FLOAT_ROUNDING_ERROR;
	}

	/**
	 * Returns true if the value is zero.
	 * @param value N/A
	 * @param tolerance represent an upper bound below which the value is considered zero.
	 */
	inline bool zero(const float value, const float tolerance) {
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
	inline float mod(const float f, const float n) {
		return fmod(fmod(f, n) + n , n);
	}

	/** Mod function that works properly for negative numbers. */
	inline int mod(const int x, const int n) {
		return (x % n + n) % n;
	}

	/** @return a sampled value based on position in an array of float values. */
	inline float sample(const std::vector<float>& values, float time) {
		time = clamp(time);
		const auto size = static_cast<float>(values.size());
		const float pos = time * (size - 1.0f);
		const auto cur = static_cast<unsigned long long>(min(time * (size - 1.0f), size - 1.0f));
		const unsigned long long next = min(values.size() + 1ULL, values.size() - 1ULL);
		const float mod = pos - static_cast<float>(cur);
		return lerp(values[cur], values[next], mod);
	}

	/** @return the input 0-1 value scaled to 0-1-0. */
	inline float slope(const float fin) {
		return 1.0f - abs(fin - 0.5f) * 2.0f;
	}

	/**Converts a 0-1 value to 0-1 when it is in [offset, 1].*/
	inline float curve(const float f, const float offset) {
		if (f < offset) {
			return 0.0f;
		}
		return (f - offset) / (1.0f - offset);
	}

	/**Converts a 0-1 value to 0-1 when it is in [offset, to].*/
	inline float curve(const float f, const float from, const float to) {
		if (f < from) {
			return 0.0f;
		}
		if (f > to) {
			return 1.0f;
		}
		return (f - from) / (to - from);
	}

	/** Transforms a 0-1 value to a value with a 0.5 plateau in the middle. When margin = 0.5, this method doesn't do anything. */
	inline float curveMargin(const float f, const float marginLeft, const float marginRight) {
		if (f < marginLeft) return f / marginLeft * 0.5f;
		if (f > 1.0f - marginRight) return (f - 1.0f + marginRight) / marginRight * 0.5f + 0.5f;
		return 0.5f;
	}

	/** Transforms a 0-1 value to a value with a 0.5 plateau in the middle. When margin = 0.5, this method doesn't do anything. */
	inline float curveMargin(const float f, const float margin) {
		return curveMargin(f, margin, margin);
	}

	inline float len(const float x, const float y) {
		return sqrt(x * x + y * y);
	}

	inline float len2(const float x, const float y) {
		return x * x + y * y;
	}

	inline float dot(const float x1, const float y1, const float x2, const float y2) {
		return x1 * x2 + y1 * y2;
	}

	inline float dst(const float x1, const float y1) {
		return sqrt(x1 * x1 + y1 * y1);
	}

	inline float dst2(const float x1, const float y1) {
		return x1 * x1 + y1 * y1;
	}

	inline float dst(const float x1, const float y1, const float x2, const float y2) {
		const float xd = x2 - x1;
		const float yd = y2 - y1;
		return sqrt(xd * xd + yd * yd);
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
}

module : private;