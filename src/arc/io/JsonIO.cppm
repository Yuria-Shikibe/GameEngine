export module Core.IO.JsonIO;

import std;
import MetaProgramming;
import ext.StaticReflection;
export import ext.Json;
import ext.Base64;
export import Core.IO.General;

template<typename T = void>
struct TriggerFailure
{
	template<typename> static constexpr auto value = false;
	static_assert(value<T>, "Json Serialization Not Support This Type");
};

export namespace Core::IO{
	struct JsonSerializateException final : std::exception{
		JsonSerializateException() = default;

		explicit JsonSerializateException(char const* _Message)
			: exception{_Message}{}

		JsonSerializateException(char const* _Message, int i)
			: exception{_Message, i}{}

		explicit JsonSerializateException(exception const& _Other)
			: exception{_Other}{}
	};

	template <typename T>
		requires !std::is_pointer_v<T>
	struct JsonSerializator;

	template <typename T>
	constexpr bool jsonDirectSerializable = requires{JsonSerializator<std::decay_t<T>>{};};

	/**
	 * @brief
	 * @tparam T TypeName
	 * @tparam isWriting true -> write | false -> read
	 */
	template <typename T, bool isWriting>
	struct JsonFieldIOCallable{
	private:
		using FieldHint = ext::reflect::Field<nullptr>;

	public:
		using ClassField = ext::reflect::ClassField<T>;
		using PassType = typename ext::ConstConditional<isWriting, T&>::type;
		using DataPassType = typename ext::ConstConditional<!isWriting, ext::json::JsonValue&>::type;

		template <std::size_t I>
		constexpr static void with(PassType val, DataPassType& src){
			if constexpr (isWriting){
				JsonFieldIOCallable::write<I>(val, src);
			}else{
				JsonFieldIOCallable::read<I>(val, src);
			}
		}

		template <std::size_t I>
		constexpr static void read(PassType& val, DataPassType& src);

		template <std::size_t I>
		constexpr static void write(PassType& val, DataPassType& src);
	};
}

export
template <typename T>
void operator <<(ext::json::JsonValue& jval, T&& val){
	using RawT = std::decay_t<T>;
	if constexpr (Core::IO::jsonDirectSerializable<RawT>){
		Core::IO::JsonSerializator<RawT>::write(jval, std::forward<T>(val));
	}else if constexpr (ext::reflect::ClassField<RawT>::defined){
		ext::reflect::ClassField<RawT>::template fieldEach<Core::IO::JsonFieldIOCallable<RawT, true>>(std::forward<T>(val), jval);
	}else{
		(void)TriggerFailure{};
	}
}

export
template <typename T>
void operator >>(const ext::json::JsonValue& jval, T& val){
	using RawT = std::decay_t<T>;

	if constexpr (Core::IO::jsonDirectSerializable<RawT>){
		Core::IO::JsonSerializator<RawT>::read(jval, val);
	}else if constexpr (ext::reflect::ClassField<RawT>::defined){
		ext::reflect::ClassField<RawT>::template fieldEach<Core::IO::JsonFieldIOCallable<RawT, false>>(val, jval);
	}else{
		(void)TriggerFailure{};
	}
}

//TODO should this thing be here???
export namespace ext::json{
	template <typename T>
	ext::json::JsonValue getJsonOf(T&& val){
		ext::json::JsonValue jval{};
		jval << std::forward<T>(val);

		return jval;
	}

	template <typename T>
	T& getValueTo(T& val, const ext::json::JsonValue& jval){
		jval >> val;
		return val;
	}
}

export namespace Core::IO{
	template <typename T, bool isWriting>
	template <std::size_t I>
	constexpr void JsonFieldIOCallable<T, isWriting>::read(PassType& val, DataPassType& src){
		using Field = typename ClassField::template FieldAt<I>;
		using FieldClassInfo = typename Field::ClassInfo;

		if(src.getTag() != ext::json::object)return; //TODO throw maybe?

		if constexpr(Field::getSrlType != ext::reflect::SrlType::disable){
			auto& member = val.*Field::fieldPtr;
			constexpr std::string_view fieldName = Field::getName;

			constexpr auto srlType = ext::conditionalVal<
				FieldClassInfo::srlType == ext::reflect::SrlType::depends,
				Field::getSrlType, FieldClassInfo::srlType>;

			const auto itr = src.asObject().find(fieldName);
			if(itr == src.asObject().end())return;
			const ext::json::JsonValue& jval = itr->second;

			if constexpr (srlType == ext::reflect::SrlType::json){
				ext::json::getValueTo(member, jval);
			}else if constexpr (srlType == ext::reflect::SrlType::binary_all){
				Core::IO::fromByte(member, ext::base64::decode<std::vector<char>>(jval.as<std::string>()));
			}else if constexpr (srlType == ext::reflect::SrlType::binary_byMember){
				//TODO binary IO support
				(void)TriggerFailure<T>{};
			}else{
				(void)TriggerFailure<T>{};
			}
		}
	}

	template <typename T, bool write>
	template <std::size_t I>
	constexpr void JsonFieldIOCallable<T, write>::write(PassType& val, DataPassType& src){
		using Field = typename ClassField::template FieldAt<I>;
		using FieldClassInfo = typename Field::ClassInfo;

		if constexpr(Field::getSrlType != ext::reflect::SrlType::disable){
			constexpr std::string_view fieldName = Field::getName;
			constexpr auto srlType = ext::conditionalVal<
				FieldClassInfo::srlType == ext::reflect::SrlType::depends,
				Field::getSrlType, FieldClassInfo::srlType>;

			(void)src.asObject(); //TODO is this really good?

			if constexpr (srlType == ext::reflect::SrlType::json){
				src.append(fieldName, ext::json::getJsonOf(std::forward<PassType>(val).*Field::fieldPtr));
			}else if constexpr (srlType == ext::reflect::SrlType::binary_all){
				ext::json::JsonValue value{};

				auto data = Core::IO::toByte(val);
				value.setData<std::string>(ext::base64::encode<std::string>(data));
				src.append(fieldName, std::move(value));
			}else if constexpr (srlType == ext::reflect::SrlType::binary_byMember){
				//TODO binary IO support
				(void)TriggerFailure<T>{};
			}else{
				(void)TriggerFailure<T>{};
			}
		}
	}
}