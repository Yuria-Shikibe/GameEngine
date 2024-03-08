//
// Created by Matrix on 2024/3/7.
//

export module Interval;

import Concepts;
import Math;
import <array>;

export namespace ext{
	/**
	 * @brief Time Unit in Game Tick! [60ticks / 1sec]
	 * @tparam size Interval Chennals
	 */
	template <size_t size = 1>
		requires requires{ size > 0; }
	class Interval {
		std::array<float, size> reloads{};

	public:
		static constexpr size_t timeSize = size;

		constexpr Interval() = default;

		template <float Spacing, size_t Index = 0, bool Strict = false>
			requires requires {Index < size;}
		void run(const float delta, Concepts::Invokable<void()> auto&& func){
			auto& reload = reloads[Index];
			reload += delta;

			if(reload >= delta){
				if constexpr(Strict){
					const int total = Math::floor(reload / Spacing);
					for(int i = 0; i < total; ++i){
						func();
					}
				}else{
					func();
				}
				reload = Math::mod(reload, Spacing);
			}
		}

		template <float Spacing, size_t Index = 0>
			requires requires {Index < size;}
		constexpr bool getValid(const float delta = 0){
			auto& reload = reloads[Index];
			reload += delta;

			if(reload >= delta){
				reload = Math::mod(reload, Spacing);
				return true;
			}

			return false;
		}

		template <float Spacing, size_t Index = 0>
			requires requires {Index < size;}
		constexpr bool isValid() const{
			return reloads[Index] > Spacing;
		}

		constexpr void clear(){
			reloads.fill(0);
		}
	};
}
