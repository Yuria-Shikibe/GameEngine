//
// Created by Matrix on 2024/6/8.
//

export module Math.Algo.TopologicalSort;

import std;
import ext.RuntimeException;

export namespace Math{
	struct TestT{
		TestT* dependencyTarget{};
		int val{};
	};

	/**
	 * @brief Sort given range by provided dependency relation
	 * @tparam Comp Comparator Type
	 * @tparam Rng Range Type
	 * @tparam Proj Projection Type <current to dependency, usually a pointer>
	 * @tparam KeyProj Projection Type to its key
	 * @tparam Pred Check if <current> is valid
	 */
	template <
		std::ranges::sized_range Rng,
		std::indirectly_unary_invocable<std::ranges::iterator_t<Rng>> Proj,
		std::indirectly_unary_invocable<std::ranges::iterator_t<Rng>> KeyProj = std::identity,
		std::indirect_unary_predicate<std::projected<std::ranges::iterator_t<Rng>, Proj>> Pred = std::identity>
		requires requires{
			requires std::invocable<Proj, typename std::projected<std::ranges::iterator_t<Rng>, Proj>::value_type>;
			requires std::permutable<std::ranges::iterator_t<Rng>>;
		}
	auto get_topological_depth_map(const Rng& rng, Proj proj, KeyProj keyProj = {}, Pred pred = {})
		-> std::unordered_map<typename std::projected<std::ranges::iterator_t<Rng>, KeyProj>::value_type, std::size_t>
	{
		using ProjTy = typename std::projected<std::ranges::iterator_t<Rng>, Proj>::value_type;
		using K = typename std::projected<std::ranges::iterator_t<Rng>, KeyProj>::value_type;

		std::unordered_map<K, std::size_t> depth{std::ranges::size(rng)};
		std::unordered_set<K> visited(std::ranges::size(rng) / 2);

		for(auto curItr = std::ranges::begin(rng); curItr != std::ranges::end(rng); ++curItr){
			visited.clear();
			ProjTy curNode = std::invoke(proj, *curItr);

			while(pred(curNode)){
				auto key = std::invoke(keyProj, curNode);

				if(visited.contains(key) || visited.size() > std::ranges::size(rng)){
					throw ext::IllegalArguments{"Loop Reference detected during topological sort"};
				}

				visited.insert(key);
				++depth[key];
				curNode = std::invoke(proj, curNode);
			}
		}

#if DEBUG_CHECK
		if(depth.size() > std::ranges::size(rng)){
			throw ext::IllegalArguments{"Illegal Key: Depth Size Not Equal To Src Size!"};
		}
#endif

		for(auto curItr = std::ranges::begin(rng); curItr != std::ranges::end(rng); ++curItr){
			depth.try_emplace(std::invoke(keyProj, *curItr), 0);
		}

		return depth;
	}

	/**
	 * @brief Sort given range by provided dependency relation
	 * @tparam Comp Comparator Type
	 * @tparam Rng Range Type
	 * @tparam Proj Projection Type <current to dependency, usually a pointer>
	 * @tparam KeyProj Projection Type to its key
	 * @tparam Pred Check if <current> is valid
	 */
	template <
		template <typename> typename Comp = std::less,
		typename Rng,
		typename Proj,
		typename KeyProj = std::identity,
		typename Pred = std::identity>
		requires requires{
			requires std::regular_invocable<Comp<std::size_t>, std::size_t, std::size_t>;
			requires std::permutable<std::ranges::iterator_t<Rng>>;
		}
	void sort_topological(Rng& rng, Proj proj, KeyProj keyProj = {}, Pred pred = {}){
		auto depth = Math::get_topological_depth_map(rng, proj, keyProj, pred);

		std::ranges::sort(rng, [&](auto& l, auto& r){
			static constexpr Comp<std::size_t> comp{};
			return comp.operator()(depth.at(std::invoke(keyProj, l)), depth.at(std::invoke(keyProj, r)));
		});
	}
}
