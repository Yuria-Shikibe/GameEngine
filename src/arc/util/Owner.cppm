//
// Created by Matrix on 2024/4/27.
//

export module ext.Owner;

import std;

export namespace ext{
	template<typename T>
		requires std::is_pointer_v<T>
	using Owner = T;
}
