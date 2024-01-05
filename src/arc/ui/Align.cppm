module;

export module Align;

import Geom.Vector2D;

export namespace Align {
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

	inline char codeOf(Mode align) {
		return static_cast<char>(align);
	}

	bool operator &(const Mode l, const Mode r) {
		return codeOf(l) & codeOf(r);
	}

	Geom::Vec2 motionOf(Mode align, const Geom::Vec2& bottomLeft, const Geom::Vec2& topRight) {
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



	using enum Mode;
}
