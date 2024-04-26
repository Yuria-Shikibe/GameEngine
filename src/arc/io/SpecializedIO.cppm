module;

#include "../src/code-gen/ReflectData_Builtin.hpp"

export module Core.IO.Specialized;

export import Core.IO.BinaryIO;
export import Core.IO.JsonIO;
export import ext.Json;

export import Geom.Vector2D;
export import Geom.Rect_Orthogonal;
export import Graphic.Color;
export import Math;
import ext.StaticReflection;
import ext.Base64;


#define IO___FLATTEN_FUNC2(x,y) x##y

#define IO___FLATTEN_FUNC1(x,y) IO___FLATTEN_FUNC2(x,y)

#define IO_GEN_NAME(x) IO___FLATTEN_FUNC1(x,__COUNTER__)

#define IO_INSTANCE_NAMESPACE _instantiation

#define IO_INSTANCE(type) namespace IO_INSTANCE_NAMESPACE{type IO_GEN_NAME(_instance_){};}

#define DEFINE_SPEC(base, templateT, type) export template <> struct ::Core::IO::JsonSerializator<templateT<type>> : base<type>{};\
IO_INSTANCE(::Core::IO::JsonSerializator<templateT<type>>)\

#define DEFINE_FUND(base, type) export template <> struct ::Core::IO::JsonSerializator<type> : base<type>{};\
IO_INSTANCE(::Core::IO::JsonSerializator<type>)\

#define DEFINE_SPEC_IO_INSTANCE(templateT, type) \
IO_INSTANCE(::Core::IO::JsonSerializator<templateT<type>>)\

namespace Core::IO{
	template <typename T>
	struct JsonSrlFundamentalBase{
		using Ty = ext::json::JsonScalarType<T>;

		static void write(ext::json::JsonValue& jsonValue, const T data){
			jsonValue.setData<Ty>(data);
		}

		static void read(const ext::json::JsonValue& jsonValue, T& data){
			data = jsonValue.as<Ty>();
		}
	};

	template <typename T>
	struct JsonSrlSectionBase{
		using Ty = ext::json::JsonScalarType<T>;

		static void write(ext::json::JsonValue& jsonValue, const Math::Section<T>& data){
			auto& val = jsonValue.asArray();
			val.resize(2);
			val[0].setData<Ty>(data.from);
			val[1].setData<Ty>(data.to);
		}

		static void read(const ext::json::JsonValue& jsonValue, Math::Section<T>& data){
			if(auto* ptr = jsonValue.tryGetValue<ext::json::array>(); ptr && ptr->size() >= 2){
				data.from = ptr->at(0).as<Ty>();
				data.to = ptr->at(1).as<Ty>();
			}
		}
	};

	template <typename T>
	struct JsonSrlVecBase{
		using Ty = ext::json::JsonScalarType<T>;

		static void write(ext::json::JsonValue& jsonValue, const Geom::Vector2D<T>& data){
			auto& val = jsonValue.asArray();
			val.resize(2);
			val[0].setData<Ty>(data.x);
			val[1].setData<Ty>(data.y);
		}

		static void read(const ext::json::JsonValue& jsonValue, Geom::Vector2D<T>& data){
			if(auto* ptr = jsonValue.tryGetValue<ext::json::array>(); ptr && ptr->size() >= 2){
				data.x = ptr->at(0).as<Ty>();
				data.y = ptr->at(1).as<Ty>();
			}
		}
	};

	template <typename T>
	struct JsonSrlRectBase{
		using Ty = ext::json::JsonScalarType<T>;

		static void write(ext::json::JsonValue& jsonValue, const Geom::Rect_Orthogonal<T>& data){
			auto& val = jsonValue.asArray();
			val.resize(4);
			val[0].setData<Ty>(data.getSrcX());
			val[1].setData<Ty>(data.getSrcY());
			val[2].setData<Ty>(data.getWidth());
			val[3].setData<Ty>(data.getHeight());
		}

		static void read(const ext::json::JsonValue& jsonValue, Geom::Rect_Orthogonal<T>& data){
			if(auto* ptr = jsonValue.tryGetValue<ext::json::array>(); ptr && ptr->size() >= 4){
				data.set(
					ptr->at(0).as<Ty>(),
					ptr->at(1).as<Ty>(),
					ptr->at(2).as<Ty>(),
					ptr->at(3).as<Ty>());
			}
		}
	};

	template <>
	struct JsonSerializator<Graphic::Color>{
		static void write(ext::json::JsonValue& jsonValue, const Graphic::Color& data){
			jsonValue.setData(data.toString());
		}

		static void read(const ext::json::JsonValue& jsonValue, Graphic::Color& data){
			Graphic::Color::valueOf(data, jsonValue.as<std::string>());
		}
	};

	template <typename T, typename Alloc = std::allocator<T>>
		requires !std::is_pointer_v<T>
	struct JsonSrlContBase_vector{
		static void write(ext::json::JsonValue& jsonValue, const std::vector<T, Alloc>& data){
			auto& val = jsonValue.asArray();
			val.resize(data.size());
			std::ranges::transform(data, val.begin(), ext::json::getJsonOf);
		}

		static void read(const ext::json::JsonValue& jsonValue, std::vector<T, Alloc>& data){
			if(auto* ptr = jsonValue.tryGetValue<ext::json::array>()){
				data.resize(ptr->size());

				for(auto& [index, element] : data | std::ranges::views::enumerate){
					ext::json::getValueTo(element, ptr->at(index));
				}
			}
		}
	};


	template <typename K, typename V,
		class Hasher = std::hash<K>, class Keyeq = std::equal_to<K>,
		class Alloc = std::allocator<std::pair<const K, V>>>
		requires
		!std::is_pointer_v<K> && !std::is_pointer_v<V> &&
		std::is_default_constructible_v<K> && std::is_default_constructible_v<V>
	struct JsonSrlContBase_unordered_map{
		//OPTM using array instead of object to be the KV in json?
		static void write(ext::json::JsonValue& jsonValue, const std::unordered_map<K, V, Hasher, Keyeq, Alloc>& data){
			auto& val = jsonValue.asArray();
			val.reserve(data.size());

			for (auto& [k, v] : data){
				ext::json::JsonValue jval{};
				jval.asObject();
				jval.append(ext::json::Key, ext::json::getJsonOf(k));
				jval.append(ext::json::Value, ext::json::getJsonOf(v));
				val.push_back(std::move(jval));
			}
		}

		static void read(const ext::json::JsonValue& jsonValue, std::unordered_map<K, V>& data){
			if(auto* ptr = jsonValue.tryGetValue<ext::json::array>()){
				data.reserve(ptr->size());

				for (const auto & jval : *ptr){
					auto& pair = jval.asObject();

					K k{}; V v{};
					ext::json::getValueTo(k, pair.at(ext::json::Key));
					ext::json::getValueTo(v, pair.at(ext::json::Value));

					data.insert_or_assign(std::move(k), std::move(v));
				}
			}
		}
	};

// #pragma region Spec
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, char);
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, signed char);
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, unsigned char);
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, wchar_t);
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, char8_t);
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, char16_t);
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, char32_t);
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, short);
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, unsigned short);
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, int);
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, unsigned int);
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, long);
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, unsigned long);
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, long long);
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, unsigned long long);
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, float);
	DEFINE_SPEC(JsonSrlSectionBase, Math::Section, double);

	DEFINE_FUND(JsonSrlFundamentalBase, char);
	DEFINE_FUND(JsonSrlFundamentalBase, signed char);
	DEFINE_FUND(JsonSrlFundamentalBase, unsigned char);
	DEFINE_FUND(JsonSrlFundamentalBase, wchar_t);
	DEFINE_FUND(JsonSrlFundamentalBase, char8_t);
	DEFINE_FUND(JsonSrlFundamentalBase, char16_t);
	DEFINE_FUND(JsonSrlFundamentalBase, char32_t);
	DEFINE_FUND(JsonSrlFundamentalBase, short);
	DEFINE_FUND(JsonSrlFundamentalBase, unsigned short);
	DEFINE_FUND(JsonSrlFundamentalBase, int);
	DEFINE_FUND(JsonSrlFundamentalBase, unsigned int);
	DEFINE_FUND(JsonSrlFundamentalBase, long);
	DEFINE_FUND(JsonSrlFundamentalBase, unsigned long);
	DEFINE_FUND(JsonSrlFundamentalBase, long long);
	DEFINE_FUND(JsonSrlFundamentalBase, unsigned long long);
	DEFINE_FUND(JsonSrlFundamentalBase, float);
	DEFINE_FUND(JsonSrlFundamentalBase, double);
	DEFINE_FUND(JsonSrlFundamentalBase, bool);

	DEFINE_SPEC(JsonSrlVecBase, Geom::Vector2D, short);
	DEFINE_SPEC(JsonSrlVecBase, Geom::Vector2D, unsigned short);
	DEFINE_SPEC(JsonSrlVecBase, Geom::Vector2D, int);
	DEFINE_SPEC(JsonSrlVecBase, Geom::Vector2D, unsigned int)
	DEFINE_SPEC(JsonSrlVecBase, Geom::Vector2D, float);

	DEFINE_SPEC(JsonSrlRectBase, Geom::Rect_Orthogonal, int);
	DEFINE_SPEC(JsonSrlRectBase, Geom::Rect_Orthogonal, unsigned int);
	DEFINE_SPEC(JsonSrlRectBase, Geom::Rect_Orthogonal, float);
// #pragma endregion

	// Core::IO::JsonSerializator<Geom::OrthoRectFloat> inst{};

	// int foo(){
	// 	Geom::OrthoRectFloat rect{0, 0, 12, 12};
	//
	// 	using Ioer = Core::IO::JsonSerializator<decltype(rect)>;
	//
	// 	ext::json::JsonValue jval{};
	//
	// 	Ioer::write(rect, jval);
	//
	// 	// ext::reflect::ClassField<TestT>::fieldEach<EachFunc<TestT>>();
	// }
}

// DEFINE_SPEC_IO_INSTANCE(Math::Section, char);
// DEFINE_SPEC_IO_INSTANCE(Math::Section, signed char);
// DEFINE_SPEC_IO_INSTANCE(Math::Section, unsigned char);
// DEFINE_SPEC_IO_INSTANCE(Math::Section, wchar_t);
// DEFINE_SPEC_IO_INSTANCE(Math::Section, char8_t);
// DEFINE_SPEC_IO_INSTANCE(Math::Section, char16_t);
// DEFINE_SPEC_IO_INSTANCE(Math::Section, char32_t);
// DEFINE_SPEC_IO_INSTANCE(Math::Section, short);
// DEFINE_SPEC_IO_INSTANCE(Math::Section, unsigned short);
// DEFINE_SPEC_IO_INSTANCE(Math::Section, int);
// DEFINE_SPEC_IO_INSTANCE(Math::Section, unsigned int);
// DEFINE_SPEC_IO_INSTANCE(Math::Section, long);
// DEFINE_SPEC_IO_INSTANCE(Math::Section, unsigned long);
// DEFINE_SPEC_IO_INSTANCE(Math::Section, long long);
// DEFINE_SPEC_IO_INSTANCE(Math::Section, unsigned long long);
// DEFINE_SPEC_IO_INSTANCE(Math::Section, float);
// DEFINE_SPEC_IO_INSTANCE(Math::Section, double);
//
// DEFINE_SPEC_IO_INSTANCE(Geom::Vector2D, short);
// DEFINE_SPEC_IO_INSTANCE(Geom::Vector2D, unsigned short);
// DEFINE_SPEC_IO_INSTANCE(Geom::Vector2D, int);
// DEFINE_SPEC_IO_INSTANCE(Geom::Vector2D, unsigned int)
// DEFINE_SPEC_IO_INSTANCE(Geom::Vector2D, float);
//
// DEFINE_SPEC_IO_INSTANCE(Geom::Rect_Orthogonal, int);
// DEFINE_SPEC_IO_INSTANCE(Geom::Rect_Orthogonal, unsigned int);
// DEFINE_SPEC_IO_INSTANCE(Geom::Rect_Orthogonal, float);
