//
// Created by Matrix on 2024/3/26.
//

export module OS.Serializable;

export import std;

export namespace OS{
	// using Writer = std::ofstream;
	// using Reader = std::ifstream;
	// template <typename T>
	// struct BinarySerializable{
	// 	static void read(Reader&, T&) = delete;
	// 	static void write(Writer&, const T&) = delete;
	// };
	//
	// template <typename T, template<typename> typename V>
	// struct BinarySerializable<V<T>>{
	// 	static void read(Reader&, V<T>&){
	//
	// 	}
	// 	static void write(Writer&, const V<T>&){
	//
	// 	}
	// };
	//
	// void foo(){
	// 	BinarySerializable<std::vector<int>>::read()
	// }
}
