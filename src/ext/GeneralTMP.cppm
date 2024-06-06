export module ext.MetaProgramming;

import std;
// import ext.RuntimeException;

namespace ext{
	template <std::size_t I, std::size_t size, typename ArgTuple, typename DefTuple>
	constexpr decltype(auto) getWithDef(ArgTuple&& argTuple, DefTuple&& defTuple){
		if constexpr(I < size){
			return std::get<I>(std::forward<ArgTuple>(argTuple));
		} else{
			return std::get<I>(std::forward<DefTuple>(defTuple));
		}
	}

	template <typename TargetTuple, typename... Args, std::size_t... I>
	constexpr decltype(auto) makeTuple_withDef_impl(std::tuple<Args...>&& args, TargetTuple&& defaults,
	                                                std::index_sequence<I...>){
		return std::make_tuple(std::tuple_element_t<I, std::decay_t<TargetTuple>>{
				ext::getWithDef<I, sizeof...(Args)>(std::move(args), std::forward<TargetTuple>(defaults))
			}...);
	}

	template<typename T, T a, T b>
	constexpr T max_const = a > b ? a : b;

	template<typename T, T a, T b>
	constexpr T min_const = a < b ? a : b;

	template <typename TargetTuple, typename FromTuple, std::size_t... I>
	constexpr bool tupleConvertableTo(std::index_sequence<I...>){
		return (std::convertible_to<std::tuple_element_t<I, FromTuple>, std::tuple_element_t<I, TargetTuple>> && ...);
	}

	template <typename TargetTuple, typename FromTuple, std::size_t... I>
	constexpr bool tupleSameAs(std::index_sequence<I...>){
		return (std::same_as<std::tuple_element_t<I, FromTuple>, std::tuple_element_t<I, TargetTuple>> && ...);
	}


	template <typename TargetTuple, typename T, std::size_t... I>
	constexpr bool tupleContains(std::index_sequence<I...>){
		return (std::same_as<T, std::tuple_element_t<I, TargetTuple>> || ...);
	}

	template <typename T, typename... Ts>
	struct UniqueTypeIndex;

	template <typename T, typename... Ts>
	struct UniqueTypeIndex<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

	template <typename T, typename U, typename... Ts>
	struct UniqueTypeIndex<T, U, Ts...> : std::integral_constant<std::size_t, 1 + UniqueTypeIndex<T, Ts...>::value> {};

	export
	template <typename T, typename... Ts>
	constexpr std::size_t uniqueTypeIndex_v = UniqueTypeIndex<T, Ts...>::value;


	template <bool Test, auto val1, decltype(val1) val2>
	struct conditional_constexpr_val{
		static constexpr auto val = val1;
	};

	template <auto val1, decltype(val1) val2>
	struct conditional_constexpr_val<false, val1, val2> {
		static constexpr auto val = val1;
	};

	export
	template <bool Test, auto val1, decltype(val1) val2>
	constexpr auto conditionalVal = conditional_constexpr_val<Test, val1, val2>::val;

	template <typename T, typename = void>
	struct is_complete_type : std::false_type {};

	template <typename T>
	struct is_complete_type<T, decltype(void(sizeof(T)))> : std::true_type{};

	export
	template <typename T>
	constexpr bool isTypeComplteted = is_complete_type<T>::value;
}


export namespace ext{
	template <typename SuperTuple, typename... Args>
	constexpr decltype(auto) makeTuple_withDef(SuperTuple&& defaultArgs, Args&&... args){
		return ext::makeTuple_withDef_impl(std::make_tuple(std::forward<Args>(args)...),
		                                   std::forward<SuperTuple>(defaultArgs),
		                                   std::make_index_sequence<std::tuple_size_v<std::decay_t<SuperTuple>>>());
	}

	template <typename SuperTuple, typename... Args>
	constexpr decltype(auto) makeTuple_withDef(Args&&... args){
		return ext::makeTuple_withDef(SuperTuple{}, std::forward<Args>(args)...);
	}

	/**
	 * @brief
	 * @tparam strict Using std::same_as or std::convertable_to
	 * @tparam SuperTuple Super sequence of the params
	 * @tparam Args given params
	 * @return Whether given param types are the subseq of the SuperTuple
	 */
	template <bool strict, typename SuperTuple, typename...Args>
	constexpr bool isArgsSubOf(){
		constexpr std::size_t fromSize = sizeof...(Args);
		constexpr std::size_t toSize = std::tuple_size_v<SuperTuple>;
		if constexpr(std::tuple_size_v<SuperTuple> < fromSize)return false;

		if constexpr(strict){
			return ext::tupleSameAs<SuperTuple, std::tuple<std::decay_t<Args>...>>(std::make_index_sequence<ext::min_const<std::size_t, toSize, fromSize>>());
		}else{
			return ext::tupleConvertableTo<SuperTuple, std::tuple<std::decay_t<Args>...>>(std::make_index_sequence<ext::min_const<std::size_t, toSize, fromSize>>());
		}
	}

	template <typename T, typename ArgsTuple>
	constexpr bool containedIn(){
		constexpr std::size_t toSize = std::tuple_size_v<ArgsTuple>;

		return ext::tupleContains<ArgsTuple, T>(std::make_index_sequence<toSize>());
	}

	template <typename T, typename...Args>
	constexpr bool containedWith(){
		constexpr std::size_t toSize = std::tuple_size_v<std::tuple<Args...>>;

		return ext::tupleContains<std::tuple<Args...>, T>(std::make_index_sequence<toSize>());
	}

	template <typename SuperTuple, typename FromTuple>
	constexpr bool isTupleSubOf(){
		constexpr std::size_t fromSize = std::tuple_size_v<FromTuple>;
		constexpr std::size_t toSize = std::tuple_size_v<SuperTuple>;
		if constexpr(toSize < fromSize)return false;

		return ext::tupleConvertableTo<SuperTuple, FromTuple>(std::make_index_sequence<ext::min_const<std::size_t, toSize, fromSize>>());
	}

	template <typename MemberPtr>
	struct GetMemberPtrInfo;

	template <typename C, typename T>
	struct GetMemberPtrInfo<T C::*>{
		using ClassType = C;
		using ValueType = T;
	};

	template <typename C, typename T>
	struct GetMemberPtrInfo<T C::* const> : GetMemberPtrInfo<T C::*>{};

	template <typename C, typename T>
	struct GetMemberPtrInfo<T C::* &> : GetMemberPtrInfo<T C::*>{};

	template <typename C, typename T>
	struct GetMemberPtrInfo<T C::* &&> : GetMemberPtrInfo<T C::*>{};

	template <typename C, typename T>
	struct GetMemberPtrInfo<T C::* const&> : GetMemberPtrInfo<T C::*>{};


	// template<auto mPtr>
	// constexpr std::size_t offset_of = std::bit_cast<std::size_t>(&(static_cast<typename GetMemberPtrClass<decltype(mPtr)>::type*>(nullptr)->*mPtr));
	//
	// constexpr auto off = offset_of<&std::pair<int, int>::first>;

	template <typename T>
	concept HasDefHasher = requires(const T& t){
		requires std::is_default_constructible_v<std::hash<T>>;
		{ std::hash<T>{}.operator()(t) } noexcept -> std::same_as<std::size_t>;
	};


	template <bool Test, class T>
	struct ConstConditional{
		using type = T;
	};


	template <class T>
	struct ConstConditional<true, T>{
		using type = std::add_const_t<T>;
	};


	template <class T>
	struct ConstConditional<true, T&>{
		using type = std::add_lvalue_reference_t<std::add_const_t<T>>;
	};


	template <typename T>
	constexpr bool isConstRef = std::is_const_v<std::remove_reference_t<T>>;

	//
	// template <auto... ptr>
	// struct SeqMemberPtrAccessor;
	//
	//
	// template <auto headPtr, auto... ptr>
	// struct SeqMemberPtrAccessor<headPtr, ptr...>{
	// 	static constexpr auto size = sizeof...(ptr) + 1;
	// 	static_assert(size > 0);
	// 	using TupleType = std::tuple<decltype(headPtr), decltype(ptr)...>;
	// 	static constexpr std::tuple ptrs = std::make_tuple(headPtr, ptr...);
	//
	// 	using RetType = typename GetMemberPtrType<std::tuple_element_t<size - 1, TupleType>>::type;
	// 	using InType = typename GetMemberPtrInfo<std::tuple_element_t<0, TupleType>>::type;
	// 	using NextInType = typename GetMemberPtrType<std::tuple_element_t<0, TupleType>>::type;
	//
	// 	static constexpr bool isPtr = std::is_pointer_v<InType>;
	// 	using NextGroupType = SeqMemberPtrAccessor<ptr...>;
	//
	// 	template <typename T>
	// 		requires std::same_as<std::remove_cvref_t<T>, InType>
	// 	static auto getNext(T& in) -> typename ConstConditional<isConstRef<T>, NextInType&>::type{
	// 		if constexpr(isPtr){
	// 			if(in != nullptr){
	// 				return in->*headPtr;
	// 			}
	// 			throw NullPointerException{};
	// 		}
	// 		return in.*headPtr;
	// 	}
	//
	// 	template <typename T>
	// 		requires std::same_as<std::remove_cvref_t<T>, InType>
	// 	static decltype(auto) get(T& in){
	// 		return NextGroupType::get(SeqMemberPtrAccessor::getNext(in));
	// 	}
	// };
	//
	//
	// template <auto headPtr>
	// struct SeqMemberPtrAccessor<headPtr>{
	// 	using RetType = typename GetMemberPtrType<decltype(headPtr)>::type;
	// 	using InType = typename GetMemberPtrInfo<decltype(headPtr)>::type;
	// 	static constexpr bool isPtr = std::is_pointer_v<InType>;
	//
	// 	template <typename T>
	// 		requires std::same_as<std::remove_cvref_t<T>, InType>
	// 	static auto getNext(T& in) -> typename ConstConditional<isConstRef<T>, RetType&>::type{
	// 		if constexpr(isPtr){
	// 			if(in != nullptr){
	// 				return in->*headPtr;
	// 			}
	// 			throw NullPointerException{};
	// 		}
	// 		return in.*headPtr;
	// 	}
	//
	// 	template <typename T>
	// 		requires std::same_as<std::remove_cvref_t<T>, InType>
	// 	static decltype(auto) get(T& in){
	// 		return SeqMemberPtrAccessor::getNext(in);
	// 	}
	// };
}
