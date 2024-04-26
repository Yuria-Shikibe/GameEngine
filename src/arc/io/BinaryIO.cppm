//
// Created by Matrix on 2024/4/23.
//

export module Core.IO.BinaryIO;

import std;

export namespace Core::IO{
	using Writer = std::ostream;
	using Reader = std::istream;


	template <typename T>
	struct BinarySerializable{
		static void read(Reader&, T&) = delete;

		static void write(Writer&, const T&) = delete;
	};


	template <typename T>
		requires std::is_trivial_v<T>
	struct BinarySerializable<T>{
		static void read(Reader& reader, T& val){
			reader.read(reinterpret_cast<char*>(&val), sizeof(val));
		}

		static void write(Writer& writer, const T& val){
			writer.write(reinterpret_cast<const char*>(&val), sizeof(val));
		}
	};


	template <template<typename T, typename Alloc> typename V, typename T, typename Alloc>
		requires requires(V<T, Alloc> container){
			requires std::ranges::range<V<T, Alloc>>;
			requires std::is_arithmetic_v<typename V<T, Alloc>::size_type>;
			requires std::is_default_constructible_v<T>;
			container.resize(typename V<T, Alloc>::size_type{0});
		}
	struct BinarySerializable<V<T, Alloc>>{
		using Container = V<T, Alloc>;

		static void read(Reader& reader, Container& container){
			typename Container::size_type size{0};
			reader.read(reinterpret_cast<char*>(&size), sizeof(size));

			container.resize(size);

			for(const auto& element : container){
				BinarySerializable<T>::read(reader, element);
			}
		}

		static void write(Writer& writer, const Container& container){
			const typename Container::size_type size = container.size();
			writer.write(reinterpret_cast<const char*>(&size), sizeof(size));

			for(const auto& element : container){
				BinarySerializable<T>::write(writer, element);
			}
		}
	};
}
