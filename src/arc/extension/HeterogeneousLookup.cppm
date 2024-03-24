//
// Created by Matrix on 2024/3/23.
//

export module Heterogeneous;

import std;

export namespace ext{
	struct StringEqualComparator{
		using is_transparent = void;
		bool operator()(const std::string_view a, const std::string_view b) const {
			return a == b;
		}

		bool operator()(const std::string_view a, const std::string& b) const {
			return a == static_cast<std::string_view>(b);
		}

		bool operator()(const std::string& b, const std::string_view a) const {
			return a == static_cast<std::string_view>(b);
		}

		bool operator()(const std::string& a, const std::string& b) const {
			return a == b;
		}
	};

	struct StringHasher{
		using is_transparent = void;

		size_t operator()(const std::string_view val) const noexcept {
			constexpr std::hash<std::string_view> hasher{};
			return hasher(val);
		}

		size_t operator()(const std::string& val) const noexcept {
			constexpr std::hash<std::string> hasher{};
			return hasher(val);
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

	template <typename V>
	using StringMultiMap = std::unordered_multimap<std::string, V, StringHasher, StringEqualComparator>;
}