//
// Created by Matrix on 2024/3/23.
//

export module ext.Heterogeneous;

import std;
import ext.MetaProgramming;

export namespace ext::transparent{

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

	template <template <typename > typename Comp>
		requires std::regular_invocable<Comp<std::string_view>, std::string_view, std::string_view>
	struct StringComparator{
		static constexpr Comp<std::string_view> comp{};
		using is_transparent = void;
		auto operator()(const std::string_view a, const std::string_view b) const noexcept {
			return comp(a, b);
		}

		auto operator()(const std::string_view a, const std::string& b) const noexcept {
			return comp(a, b);
		}

		auto operator()(const std::string& a, const std::string_view b) const noexcept {
			return comp(a, b);
		}

		auto operator()(const std::string& a, const std::string& b) const noexcept {
			return comp(a, b);
		}
	};

	struct StringHasher{
		using is_transparent = void;

		std::size_t operator()(const std::string_view val) const noexcept {
			static constexpr std::hash<std::string_view> hasher{};
			return hasher(val);
		}

		std::size_t operator()(const std::string& val) const noexcept {
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

		std::size_t operator()(const T* a) const noexcept {
			return hasher(a);
		}

		std::size_t operator()(const std::unique_ptr<T, Deleter>& a) const noexcept {
			return hasher(a.get());
		}
	};
}

export namespace ext{
	template <typename T, auto T::* ptr>
		requires requires(T& t){
			requires ext::HasDefHasher<T> && ext::HasDefHasher<typename GetMemberPtrInfo<decltype(ptr)>::ValueType>;
		}
	struct MemberHasher{
		using MemberType = typename GetMemberPtrInfo<decltype(ptr)>::ValueType;
		using is_transparent = void;

		std::size_t operator()(const T& val) const noexcept {
			static constexpr std::hash<T> hasher{};
			return hasher(val);
		}

		std::size_t operator()(const MemberType& val) const noexcept {
			static constexpr std::hash<MemberType> hasher{};
			return hasher(val);
		}
	};

	template <typename T, auto T::* ptr>
		requires requires(T& t){
			requires ext::HasDefHasher<T> && ext::HasDefHasher<typename GetMemberPtrInfo<decltype(ptr)>::ValueType>;
		}
	struct MemberEqualTo{
		using MemberType = typename GetMemberPtrInfo<decltype(ptr)>::ValueType;
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

	template <typename Alloc = std::allocator<std::string>>
	using StringHashSet = std::unordered_set<std::string, transparent::StringHasher, transparent::StringEqualComparator>;

	template <template<typename > typename Comp = std::less, typename Alloc = std::allocator<std::string>>
	using StringSet = std::set<std::string, transparent::StringComparator<Comp>, Alloc>;

	template <typename V, template<typename > typename Comp = std::less, typename Alloc = std::allocator<std::pair<const std::string, V>>>
	using StringMap = std::map<std::string, V, transparent::StringComparator<Comp>, Alloc>;

	template <typename V>
	class StringHashMap : public std::unordered_map<std::string, V, transparent::StringHasher, transparent::StringEqualComparator>{
	private:
		using SelfType = std::unordered_map<std::string, V, transparent::StringHasher, transparent::StringEqualComparator>;
	public:
		using SelfType::unordered_map;

		V& at(const std::string_view key){
			return this->find(key)->second;
		}

		const V& at(const std::string_view key) const {
			return this->find(key)->second;
		}

		V at(const std::string_view key, const V& def) const requires std::is_copy_assignable_v<V>{
			if(const auto itr = this->find(key); itr != this->end()){
				return itr->second;
			}
			return def;
		}

		V* tryFind(const std::string_view key){
			if(const auto itr = this->find(key); itr != this->end()){
				return &itr->second;
			}
			return nullptr;
		}

		const V* tryFind(const std::string_view key) const {
			if(const auto itr = this->find(key); itr != this->end()){
				return &itr->second;
			}
			return nullptr;
		}

		using SelfType::insert_or_assign;

		template <class Arg>
		std::pair<typename SelfType::iterator, bool> insert_or_assign(const std::string_view key, Arg&& val) {
			return this->insert_or_assign(static_cast<std::string>(key), std::forward<Arg>(val));
		}

		using SelfType::operator[];

		V& operator[](const std::string_view key) {
			return this->try_emplace(std::string(key)).first->second;
		}
	};


	template <typename T, typename Deleter = std::default_delete<T>>
	using UniquePtrHashMap = std::unordered_map<std::unique_ptr<T, Deleter>, transparent::UniqueHasher<T, Deleter>, transparent::UniquePtrEqualer<T, Deleter>>;

	template <typename T, typename Deleter = std::default_delete<T>>
	using UniquePtrSet = std::unordered_set<std::unique_ptr<T, Deleter>, transparent::UniqueHasher<T, Deleter>, transparent::UniquePtrEqualer<T, Deleter>>;

	template <typename V>
	using StringMultiMap = std::unordered_multimap<std::string, V, transparent::StringHasher, transparent::StringEqualComparator>;

	template <typename T, auto T::* ptr>
	using HashSet_ByMember = std::unordered_set<T, MemberHasher<T, ptr>, MemberEqualTo<T, ptr>>;
}