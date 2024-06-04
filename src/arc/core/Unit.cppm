export module Core.Unit;

import std;

export namespace Core{
	using TickRatio = std::ratio<1, 60>;

	template <typename T = float, typename Ratio = std::ratio<1>>
	struct DirectAccessTimeUnit : std::chrono::duration<T, Ratio>{
		using std::chrono::duration<T, Ratio>::count;
		using std::chrono::duration<T, Ratio>::rep;
		using std::chrono::duration<T, Ratio>::duration;
		[[nodiscard]] constexpr DirectAccessTimeUnit() noexcept = default;

		[[nodiscard]] constexpr DirectAccessTimeUnit(const T _Val) noexcept
			: std::chrono::duration<T, Ratio>(_Val) {}

		[[nodiscard]] constexpr operator T() const noexcept{
			return this->count();
		}

		using std::chrono::duration<T, Ratio>::operator++;
		using std::chrono::duration<T, Ratio>::operator--;

		using std::chrono::duration<T, Ratio>::operator%=;

		using std::chrono::duration<T, Ratio>::operator+=;
		using std::chrono::duration<T, Ratio>::operator-=;
		using std::chrono::duration<T, Ratio>::operator*=;
		using std::chrono::duration<T, Ratio>::operator/=;

		using std::chrono::duration<T, Ratio>::operator+;
		using std::chrono::duration<T, Ratio>::operator-;

		constexpr DirectAccessTimeUnit& operator++() noexcept(std::is_arithmetic_v<rep>) /* strengthened */ {
			this->std::chrono::duration<T, Ratio>::operator++();
			return *this;
		}

		constexpr DirectAccessTimeUnit operator++(int) noexcept(std::is_arithmetic_v<rep>) /* strengthened */ {
			auto t = *this;
			this->operator++();
			return t;
		}

		constexpr DirectAccessTimeUnit& operator--() noexcept(std::is_arithmetic_v<rep>) /* strengthened */ {
			this->std::chrono::duration<T, Ratio>::operator--();
			return *this;
		}

		constexpr DirectAccessTimeUnit operator--(int) noexcept(std::is_arithmetic_v<rep>) /* strengthened */ {
			auto t = *this;
			this->operator--();
			return t;
		}
	};

	using Tick = DirectAccessTimeUnit<float, TickRatio>;
	using Sec = DirectAccessTimeUnit<float>;
}