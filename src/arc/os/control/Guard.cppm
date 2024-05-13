//
// Created by Matrix on 2024/5/12.
//

export module OS.Ctrl:Guard;

import ext.MetaProgramming;
import ext.Concepts;
import std;
import :FocusInterface;

export namespace Ctrl{
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
		[[nodiscard]] constexpr Guard(T& tgt, DataType& data) requires (!passByMove) : tgt{tgt}, original{tgt.*mem}{
			tgt.*mem = data;
		}

		[[nodiscard]] constexpr Guard(T& tgt, DataType&& data) requires (passByMove) : tgt{tgt}, original{std::move(tgt.*mem)}{
			tgt.*mem = std::move(data);
		}

		constexpr ~Guard(){
			if constexpr (passByMove){
				tgt.*mem = std::move(original);
			}else{
				tgt.*mem = original;
			}
		}
	};
}
