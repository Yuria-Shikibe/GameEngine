export module Core.Unit;

import std;

export namespace Core{
	using TickRatio = std::ratio<1, 60>;

	template <typename T = float, typename Ratio = std::ratio<1>>
	struct DirectAccessTimeUnit : std::chrono::duration<T, Ratio>{
		using std::chrono::duration<T, Ratio>::count;
		using std::chrono::duration<T, Ratio>::rep;
		[[nodiscard]] constexpr DirectAccessTimeUnit() noexcept = default;

		[[nodiscard]] constexpr DirectAccessTimeUnit(const T _Val) noexcept
			: std::chrono::duration<T, Ratio>(_Val) {}

		template <class Rep>
		[[nodiscard]]  constexpr explicit DirectAccessTimeUnit(const Rep& _Val) noexcept(std::is_arithmetic_v<rep> && std::is_arithmetic_v<Rep>) // strengthened
			: std::chrono::duration<T, Ratio>(static_cast<rep>(_Val)) {}

		template <class Rep, class Period>
		[[nodiscard]] explicit constexpr DirectAccessTimeUnit(const std::chrono::duration<Rep, Period>& _Dur) noexcept(
			std::is_arithmetic_v<rep>&& std::is_arithmetic_v<Rep>) // strengthened
			: std::chrono::duration<T, Ratio>(std::chrono::duration_cast<std::chrono::duration<T, Ratio>>(_Dur).count()) {}

		[[nodiscard]] constexpr operator T() const noexcept{
			return this->count();
		}
	};

	using Tick = DirectAccessTimeUnit<float, TickRatio>;
	using Sec = DirectAccessTimeUnit<float>;
}