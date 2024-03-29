//
// Created by Matrix on 2024/3/8.
//

export module Graphic.Timed;

export import Math;
export import Math.Interpolation;
import Concepts;

//TODO is this namespace appropriate
export namespace Graphic{
	struct Timed {
		float lifetime{};
		float time{};

		template <bool autoClamp = false>
		constexpr void set(const float time, const float lifetime){
			if constexpr (autoClamp){
				this->lifetime = Math::max(lifetime, 0.0f);
				this->time = Math::clamp(time, 0.0f, lifetime);
			}else{
				this->lifetime = lifetime;
				this->time = time;
			}

		}

		template <bool autoClamp = false>
		[[nodiscard]] constexpr float get() const{
			if constexpr (autoClamp){
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
