//
// Created by Matrix on 2024/3/23.
//

export module Heterogeneous;

import std;
import TypeTraits;

namespace ext{
	struct StringEqualComparator{
		using is_transparent = void;
		bool operator()(const std::string_view a, const std::string_view b) const noexcept {
			return a == b;
		}

		bool operator()(const std::string_view a, const std::string& b) const noexcept {
			return a == static_cast<std::string_view>(b);
		}

		bool operator()(const std::string& b, const std::string_view a) const noexcept {
			return a == static_cast<std::string_view>(b);
		}

		bool operator()(const std::string& b, const std::string& a) const noexcept {
			return a == b;
		}
	};

	struct StringHasher{
		using is_transparent = void;

		size_t operator()(const std::string_view val) const noexcept {
			static constexpr std::hash<std::string_view> hasher{};
			return hasher(val);
		}

		size_t operator()(const std::string& val) const noexcept {
			static constexpr std::hash<std::string> hasher{};
			return hasher(val);
		}
	};

	template <typename T, typename Deleter = std::default_delete<T>>
	struct UniquePtrEqualer{
		using is_transparent = void;

		bool operator()(const T* a, const std::unique_ptr<T, Deleter>& b) const noexcept {
			return a == b.get();
		}

		bool operator()(const std::unique_ptr<T, Deleter>& b, const T* a) const noexcept {
			return a == b.get();
		}

		bool operator()(const std::unique_ptr<T, Deleter>& a, const std::unique_ptr<T, Deleter>& b) const noexcept {
			return a == b;
		}
	};

	template <typename T, typename Deleter = std::default_delete<T>>
	struct UniqueHasher{
		using is_transparent = void;
		static constexpr std::hash<const T*> hasher{};

		size_t operator()(const T* a) const noexcept {
			return hasher(a);
		}

		size_t operator()(const std::unique_ptr<T, Deleter>& a) const noexcept {
			return hasher(a.get());
		}
	};
}

export namespace ext{
	template <typename T, auto T::* ptr>
		requires requires(T& t){
			requires ext::HasDefHasher<T> && ext::HasDefHasher<typename GetMemberPtrType<decltype(ptr)>::type>;
		}
	struct MemberHasher{
		using MemberType = typename GetMemberPtrType<decltype(ptr)>::type;
		using is_transparent = void;

		size_t operator()(const T& val) const noexcept {
			static constexpr std::hash<T> hasher{};
			return hasher(val);
		}

		size_t operator()(const MemberType& val) const noexcept {
			static constexpr std::hash<MemberType> hasher{};
			return hasher(val);
		}
	};

	template <typename T, auto T::* ptr>
		requires requires(T& t){
			requires ext::HasDefHasher<T> && ext::HasDefHasher<typename GetMemberPtrType<decltype(ptr)>::type>;
		}
	struct MemberEqualTo{
		using MemberType = typename GetMemberPtrType<decltype(ptr)>::type;
		using is_transparent = void;

		bool operator()(const T& a, const T& b) const noexcept{
			static constexpr std::equal_to<T> equal{};
			return equal.operator()(a, b);
		}

		bool operator()(const MemberType& a, const T& b) const noexcept{
			static constexpr std::equal_to<MemberType> equal{};
			return equal(a, b.*ptr);
		}
	};



	template <typename V>
	class StringMap : public std::unordered_map<std::string, V, StringHasher, StringEqualComparator>{
	public:
		StringMap() = default;

		V& at(const std::string_view key){
			return this->find(key)->second;
		}

		const V& at(const std::string_view key) const {
			return this->find(key)->second;
		}
	};

	template <typename T, typename Deleter = std::default_delete<T>>
	using UniquePtrHashMap = std::unordered_map<std::unique_ptr<T, Deleter>, UniqueHasher<T, Deleter>, UniquePtrEqualer<T, Deleter>>;

	template <typename T, typename Deleter = std::default_delete<T>>
	using UniquePtrSet = std::unordered_set<std::unique_ptr<T, Deleter>, UniqueHasher<T, Deleter>, UniquePtrEqualer<T, Deleter>>;

	template <typename V>
	using StringMultiMap = std::unordered_multimap<std::string, V, StringHasher, StringEqualComparator>;
}