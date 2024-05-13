//
// Created by Matrix on 2024/4/20.
//

export module ext.Algorithm;
import ext.Concepts;
import std;

export namespace ext{
	template<typename Range>
		requires Concepts::Iterable<Range>
	auto partBy(Range&& range, Concepts::Invokable<bool(const std::ranges::range_value_t<Range>&)> auto&& pred){
		return std::make_pair(range | std::ranges::views::filter(pred), range | std::ranges::views::filter(std::not_fn(pred)));
	}
}
