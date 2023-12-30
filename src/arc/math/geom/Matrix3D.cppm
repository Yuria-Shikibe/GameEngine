module ;

export module Geom.Matrix3D;

import <exception>;
import <sstream>;
import <array>;
import Geom.Vector3D;
export import Geom.Vector2D;
import Math;
import <ostream>;

export namespace Geom{
	using MatRaw = std::array<float, 9>;

	struct Matrix3D{
	private: MatRaw tmp{};

	public:
		static constexpr int M00 = 0;
		static constexpr int M01 = 3;
		static constexpr int M02 = 6;
		static constexpr int M10 = 1;
		static constexpr int M11 = 4;
		static constexpr int M12 = 7;
		static constexpr int M20 = 2;
		static constexpr int M21 = 5;
		static constexpr int M22 = 8;

		MatRaw val{};

	private:
		static void mul(MatRaw& mat_a, const MatRaw& mat_b) {
			const float v00 = mat_a[M00] * mat_b[M00] + mat_a[M01] * mat_b[M10] + mat_a[M02] * mat_b[M20];
			const float v01 = mat_a[M00] * mat_b[M01] + mat_a[M01] * mat_b[M11] + mat_a[M02] * mat_b[M21];
			const float v02 = mat_a[M00] * mat_b[M02] + mat_a[M01] * mat_b[M12] + mat_a[M02] * mat_b[M22];

			const float v10 = mat_a[M10] * mat_b[M00] + mat_a[M11] * mat_b[M10] + mat_a[M12] * mat_b[M20];
			const float v11 = mat_a[M10] * mat_b[M01] + mat_a[M11] * mat_b[M11] + mat_a[M12] * mat_b[M21];
			const float v12 = mat_a[M10] * mat_b[M02] + mat_a[M11] * mat_b[M12] + mat_a[M12] * mat_b[M22];

			const float v20 = mat_a[M20] * mat_b[M00] + mat_a[M21] * mat_b[M10] + mat_a[M22] * mat_b[M20];
			const float v21 = mat_a[M20] * mat_b[M01] + mat_a[M21] * mat_b[M11] + mat_a[M22] * mat_b[M21];
			const float v22 = mat_a[M20] * mat_b[M02] + mat_a[M21] * mat_b[M12] + mat_a[M22] * mat_b[M22];

			mat_a[M00] = v00;
			mat_a[M10] = v10;
			mat_a[M20] = v20;
			mat_a[M01] = v01;
			mat_a[M11] = v11;
			mat_a[M21] = v21;
			mat_a[M02] = v02;
			mat_a[M12] = v12;
			mat_a[M22] = v22;
		}

	public:
		[[nodiscard]] const float* getRawVal() const{
			return val.data();
		}

		friend bool operator==(const Matrix3D& lhs, const Matrix3D& rhs){
			return std::equal(lhs.val.begin(), lhs.val.end(), rhs.val.begin());
		}

		friend bool operator!=(const Matrix3D& lhs, const Matrix3D& rhs){
			return !(lhs == rhs);
		}

		~Matrix3D() = default;

		friend Vector2D operator*(const Matrix3D& mat, const Vector2D& vec2) {
			Vector2D vector2D{};
			return vector2D.set(vec2.x * mat.val[0] + vec2.y * mat.val[3] + mat.val[6], vec2.x * mat.val[1] + vec2.y * mat.val[4] + mat.val[7]);
		}

		friend Vector2D operator*=(Vector2D& vec2, const Matrix3D& mat) {
			return vec2.set(vec2.x * mat.val[0] + vec2.y * mat.val[3] + mat.val[6], vec2.x * mat.val[1] + vec2.y * mat.val[4] + mat.val[7]);
		}

		friend Matrix3D& operator*=(const Matrix3D& lhs, Matrix3D& rhs) {
			return rhs.mulLeft(lhs);
		}

		Matrix3D& operator*=(const Matrix3D& lhs) {
			return mulLeft(lhs);
		}

		Matrix3D& operator~() {
			return inv();
		}

		float operator*() const {
			return det();
		}

		Matrix3D() {
			idt();
		}

		explicit Matrix3D(const float values[]) {
			set(values);
		}

		explicit Matrix3D(const MatRaw& arr) {
			set(arr.data());
		}

		friend std::ostream& operator<<(std::ostream& os, const Matrix3D& obj) {
			return os
				<< "[" << obj.val[M00] << "|" << obj.val[M01] << "|" << obj.val[M02] << "]\n" //
				<< "[" << obj.val[M10] << "|" << obj.val[M11] << "|" << obj.val[M12] << "]\n" //
				<< "[" << obj.val[M20] << "|" << obj.val[M21] << "|" << obj.val[M22] << "]";
		}

		Matrix3D(const Matrix3D& other) {  // NOLINT(cppcoreguidelines-pro-type-member-init)
			set(other);
		}

		Matrix3D(Matrix3D&& other) noexcept {  // NOLINT(cppcoreguidelines-pro-type-member-init)
			set(other);
		}

		Matrix3D& operator=(const Matrix3D& other) {
			if (this == &other) return *this;
			set(other);
			return *this;
		}

		Matrix3D& operator=(Matrix3D&& other) noexcept {
			if (this == &other) return *this;
			set(other);
			return *this;
		}

		Matrix3D& setOrthogonal(const float x, const float y, const float width, const float height) {
			const float right = x + width, top = y + height;

			const float x_orth = 2.0f / (right - x);
			const float y_orth = 2.0f / (top - y);

			const float tx = -(right + x) / (right - x);
			const float ty = -(top + y) / (top - y);

			val[M00] = x_orth;
			val[M11] = y_orth;

			val[M02] = tx;
			val[M12] = ty;
			val[M22] = 1.0f;

			return *this;
		}

		Matrix3D& idt() {
			val[M00] = 1.0f;
			val[M10] = 0.0f;
			val[M20] = 0.0f;
			val[M01] = 0.0f;
			val[M11] = 1.0f;
			val[M21] = 0.0f;
			val[M02] = 0.0f;
			val[M12] = 0.0f;
			val[M22] = 1.0f;
			return *this;
		}

		Matrix3D& mul(const Matrix3D& m) {
			const float v00 = val[M00] * m.val[M00] + val[M01] * m.val[M10] + val[M02] * m.val[M20];
			const float v01 = val[M00] * m.val[M01] + val[M01] * m.val[M11] + val[M02] * m.val[M21];
			const float v02 = val[M00] * m.val[M02] + val[M01] * m.val[M12] + val[M02] * m.val[M22];

			const float v10 = val[M10] * m.val[M00] + val[M11] * m.val[M10] + val[M12] * m.val[M20];
			const float v11 = val[M10] * m.val[M01] + val[M11] * m.val[M11] + val[M12] * m.val[M21];
			const float v12 = val[M10] * m.val[M02] + val[M11] * m.val[M12] + val[M12] * m.val[M22];

			const float v20 = val[M20] * m.val[M00] + val[M21] * m.val[M10] + val[M22] * m.val[M20];
			const float v21 = val[M20] * m.val[M01] + val[M21] * m.val[M11] + val[M22] * m.val[M21];
			const float v22 = val[M20] * m.val[M02] + val[M21] * m.val[M12] + val[M22] * m.val[M22];

			val[M00] = v00;
			val[M10] = v10;
			val[M20] = v20;
			val[M01] = v01;
			val[M11] = v11;
			val[M21] = v21;
			val[M02] = v02;
			val[M12] = v12;
			val[M22] = v22;

			return *this;
		}

		Matrix3D& mulLeft(const Matrix3D& m) {
			const float v00 = m.val[M00] * val[M00] + m.val[M01] * val[M10] + m.val[M02] * val[M20];
			const float v01 = m.val[M00] * val[M01] + m.val[M01] * val[M11] + m.val[M02] * val[M21];
			const float v02 = m.val[M00] * val[M02] + m.val[M01] * val[M12] + m.val[M02] * val[M22];

			const float v10 = m.val[M10] * val[M00] + m.val[M11] * val[M10] + m.val[M12] * val[M20];
			const float v11 = m.val[M10] * val[M01] + m.val[M11] * val[M11] + m.val[M12] * val[M21];
			const float v12 = m.val[M10] * val[M02] + m.val[M11] * val[M12] + m.val[M12] * val[M22];

			const float v20 = m.val[M20] * val[M00] + m.val[M21] * val[M10] + m.val[M22] * val[M20];
			const float v21 = m.val[M20] * val[M01] + m.val[M21] * val[M11] + m.val[M22] * val[M21];
			const float v22 = m.val[M20] * val[M02] + m.val[M21] * val[M12] + m.val[M22] * val[M22];

			val[M00] = v00;
			val[M10] = v10;
			val[M20] = v20;
			val[M01] = v01;
			val[M11] = v11;
			val[M21] = v21;
			val[M02] = v02;
			val[M12] = v12;
			val[M22] = v22;

			return *this;
		}

		Matrix3D& setToRotation(const float degrees) {
			return setToRotationRad(Math::DEGREES_TO_RADIANS * degrees);
		}

		Matrix3D& setToRotationRad(const float radians) {
			const float cos = Math::cos(radians);
			const float sin = Math::sin(radians);

			val[M00] = cos;
			val[M10] = sin;
			val[M20] = 0;

			val[M01] = -sin;
			val[M11] = cos;
			val[M21] = 0;

			val[M02] = 0;
			val[M12] = 0;
			val[M22] = 1;

			return *this;
		}

		Matrix3D& setToRotation(const Vector3D axis, const float degrees) {
			return setToRotation(axis, Math::cosDeg(degrees), Math::sinDeg(degrees));
		}

		Matrix3D& setToRotation(const Vector3D axis, const float cos, const float sin) {
			const float oc = 1.0f - cos;
			val[M00] = oc * axis.x * axis.x + cos;
			val[M10] = oc * axis.x * axis.y - axis.z * sin;
			val[M20] = oc * axis.z * axis.x + axis.y * sin;
			val[M01] = oc * axis.x * axis.y + axis.z * sin;
			val[M11] = oc * axis.y * axis.y + cos;
			val[M21] = oc * axis.y * axis.z - axis.x * sin;
			val[M02] = oc * axis.z * axis.x - axis.y * sin;
			val[M12] = oc * axis.y * axis.z + axis.x * sin;
			val[M22] = oc * axis.z * axis.z + cos;
			return *this;
		}

		Matrix3D& setToTranslation(const float x, const float y) {
			val[M00] = 1;
			val[M10] = 0;
			val[M20] = 0;

			val[M01] = 0;
			val[M11] = 1;
			val[M21] = 0;

			val[M02] = x;
			val[M12] = y;
			val[M22] = 1;

			return *this;
		}

		Matrix3D& setTranslation(const float x, const float y) {
			val[M02] = x;
			val[M12] = y;
			val[M22] = 1;

			return *this;
		}

		Matrix3D& setToTranslation(const Vector2D& translation) {
			val[M00] = 1;
			val[M10] = 0;
			val[M20] = 0;

			val[M01] = 0;
			val[M11] = 1;
			val[M21] = 0;

			val[M02] = translation.x;
			val[M12] = translation.y;
			val[M22] = 1;

			return *this;
		}

		Matrix3D& setToScaling(const float scaleX, const float scaleY) {
			val[M00] = scaleX;
			val[M10] = 0;
			val[M20] = 0;
			val[M01] = 0;
			val[M11] = scaleY;
			val[M21] = 0;
			val[M02] = 0;
			val[M12] = 0;
			val[M22] = 1;
			return *this;
		}

		Matrix3D& setToScaling(const Vector2D& scale) {
			val[M00] = scale.x;
			val[M10] = 0;
			val[M20] = 0;
			val[M01] = 0;
			val[M11] = scale.y;
			val[M21] = 0;
			val[M02] = 0;
			val[M12] = 0;
			val[M22] = 1;
			return *this;
		}

		[[nodiscard]] std::string toString() const {
			std::stringstream ss;

			ss << *this;

			return ss.str();
		}

		[[nodiscard]] float det() const {
			return val[M00] * val[M11] * val[M22] + val[M01] * val[M12] * val[M20] + val[M02] * val[M10] * val[M21] - val[M00]
			                                                                                                          * val[M12] * val[M21] - val[M01] * val[M10] * val[M22] - val[M02] * val[M11] * val[M20];
		}

		Matrix3D& inv() {
			const float detV = det();
			if (detV == 0) throw std::exception("Can't invert a singular matrix");  // NOLINT(clang-diagnostic-float-equal)

			const float inv_det = 1.0f / detV;

			tmp[M00] = val[M11] * val[M22] - val[M21] * val[M12];
			tmp[M10] = val[M20] * val[M12] - val[M10] * val[M22];
			tmp[M20] = val[M10] * val[M21] - val[M20] * val[M11];
			tmp[M01] = val[M21] * val[M02] - val[M01] * val[M22];
			tmp[M11] = val[M00] * val[M22] - val[M20] * val[M02];
			tmp[M21] = val[M20] * val[M01] - val[M00] * val[M21];
			tmp[M02] = val[M01] * val[M12] - val[M11] * val[M02];
			tmp[M12] = val[M10] * val[M02] - val[M00] * val[M12];
			tmp[M22] = val[M00] * val[M11] - val[M10] * val[M01];

			val[M00] = inv_det * tmp[M00];
			val[M10] = inv_det * tmp[M10];
			val[M20] = inv_det * tmp[M20];
			val[M01] = inv_det * tmp[M01];
			val[M11] = inv_det * tmp[M11];
			val[M21] = inv_det * tmp[M21];
			val[M02] = inv_det * tmp[M02];
			val[M12] = inv_det * tmp[M12];
			val[M22] = inv_det * tmp[M22];

			return *this;
		}

		Matrix3D& set(const Matrix3D& mat) {
			std::ranges::copy(mat.val, val.begin());
			return *this;
		}

		Matrix3D& set(const float values[]) {
			// if (size < 9)throw std::exception("Cannot Construct A Matrix Without 9 Numbers.");
			std::copy_n(values, 9, val.begin());
			return *this;
		}

		Matrix3D& trn(const Vector2D& vector) {
			val[M02] += vector.x;
			val[M12] += vector.y;
			return *this;
		}

		Matrix3D& trn(const float x, const float y) {
			val[M02] += x;
			val[M12] += y;
			return *this;
		}

		Matrix3D& trn(const Vector3D vector) {
			val[M02] += vector.x;
			val[M12] += vector.y;
			return *this;
		}

		Matrix3D& translate(const float x, const float y) {
			tmp[M00] = 1;
			tmp[M10] = 0;
			tmp[M20] = 0;

			tmp[M01] = 0;
			tmp[M11] = 1;
			tmp[M21] = 0;

			tmp[M02] = x;
			tmp[M12] = y;
			tmp[M22] = 1;
			mul(val, tmp);
			return *this;
		}

		Matrix3D& translate(const Vector2D& translation) {
			tmp[M00] = 1;
			tmp[M10] = 0;
			tmp[M20] = 0;

			tmp[M01] = 0;
			tmp[M11] = 1;
			tmp[M21] = 0;

			tmp[M02] = translation.x;
			tmp[M12] = translation.y;
			tmp[M22] = 1;
			mul(val, tmp);
			return *this;
		}

		Matrix3D& rotate(const float degrees) {
			return rotateRad(Math::DEGREES_TO_RADIANS * degrees);
		}

		Matrix3D& rotateRad(const float radians) {
			if (radians == 0) return *this;  // NOLINT(clang-diagnostic-float-equal)
			const float cos = Math::cos(radians);
			const float sin = Math::sin(radians);

			tmp[M00] = cos;
			tmp[M10] = sin;
			tmp[M20] = 0;

			tmp[M01] = -sin;
			tmp[M11] = cos;
			tmp[M21] = 0;

			tmp[M02] = 0;
			tmp[M12] = 0;
			tmp[M22] = 1;
			mul(val, tmp);
			return *this;
		}

		Matrix3D& scale(const float scaleX, const float scaleY) {
			tmp[M00] = scaleX;
			tmp[M10] = 0;
			tmp[M20] = 0;
			tmp[M01] = 0;
			tmp[M11] = scaleY;
			tmp[M21] = 0;
			tmp[M02] = 0;
			tmp[M12] = 0;
			tmp[M22] = 1;
			mul(val, tmp);
			return *this;
		}

		Matrix3D& scale(const Vector2D& scale) {
			tmp[M00] = scale.x;
			tmp[M10] = 0;
			tmp[M20] = 0;
			tmp[M01] = 0;
			tmp[M11] = scale.y;
			tmp[M21] = 0;
			tmp[M02] = 0;
			tmp[M12] = 0;
			tmp[M22] = 1;
			mul(val, tmp);
			return *this;
		}

		[[nodiscard]] MatRaw copyData() const{
			return MatRaw{val};
		}

		Vector2D& getTranslation(Vector2D& position) const {
			position.x = val[M02];
			position.y = val[M12];
			return position;
		}

		Vector2D& getScale(Vector2D& scale) const {
			scale.x = sqrt(val[M00] * val[M00] + val[M01] * val[M01]);
			scale.y = sqrt(val[M10] * val[M10] + val[M11] * val[M11]);
			return scale;
		}

		[[nodiscard]] float getRotation() const {
			return Math::RADIANS_TO_DEGREES * Math::atan2(val[M10], val[M00]);
		}

		[[nodiscard]] float getRotationRad() const {
			return Math::atan2(val[M10], val[M00]);
		}

		Matrix3D& scl(const float scale) {
			val[M00] *= scale;
			val[M11] *= scale;
			return *this;
		}

		Matrix3D& scl(const Vector2D& scale) {
			val[M00] *= scale.x;
			val[M11] *= scale.y;
			return *this;
		}

		Matrix3D& scl(const Vector3D scale) {
			val[M00] *= scale.x;
			val[M11] *= scale.y;
			return *this;
		}

		Matrix3D& transpose() {
			// Where MXY you do not have to change MXX

			std::swap(val[M10], val[M01]);
			std::swap(val[M20], val[M02]);
			std::swap(val[M12], val[M21]);

			return *this;
		}

	};


}
