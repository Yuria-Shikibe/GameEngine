//
// Created by Matrix on 2024/4/20.
//

export module GL.Shader.UniformWrapper;
import std;
import ext.MetaProgramming;
import ext.Concepts;

export namespace GL{
	template<typename ...Args>
	struct UniformTupleWrapper{
		static constexpr std::size_t size = sizeof...(Args);
		using ArgsType = std::tuple<Args...>;
		ArgsType defData{};
		ArgsType data{};

		[[nodiscard]] constexpr UniformTupleWrapper() noexcept = default;

		[[nodiscard]] constexpr explicit UniformTupleWrapper(ArgsType&& defParams) noexcept : defData{std::move(defParams)}, data{defData}{}

		template <typename ...T>
		[[nodiscard]] constexpr explicit UniformTupleWrapper(T&& ...args) noexcept : defData(std::make_tuple(std::forward<T>(args) ... )), data{defData}{}

		template <typename ...T>
			requires requires {requires ext::isArgsSubOf<true, ArgsType, T...>();}
		constexpr void set(T&&... args) noexcept{
			data = ext::makeTuple_withDef(defData, std::forward<T>(args) ...);
		}

		constexpr void setDef() noexcept{
			data = defData;
		}

		template <std::size_t argIndex, bool asReference = false>
			requires requires{ requires argIndex < UniformTupleWrapper::size; }
		[[nodiscard]] constexpr typename Concepts::RefConditional<!asReference, std::tuple_element_t<argIndex, ArgsType>>::type
			get() noexcept{
			return std::get<argIndex>(data);
		}

		template <std::size_t argIndex, bool asReference = true>
			requires requires{ requires argIndex < UniformTupleWrapper::size; }
		[[nodiscard]] constexpr typename Concepts::RefConditional<!asReference, std::tuple_element_t<argIndex, ArgsType>>::type
			get() const noexcept{
			return std::get<argIndex>(data);
		}
	};

	template<typename ...Args>
	UniformTupleWrapper(std::tuple<Args...>) -> UniformTupleWrapper<Args...>;

	template<typename ...Args>
	UniformTupleWrapper(Args...) -> UniformTupleWrapper<Args...>;

	// template <typename T>
	// 	// requires std::is_nothrow_constructible_v<T> && std::is_copy_assignable_v<T>
	// struct UniformStructWrapper{
	// 	using DataType = T;
	// 	T defData;
	// 	T data;
	//
	// 	constexpr UniformStructWrapper() noexcept = default;
	//
	// 	template <typename... Args>
	// 		requires requires(Args&&... args){
	// 			T{std::forward<Args>(args)...};
	// 		}
	// 	explicit constexpr UniformStructWrapper(Args&&... args) noexcept : defData{std::forward<Args>(args)...}, data{defData}{}
	//
	// 	template <typename... Args>
	// 		requires requires(T t, Args&&... args){
	// 			t = {std::forward<Args>(args)...};
	// 		}
	// 	constexpr void set(Args&&... args) noexcept{
	// 		data = {args...};
	// 	}
	//
	// 	constexpr void setDef() noexcept{
	// 		data = defData;
	// 	}
	// };

	template <typename WT, typename ...Args>
		requires requires{
			typename WT::ArgsType;
			requires ext::isArgsSubOf<false, typename WT::ArgsType, Args...>();
		}
	struct UniformGuard{
		using ArgsType = typename WT::ArgsType;
		using WrapperType = WT;

		WrapperType& wrapper{};

		template <typename ...T>
			requires requires {requires ext::isArgsSubOf<false, ArgsType, T...>();}
		[[nodiscard]] explicit constexpr UniformGuard(WrapperType& wrapper, T&&... args) noexcept : wrapper{wrapper} {
			wrapper.set(std::forward<T>(args) ...);
		}

		constexpr ~UniformGuard() noexcept{
			wrapper.setDef();
		}
	};
	//
	// template <typename T>
	// struct UniformGuard{
	// 	using WrapperType = UniformStructWrapper<T>;
	//
	// 	WrapperType& wrapper{};
	//
	// 	template <typename ...Args>
	// 	[[nodiscard]] explicit constexpr UniformGuard(WrapperType& wrapper, Args&&... args) noexcept : wrapper{wrapper} {
	// 		wrapper.set(std::forward<Args>(args) ...);
	// 	}
	//
	// 	constexpr ~UniformGuard() noexcept{
	// 		wrapper.setDef();
	// 	}
	// };

	template <typename WT, typename ...Args>
		requires requires {requires ext::isArgsSubOf<false, typename WT::ArgsType, Args...>();}
	UniformGuard(WT, Args...) -> UniformGuard<WT, Args...>;

	// template <typename T, typename ...Args>
	// UniformGuard(UniformStructWrapper<T>, Args...) -> UniformGuard<UniformStructWrapper<T>>;
}
