export module Concepts;

import std;

namespace Concepts {
	/**
	 * \brief coonditional variant but friendly to IDEs
	 */
	template <bool Test, class T>
	struct RefConditional {
		using type = T;
	};

	template <class T>
	struct RefConditional<false, T> {
		using type = std::add_lvalue_reference_t<T>;
	};

	template<typename T>
	struct FunctionTraits;
}

export namespace Concepts {
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

	/**
	 * \brief Decide whether to use reference or value version of the given of the given template classes
	 * \tparam T Value Type
	 */
	template <typename T, size_t size>
	using ParamPassType = typename RefConditional<
		size <= sizeof(void*) * 2,
		T
	>::type;

    template <typename T>
    concept Number = std::is_arithmetic_v<T>;

	template <typename T, typename R>
	concept Pass = std::same_as<std::remove_cvref_t<T>, R>;

    template <typename T>
    concept NumberSingle = std::is_arithmetic_v<T> && sizeof(T) <= 4;

    template <class DerivedT, class Base>
    concept Derived = std::derived_from<DerivedT, Base>;

    template <class DerivedT, class... Bases>
    concept DerivedMulti = (std::derived_from<Bases, DerivedT> && ...);;

	template <class T>
	concept DefConstructable = std::is_default_constructible_v<T>;

	template <typename T, typename functype>
	concept Invokable = FunctionTraits<functype>::template invocableAs_v<T>();

	template <typename T, typename functype>
	concept InvokeNullable = std::same_as<std::nullptr_t, T> || FunctionTraits<functype>::template invocableAs_v<T>();

	template <typename T, typename functype>
	concept InvokableFunc = std::is_convertible_v<T, std::function<functype>>;

	template <typename T>
	concept Enum = std::is_enum_v<T>;

	template <typename T>
	concept NonNegative = std::is_unsigned_v<T>;

	template <typename T>
	concept Signed = !std::is_unsigned_v<T> && Number<T>;

	template <typename T>
	concept Iterable = std::ranges::range<T>;

	template <typename T, typename Item>
	concept IterableDerived = std::ranges::range<T> && requires{
		requires std::is_base_of_v<decltype(*std::begin(std::declval<T&>())), Item>;
		requires std::is_base_of_v<decltype(*std::end(std::declval<T&>())), Item>;
	};

	template <typename T, typename NumberType = float>
	concept Pos = requires(T t){
		std::is_base_of_v<decltype(t.getX()), NumberType>;
		std::is_base_of_v<decltype(t.getY()), NumberType>;
	};
}