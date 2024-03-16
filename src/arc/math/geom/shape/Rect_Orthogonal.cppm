export module Geom.Shape.Rect_Orthogonal;

import <algorithm>;
import <vector>;

import Concepts;
import Math;
import Geom.Vector2D;
import <ostream>;

export namespace Geom::Shape{
	/**
	 * \brief All params should be positive. At least when it is going to make geom calculation.
	 * \tparam T 
	 */
	template <Concepts::Number T>
	class Rect_Orthogonal final/* : public Shape<Rect_Orthogonal<T>, T>*/{
		static constexpr T HALF = static_cast<T>(2);

		T srcX{0};
		T srcY{0};
		T width{0};
		T height{0};

	public:
		constexpr Rect_Orthogonal(const T srcX, const T srcY, const T width, const T height)
			: srcX(srcX),
			  srcY(srcY){
			this->setSize(width, height);
		}

		constexpr Rect_Orthogonal(const Vec2& center, const T width, const T height){
			this->setSize(width, height);
			this->setCenter(center.x, center.y);
		}

		constexpr Rect_Orthogonal(const T width, const T height){
			this->setSize(width, height);
		}

		constexpr explicit Rect_Orthogonal(const T size){
			this->setSize(size, size);
		}

		constexpr Rect_Orthogonal() = default;

		constexpr ~Rect_Orthogonal() = default;

		friend constexpr bool operator==(const Rect_Orthogonal& lhs, const Rect_Orthogonal& rhs) {
			return lhs.srcX == rhs.srcX
			       && lhs.srcY == rhs.srcY
			       && lhs.width == rhs.width
			       && lhs.height == rhs.height;
		}

		friend constexpr bool operator!=(const Rect_Orthogonal& lhs, const Rect_Orthogonal& rhs) {
			return !(lhs == rhs);
		}

		friend std::ostream& operator<<(std::ostream& os, const Rect_Orthogonal& obj) {
			return os
			       << "srcX: " << obj.srcX
			       << " srcY: " << obj.srcY
			       << " width: " << obj.width
			       << " height: " << obj.height;
		}

		[[nodiscard]] constexpr T getSrcX() const{
			return srcX;
		}

		[[nodiscard]] constexpr T* getSrcXRaw(){
			return &srcX;
		}

		constexpr void setSrcX(const T x){
			this->srcX = x;
		}

		[[nodiscard]] constexpr T getSrcY() const{
			return srcY;
		}

		[[nodiscard]] constexpr T* getSrcYRaw(){
			return &srcY;
		}

		constexpr void setSrcY(const T y){
			this->srcY = y;
		}

		[[nodiscard]] constexpr T getWidth() const{
			return width;
		}

		[[nodiscard]] constexpr T getHeight() const{
			return height;
		}

		[[nodiscard]] constexpr T* getWidthRaw(){
			return &width;
		}

		[[nodiscard]] constexpr T* getHeightRaw(){
			return &height;
		}

		template <Concepts::Number T_>
		constexpr Rect_Orthogonal<T_> as() const {
			return Rect_Orthogonal<T_>{
				static_cast<T_>(srcX),
				static_cast<T_>(srcY),
				static_cast<T_>(width),
				static_cast<T_>(height),
			};
		}

		template <Concepts::Number N>
		constexpr void setWidth(const N w){
			if constexpr(std::is_unsigned_v<N>) {
				this->width = w;
			}else {
				if(w >= 0){
					this->width = w;
				}else{
					T abs = w < 0 ? -w : w;
					srcX -= abs;
					this->width = abs;
				}
			}
		}

		template <Concepts::Number N>
		constexpr void setHeight(const N h){
			if constexpr(std::is_unsigned_v<N>) {
				this->height = h;
			}else {
				if(h >= 0){
					this->height = h;
				}else{
					T abs = h < 0 ? -h : h;
					srcY -= abs;
					this->height = abs;
				}
			}
		}

		constexpr Rect_Orthogonal& addSize(const T x, const T y) requires Concepts::Signed<T> {
			this->template setWidth<T>(width + x);
			this->template setHeight<T>(height + y);

			return *this;
		}

		constexpr Rect_Orthogonal& addSize(const T x, const T y) requires Concepts::NonNegative<T> {
			using S = std::make_signed_t<T>;
			this->template setWidth<S>(static_cast<S>(width) + static_cast<S>(x));
			this->template setHeight<S>(static_cast<S>(height) + static_cast<S>(y));

			return *this;
		}

		template <Concepts::Number N>
		constexpr Rect_Orthogonal& addSize(const N x, const N y){
			using S = std::make_signed_t<T>;
			this->template setWidth<S>(static_cast<S>(width) + static_cast<S>(x));
			this->template setHeight<S>(static_cast<S>(height) + static_cast<S>(y));

			return *this;
		}

		constexpr void setLargerWidth(const T v) {
			if constexpr(std::is_unsigned_v<T>) {
				if(v > width) {
					this->template setWidth<T>(v);
				}
			}else {
				T abs = v < 0 ? -v : v;
				if(abs > width) {
					this->template setWidth<T>(v);
				}
			}

		}

		constexpr void setLargerHeight(const T v) {
			if constexpr(std::is_unsigned_v<T>) {
				if(v > height) {
					this->template setHeight<T>(v);
				}
			}else {
				T abs = v < 0 ? -v : v;
				if(abs > height) {
					this->template setHeight<T>(v);
				}
			}
		}

		[[nodiscard]] constexpr bool contains(const Rect_Orthogonal& other) const{
			return
				other.srcX > srcX && other.srcX + other.width < srcX + width &&
				other.srcY > srcY && other.srcY + other.height < srcY + height;
		}

		[[nodiscard]] constexpr bool overlap(const Rect_Orthogonal& r) const{
			return
				getSrcX() < r.getEndX() &&
				getEndX() > r.getSrcX() &&
				getSrcY() < r.getEndY() &&
				getEndY() > r.getSrcY();
		}

		[[nodiscard]] constexpr bool containsPos_edgeExclusive(const Vec2& v) const{
			return v.x > srcX && v.y > srcY && v.x < srcX + width && v.y < srcY + height;
		}

		[[nodiscard]] constexpr bool containsPos_edgeInclusive(const Vec2& v) const{
			return v.x >= srcX && v.y >= srcY && v.x <= srcX + width && v.y <= srcY + height;
		}

		[[nodiscard]] constexpr T getEndX() const{
			return srcX + width;
		}

		[[nodiscard]] constexpr T getEndY() const{
			return srcY + height;
		}

		[[nodiscard]] constexpr T getCenterX() const{
			return srcX + width / HALF;
		}

		[[nodiscard]] constexpr T getCenterY() const{
			return srcY + height / HALF;
		}

		[[nodiscard]] constexpr Geom::Vector2D<T> getCenter() const{
			return {getCenterX(), getCenterY()};
		}

		[[nodiscard]] constexpr T maxDiagonalSqLen() const{
			return width * width + height * height;
		}

		constexpr Rect_Orthogonal& setSrc(const T x, const T y){
			srcX = x;
			srcY = y;

			return *this;
		}

		constexpr Rect_Orthogonal& setSrc(const Vec2& v) {
			srcX = v.x;
			srcY = v.y;

			return *this;
		}

		constexpr Rect_Orthogonal& setSize(const T x, const T y) {
			this->setWidth(x);
			this->setHeight(y);

			return *this;
		}

		constexpr Rect_Orthogonal& setSize(const Rect_Orthogonal& other) {
			this->setWidth(other.width);
			this->setHeight(other.height);

			return *this;
		}

		constexpr Rect_Orthogonal& move(const T x, const T y) {
			srcX += x;
			srcY += y;

			return *this;
		}

		constexpr Rect_Orthogonal& setSrc(const Rect_Orthogonal& other) {
			srcX = other.srcX;
			srcY = other.srcY;

			return *this;
		}

		template <Concepts::Number T1, Concepts::Number T2>
		constexpr Rect_Orthogonal& scl(const T1 xScl, const T2 yScl) {
			width = static_cast<T>(static_cast<T1>(width) * xScl);
			height = static_cast<T>(static_cast<T1>(height) * yScl);

			return *this;
		}

		constexpr void set(const T srcx, const T srcy, const T width, const T height) {
			srcX = srcx;
			srcY = srcy;

			this->template setWidth<T>(width);
			this->template setHeight<T>(height);
		}

		constexpr Rect_Orthogonal& setSize(const Vec2& v) {
			return this->setSize(static_cast<T>(v.x), static_cast<T>(v.y));
		}

		constexpr Rect_Orthogonal& setCenter(const T x, const T y){
			this->setSrc(x - width / HALF, y - height / HALF);

			return *this;
		}

		constexpr Rect_Orthogonal& setCenter(const Vec2& v) {
			this->setSrc(static_cast<T>(v.x) - width / HALF, static_cast<T>(v.y) - height / HALF);

			return *this;
		}

		[[nodiscard]] constexpr float xOffsetRatio(const T x) const{
			return Math::curve(x, static_cast<float>(srcX), static_cast<float>(srcX + width));
		}

		[[nodiscard]] constexpr float yOffsetRatio(const T y) const{
			return Math::curve(y, static_cast<float>(srcY), static_cast<float>(srcY + height));
		}

		[[nodiscard]] constexpr Vec2 offsetRatio(const Vec2& v){
			return { xOffsetRatio(v.x), yOffsetRatio(v.y) };
		}

		[[nodiscard]] constexpr Vec2 vert_00()const {
			return { static_cast<float>(srcX), static_cast<float>(srcY) };
		}

		[[nodiscard]] constexpr Vec2 vert_10() const {
			return { static_cast<float>(srcX + width), static_cast<float>(srcY) };
		}

		[[nodiscard]] constexpr Vec2 vert_01() const {
			return { static_cast<float>(srcX), static_cast<float>(srcY + height) };
		}

		[[nodiscard]] constexpr Vec2 vert_11() const {
			return {static_cast<float>(srcX + width), static_cast<float>(srcY + height) };
		}

		[[nodiscard]] constexpr T area() const{
			return width * height;
		}

		[[nodiscard]] constexpr float ratio() const{
			return static_cast<float>(width) / static_cast<float>(height);
		}

		std::vector<Vec2>& vertices(std::vector<Vec2>& collector) const{
			collector.push_back(vert_00());
			collector.push_back(vert_01());
			collector.push_back(vert_11());
			collector.push_back(vert_01());

			return collector;
		}

		constexpr void setVert(const T srcX, const T srcY, const T endX, const T endY) {
			this->set(srcX, srcY, endX - srcX, endY - srcY);
		}

		constexpr void expand(const T x, const T y){
			this->set(srcX - x, srcY - y, width + x * 2,  height + y * 2);
		}
	};

	using OrthoRectFloat = Rect_Orthogonal<float>;
	using OrthoRectInt = Rect_Orthogonal<int>;
	using OrthoRectUInt = Rect_Orthogonal<unsigned int>;
}
