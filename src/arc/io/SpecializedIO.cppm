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
import ext.RuntimeException;


#define IO___FLATTEN_FUNC2(x,y) x##y

#define IO___FLATTEN_FUNC1(x,y) IO___FLATTEN_FUNC2(x,y)

#define IO_GEN_NAME(x) IO___FLATTEN_FUNC1(x,__COUNTER__)

#define IO_INSTANCE_NAMESPACE Core::IO::_instantiation

#define EXPLICITE_VALID(type) template<> constexpr bool ::Core::IO::jsonDirectSerializable<type> = true;

#define IO_INSTANCE(type) namespace IO_INSTANCE_NAMESPACE{ type IO_GEN_NAME(_instance_){};}\

// EXPLICITE_VALID(type)\

#define DEFINE_SPEC(base, templateT, type) export template <> struct ::Core::IO::JsonSerializator<templateT<type>> : base<type>{};\
IO_INSTANCE(::Core::IO::JsonSerializator<templateT<type>>)\

#define DEFINE_FUND(base, type) export template <> struct ::Core::IO::JsonSerializator<type> : base<type>{};\
IO_INSTANCE(::Core::IO::JsonSerializator<type>)\

#define DEFINE_SPEC_IO_INSTANCE(templateT, type) \
IO_INSTANCE(::Core::IO::JsonSerializator<templateT<type>>)\


namespace Core::IO{


	export
	template <typename T>
	struct JsonSrlFundamentalBase{
		using Ty = ext::json::JsonScalarType<T>;

		static void write(ext::json::JsonValue& jsonValue, const T data){
			jsonValue.setData<Ty>(data);
		}

		static void read(const ext::json::JsonValue& jsonValue, T& data){
			data = jsonValue.asType<Ty>();
		}
	};

	export
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
				data.from = ptr->at(0).asType<Ty>();
				data.to = ptr->at(1).asType<Ty>();
			}
		}
	};

	export
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
				data.x = ptr->at(0).asType<Ty>();
				data.y = ptr->at(1).asType<Ty>();
			}
		}
	};

	export
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
					ptr->at(0).asType<Ty>(),
					ptr->at(1).asType<Ty>(),
					ptr->at(2).asType<Ty>(),
					ptr->at(3).asType<Ty>());
			}
		}
	};

	template <>
	struct JsonSerializator<Graphic::Color>{
		static void write(ext::json::JsonValue& jsonValue, const Graphic::Color& data){
			jsonValue.setData(data.toString());
		}

		static void read(const ext::json::JsonValue& jsonValue, Graphic::Color& data){
			Graphic::Color::valueOf(data, jsonValue.asType<std::string>());
		}
	};

	template <>
	struct JsonSerializator<std::string_view>{
		static void write(ext::json::JsonValue& jsonValue, const std::string_view data){
			jsonValue.setData<std::string>(static_cast<std::string>(data));
		}

		static void read(const ext::json::JsonValue& jsonValue, Graphic::Color& data){
			throw ext::IllegalArguments{"Read data to view should never happens"};
		}
	};



	template <std::ranges::sized_range Cont>
		requires !std::is_pointer_v<std::ranges::range_value_t<Cont>> && Core::IO::jsonSerializable<
			std::ranges::range_value_t<Cont>>
	struct JsonSrlContBase_vector{
		static void write(ext::json::JsonValue& jsonValue, const Cont& data){
			auto& val = jsonValue.asArray();
			val.resize(data.size());
			std::ranges::transform(data, val.begin(), ext::json::getJsonOf);
		}

		static void read(const ext::json::JsonValue& jsonValue, Cont& data){
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

			for(auto& [k, v] : data){
				ext::json::JsonValue jval{};
				jval.asObject();
				jval.append(ext::json::keys::Key, ext::json::getJsonOf(k));
				jval.append(ext::json::keys::Value, ext::json::getJsonOf(v));
				val.push_back(std::move(jval));
			}
		}

		static void read(const ext::json::JsonValue& jsonValue, std::unordered_map<K, V>& data){
			if(auto* ptr = jsonValue.tryGetValue<ext::json::array>()){
				data.reserve(ptr->size());

				for(const auto& jval : *ptr){
					auto& pair = jval.asObject();

					K k{};
					V v{};
					ext::json::getValueTo(k, pair.at(ext::json::keys::Key));
					ext::json::getValueTo(v, pair.at(ext::json::keys::Value));

					data.insert_or_assign(std::move(k), std::move(v));
				}
			}
		}
	};
}


DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, char);
DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, signed char);
DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, unsigned char);
DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, wchar_t);
DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, char8_t);
DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, char16_t);
DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, char32_t);
DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, short);
DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, unsigned short);
DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, int);
DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, unsigned int);
DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, long);
DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, unsigned long);
DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, long long);
DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, unsigned long long);
DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, float);
DEFINE_SPEC(::Core::IO::JsonSrlSectionBase, Math::Section, double);

DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, char);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, signed char);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, unsigned char);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, wchar_t);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, char8_t);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, char16_t);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, char32_t);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, short);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, unsigned short);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, int);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, unsigned int);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, long);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, unsigned long);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, long long);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, unsigned long long);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, float);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, double);
DEFINE_FUND(::Core::IO::JsonSrlFundamentalBase, bool);

DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<char>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<signed char>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<unsigned char>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<wchar_t>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<char8_t>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<char16_t>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<char32_t>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<short>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<unsigned short>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<int>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<unsigned int>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<long>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<unsigned long>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<long long>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<unsigned long long>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<float>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<double>);
DEFINE_FUND(::Core::IO::JsonSrlContBase_vector, std::vector<bool>);

DEFINE_SPEC(::Core::IO::JsonSrlVecBase, Geom::Vector2D, short);
DEFINE_SPEC(::Core::IO::JsonSrlVecBase, Geom::Vector2D, unsigned short);
DEFINE_SPEC(::Core::IO::JsonSrlVecBase, Geom::Vector2D, int);
DEFINE_SPEC(::Core::IO::JsonSrlVecBase, Geom::Vector2D, unsigned int)
DEFINE_SPEC(::Core::IO::JsonSrlVecBase, Geom::Vector2D, float);


DEFINE_SPEC(::Core::IO::JsonSrlRectBase, Geom::Rect_Orthogonal, int);
DEFINE_SPEC(::Core::IO::JsonSrlRectBase, Geom::Rect_Orthogonal, unsigned int);
DEFINE_SPEC(::Core::IO::JsonSrlRectBase, Geom::Rect_Orthogonal, float);



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
