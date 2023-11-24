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
}