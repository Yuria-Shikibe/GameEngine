//
// Created by Matrix on 2024/3/8.
//

export module Graphic.Timed;

export import Math;
export import Math.Interpolation;
import Concepts;

export namespace Graphic{
	struct Timed {
		/**
		 * @brief Should Between [0, 1]
		 */
		float lifetime{};
		float time{};

		constexpr void set(const float time, const float lifetime){
			this->time = time;
			this->lifetime = lifetime;
		}

		template <bool clamp = false>
		[[nodiscard]] constexpr float get() const{
			if constexpr (clamp){
				return Math::clamp(time / lifetime);
			}

			return time / lifetime;
		}

		[[nodiscard]] float get(Concepts::Invokable<float(float)> auto&& interp) const{
			return interp(get());
		}

		/**
		 * @brief [margin, 1]
		 */
		[[nodiscard]] float getMargin(const float margin) const{
			return margin + get() * (1 - margin);
		}

		[[nodiscard]] constexpr float getInv() const{
			return 1 - get();
		}

		[[nodiscard]] constexpr float getSlope() const{
			return Math::slope(get());
		}
	};
}
