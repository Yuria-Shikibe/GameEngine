//
// Created by Matrix on 2024/3/9.
//

export module Container.ContinuousPool;

import <array>;
import <bitset>;
import <memory>;

export namespace Container{
	// template <typename T, size_t MaxSize>
	// class ContinuousPool {
	// 	std::array<T, MaxSize> buffer{};
	// 	std::bitset<MaxSize> borrowed{};
	//
	// 	struct Deleter {
	// 		ContinuousPool& src{};
	// 		size_t index{0};
	//
	// 		Deleter(ContinuousPool<T, MaxSize>& src, size_t index)
	// 			: src(src),
	// 			  index(index){
	// 		}
	//
	// 		void operator()(const T* t){
	// 			if(index >= MaxSize){
	// 				delete t;
	// 			}else{
	// 				src.borrowed
	// 			}
	// 		}
	// 	};
	//
	// 	using Ptr = std::unique_ptr<T, Deleter>;
	//
	// 	Ptr obtain(){
	// 		borrowed.
	// 	}
	// };
}
