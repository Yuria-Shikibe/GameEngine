export module ext.Concepts;

import std;

namespace Concepts {

	/**
	 * \brief coonditional variant but friendly to IDEs
	 */
	export
	template <bool Test, class T>
	struct RefConditional {
		using type = std::add_lvalue_reference_t<T>;
	};

	template <class T>
	struct RefConditional<false, T> {
		using type = T;
	};

	export
	template<typename T>
	struct FunctionTraits;

	export
	template<typename Ret, typename... Args>
	struct FunctionTraits<Ret(Args...)>{
		using ReturnType = Ret;
		using ArgsTuple = std::tuple<Args...>;
		static constexpr std::size_t argsArity = sizeof...(Args);

		template <typename Func>
		static constexpr bool isInvocable = std::is_invocable_r_v<Ret, Func, Args...>;

		template <typename Func>
		static constexpr bool invocableAs_v() {
			return std::is_invocable_r_v<Ret, Func, Args...>;
		}

		template <typename Func>
		static constexpr bool invocableVoidAs_v() {
			return std::is_invocable_v<Func, Args...>;
		}

		template <typename Func>
		static constexpr bool isReuglarOf = std::regular_invocable<Func, Args...>;
	};

#define Variant(ext) export template<typename Ret, typename... Args> struct FunctionTraits<Ret(Args...) ext> : FunctionTraits<Ret(Args...)>{};
	Variant(&);
	Variant(&&);
	Variant(const);
	Variant(const &);
	Variant(noexcept);
	Variant(& noexcept);
	Variant(&& noexcept);
	Variant(const noexcept);
	Variant(const& noexcept);

	export
	template <typename FuncType, std::size_t N>
	struct argAt{
		using Trait = FunctionTraits<FuncType>;
		static_assert(N < Trait::argsArity, "error: invalid parameter index.");
		using type = std::tuple_element_t<N, typename Trait::ArgsTuple>;
	};

	/**
	 * \brief Decide whether to use reference or value version of the given of the given template classes
	 * \tparam T Value Type
	 */
	export template <typename T, size_t size>
	using ParamPassType = typename RefConditional<
		(size > sizeof(void*) * 2),
		T
	>::type;

    export template <typename T>
    concept Number = std::is_arithmetic_v<T>;

    export template <typename T>
    concept NumberSingle = std::is_arithmetic_v<T> && sizeof(T) <= 4;

    export template <class DerivedT, class Base>
    concept Derived = std::derived_from<DerivedT, Base>;

    export template <class DerivedT, class... Bases>
    concept DerivedMulti = (std::derived_from<Bases, DerivedT> && ...);

	export template <class T>
	concept DefConstructable = std::is_default_constructible_v<T>;

	export template <typename T, typename functype>
	concept Invokable = FunctionTraits<functype>::template isInvocable<T>;

	export template <typename T, typename functype>
	concept InvokableRegular = FunctionTraits<functype>::template isInvocable<T> && FunctionTraits<functype>::template isReuglarOf<T>;

	export template <typename T, typename functype>
	concept InvokableVoid = FunctionTraits<functype>::template invocableVoidAs_v<T>();

	export template <typename T, typename functype>
	concept InvokeNullable = std::same_as<std::nullptr_t, T> || Invokable<T, functype>;

	export template <typename T, typename functype>
	concept InvokableFunc = std::is_convertible_v<T, std::function<functype>>;

	export template <typename T>
	concept Enum = std::is_enum_v<T>;

	export template <typename T>
	concept NonNegative = std::is_unsigned_v<T>;

	export template <typename T>
	concept Signed = !std::is_unsigned_v<T> && Number<T>;

	export template <typename T, typename ItemType = std::nullptr_t>
	concept Iterable = requires{
		requires std::ranges::range<T>;
		requires std::same_as<ItemType, std::nullptr_t> || std::same_as<std::ranges::range_value_t<T>, ItemType>;
	};

	export template <typename Callable>
	concept InvokeNoexcept = noexcept(Callable()) || noexcept(std::declval<Callable>()());

	export template <typename T, typename NumberType = float>
	concept Pos = requires(T t){
		std::is_base_of_v<decltype(t.getX()), NumberType>;
		std::is_base_of_v<decltype(t.getY()), NumberType>;
	};


	template <template <class...> class Template, class... Args>
	// ReSharper disable once CppFunctionIsNotImplemented
	void derived_from_specialization_impl(const Template<Args...>&);

	export
	template <class T, template <class...> class Template>
	concept SpecDeriveOf = requires(const T& obj) {
		Concepts::derived_from_specialization_impl<Template>(obj);
	};


	export template <typename T, typename Type>
	concept Iterator = requires(T t){
		{ t.operator++() } -> std::convertible_to<T>;
		{ t.operator*() } -> std::convertible_to<Type>;
	};
}