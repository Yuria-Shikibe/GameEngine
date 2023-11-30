export module Geom.Shape.Rect_Orthogonal;

import <algorithm>;
import <vector>;

import Concepts;
import Math;
import Geom.Vector2D;
import Geom.Shape;
import <ostream>;

export namespace Geom::Shape{
	/**
	 * \brief All params should be positive. At least when it is going to make geom calculation.
	 * \tparam T 
	 */
	template <Concepts::Number T>
	class Rect_Orthogonal final : virtual public Shape<Rect_Orthogonal<T>, T>{
		static constexpr T HALF = static_cast<T>(2);

		T srcX{0};
		T srcY{0};
		T width{0};
		T height{0};

	public:
		Rect_Orthogonal(const T srcX, const T srcY, const T width, const T height)
			: srcX(srcX),
			  srcY(srcY){
			this->setSize(width, height);
		}

		Rect_Orthogonal(const Vector2D& center, const T width, const T height){
			this->setSize(width, height);
			this->setCenter(center.x, center.y);
		}

		Rect_Orthogonal(const T width, const T height){
			this->setSize(width, height);
		}

		explicit Rect_Orthogonal(const T size){
			this->setSize(size, size);
		}

		Rect_Orthogonal() = default;

		~Rect_Orthogonal() = default;

		Rect_Orthogonal(const Rect_Orthogonal& other)
			: srcX(other.srcX),
			  srcY(other.srcY),
			  width(other.width),
			  height(other.height){
		}

		Rect_Orthogonal(Rect_Orthogonal&& other) noexcept
			: srcX(other.srcX),
			srcY(other.srcY),
			width(other.width),
			height(other.height) {
		}

		Rect_Orthogonal& operator=(const Rect_Orthogonal& other){
			if(this == &other) return *this;
			srcX = other.srcX;
			srcY = other.srcY;
			width = other.width;
			height = other.height;
			return *this;
		}

		Rect_Orthogonal& operator=(Rect_Orthogonal&& other) noexcept {
			if (this == &other) return *this;
			srcX = other.srcX;
			srcY = other.srcY;
			width = other.width;
			height = other.height;
			return *this;
		}

		friend bool operator==(const Rect_Orthogonal& lhs, const Rect_Orthogonal& rhs) {
			return lhs.srcX == rhs.srcX
			       && lhs.srcY == rhs.srcY
			       && lhs.width == rhs.width
			       && lhs.height == rhs.height;
		}

		friend bool operator!=(const Rect_Orthogonal& lhs, const Rect_Orthogonal& rhs) {
			return !(lhs == rhs);
		}

		friend std::ostream& operator<<(std::ostream& os, const Rect_Orthogonal& obj) {
			return os
			       << "srcX: " << obj.srcX
			       << " srcY: " << obj.srcY
			       << " width: " << obj.width
			       << " height: " << obj.height;
		}

		[[nodiscard]] T getSrcX() const{
			return srcX;
		}

		void setSrcX(const T x){
			this->srcX = x;
		}

		[[nodiscard]] T getSrcY() const{
			return srcY;
		}

		void setSrcY(const T y){
			this->srcY = y;
		}

		[[nodiscard]] T getWidth() const{
			return width;
		}

		void setWidth(const T w){
			if(w >= 0){
				this->width = w;
			} else{
				T abs = w < 0 ? -w : w; // abs
				srcX -= abs;
				this->width = abs;
			}
		}

		[[nodiscard]] T getHeight() const{
			return height;
		}

		[[nodiscard]] Vector2D getCenter() const{
			return { srcX + width / HALF, srcY + height / HALF };
		}

		template <Concepts::Number T_>
		Rect_Orthogonal<T_> as() const {
			return Rect_Orthogonal<T_>{
				static_cast<T_>(srcX),
				static_cast<T_>(srcY),
				static_cast<T_>(width),
				static_cast<T_>(height),
			};
		}

		void setHeight(const T h){
			if (h >= 0) {
				this->height = h;
			}
			else {
				T abs = h < 0 ? -h : h; // abs
				srcY -= abs;
				this->height = abs;
			}
		}

		[[nodiscard]] bool contains(const Rect_Orthogonal& other) const override{
			return
				other.srcX > srcX && other.srcX + other.width < srcX + width &&
				other.srcY > srcY && other.srcY + other.height < srcY + height;
		}

		[[nodiscard]] bool overlap(const Rect_Orthogonal& other) const override{
			return !(
				srcX >= other.srcX + other.width  ||
				srcX +  width	  <= other.srcX   ||
				srcY >= other.srcY + other.height ||
				srcY +  height	  <= other.srcY
			);
		}

		[[nodiscard]] bool inBound_edgeExclusive(const Vector2D& v) const override{
			return v.x > srcX && v.y > srcY && v.x < srcX + width && v.y < srcY + height;
		}

		[[nodiscard]] bool inBound_edgeInclusive(const Vector2D& v) const override{
			return v.x >= srcX && v.y >= srcY && v.x <= srcX + width && v.y <= srcY + height;
		}

		[[nodiscard]] T getEndX() const{
			return srcX + width;
		}

		[[nodiscard]] T getEndY() const{
			return srcY + height;
		}

		[[nodiscard]] T maxDiagonalSqLen() const override{
			return width * width + height * height;
		}

		Rect_Orthogonal& setSrc(const T x, const T y){
			srcX = x;
			srcY = y;

			return *this;
		}

		Rect_Orthogonal& setSrc(const Vector2D& v) {
			srcX = v.x;
			srcY = v.y;

			return *this;
		}

		Rect_Orthogonal& setSize(const T x, const T y) {
			this->setWidth(x);
			this->setHeight(y);

			return *this;
		}

		Rect_Orthogonal& move(const T x, const T y) {
			srcX += x;
			srcY += y;

			return *this;
		}

		template <Concepts::Number T1, Concepts::Number T2>
		Rect_Orthogonal& scl(const T1 xScl, const T2 yScl) {
			width = static_cast<T>(static_cast<T1>(width) * xScl);
			height = static_cast<T>(static_cast<T1>(height) * yScl);

			return *this;
		}

		Rect_Orthogonal& setSize(const Vector2D& v) {
			return *setSize(v.x, v.y);
		}

		Rect_Orthogonal& setCenter(const T x, const T y){
			this->setSrc(x - width / HALF, y - height / HALF);

			return *this;
		}

		Rect_Orthogonal& setCenter(const Vector2D& v) {
			this->setSrc(static_cast<T>(v.x) - width / HALF, static_cast<T>(v.y) - height / HALF);

			return *this;
		}

		[[nodiscard]] float xOffsetRatio(const T x) const{
			return Math::curve(x, static_cast<float>(srcX), static_cast<float>(srcX + width));
		}

		[[nodiscard]] float yOffsetRatio(const T y) const{
			return Math::curve(y, static_cast<float>(srcY), static_cast<float>(srcY + height));
		}

		[[nodiscard]] Vector2D offsetRatio(const Vector2D& v){
			return { xOffsetRatio(v.x), yOffsetRatio(v.y) };
		}

		[[nodiscard]] Vector2D vert_00()const {
			return { static_cast<float>(srcX), static_cast<float>(srcY) };
		}

		[[nodiscard]] Vector2D vert_10() const {
			return { static_cast<float>(srcX + width), static_cast<float>(srcY) };
		}

		[[nodiscard]] Vector2D vert_01() const {
			return { static_cast<float>(srcX), static_cast<float>(srcY + height) };
		}

		[[nodiscard]] Vector2D vert_11() const {
			return {static_cast<float>(srcX + width), static_cast<float>(srcY + height) };
		}

		[[nodiscard]] T area() const{
			return width * height;
		}

		[[nodiscard]] float ratio() const{
			return static_cast<float>(width) / static_cast<float>(height);
		}

		std::vector<Vector2D>& vertices(std::vector<Vector2D>& collector) const override{
			collector.push_back(vert_00());
			collector.push_back(vert_01());
			collector.push_back(vert_11());
			collector.push_back(vert_01());

			return collector;
		}
	};

	using OrthoRectFloat = Rect_Orthogonal<float>;
	using OrthoRectInt = Rect_Orthogonal<int>;
	using OrthoRectUInt = Rect_Orthogonal<unsigned int>;
}
