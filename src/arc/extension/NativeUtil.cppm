//
// Created by Matrix on 2023/11/20.
//
module ;

export module NativeUtil;

import <functional>;

//ext for 'Extension', my shitty codes
export namespace ext{
	template <typename T>
	std::function<void(const T&)> bindVoid(const T& object, std::function<void()>&& func){
		return std::bind(func, object);
	}
}
