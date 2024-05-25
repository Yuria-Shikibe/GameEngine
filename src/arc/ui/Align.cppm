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

		[[nodiscard]] Geom::Vec2 bot_lft() const noexcept{
			return {left, bottom};
		}

		[[nodiscard]] Geom::Vec2 top_rit() const noexcept{
			return {right, top};
		}

		friend bool operator==(const Spacing& lhs, const Spacing& rhs) noexcept{
			return lhs.left == rhs.left
				&& lhs.right == rhs.right
				&& lhs.top == rhs.top
				&& lhs.bottom == rhs.bottom;
		}

		friend bool operator!=(const Spacing& lhs, const Spacing& rhs) noexcept{ return !(lhs == rhs); }

		friend bool operator==(const Spacing& lhs, const float val) noexcept{
			return lhs.left == val
				&& lhs.right == val
				&& lhs.top == val
				&& lhs.bottom == val;
		}

		friend bool operator!=(const Spacing& lhs, const float rhs) noexcept{ return !(lhs == rhs); }

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

		[[nodiscard]] constexpr float getRemainHeight(float total = 1.0f) const noexcept{
			return total - getHeight();
		}

		constexpr void set(const float val) noexcept{
			bottom = top = left = right = val;
		}

		constexpr void set(const float l, const float r, const float b, const float t) noexcept{
			left = l;
			right = r;
			bottom = b;
			top = t;
		}

		constexpr void setZero() noexcept{
			set(0);
		}
	};
	enum class Mode : unsigned char{
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
		float xMove = 0;
		float yMove = 0;

		if(align & Align::Mode::top) {
			yMove = -topRight.y;
		}else if(align & Align::Mode::bottom){
			yMove = bottomLeft.y;
		}

		if(align & Align::Mode::right) {
			xMove = -topRight.x;
		}else if(align & Align::Mode::left){
			xMove = bottomLeft.x;
		}

		return {xMove, yMove};
	}

	constexpr Geom::Vec2 getOffsetOf(const Mode align, const Spacing margin) {
		float xMove = 0;
		float yMove = 0;

		if(align & Align::Mode::top) {
			yMove = -margin.top;
		}else if(align & Align::Mode::bottom){
			yMove = margin.bottom;
		}

		if(align & Align::Mode::right) {
			xMove = -margin.right;
		}else if(align & Align::Mode::left){
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
	constexpr Geom::Vector2D<T> getOffsetOf(const Mode align, const Geom::Rect_Orthogonal<T>& bound) {
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

	/**
	 * @brief
	 * @tparam T arithmetic type, does not accept unsigned type
	 * @return
	 */
	template <Concepts::Signed T>
	constexpr Geom::Vector2D<T> getVert(const Mode align, const Geom::Rect_Orthogonal<T>& bound) {
		Geom::Vector2D<T> offset{bound.getSrc()};


		if(align & Align::Mode::top) {
			offset.y = bound.getEndY();
		}else if(align & Align::Mode::center_y){
			offset.y += bound.getHeight() / static_cast<T>(2);
		}

		if(align & Align::Mode::right) {
			offset.x = bound.getEndX();
		}else if(align & Align::Mode::center_x){
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
	[[nodiscard]] constexpr Geom::Vector2D<T> getOffsetOf(const Mode align, const Geom::Vector2D<T>& internal_toAlignSize, const Geom::Rect_Orthogonal<T>& external) {
		Geom::Vector2D<T> offset{};

		if(align & Align::Mode::top) {
			offset.y = external.getEndY() - internal_toAlignSize.y;
		}else if(align & Align::Mode::bottom){
			offset.y = external.getSrcY();
		}else { //centerY
			offset.y = external.getSrcY() + (external.getHeight() - internal_toAlignSize.y) / static_cast<T>(2);
		}

		if(align & Align::Mode::right) {
			offset.x = external.getEndX() - internal_toAlignSize.x;
		}else if(align & Align::Mode::left){
			offset.x = external.getSrcX();
		}else { //centerX
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
	constexpr Geom::Vector2D<T> getOffsetOf(const Mode align, const Geom::Rect_Orthogonal<T>& internal_toAlign, const Geom::Rect_Orthogonal<T>& external) {
		return ::Align::getOffsetOf(align, internal_toAlign.getSize(), external);
	}

	using enum Mode;
}
