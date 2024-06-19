module;

#include "../src/code-gen/ReflectData_Builtin.hpp"

export module Core.IO.Specialized;

export import Core.IO.BinaryIO;
export import ext.json;
export import ext.json.io;

export import Geom.Vector2D;
export import Geom.Transform;
export import Geom.Rect_Orthogonal;
export import Graphic.Color;
export import Math;

import ext.StaticReflection;
import ext.Base64;
import ext.RuntimeException;
import ext.Heterogeneous;

#define IO_FLATTEN_FUNC2(x,y) x##y

#define IO_FLATTEN_FUNC1(x,y) IO_FLATTEN_FUNC2(x,y)

#define IO_GEN_NAME(x) IO_FLATTEN_FUNC1(x,__COUNTER__)

#define IO_INSTANCE_NAMESPACE Core::IO::_instantiation

#define EXPLICITE_VALID(type) template<> constexpr bool ::ext::json::jsonDirectSerializable<type> = true;

#define IO_INSTANCE(type) namespace IO_INSTANCE_NAMESPACE{ type IO_GEN_NAME(_instance_){};}\

// EXPLICITE_VALID(type)\

#define DEFINE_SPEC(base, templateT, type) export template <> struct ::ext::json::JsonSerializator<templateT<type>> : base<type>{};\
IO_INSTANCE(::ext::json::JsonSerializator<templateT<type>>)\

#define DEFINE_FUND(base, type) export template <> struct ::ext::json::JsonSerializator<type> : base<type>{};\
IO_INSTANCE(::ext::json::JsonSerializator<type>)\

#define DEFINE_SPEC_IO_INSTANCE(templateT, type) \
IO_INSTANCE(::ext::json::JsonSerializator<templateT<type>>)\


namespace ext::json{
	export
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
				data.from = ptr->at(0).as<Ty>();
				data.to = ptr->at(1).as<Ty>();
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
				data.x = ptr->at(0).as<Ty>();
				data.y = ptr->at(1).as<Ty>();
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
					ptr->at(0).as<Ty>(),
					ptr->at(1).as<Ty>(),
					ptr->at(2).as<Ty>(),
					ptr->at(3).as<Ty>());
			}
		}
	};

	export
	template <>
	struct ext::json::JsonSerializator<Graphic::Color>{
		static void write(ext::json::JsonValue& jsonValue, const Graphic::Color& data){
			jsonValue.setData(data.toString());
		}

		static void read(const ext::json::JsonValue& jsonValue, Graphic::Color& data){
			Graphic::Color::valueOf(data, jsonValue.as<std::string>());
		}
	};

	export
	template <>
	struct ext::json::JsonSerializator<Geom::Transform>{
		using Ty = ext::json::JsonScalarType<float>;
		static constexpr std::size_t ExpectedDataMinSize = 3;

		static void write(ext::json::JsonValue& jsonValue, const Geom::Transform& data){
			auto& val = jsonValue.asArray();
			val.resize(3);
			val[0].setData<Ty>(data.vec.x);
			val[1].setData<Ty>(data.vec.y);
			val[2].setData<Ty>(data.rot);
		}

		static void read(const ext::json::JsonValue& jsonValue, Geom::Transform& data){
			if(auto* ptr = jsonValue.tryGetValue<ext::json::array>(); ptr && ptr->size() >= ExpectedDataMinSize){
				data.vec.x = ptr->at(0).as<Ty>();
				data.vec.y = ptr->at(1).as<Ty>();
				data.rot = ptr->at(2).as<Ty>();
			}
		}
	};
}


DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, char);
DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, signed char);
DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, unsigned char);
DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, wchar_t);
DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, char8_t);
DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, char16_t);
DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, char32_t);
DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, short);
DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, unsigned short);
DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, int);
DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, unsigned int);
DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, long);
DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, unsigned long);
DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, long long);
DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, unsigned long long);
DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, float);
DEFINE_SPEC(::ext::json::JsonSrlSectionBase, Math::Section, double);

DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, char);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, signed char);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, unsigned char);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, wchar_t);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, char8_t);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, char16_t);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, char32_t);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, short);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, unsigned short);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, int);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, unsigned int);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, long);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, unsigned long);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, long long);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, unsigned long long);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, float);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, double);
DEFINE_FUND(::ext::json::JsonSrlFundamentalBase, bool);

DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<char>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<signed char>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<unsigned char>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<wchar_t>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<char8_t>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<char16_t>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<char32_t>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<short>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<unsigned short>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<int>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<unsigned int>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<long>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<unsigned long>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<long long>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<unsigned long long>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<float>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<double>);
DEFINE_FUND(::ext::json::JsonSrlContBase_vector, std::vector<bool>);

DEFINE_SPEC(::ext::json::JsonSrlVecBase, Geom::Vector2D, short);
DEFINE_SPEC(::ext::json::JsonSrlVecBase, Geom::Vector2D, unsigned short);
DEFINE_SPEC(::ext::json::JsonSrlVecBase, Geom::Vector2D, int);
DEFINE_SPEC(::ext::json::JsonSrlVecBase, Geom::Vector2D, unsigned int)
DEFINE_SPEC(::ext::json::JsonSrlVecBase, Geom::Vector2D, float);


DEFINE_SPEC(::ext::json::JsonSrlRectBase, Geom::Rect_Orthogonal, int);
DEFINE_SPEC(::ext::json::JsonSrlRectBase, Geom::Rect_Orthogonal, unsigned int);
DEFINE_SPEC(::ext::json::JsonSrlRectBase, Geom::Rect_Orthogonal, float);



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
