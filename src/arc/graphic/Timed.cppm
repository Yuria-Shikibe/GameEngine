//
// Created by Matrix on 2024/3/8.
//

export module Graphic.Timed;

export import Math;
export import Math.Interpolation;

export namespace Graphic{
	struct Timed {
		/**
		 * @brief Should Between [0, 1]
		 */
		Math::Progress progress{};

		constexpr void update(const float delta){
			progress += delta;
		}

		constexpr float get() const{
			return progress;
		}

		constexpr float getInv() const{
			return 1 - progress;
		}

		constexpr float getSlope() const{
			return Math::slope(progress);
		}
	};
}
