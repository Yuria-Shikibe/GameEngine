//
// Created by Matrix on 2024/3/8.
//

export module Math.Timed;

export import Math;
export import Math.Interpolation;
import ext.Concepts;

//TODO is this namespace appropriate?
export namespace Math{
	struct Timed {
		float time{};
		float lifetime{};

		template <bool autoClamp = false>
		constexpr void set(const float time, const float lifetime){
			if constexpr (autoClamp){
				this->lifetime = max(lifetime, 0.0f);
				this->time = clamp(time, 0.0f, lifetime);
			}else{
				this->lifetime = lifetime;
				this->time = time;
			}

		}

		template <bool autoClamp = false>
		void update(const float delta){
			time += delta;

			if constexpr (autoClamp){
				time = clamp(time, 0.0f, lifetime);
			}
		}

		template <bool autoClamp = false>
		[[nodiscard]] constexpr float get() const{
			if constexpr (autoClamp){
				return clamp(time / lifetime);
			}

			return time / lifetime;
		}

		template <bool autoClamp = true>
		[[nodiscard]] constexpr float getWith(const float otherLifetime) const{
			if constexpr (autoClamp){
				return clamp(time / otherLifetime);
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

		[[nodiscard]] float getSlope() const{
			return slope(get());
		}
	};
}
