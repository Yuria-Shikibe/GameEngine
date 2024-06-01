//
// Created by Matrix on 2024/5/25.
//

export module ext.BooleanOperation;

import std;

export namespace ext{
	enum struct BooleanOperation : int{
		Replace = 0,
		Or = 0b0000'0001,
		And = 0b0000'0010,
		Not = 0b0000'0100,
		Xor = 0b0000'1000,

		Reserve = 0b0001'0000,
	};

	constexpr std::array AllBoolOp{
		BooleanOperation::Replace,
		BooleanOperation::Or,
		BooleanOperation::And,
		BooleanOperation::Not,
		BooleanOperation::Xor,
	};

	constexpr std::array AllNamedBoolOp{
		std::pair<BooleanOperation, std::string_view>{BooleanOperation::Replace, {"subtract-selection"}},
		std::pair<BooleanOperation, std::string_view>{BooleanOperation::Or, {"union-selection"}},
		std::pair<BooleanOperation, std::string_view>{BooleanOperation::And, {"intersect-selection"}},
		std::pair<BooleanOperation, std::string_view>{BooleanOperation::Not, {"subtract-selection-one"}},
		std::pair<BooleanOperation, std::string_view>{BooleanOperation::Xor, {"exclude-selection"}},
	};

	constexpr std::string_view getBoolOpName(BooleanOperation op){
		op = BooleanOperation{static_cast<std::underlying_type_t<BooleanOperation>>(op) & 0x0fu};

		switch(op){
			case BooleanOperation::Replace : return AllNamedBoolOp[0].second;
			case BooleanOperation::Or : return AllNamedBoolOp[1].second;
			case BooleanOperation::And : return AllNamedBoolOp[2].second;
			case BooleanOperation::Not : return AllNamedBoolOp[3].second;
			case BooleanOperation::Xor : return AllNamedBoolOp[4].second;

			default: std::unreachable();
		}


	}
}

export ext::BooleanOperation operator|(const ext::BooleanOperation l, const ext::BooleanOperation r){
	return ext::BooleanOperation{static_cast<std::underlying_type_t<ext::BooleanOperation>>(l) | static_cast<std::underlying_type_t<ext::BooleanOperation>>(r)};
}

export bool operator&(const ext::BooleanOperation l, const ext::BooleanOperation r){
	return static_cast<std::underlying_type_t<ext::BooleanOperation>>(l) & static_cast<std::underlying_type_t<ext::BooleanOperation>>(r);
}

export namespace ext{
	template <typename K, typename V, typename Hs, typename Eq, typename Alloc, typename Append>
		requires std::same_as<std::decay_t<Append>, std::unordered_map<K, V, Hs, Eq, Alloc>>
	void booleanConj(const BooleanOperation op,
		std::unordered_map<K, V, Hs, Eq, Alloc>& to,
		Append&& append
	){
		const auto opTy = BooleanOperation{static_cast<std::underlying_type_t<BooleanOperation>>(op) & 0x0fu};
		const bool replace = !(op & BooleanOperation::Reserve);

		if(opTy == BooleanOperation::Replace){
			to = std::forward<std::unordered_map<K, V, Hs, Eq, Alloc>>(append);
			return;
		}

		std::unordered_set<K> checked{};
		switch(opTy){
			case BooleanOperation::Or: break;
			case BooleanOperation::And :{
				checked = to | std::ranges::views::keys | std::ranges::to<std::unordered_set<K>>();
			}
			default: checked.reserve(to.size());
		}

		for (auto&& [key, val] : std::forward<std::unordered_map<K, V, Hs, Eq, Alloc>>(append)){
			switch(opTy){
				case BooleanOperation::Or:{
					if(replace){
						to.insert_or_assign(std::forward<decltype(key)>(key), std::forward<decltype(val)>(val));
					}else{
						to.try_emplace(std::forward<decltype(key)>(key), std::forward<decltype(val)>(val));
					}
					break;
				}

				case BooleanOperation::And:{
					auto itr = to.find(key);

					if(itr == to.end())break;

					checked.erase(key);

					if(replace){
						to.insert_or_assign(itr, std::forward<decltype(key)>(key), std::forward<decltype(val)>(val));
					}

					break;
				}

				case BooleanOperation::Xor:{
					auto itr = to.find(key);

					if(itr == to.end()){
						to.try_emplace(std::forward<decltype(key)>(key), std::forward<decltype(val)>(val));
					}else{
						checked.insert(key);
					}

					break;
				}

				case BooleanOperation::Not:{
					auto itr = to.find(key);

					if(itr != to.end())checked.insert(key);

					break;
				}

				default: std::unreachable();
			}
		}

		for (const auto& k : checked){
			to.erase(k);
		}
	}
}
