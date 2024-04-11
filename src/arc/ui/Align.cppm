module;

export module UI.Align;

import Geom.Vector2D;
import Geom.Rect_Orthogonal;

import std;

export namespace Align {
	struct Spacing{
		/**@brief Left Spacing*/
		float left{};
		/**@brief Right Spacing*/
		float right{};
		/**@brief Bottom Spacing*/
		float bottom{};
		/**@brief Top Spacing*/
		float top{};

		[[nodiscard]] Geom::Vec2 bot_lft() const {
			return {left, bottom};
		}

		[[nodiscard]] Geom::Vec2 top_rit() const {
			return {right, top};
		}

		friend bool operator==(const Spacing& lhs, const Spacing& rhs){
			return lhs.left == rhs.left
				&& lhs.right == rhs.right
				&& lhs.top == rhs.top
				&& lhs.bottom == rhs.bottom;
		}

		friend bool operator!=(const Spacing& lhs, const Spacing& rhs){ return !(lhs == rhs); }

		friend bool operator==(const Spacing& lhs, const float val){
			return lhs.left == val
				&& lhs.right == val
				&& lhs.top == val
				&& lhs.bottom == val;
		}

		friend bool operator!=(const Spacing& lhs, const float rhs){ return !(lhs == rhs); }

		[[nodiscard]] constexpr float getWidth() const{
			return left + right;
		}

		[[nodiscard]] constexpr float getHeight() const{
			return bottom + top;
		}

		[[nodiscard]] constexpr float getRemainWidth(const float total = 1.0f) const{
			return total - getWidth();
		}

		[[nodiscard]] constexpr float getRemainHeight(float total = 1.0f) const{
			return total - getHeight();
		}

		constexpr void set(const float val){
			bottom = top = left = right = val;
		}

		constexpr void set(const float l, const float r, const float b, const float t){
			left = l;
			right = r;
			bottom = b;
			top = t;
		}

		constexpr void setZero(){
			set(0);
		}
	};
	enum class Mode : char{
		left = 0b0000'0001,
		right = 0b0000'0010,
		center_x = 0b0000'0100,

		top = 0b0000'1000,
		bottom = 0b0001'0000,
		center_y = 0b0010'0000,

		top_left      = top | left,
		top_center    = top | center_x,
		top_right     = top | right,

		center_left   = center_y | left,
		center        = center_y | center_x,
		center_right  = center_y | right,

		bottom_left   = bottom | left,
		bottom_center = bottom | center_x,
		bottom_right  = bottom | right,
	};

	constexpr char codeOf(Mode align) {
		return static_cast<char>(align);
	}

	constexpr bool operator &(const Mode l, const Mode r) {
		return codeOf(l) & codeOf(r);
	}

	constexpr Geom::Vec2 getOffsetOf(const Mode align, const Geom::Vec2 bottomLeft, const Geom::Vec2 topRight) {
		float xSign = 0;
		float ySign = 0;

		if(align & Align::Mode::top) {
			ySign = -1;
		}else if(align & Align::Mode::bottom){
			ySign = 1;
		}

		if(align & Align::Mode::right) {
			xSign = -1;
		}else if(align & Align::Mode::left){
			xSign = 1;
		}

		const float xMove = xSign * (xSign == 1 ? bottomLeft.x : topRight.x);
		const float yMove = ySign * (ySign == 1 ? (bottomLeft.y) : topRight.y);

		return {xMove, yMove};
	}

	constexpr Geom::Vec2 getOffsetOf(const Mode align, const Spacing margin) {
		float xSign = 0;
		float ySign = 0;

		if(align & Align::Mode::top) {
			ySign = -1;
		}else if(align & Align::Mode::bottom){
			ySign = 1;
		}

		if(align & Align::Mode::right) {
			xSign = -1;
		}else if(align & Align::Mode::left){
			xSign = 1;
		}

		const float xMove = xSign * (xSign == 1 ? margin.left : margin.right);
		const float yMove = ySign * (ySign == 1 ? margin.bottom : margin.top);

		return {xMove, yMove};
	}


	/**
	 * @brief
	 * @tparam T arithmetic type, does not accept unsigned type
	 * @return
	 */
	template <Concepts::Signed T>
	constexpr Geom::Vector2D<T> getOffsetOf(const Mode align, const Geom::Rect_Orthogonal<T> bound) {
		Geom::Vector2D<T> offset{};

		if(align & Align::Mode::top) {
			offset.y = -bound.getHeight();
		}else if(align & Align::Mode::center_y){
			offset.y = -bound.getHeight() / static_cast<T>(2);
		}

		if(align & Align::Mode::right) {
			offset.x = -bound.getWidth();
		}else if(align & Align::Mode::center_x){
			offset.x = -bound.getWidth() / static_cast<T>(2);
		}

		return offset;
	}

	using enum Mode;
}
