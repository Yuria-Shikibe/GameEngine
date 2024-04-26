//
// Created by Matrix on 2024/4/26.
//

export module Core.IO.General;

import std;
import ext.DynamicBuffer;

export namespace Core::IO{
	using BinDataType = unsigned char;

	template <typename T>
	[[nodiscard]] std::vector<BinDataType> toByte(const T& val){
		std::vector<BinDataType> buffer(sizeof(val));
		std::memcpy(buffer.data(), &val, sizeof(val));
		return buffer;
	}

	template <typename T>
		requires std::is_default_constructible_v<T> && std::is_trivially_assignable_v<T, T>
	[[nodiscard]] T fromByte(const std::vector<BinDataType>& data){
		T val{};

		if(data.size() >= sizeof(T)){
			std::memcpy(&val, data.data(), sizeof(val));
		}

		return val;
	}

	template <typename T>
		requires std::is_default_constructible_v<T> && std::is_trivially_assignable_v<T, T>
	void fromByte(T& val, const std::vector<BinDataType>& data){
		if(data.size() >= sizeof(T)){
			std::memcpy(&val, data.data(), sizeof(val));
		}
	}
}
