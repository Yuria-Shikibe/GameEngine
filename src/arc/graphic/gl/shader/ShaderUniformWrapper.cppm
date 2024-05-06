//
// Created by Matrix on 2024/4/20.
//

export module GL.Shader.UniformWrapper;
import std;
import MetaProgramming;
import Concepts;

export namespace GL{
	template<typename ...Args>
	struct UniformWrapper{
		static constexpr std::size_t size = sizeof...(Args);
		using ArgsType = std::tuple<Args...>;
		ArgsType defData{};
		ArgsType data{};

		constexpr UniformWrapper() noexcept = default;

		constexpr explicit UniformWrapper(ArgsType&& defParams) noexcept : defData{std::move(defParams)}, data{defData}{}

		template <typename ...T>
		constexpr explicit UniformWrapper(T&& ...args) noexcept : defData(std::make_tuple(std::forward<T>(args) ... )), data{defData}{}

		template <typename ...T>
			requires requires {requires ext::isArgsSubOf<true, ArgsType, T...>();}
		constexpr void set(T&&... args) noexcept{
			data = ext::makeTuple_withDef(defData, std::forward<T>(args) ...);
		}

		constexpr void setDef() noexcept{
			data = defData;
		}

		template <std::size_t argIndex, bool asReference = false>
			requires requires{ requires argIndex < size; }
		inline constexpr typename Concepts::ValueConditional<!asReference, std::tuple_element_t<argIndex, ArgsType>>::type
			get() noexcept{
			return std::get<argIndex>(data);
		}

		template <std::size_t argIndex, bool asReference = true>
			requires requires{ requires argIndex < size; }
		inline constexpr typename Concepts::ValueConditional<!asReference, std::tuple_element_t<argIndex, ArgsType>>::type
			get() const noexcept{
			return std::get<argIndex>(data);
		}
	};

	template<typename ...Args>
	UniformWrapper(std::tuple<Args...>) -> UniformWrapper<Args...>;

	template<typename ...Args>
	UniformWrapper(Args...) -> UniformWrapper<Args...>;
}
