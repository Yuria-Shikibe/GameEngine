//
// Created by Matrix on 2024/3/7.
//

export module ext.Timer;

import Concepts;
import Math;
import std;

export namespace ext{
	/**
	 * @brief Time Unit in Game Tick! [60ticks / 1sec]
	 * @tparam size Interval Chennals
	 */
	template <size_t size = 1>
		requires requires{ size > 0; }
	class Timer {
		std::array<float, size> reloads{};

	public:
		static constexpr size_t timeSize = size;

		constexpr Timer() = default;

		template <size_t Index = 0, bool Strict = false>
			requires requires {Index < size;}
		void run(const float spacing, const float delta, Concepts::Invokable<void()> auto&& func){
			auto& reload = reloads[Index];
			reload += delta;

			if(reload >= spacing){
				if constexpr(Strict){
					const int total = Math::floorPositive(reload / spacing);
					for(int i = 0; i < total; ++i){
						func();
					}
				}else{
					func();
				}
				reload = Math::mod(reload, spacing);
			}
		}

		template <size_t Index = 0>
			requires requires {Index < size;}
		constexpr bool getValid(const float spacing, const float delta = 0){
			auto& reload = reloads[Index];
			reload += delta;

			if(reload >= delta){
				reload = Math::mod(reload, spacing);
				return true;
			}

			return false;
		}

		template <size_t Index = 0>
			requires requires {Index < size;}
		[[nodiscard]] constexpr bool isValid(const float spacing) const{
			return reloads[Index] > spacing;
		}

		constexpr void clear(){
			reloads.fill(0);
		}
	};
}
