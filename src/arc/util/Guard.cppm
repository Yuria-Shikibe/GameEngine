//
// Created by Matrix on 2024/5/12.
//

export module ext.Guard;

import ext.MetaProgramming;
import ext.Concepts;
import std;

export namespace ext{
	template <typename T, auto T::* mem, bool passByMove = false>
		requires requires{
			requires
				(passByMove && std::is_move_assignable_v<typename ext::GetMemberPtrInfo<decltype(mem)>::ValueType>) ||
				(!passByMove && std::is_copy_assignable_v<typename ext::GetMemberPtrInfo<decltype(mem)>::ValueType>);
		}
	class Guard{
		using DataType = typename ext::GetMemberPtrInfo<decltype(mem)>::ValueType;

		T& tgt;
		DataType original{};
	public:
		[[nodiscard]] constexpr Guard(T& tgt, DataType& data) requires (!passByMove) : tgt{tgt}, original{std::invoke(mem, tgt)}{
			std::invoke(mem, tgt) = data;
		}

		[[nodiscard]] constexpr Guard(T& tgt, DataType&& data) requires (passByMove) : tgt{tgt}, original{std::move(std::invoke(mem, tgt))}{
			std::invoke(mem, tgt) = std::move(data);
		}

		constexpr ~Guard(){
			if constexpr (passByMove){
				std::invoke(mem, tgt) = std::move(original);
			}else{
				std::invoke(mem, tgt) = original;
			}
		}
	};
}
