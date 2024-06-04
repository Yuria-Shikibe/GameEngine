module;

export module UI.Align;

import Geom.Vector2D;
import Geom.Rect_Orthogonal;

import std;

export namespace Align{
	struct Spacing{
		/**@brief Left Spacing*/
		float left{};
		/**@brief Right Spacing*/
		float right{};
		/**@brief Bottom Spacing*/
		float bottom{};
		/**@brief Top Spacing*/
		float top{};

		[[nodiscard]] constexpr Geom::Vec2 bot_lft() const noexcept{
			return {left, bottom};
		}

		[[nodiscard]] constexpr Geom::Vec2 top_rit() const noexcept{
			return {right, top};
		}

		[[nodiscard]] constexpr Geom::Vec2 top_lft() const noexcept{
			return {left, top};
		}

		[[nodiscard]] constexpr Geom::Vec2 bot_rit() const noexcept{
			return {right,bottom};
		}

		[[nodiscard]] friend constexpr bool operator==(const Spacing& lhs, const Spacing& rhs) noexcept{
			return lhs.left == rhs.left
				&& lhs.right == rhs.right
				&& lhs.top == rhs.top
				&& lhs.bottom == rhs.bottom;
		}

		[[nodiscard]] friend constexpr bool operator!=(const Spacing& lhs, const Spacing& rhs) noexcept{ return !(lhs == rhs); }

		[[nodiscard]] friend constexpr bool operator==(const Spacing& lhs, const float val) noexcept{
			return lhs.left == val
				&& lhs.right == val
				&& lhs.top == val
				&& lhs.bottom == val;
		}

		[[nodiscard]] friend constexpr bool operator!=(const Spacing& lhs, const float rhs) noexcept{ return !(lhs == rhs); }

		constexpr void expand(float x, float y) noexcept{
			x *= 0.5f;
			y *= 0.5f;

			left += x;
			right += x;
			top += y;
			bottom += y;
		}

		constexpr void expand(const float val) noexcept{
			expand(val, val);
		}

		[[nodiscard]] constexpr float getWidth() const noexcept{
			return left + right;
		}

		[[nodiscard]] constexpr float getHeight() const noexcept{
			return bottom + top;
		}

		[[nodiscard]] constexpr float getRemainWidth(const float total = 1.0f) const noexcept{
			return total - getWidth();
		}

		[[nodiscard]] constexpr float getRemainHeight(const float total = 1.0f) const noexcept{
			return total - getHeight();
		}

		constexpr Spacing& set(const float val) noexcept{
			bottom = top = left = right = val;
			return *this;
		}

		constexpr Spacing& set(const float l, const float r, const float b, const float t) noexcept{
			left = l;
			right = r;
			bottom = b;
			top = t;
			return *this;
		}

		constexpr Spacing& setZero() noexcept{
			return set(0);
		}
	};

	enum class Layout : unsigned char{
		left     = 0b0000'0001,
		right    = 0b0000'0010,
		center_x = 0b0000'0100,

		top      = 0b0000'1000,
		bottom   = 0b0001'0000,
		center_y = 0b0010'0000,

		top_left   = top | left,
		top_center = top | center_x,
		top_right  = top | right,

		center_left  = center_y | left,
		center       = center_y | center_x,
		center_right = center_y | right,

		bottom_left   = bottom | left,
		bottom_center = bottom | center_x,
		bottom_right  = bottom | right,
	};

	enum class Scale : unsigned char{
		/** The source is not scaled. */
		none,

		/**
		 * Scales the source to fit the target while keeping the same aspect ratio. This may cause the source to be smaller than the
		 * target in one direction.
		 */
		fit,

		/**
		 * Scales the source to fit the target if it is larger, otherwise does not scale.
		 */
		bounded,

		/**
		 * Scales the source to fill the target while keeping the same aspect ratio. This may cause the source to be larger than the
		 * target in one direction.
		 */
		fill,

		/**
		 * Scales the source to fill the target in the x direction while keeping the same aspect ratio. This may cause the source to be
		 * smaller or larger than the target in the y direction.
		 */
		fillX,

		/**
		 * Scales the source to fill the target in the y direction while keeping the same aspect ratio. This may cause the source to be
		 * smaller or larger than the target in the x direction.
		 */
		fillY,

		/** Scales the source to fill the target. This may cause the source to not keep the same aspect ratio. */
		stretch,

		/**
		 * Scales the source to fill the target in the x direction, without changing the y direction. This may cause the source to not
		 * keep the same aspect ratio.
		 */
		stretchX,

		/**
		 * Scales the source to fill the target in the y direction, without changing the x direction. This may cause the source to not
		 * keep the same aspect ratio.
		 */
		stretchY,
	};

	template <Concepts::Number T>
	Geom::Vector2D<T> embedTo(const Scale stretch, Geom::Vector2D<T> src, Geom::Vector2D<T> tgt){
		switch(stretch){
			case Scale::fit :{
				const float targetRatio = tgt.y / tgt.x;
				const float sourceRatio = src.y / src.x;
				float scale = targetRatio > sourceRatio ? tgt.x / src.x : tgt.y / src.y;
				return {src.x * scale, src.y * scale};
			}
			case Scale::fill :{
				const float targetRatio = tgt.y / tgt.x;
				const float sourceRatio = src.y / src.x;
				float scale = targetRatio < sourceRatio ? tgt.x / src.x : tgt.y / src.y;
				return {src.x * scale, src.y * scale};
			}
			case Scale::fillX :{
				float scale = tgt.x / src.x;
				return {src.x * scale, src.y * scale};
			}
			case Scale::fillY :{
				float scale = tgt.y / src.y;
				return {src.x * scale, src.y * scale};
			}
			case Scale::stretch : return tgt;
			case Scale::stretchX : return {tgt.x, src.y};
			case Scale::stretchY : return {src.x, tgt.y};
			case Scale::bounded :
				if(src.y > tgt.y || src.x > tgt.x){
					return Align::embedTo<T>(Scale::fit, src, tgt);
				} else{
					return Align::embedTo<T>(Scale::none, src, tgt);
				}
			case Scale::none :
				return src;
		}
		
		std::unreachable();
	}

	constexpr char codeOf(Layout align){
		return static_cast<char>(align);
	}

	constexpr bool operator &(const Layout l, const Layout r){
		return codeOf(l) & codeOf(r);
	}

	constexpr Geom::Vec2 getOffsetOf(const Layout align, const Geom::Vec2 bottomLeft, const Geom::Vec2 topRight){
		float xMove = 0;
		float yMove = 0;

		if(align & Align::Layout::top){
			yMove = -topRight.y;
		} else if(align & Align::Layout::bottom){
			yMove = bottomLeft.y;
		}

		if(align & Align::Layout::right){
			xMove = -topRight.x;
		} else if(align & Align::Layout::left){
			xMove = bottomLeft.x;
		}

		return {xMove, yMove};
	}

	constexpr Geom::Vec2 getOffsetOf(const Layout align, const Spacing margin){
		float xMove = 0;
		float yMove = 0;

		if(align & Align::Layout::top){
			yMove = -margin.top;
		} else if(align & Align::Layout::bottom){
			yMove = margin.bottom;
		}

		if(align & Align::Layout::right){
			xMove = -margin.right;
		} else if(align & Align::Layout::left){
			xMove = margin.left;
		}

		return {xMove, yMove};
	}


	/**
	 * @brief
	 * @tparam T arithmetic type, does not accept unsigned type
	 * @return
	 */
	template <Concepts::Signed T>
	constexpr Geom::Vector2D<T> getOffsetOf(const Layout align, const Geom::Rect_Orthogonal<T>& bound){
		Geom::Vector2D<T> offset{};

		if(align & Align::Layout::top){
			offset.y = -bound.getHeight();
		} else if(align & Align::Layout::center_y){
			offset.y = -bound.getHeight() / static_cast<T>(2);
		}

		if(align & Align::Layout::right){
			offset.x = -bound.getWidth();
		} else if(align & Align::Layout::center_x){
			offset.x = -bound.getWidth() / static_cast<T>(2);
		}

		return offset;
	}

	/**
	 * @brief
	 * @tparam T arithmetic type, does not accept unsigned type
	 * @return
	 */
	template <Concepts::Signed T>
	constexpr Geom::Vector2D<T> getVert(const Layout align, const Geom::Rect_Orthogonal<T>& bound){
		Geom::Vector2D<T> offset{bound.getSrc()};


		if(align & Align::Layout::top){
			offset.y = bound.getEndY();
		} else if(align & Align::Layout::center_y){
			offset.y += bound.getHeight() / static_cast<T>(2);
		}

		if(align & Align::Layout::right){
			offset.x = bound.getEndX();
		} else if(align & Align::Layout::center_x){
			offset.x += bound.getWidth() / static_cast<T>(2);
		}

		return offset;
	}

	/**
	 * @brief
	 * @tparam T arithmetic type, does not accept unsigned type
	 * @return
	 */
	template <Concepts::Signed T>
	[[nodiscard]] constexpr Geom::Vector2D<T> getOffsetOf(const Layout align,
	                                                      const Geom::Vector2D<T>& internal_toAlignSize,
	                                                      const Geom::Rect_Orthogonal<T>& external){
		Geom::Vector2D<T> offset{};

		if(align & Align::Layout::top){
			offset.y = external.getEndY() - internal_toAlignSize.y;
		} else if(align & Align::Layout::bottom){
			offset.y = external.getSrcY();
		} else{
			//centerY
			offset.y = external.getSrcY() + (external.getHeight() - internal_toAlignSize.y) / static_cast<T>(2);
		}

		if(align & Align::Layout::right){
			offset.x = external.getEndX() - internal_toAlignSize.x;
		} else if(align & Align::Layout::left){
			offset.x = external.getSrcX();
		} else{
			//centerX
			offset.x = external.getSrcX() + (external.getWidth() - internal_toAlignSize.x) / static_cast<T>(2);
		}

		return offset;
	}

	/**
	 * @brief
	 * @tparam T arithmetic type, does not accept unsigned type
	 * @return
	 */
	template <Concepts::Signed T>
	constexpr Geom::Vector2D<T> getOffsetOf(const Layout align, const Geom::Rect_Orthogonal<T>& internal_toAlign,
	                                        const Geom::Rect_Orthogonal<T>& external){
		return ::Align::getOffsetOf(align, internal_toAlign.getSize(), external);
	}
}
