export module Geom.Rect_Orthogonal;

import std;

import Concepts;
import Math;
import Geom.Vector2D;

export namespace Geom{
	/**
	 * \brief width, height should be always non-negative.
	 * \tparam T Arithmetic Type
	 */
	template <Concepts::Number T>
	class Rect_Orthogonal/* : public Shape<Rect_Orthogonal<T>, T>*/{
		static constexpr T TWO{2};

		T srcX{0};
		T srcY{0};

		T width{0};
		T height{0};

	public:
		constexpr Rect_Orthogonal(const T srcX, const T srcY, const T width, const T height) noexcept
			: srcX(srcX),
			  srcY(srcY){
			this->setSize(width, height);
		}

		constexpr Rect_Orthogonal(const typename Vector2D<T>::PassType center, const T width, const T height) noexcept{
			this->setSize(width, height);
			this->setCenter(center.x, center.y);
		}

		constexpr Rect_Orthogonal(const T width, const T height) noexcept{
			this->setSize(width, height);
		}

		constexpr explicit Rect_Orthogonal(const T size) noexcept{
			this->setSize(size, size);
		}

		constexpr Rect_Orthogonal() noexcept = default;

		constexpr ~Rect_Orthogonal() noexcept = default;

		Rect_Orthogonal(const Rect_Orthogonal& other) noexcept = default;

		Rect_Orthogonal(Rect_Orthogonal&& other) noexcept = default;

		Rect_Orthogonal& operator=(const Rect_Orthogonal& other) noexcept = default;

		Rect_Orthogonal& operator=(Rect_Orthogonal&& other) noexcept = default;

		friend constexpr bool operator==(const Rect_Orthogonal& lhs, const Rect_Orthogonal& rhs) noexcept{
			return lhs.srcX == rhs.srcX
			       && lhs.srcY == rhs.srcY
			       && lhs.width == rhs.width
			       && lhs.height == rhs.height;
		}

		friend constexpr bool operator!=(const Rect_Orthogonal& lhs, const Rect_Orthogonal& rhs) noexcept{
			return !(lhs == rhs);
		}

		friend std::ostream& operator<<(std::ostream& os, const Rect_Orthogonal& obj) noexcept{
			return os
			       << "srcX: " << obj.srcX
			       << " srcY: " << obj.srcY
			       << " width: " << obj.width
			       << " height: " << obj.height;
		}

		[[nodiscard]] constexpr T getSrcX() const noexcept{
			return srcX;
		}

		[[nodiscard]] constexpr Geom::Vector2D<T> getSrc() const noexcept{
			return {srcX, srcY};
		}

		[[nodiscard]] constexpr Geom::Vector2D<T> getEnd() const noexcept{
			return {getEndX(), getEndY()};
		}

		[[nodiscard]] constexpr T* getSrcXRaw() noexcept{
			return &srcX;
		}

		constexpr void setSrcX(const T x) noexcept{
			this->srcX = x;
		}

		Rect_Orthogonal& expandBy(const Rect_Orthogonal& other) noexcept{
			Geom::Vector2D<T> min{Math::min(getSrcX(), other.getSrcX()), Math::min(getSrcY(), other.getSrcY())};
			Geom::Vector2D<T> max{Math::max(getEndX(), other.getEndX()), Math::max(getEndY(), other.getEndY())};

			this->setVert(min, max);

			return *this;
		}

		[[nodiscard]] constexpr T getSrcY() const noexcept{
			return srcY;
		}

		[[nodiscard]] constexpr T* getSrcYRaw() noexcept{
			return &srcY;
		}

		constexpr void setSrcY(const T y) noexcept{
			this->srcY = y;
		}

		[[nodiscard]] constexpr T getWidth() const noexcept{
			return width;
		}

		[[nodiscard]] constexpr Vector2D<T> getSize() const noexcept{
			return {width, height};
		}

		[[nodiscard]] constexpr T getHeight() const noexcept{
			return height;
		}

		[[nodiscard]] constexpr T* getWidthRaw() noexcept{
			return &width;
		}

		[[nodiscard]] constexpr T* getHeightRaw() noexcept{
			return &height;
		}

		template <Concepts::Number T_>
		constexpr Rect_Orthogonal<T_> as() const noexcept{
			return Rect_Orthogonal<T_>{
				static_cast<T_>(srcX),
				static_cast<T_>(srcY),
				static_cast<T_>(width),
				static_cast<T_>(height),
			};
		}

		template <Concepts::Number N>
		constexpr void setWidth(const N w) noexcept{
			if constexpr(std::is_unsigned_v<N>) {
				this->width = w;
			}else {
				if(w >= 0){
					this->width = static_cast<T>(w);
				}else{
					T abs = static_cast<T>(w < 0 ? -w : w);
					srcX -= abs;
					this->width = abs;
				}
			}
		}

		template <Concepts::Number N>
		constexpr void setHeight(const N h) noexcept{
			if constexpr(std::is_unsigned_v<N>) {
				this->height = h;
			}else {
				if(h >= 0){
					this->height = static_cast<T>(h);
				}else{
					T abs = static_cast<T>(h < 0 ? -h : h);
					srcY -= abs;
					this->height = abs;
				}
			}
		}

		constexpr Rect_Orthogonal& addSize(const T x, const T y) noexcept requires Concepts::Signed<T> {
			this->template setWidth<T>(width + x);
			this->template setHeight<T>(height + y);

			return *this;
		}

		constexpr Rect_Orthogonal& addWidth(const T x) noexcept requires Concepts::Signed<T> {
			this->template setWidth<T>(width + x);

			return *this;
		}

		constexpr Rect_Orthogonal& addHeight(const T y) noexcept requires Concepts::Signed<T> {
			this->template setHeight<T>(height + y);

			return *this;
		}

		constexpr Rect_Orthogonal& addSize(const T x, const T y) noexcept requires Concepts::NonNegative<T> {
			using S = std::make_signed_t<T>;
			this->template setWidth<S>(static_cast<S>(width) + static_cast<S>(x));
			this->template setHeight<S>(static_cast<S>(height) + static_cast<S>(y));

			return *this;
		}

		template <Concepts::Number N>
		constexpr Rect_Orthogonal& addSize(const N x, const N y) noexcept{
			using S = std::make_signed_t<T>;
			this->template setWidth<S>(static_cast<S>(width) + static_cast<S>(x));
			this->template setHeight<S>(static_cast<S>(height) + static_cast<S>(y));

			return *this;
		}

		constexpr void setLargerWidth(const T v) noexcept{
			if constexpr(std::is_unsigned_v<T>) {
				if(v > width) {
					this->template setWidth<T>(v);
				}
			}else {
				T abs = static_cast<T>(v < 0 ? -v : v);
				if(abs > width) {
					this->template setWidth<T>(v);
				}
			}

		}

		constexpr void setLargerHeight(const T v) noexcept{
			if constexpr(std::is_unsigned_v<T>) {
				if(v > height) {
					this->template setHeight<T>(v);
				}
			}else {
				T abs = static_cast<T>(v < 0 ? -v : v);
				if(abs > height) {
					this->template setHeight<T>(v);
				}
			}
		}

		constexpr void setShorterWidth(const T v) noexcept{
			if constexpr(std::is_unsigned_v<T>) {
				if(v < width) {
					this->template setWidth<T>(v);
				}
			}else {
				T abs = static_cast<T>(v < 0 ? -v : v);
				if(abs < width) {
					this->template setWidth<T>(v);
				}
			}

		}

		constexpr void setShorterHeight(const T v) noexcept{
			if constexpr(std::is_unsigned_v<T>) {
				if(v < height) {
					this->template setHeight<T>(v);
				}
			}else {
				T abs = static_cast<T>(v < 0 ? -v : v);
				if(abs < height) {
					this->template setHeight<T>(v);
				}
			}
		}

		[[nodiscard]] constexpr bool containsStrict(const Rect_Orthogonal& other) const noexcept{
			return
				other.srcX > srcX && other.srcX + other.width < srcX + width &&
				other.srcY > srcY && other.srcY + other.height < srcY + height;
		}

		[[nodiscard]] constexpr bool contains(const Rect_Orthogonal& other) const noexcept{
			return
				other.srcX >= srcX && other.getEndX() < getEndX() &&
				other.srcY >= srcY && other.getEndY() < getEndY();
		}

		[[nodiscard]] constexpr Rect_Orthogonal getOverlap(const Rect_Orthogonal& r) noexcept{
			T minEndX = Math::min(getEndX(), r.getEndX());
			T minEndY = Math::min(getEndY(), r.getEndY());
			T maxSrcX = Math::max(getSrcX(), r.getSrcX());
			T maxSrcY = Math::max(getSrcY(), r.getSrcY());

			return Rect_Orthogonal{maxSrcX, maxSrcY, Math::clampPositive(minEndX - maxSrcX), Math::clampPositive(minEndY - maxSrcY)};
		}

		[[nodiscard]] constexpr bool overlap(const Rect_Orthogonal& r) const noexcept{
			return
				getSrcX() < r.getEndX() &&
				getEndX() > r.getSrcX() &&
				getSrcY() < r.getEndY() &&
				getEndY() > r.getSrcY();
		}

		[[nodiscard]] constexpr bool overlap(const Rect_Orthogonal& r, const typename Vector2D<T>::PassType selfTrans, const typename Vector2D<T>::PassType otherTrans) const noexcept{
			return
				getSrcX() + selfTrans.x < r.getEndX() + otherTrans.x &&
				getEndX() + selfTrans.x > r.getSrcX() + otherTrans.x &&
				getSrcY() + selfTrans.y < r.getEndY() + otherTrans.y &&
				getEndY() + selfTrans.y > r.getSrcY() + otherTrans.y;
		}

		[[nodiscard]] constexpr bool containsPos_edgeExclusive(const typename Vector2D<T>::PassType v) const noexcept{
			return v.x > srcX && v.y > srcY && v.x < getEndX() && v.y < getEndY();
		}

		[[nodiscard]] constexpr bool containsPos_edgeInclusive(const typename Vector2D<T>::PassType v) const noexcept{
			return v.x >= srcX && v.y >= srcY && v.x < getEndX() && v.y < getEndY();
		}

		[[nodiscard]] constexpr T getEndX() const noexcept{
			return srcX + width;
		}

		[[nodiscard]] constexpr T getEndY() const noexcept{
			return srcY + height;
		}

		[[nodiscard]] constexpr T getCenterX() const noexcept{
			return srcX + width / TWO;
		}

		[[nodiscard]] constexpr T getCenterY() const noexcept{
			return srcY + height / TWO;
		}

		[[nodiscard]] constexpr Vector2D<T> getCenter() const noexcept{
			return {getCenterX(), getCenterY()};
		}

		[[nodiscard]] constexpr T maxDiagonalSqLen() const noexcept{
			return width * width + height * height;
		}

		constexpr Rect_Orthogonal& setSrc(const T x, const T y) noexcept{
			srcX = x;
			srcY = y;

			return *this;
		}

		constexpr Rect_Orthogonal& setEnd(const T x, const T y) noexcept{
			this->setWidth(x - srcX);
			this->setHeight(y - srcY);

			return *this;
		}

		constexpr Rect_Orthogonal& setEndX(const T x) noexcept{
			this->setWidth(x - srcX);

			return *this;
		}

		constexpr Rect_Orthogonal& setEndY(const T y) noexcept{
			this->setHeight(y - srcY);

			return *this;
		}

		constexpr Rect_Orthogonal& setSrc(const typename Vector2D<T>::PassType v) noexcept{
			srcX = v.x;
			srcY = v.y;

			return *this;
		}

		constexpr Rect_Orthogonal& setSize(const T x, const T y) noexcept{
			this->setWidth(x);
			this->setHeight(y);

			return *this;
		}

		constexpr Rect_Orthogonal& setSize(const Rect_Orthogonal& other) noexcept{
			this->setWidth(other.width);
			this->setHeight(other.height);

			return *this;
		}

		constexpr Rect_Orthogonal& moveY(const T y) noexcept{
			srcY += y;

			return *this;
		}

		constexpr Rect_Orthogonal& moveX(const T x) noexcept{
			srcX += x;

			return *this;
		}

		constexpr Rect_Orthogonal& move(const T x, const T y) noexcept{
			srcX += x;
			srcY += y;

			return *this;
		}

		constexpr Rect_Orthogonal& move(const typename Geom::Vector2D<T>::PassType vec) noexcept{
			srcX += vec.x;
			srcY += vec.y;

			return *this;
		}

		constexpr Rect_Orthogonal& setSrc(const Rect_Orthogonal& other) noexcept{
			srcX = other.srcX;
			srcY = other.srcY;

			return *this;
		}

		template <Concepts::Number T1, Concepts::Number T2>
		constexpr Rect_Orthogonal& sclSize(const T1 xScl, const T2 yScl) noexcept{
			width = static_cast<T>(static_cast<T1>(width) * xScl);
			height = static_cast<T>(static_cast<T1>(height) * yScl);

			return *this;
		}

		template <Concepts::Number T1, Concepts::Number T2>
		constexpr Rect_Orthogonal& sclPos(const T1 xScl, const T2 yScl) noexcept{
			srcX = static_cast<T>(static_cast<T1>(srcX) * xScl);
			srcY = static_cast<T>(static_cast<T1>(srcY) * yScl);

			return *this;
		}

		template <Concepts::Number T1, Concepts::Number T2>
		constexpr Rect_Orthogonal& scl(const T1 xScl, const T2 yScl) noexcept{
			(void)this->template sclPos<T1, T2>(xScl, yScl);
			(void)this->template sclSize<T1, T2>(xScl, yScl);

			return *this;
		}

		constexpr void set(const T srcx, const T srcy, const T width, const T height) noexcept{
			srcX = srcx;
			srcY = srcy;

			this->template setWidth<T>(width);
			this->template setHeight<T>(height);
		}

		template <std::integral N>
		Rect_Orthogonal<N> trac() noexcept{
			return Rect_Orthogonal<N>{Math::trac<N>(srcX), Math::trac<N>(srcY), Math::trac<N>(width), Math::trac<N>(height)};
		}

		template <std::integral N>
		Rect_Orthogonal<N> round() noexcept{
			return Rect_Orthogonal<N>{Math::round<N>(srcX), Math::round<N>(srcY), Math::round<N>(width), Math::round<N>(height)};
		}

		constexpr Rect_Orthogonal& setSize(const typename Vector2D<T>::PassType v) noexcept{
			return this->setSize(v.x, v.y);
		}

		constexpr Rect_Orthogonal& setCenter(const T x, const T y) noexcept{
			this->setSrc(x - width / TWO, y - height / TWO);

			return *this;
		}

		constexpr Rect_Orthogonal& setCenter(const typename Vector2D<T>::PassType v) noexcept{
			this->setSrc(v.x - width / TWO, v.y - height / TWO);

			return *this;
		}

		[[nodiscard]] constexpr float xOffsetRatio(const T x) const noexcept{
			return Math::curve(x, static_cast<float>(srcX), static_cast<float>(srcX + width));
		}

		[[nodiscard]] constexpr float yOffsetRatio(const T y) const noexcept{
			return Math::curve(y, static_cast<float>(srcY), static_cast<float>(srcY + height));
		}

		[[nodiscard]] constexpr Vec2 offsetRatio(const Vec2& v) noexcept{
			return { xOffsetRatio(v.x), yOffsetRatio(v.y) };
		}

		[[nodiscard]] constexpr Vector2D<T> vert_00()const noexcept{
			return { srcX, srcY };
		}

		[[nodiscard]] constexpr Vector2D<T> vert_10() const noexcept{
			return { srcX + width, srcY };
		}

		[[nodiscard]] constexpr Vector2D<T> vert_01() const noexcept{
			return { srcX, srcY + height };
		}

		[[nodiscard]] constexpr Vector2D<T> vert_11() const noexcept{
			return { srcX + width, srcY + height };
		}

		[[nodiscard]] constexpr T area() const noexcept{
			return width * height;
		}

		[[nodiscard]] constexpr float ratio() const noexcept{
			return static_cast<float>(width) / static_cast<float>(height);
		}

		std::vector<Vec2>& vertices(std::vector<Vec2>& collector) const noexcept{
			collector.push_back(vert_00());
			collector.push_back(vert_01());
			collector.push_back(vert_11());
			collector.push_back(vert_01());

			return collector;
		}

		constexpr Rect_Orthogonal& setVert(const T srcX, const T srcY, const T endX, const T endY) noexcept{
			auto [minX, maxX] = Math::minmax(srcX, endX);
			auto [minY, maxY] = Math::minmax(srcY, endY);
			this->set(minX, minY, maxX - minX, maxY - minY);

			return *this;
		}

		constexpr Rect_Orthogonal& setVert(const typename Vector2D<T>::PassType src, const typename Vector2D<T>::PassType end) noexcept{
			return this->setVert(src.x, src.y, end.x, end.y);
		}

		constexpr void expand(const T x, const T y) noexcept{
			this->set(srcX - x, srcY - y, width + x * TWO,  height + y * TWO);
		}

		/**
		 * @brief
		 * @param marginX Negative is acceptable
		 * @return
		 */
		constexpr Rect_Orthogonal& shrinkX(T marginX) noexcept{
			marginX = Math::min(marginX, width / TWO);
			srcX += marginX;
			width -= marginX * TWO;

			return *this;
		}

		constexpr Rect_Orthogonal& shrinkY(T marginY) noexcept{
			marginY = Math::min(marginY, height / TWO);
			srcY += marginY / TWO;
			height -= marginY * TWO;

			return *this;
		}

		constexpr Rect_Orthogonal& shrink(const T marginX, const T marginY) noexcept{
			(void)this->shrinkX(marginX);
			(void)this->shrinkY(marginY);

			return *this;
		}

		constexpr Rect_Orthogonal& shrink(const T margin) noexcept{
			return this->shrink(margin, margin);
		}

		[[nodiscard]] constexpr Rect_Orthogonal copy() noexcept{
			return *this;
		}

		void each(Concepts::Invokable<void(Vector2D<T>)> auto&& pred) const requires std::is_integral_v<T>{
			for(T x = srcX; x < getEndX(); ++x){
				for(T y = srcY; y < getEndY(); ++y){
					pred(Vector2D<T>{x, y});
				}
			}
		}

		struct iterator{
			Vector2D<T> cur{};
			T srcX{};
			T endX{};

			constexpr iterator& operator++() noexcept{
				++cur.x;
				if(cur.x == endX){
					cur.x = srcX;
					++cur.y;
				}

				return *this;
			}

			constexpr iterator operator++(int) noexcept{
				iterator cpy = *this;
				this->operator++();

				return cpy;
			}

			constexpr Vector2D<T> operator*() const noexcept{
				return cur;
			}

			constexpr Vector2D<T> operator->() const noexcept{
				return cur;
			}

			constexpr Vector2D<T> get() const noexcept{
				return cur;
			}

			constexpr friend bool operator==(const iterator& lhs, const iterator& rhs) noexcept{ return lhs.cur == rhs.cur; }

			constexpr friend bool operator!=(const iterator& lhs, const iterator& rhs) noexcept{ return !(lhs == rhs); }
		};

		constexpr iterator begin() const noexcept{
			return {getSrc(), getSrcX(), getEndX()};
		}

		constexpr iterator end() const noexcept{
			return {getEnd(), getEndX(), getEndX()};
		}
	};

	using OrthoRectFloat = Rect_Orthogonal<float>;
	using OrthoRectInt = Rect_Orthogonal<int>;
	using OrthoRectUInt = Rect_Orthogonal<unsigned int>;
}
