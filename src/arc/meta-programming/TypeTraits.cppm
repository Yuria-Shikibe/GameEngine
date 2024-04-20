export module MetaProgramming;

import std;
import ext.RuntimeException;

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
}


export namespace ext{
	template <typename TargetTuple, typename... Args>
	constexpr decltype(auto) makeTuple_withDef(TargetTuple&& defaultArgs, Args&&... args){
		return ext::makeTuple_withDef_impl(std::make_tuple(std::forward<Args>(args)...),
		                                   std::forward<TargetTuple>(defaultArgs),
		                                   std::make_index_sequence<std::tuple_size_v<std::decay_t<TargetTuple>>>());
	}

	template <typename TargetTuple, typename... Args>
	constexpr decltype(auto) makeTuple_withDef(Args&&... args){
		return ext::makeTuple_withDef(TargetTuple{}, std::forward<Args>(args)...);
	}


	template <typename MemberPtr>
	struct GetMemberPtrClass;


	template <typename C, typename T>
	struct GetMemberPtrClass<T C::*>{
		using type = C;
	};


	template <typename MemberPtr>
	struct GetMemberPtrType;


	template <typename C, typename T>
	struct GetMemberPtrType<T C::*>{
		using type = std::decay_t<T>;
	};


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

	template <auto... ptr>
	struct SeqMemberPtrAccessor;


	template <auto headPtr, auto... ptr>
	struct SeqMemberPtrAccessor<headPtr, ptr...>{
		static constexpr auto size = sizeof...(ptr) + 1;
		static_assert(size > 0);
		using TupleType = std::tuple<decltype(headPtr), decltype(ptr)...>;
		static constexpr std::tuple ptrs = std::make_tuple(headPtr, ptr...);

		using RetType = typename GetMemberPtrType<std::tuple_element_t<size - 1, TupleType>>::type;
		using InType = typename GetMemberPtrClass<std::tuple_element_t<0, TupleType>>::type;
		using NextInType = typename GetMemberPtrType<std::tuple_element_t<0, TupleType>>::type;

		static constexpr bool isPtr = std::is_pointer_v<InType>;
		using NextGroupType = SeqMemberPtrAccessor<ptr...>;

		template <typename T>
			requires std::same_as<std::remove_cvref_t<T>, InType>
		static auto getNext(T& in) -> typename ConstConditional<isConstRef<T>, NextInType&>::type{
			if constexpr(isPtr){
				if(in != nullptr){
					return in->*headPtr;
				}
				throw NullPointerException{};
			}
			return in.*headPtr;
		}

		template <typename T>
			requires std::same_as<std::remove_cvref_t<T>, InType>
		static decltype(auto) get(T& in){
			return NextGroupType::get(SeqMemberPtrAccessor::getNext(in));
		}
	};


	template <auto headPtr>
	struct SeqMemberPtrAccessor<headPtr>{
		using RetType = typename GetMemberPtrType<decltype(headPtr)>::type;
		using InType = typename GetMemberPtrClass<decltype(headPtr)>::type;
		static constexpr bool isPtr = std::is_pointer_v<InType>;

		template <typename T>
			requires std::same_as<std::remove_cvref_t<T>, InType>
		static auto getNext(T& in) -> typename ConstConditional<isConstRef<T>, RetType&>::type{
			if constexpr(isPtr){
				if(in != nullptr){
					return in->*headPtr;
				}
				throw NullPointerException{};
			}
			return in.*headPtr;
		}

		template <typename T>
			requires std::same_as<std::remove_cvref_t<T>, InType>
		static decltype(auto) get(T& in){
			return SeqMemberPtrAccessor::getNext(in);
		}
	};
}
