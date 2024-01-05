module ;

export module Concepts;

import <functional>;
import <concepts>;
import <type_traits>;

export namespace Concepts {
	template<typename T>
	struct FunctionTraits;

	template<typename Ret, typename... Args>
	struct FunctionTraits<Ret(Args...)>
	{
		using ReturnType = Ret;
		using ArgsTuple = std::tuple<Args...>;
		static constexpr std::size_t argsArity = sizeof...(Args);

		template <std::size_t N>
		struct argAt
		{
			static_assert(N < argsArity, "error: invalid parameter index.");
			using type = std::tuple_element_t<N, ArgsTuple>;
		};

		template <typename Func>
		using invocableAs = std::is_invocable_r<Ret, Func, Args...>;

		template <typename Func>
		static constexpr bool invocableAs_v() {
			return std::is_invocable_r_v<Ret, Func, Args...>;
		}
	};

    template <typename T>
    concept Number = std::is_arithmetic_v<T>;

    template <typename T>
    concept NumberSingle = std::is_arithmetic_v<T> && sizeof(T) <= 4;

    template <class T, class DerivedT>
    concept Derived = std::is_base_of_v<DerivedT, T>;

    template <class DerivedT, class... Bases>
    concept DerivedMulti = requires{
	    (std::is_base_of_v<Bases, DerivedT> && ...);
    };

    template <class T>
    concept DefConstructable = std::is_default_constructible_v<T>;

	template <typename T, typename functype>
	concept Invokable = FunctionTraits<functype>::template invocableAs_v<T>();

	template <typename T, typename functype>
	concept InvokableFunc = std::is_convertible_v<T, functype>;

	template <typename T>
	concept Enum = std::is_enum_v<T>;

	template <typename T>
	concept NonNegative = std::is_unsigned_v<T>;

	template <typename T>
	concept Signed = !std::is_unsigned_v<T> && Number<T>;

	template <typename T, typename Item>
	concept Iterable = requires(T t){
		std::is_same_v<decltype(std::begin(std::declval<T&>())), Item>;
		std::is_same_v<decltype(std::end(std::declval<T&>())), Item>;
	};

	template <typename T, typename Item>
	concept IterableDerived = requires(T t){
		std::is_base_of_v<decltype(std::begin(std::declval<T&>())), Item>;
		std::is_base_of_v<decltype(std::end(std::declval<T&>())), Item>;
	};

	template <typename T, typename NumberType = float>
	concept Pos = requires(T t){
		std::is_base_of_v<decltype(t.getX()), NumberType>;
		std::is_base_of_v<decltype(t.getY()), NumberType>;
	};
}