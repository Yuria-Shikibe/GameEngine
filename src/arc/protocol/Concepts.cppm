module ;

export module Concepts;

import <functional>;
import <type_traits>;

export namespace Concepts {
    template <typename T>
    concept Number = std::is_arithmetic_v<T>;

    template <typename T>
    concept NumberSingle = std::is_arithmetic_v<T> && sizeof(T) <= 4;

    template <class T, class DerivedT>
    concept Derived = std::is_base_of_v<DerivedT, T>;

    template <class DerivedT, class... Bases>
    concept DerivedMulti = (std::is_base_of_v<Bases, DerivedT> && ...);

    template <class T>
    concept HasDefConstructor = std::is_default_constructible_v<T>;

	template <typename T, typename functype>
	concept Invokable = std::is_convertible_v<T, std::function<functype>>;

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
}