module ;

export module Geom.Vector3D;
import Concepts;

import Math;
import std;

export namespace Geom {
	template <Concepts::Number T>
	struct Vector3D {
		[[nodiscard]] constexpr Vector3D(const T x, const T y, const T z)
			: x(x),
			y(y),
			z(z) {
		}

		[[nodiscard]] constexpr Vector3D(const T x, const T y) : Vector3D(x, y, 0) {
			// std::size()
		}

		[[nodiscard]] constexpr Vector3D() = default;

		T x = 0, y = 0, z = 0;

		using PassType = Concepts::ParamPassType<Vector3D, sizeof(T) * 3>;

		/** @return The euclidean length */
		[[nodiscard]] static constexpr float len(const T x, const T y, const T z) {
			return std::sqrtf(static_cast<float>(x * x + y * y + z * z));
		}

		/** @return The squared euclidean length */
		[[nodiscard]] static constexpr T len2(const T x, const T y, const T z) {
			return x * x + y * y + z * z;
		}

		/** @return The euclidean distance between the two specified vectors */
		[[nodiscard]] static constexpr float dst(const T x1, const T y1, const T z1, const T x2, const T y2, const T z2) {
			const T a = Math::safeDst(x2 - x1);
			const T b = Math::safeDst(y2 - y1);
			const T c = Math::safeDst(z2 - z1);
			return std::sqrtf(static_cast<float>(a * a + b * b + c * c));
		}

		/** @return the squared distance between the given points */
		[[nodiscard]] static constexpr T dst2(const T x1, const T y1, const T z1, const T x2, const T y2, const T z2) {
			const T a = Math::safeDst(x2 - x1);
			const T b = Math::safeDst(y2 - y1);
			const T c = Math::safeDst(z2 - z1);
			return a * a + b * b + c * c;
		}

		/** @return The dot product between the two vectors */
		[[nodiscard]] static constexpr T dot(const T x1, const T y1, const T z1, const T x2, const T y2, const T z2) {
			return x1 * x2 + y1 * y2 + z1 * z2;
		}

		constexpr Vector3D& operator=(const Vector3D& other) {
			if(this == &other) return *this;
			x = other.x;
			y = other.y;
			z = other.z;

			return *this;
		}

		constexpr Vector3D& operator=(Vector3D&& other) noexcept {
			if(this == &other) return *this;
			x = other.x;
			y = other.y;
			z = other.z;
			return *this;
		}

		constexpr Vector3D& div(const Vector3D& other) {
			x /= other.x;
			y /= other.y;
			z /= other.z;

			return *this;
		}

		constexpr Vector3D& set(const Vector3D& other) {
			this->operator=(other);
			return *this;
		}

		constexpr Vector3D& set(const T x, const T y, const T z) {
			this->x = x;
			this->y = y;
			this->z = z;
			return *this;
		}

		/**
		 * Sets the components from the given spherical coordinate
		 * @param azimuthalAngle The angle between x-axis in radians [0, 2pi]
		 * @param polarAngle The angle between z-axis in radians [0, pi]
		 * @return This vector for chaining
		 */
		constexpr Vector3D& setFromSpherical(float azimuthalAngle, float polarAngle) {
			const float cosPolar = Math::cos(polarAngle);
			const float sinPolar = Math::sin(polarAngle);

			const float cosAzim = Math::cos(azimuthalAngle);
			const float sinAzim = Math::sin(azimuthalAngle);

			return this->set(cosAzim * sinPolar, sinAzim * sinPolar, cosPolar);
		}

		[[nodiscard]] constexpr Vector3D& copy() {
			return Vector3D{ *this };
		}

		constexpr Vector3D& add(const Vector3D& vector) {
			return this->add(vector.x, vector.y, vector.z);
		}

		constexpr Vector3D& add(const Vector3D& vector, const T scale) {
			return this->add(vector.x * scale, vector.y * scale, vector.z * scale);
		}

		constexpr Vector3D& sub(const Vector3D& vector, const T scale) {
			return this->sub(vector.x * scale, vector.y * scale, vector.z * scale);
		}

		constexpr Vector3D& add(const T x, const T y, const T z) {
			this->x += x;
			this->y += y;
			this->z += z;

			return *this;
		}


		constexpr Vector3D& add(const T val) {
			return this->add(val, val, val);
		}


		constexpr Vector3D& sub(const Vector3D& other) {
			return this->sub(other.x, other.y, other.z);
		}

		constexpr Vector3D& sub(const T x, const T y, const T z) {
			this->x -= x;
			this->y -= y;
			this->z -= z;
			return *this;
		}

		constexpr Vector3D& sub(const T value) {
			return this->sub(value, value, value);
		}

		constexpr Vector3D& scl(const T x, const T y, const T z) {
			this->x *= x;
			this->y *= y;
			this->z *= z;
			return *this;
		}

		constexpr Vector3D& scl(const T scalar) {
			this->x *= scalar;
			this->y *= scalar;
			this->z *= scalar;
			return *this;
		}

		constexpr Vector3D& scl(const Vector3D& other) {
			return this->scl(other.x, other.y, other.z);
		}

		constexpr Vector3D& mulAdd(Vector3D& vec, const T scalar) {
			this->x += vec.x * scalar;
			this->y += vec.y * scalar;
			this->z += vec.z * scalar;
			return *this;
		}

		constexpr Vector3D& mulAdd(Vector3D& vec, Vector3D& mulVec) {
			this->x += vec.x * mulVec.x;
			this->y += vec.y * mulVec.y;
			this->z += vec.z * mulVec.z;
			return *this;
		}

		[[nodiscard]] float len() const {
			return std::sqrtf(static_cast<float>(x * x + y * y + z * z));
		}

		[[nodiscard]] constexpr T len2() const {
			return x * x + y * y + z * z;
		}

		[[nodiscard]] constexpr bool equalTo(const Vector3D& vector) const {
			return x == vector.x && y == vector.y && z == vector.z;
		}


		[[nodiscard]] float dst(const Vector3D& vector) const {
			const T a = Math::safeDst(vector.x - x);
			const T b = Math::safeDst(vector.y - y);
			const T c = Math::safeDst(vector.z - z);
			return std::sqrtf(static_cast<float>(a * a + b * b + c * c));
		}

		/** @return the distance between this point and the given point */
		[[nodiscard]] float dst(const T x, const T y, const T z) const {
			const T a = Math::safeDst(x - this->x);
			const T b = Math::safeDst(y - this->y);
			const T c = Math::safeDst(z - this->z);
			return std::sqrtf(static_cast<float>(a * a + b * b + c * c));
		}

		[[nodiscard]] constexpr T dst2(const Vector3D& point) const {
			const T a = Math::safeDst(x - point.x);
			const T b = Math::safeDst(y - point.y);
			const T c = Math::safeDst(z - point.z);
			return a * a + b * b + c * c;
		}

		/**
		 * Returns the squared distance between this point and the given point
		 * @param x The x-component of the other point
		 * @param y The y-component of the other point
		 * @param z The z-component of the other point
		 * @return The squared distance
		 */
		[[nodiscard]] constexpr T dst2(const T x, const T y, const T z) const {
			const T a = Math::safeDst(x - this->x);
			const T b = Math::safeDst(y - this->y);
			const T c = Math::safeDst(z - this->z);
			return a * a + b * b + c * c;
		}

		[[nodiscard]] constexpr bool within(const Vector3D& v, const T dst) const {
			return this->dst2(v) < dst * dst;
		}

		Vector3D& normalize() {
			const auto len2 = this->len2();
			if(len2 == static_cast<T>(0) || len2 == static_cast<T>(1)) return *this;
			return this->scl(1.0f / std::sqrt(static_cast<float>(len2)));
		}

		[[nodiscard]] constexpr T dot(const Vector3D& vector) const {
			return x * vector.x + y * vector.y + z * vector.z;
		}

		/** @return the angle to the other vector, in radians. */
		[[nodiscard]] float angleRad(const Vector3D& vector) const {
			float l  = len();
			float l2 = vector.len();
			return (float)std::acos(this->dot(x / l, y / l, z / l, vector.x / l2, vector.y / l2, vector.z / l2));
		}

		/** @return the angle to the other vector, in degrees. */
		[[nodiscard]] float angle(const Vector3D& vector) const {
			return this->angleRad(vector) * Math::RADIANS_TO_DEGREES;
		}

		/**
		 * Returns the dot product between this and the given vector.
		 * @param x The x-component of the other vector
		 * @param y The y-component of the other vector
		 * @param z The z-component of the other vector
		 * @return The dot product
		 */
		[[nodiscard]] T dot(const T x, const T y, const T z) const {
			return this->x * x + this->y * y + this->z * z;
		}

		/**
		 * Sets this vector to the cross product between it and the other vector.
		 * @param vector The other vector
		 * @return This vector for chaining
		 */
		Vector3D& crs(const Vector3D& vector) const {
			return this->set(y * vector.z - z * vector.y, z * vector.x - x * vector.z, x * vector.y - y * vector.x);
		}

		/**
		 * Sets this vector to the cross product between it and the other vector.
		 * @param x The x-component of the other vector
		 * @param y The y-component of the other vector
		 * @param z The z-component of the other vector
		 * @return This vector for chaining
		 */
		Vector3D& crs(const T x, const T y, const T z) const {
			return this->set(this->y * z - this->z * y, this->z * x - this->x * z, this->x * y - this->y * x);
		}

		// /**
		//  * Left-multiplies the vector by the given 4x3 column major matrix. The matrix should be composed by a 3x3 matrix representing
		//  * rotation and scale plus a 1x3 matrix representing the translation.
		//  * @param matrix The matrix
		//  * @return This vector for chaining
		//  */
		// Vector3D& mul4x3(float[] matrix){
		//     return set(x * matrix[0] + y * matrix[3] + z * matrix[6] + matrix[9], x * matrix[1] + y * matrix[4] + z * matrix[7]
		//     + matrix[10], x * matrix[2] + y * matrix[5] + z * matrix[8] + matrix[11]);
		// }
		//
		// /**
		//  * Left-multiplies the vector by the given matrix.
		//  * @param matrix The matrix
		//  * @return This vector for chaining
		//  */
		// Vector3D& mul(Mat matrix){
		//     const float[] l_mat = matrix.val;
		//     return set(x * l_mat[Mat.M00] + y * l_mat[Mat.M01] + z * l_mat[Mat.M02], x * l_mat[Mat.M10] + y
		//     * l_mat[Mat.M11] + z * l_mat[Mat.M12], x * l_mat[Mat.M20] + y * l_mat[Mat.M21] + z * l_mat[Mat.M22]);
		// }

		// /**
		//  * Multiplies the vector by the transpose of the given matrix.
		//  * @param matrix The matrix
		//  * @return This vector for chaining
		//  */
		// Vector3D& traMul(Mat matrix){
		//     const float[] l_mat = matrix.val;
		//     return set(x * l_mat[Mat.M00] + y * l_mat[Mat.M10] + z * l_mat[Mat.M20], x * l_mat[Mat.M01] + y
		//     * l_mat[Mat.M11] + z * l_mat[Mat.M21], x * l_mat[Mat.M02] + y * l_mat[Mat.M12] + z * l_mat[Mat.M22]);
		// }

		// /**
		//  * Rotates this vector by the given angle in degrees around the given axis.
		//  * @param axis the axis
		//  * @param degrees the angle in degrees
		//  * @return This vector for chaining
		//  */
		// Vector3D& rotate(const Vector3D& axis, float degrees){
		//     tmpMat.setToRotation(axis, degrees);
		//     return this->mul(tmpMat);
		// }

		[[nodiscard]] constexpr bool isUnit() const {
			return isUnit(0.000000001f);
		}

		[[nodiscard]] constexpr bool isUnit(const T margin) const {
			return Math::abs(len2() - 1) < margin;
		}

		[[nodiscard]] constexpr bool isZero() const {
			return x == 0 && y == 0 && z == 0;
		}

		[[nodiscard]] constexpr bool isZero(const T margin) const {
			return len2() < margin;
		}

		[[nodiscard]] constexpr bool isOnLine(const Vector3D& other,
		                        const T epsilon = static_cast<T>(Math::FLOAT_ROUNDING_ERROR)) const {
			return this->len2(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x) <=
			       epsilon;
		}


		[[nodiscard]] constexpr bool isCollinear(const Vector3D& other,
		                           const T epsilon = static_cast<T>(Math::FLOAT_ROUNDING_ERROR)) const {
			return this->isOnLine(other, epsilon) && this->hasSameDirection(other);
		}

		[[nodiscard]] constexpr bool isCollinearOpposite(const Vector3D& other,
		                                   const T epsilon = static_cast<T>(Math::FLOAT_ROUNDING_ERROR)) const {
			return this->isOnLine(other, epsilon) && this->hasOppositeDirection(other);
		}

		[[nodiscard]] constexpr bool isPerpendicular(const Vector3D& vector,
		                               const T epsilon = static_cast<T>(Math::FLOAT_ROUNDING_ERROR)) const {
			return Math::zero(this->dot(vector), epsilon);
		}

		[[nodiscard]] constexpr bool hasSameDirection(const Vector3D& vector) const {
			return this->dot(vector) > 0;
		}

		[[nodiscard]] constexpr bool hasOppositeDirection(const Vector3D& vector) const {
			return this->dot(vector) < 0;
		}

		Vector3D& lerp(const Vector3D& target, float alpha) {
			x += alpha * (target.x - x);
			y += alpha * (target.y - y);
			z += alpha * (target.z - z);
			return *this;
		}

		// Vector3D& interpolate(Vector3D& target, float alpha, Interp interpolator){
		//     return lerp(target, interpolator.apply(0f, 1f, alpha));
		// }

		/**
		 * Spherically interpolates between this vector and the target vector by alpha which is in the range [0,1]. The result is
		 * stored in this vector.
		 * @param target The target vector
		 * @param alpha The interpolation coefficient
		 * @return This vector for chaining.
		 */
		//TODO apply acos table...
		Vector3D& slerp(const Vector3D& target, const float alpha) {
			const float dot = dot(target);
			// If the inputs are too close for comfort, simply linearly interpolate.
			if(dot > 0.9995 || dot < -0.9995) return this->lerp(target, alpha);

			// theta0 = angle between input vectors
			const float theta0 = (float)std::acos(dot);
			// theta = angle between this vector and result
			const float theta = theta0 * alpha;

			const float st = Math::sin(theta);
			const float tx = target.x - x * dot;
			const float ty = target.y - y * dot;
			const float tz = target.z - z * dot;
			const float l2 = tx * tx + ty * ty + tz * tz;
			const float dl = st * (l2 < 0.0001f ? 1.0f : 1.0f / (float)std::sqrtf(l2));

			return this->scl(Math::cos(theta)).add(tx * dl, ty * dl, tz * dl).normalize();
		}

		friend std::ostream& operator<<(std::ostream& os, const Vector3D& obj) {
			return os
			       << '(' << std::to_string(obj.x)
			       << ", " << std::to_string(obj.y)
			       << ", " << std::to_string(obj.z)
			       << ')';
		}


		Vector3D& limit(const T limit) {
			return this->limit2(limit * limit);
		}


		Vector3D& limit2(const T limit2) {
			float len2 = len2();
			if(len2 > limit2) {
				this->scl(std::sqrtf(limit2 / len2));
			}
			return this;
		}


		Vector3D& setLength(const T len) {
			return this->setLength2(len * len);
		}


		Vector3D& setLength2(const T len2) {
			const T oldLen2 = this->len2();
			return (oldLen2 == 0 || oldLen2 == len2) ? *this : this->scl(std::sqrtf(len2 / oldLen2));
		}


		Vector3D& clamp(const T min, const T max) {
			const T len2 = len2();
			if(len2 == 0) return *this;
			const T max2 = max * max;
			if(len2 > max2) return this->scl(std::sqrtf(max2 / len2));
			const T min2 = min * min;
			if(len2 < min2) return this->scl(std::sqrtf(min2 / len2));
			return this;
		}


		// public int hashCode(){
		//     const int prime = 31;
		//     int result = 1;
		//     result = prime * result + Float.floatToIntBits(x);
		//     result = prime * result + Float.floatToIntBits(y);
		//     result = prime * result + Float.floatToIntBits(z);
		//     return result;


		Vector3D& setZero() {
			this->x = 0;
			this->y = 0;
			this->z = 0;
			return this;
		}
	};

	using Vec3 = Vector3D<float>;
	using Point3 = Vector3D<float>;
	using Point3U = Vector3D<unsigned int>;
	using Point3S = Vector3D<short>;
	using Point3US = Vector3D<unsigned short>;

	constexpr Vec3 ZERO3{ 0, 0, 0 };
	constexpr Vec3 X3{ 1, 0, 0 };
	constexpr Vec3 Y3{ 0, 1, 0 };
	constexpr Vec3 Z3{ 0, 0, 1 };
}
