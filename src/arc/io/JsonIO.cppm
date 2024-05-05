export module Core.IO.JsonIO;

export import ext.Json;
export import Core.IO.General;
import MetaProgramming;
import ext.Owner;
import ext.Base64;
import ext.StaticReflection;

import std;

namespace Core::IO{
	export
	template <typename T>
		requires !std::is_pointer_v<T>
	struct JsonSerializator;

	export
	template <typename T>
	inline constexpr bool jsonDirectSerializable = requires{
		requires ext::isTypeComplteted<JsonSerializator<std::decay_t<T>>>;
	};
}

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

	struct DynamicJsonSerializable{
		virtual ~DynamicJsonSerializable() = default;
		virtual void writeTo(ext::json::JsonValue& jval) const = 0;
		virtual void readFrom(const ext::json::JsonValue& jval) = 0;

		void writeType(ext::json::JsonValue& jval) const {
			auto& map = jval.asObject();
			const std::string_view name = ext::reflect::classNames_RTTI().at(getTypeIndex());
			map.insert_or_assign(ext::json::keys::Typename, ext::json::JsonValue{name});
		}

		template <typename T>
		static ext::Owner<T*> generate(const ext::json::JsonValue& jval) noexcept(std::same_as<void, T>){
			if(jval.is<ext::json::object>()){
				auto& map = jval.asObject();
				if(const auto itr = map.find(ext::json::keys::Typename); itr != map.end() && itr->second.is<ext::json::string>()){
					return static_cast<T*>(ext::reflect::tryConstruct(itr->second.as<ext::json::string>()));
				}
			}

			return nullptr;
		}

		template <typename T>
		static ext::Owner<T*> generate_noCheck(const ext::json::JsonValue& jval){
			return static_cast<T*>(ext::reflect::tryConstruct(jval.asObject().at(ext::json::keys::Typename).as<ext::json::string>()));
		}

		std::type_index getTypeIndex() const noexcept{return typeid(*this);}
	};

	template <typename T = void>
	ext::Owner<T*> getObjectFrom(const ext::json::JsonValue& jval){
		if(jval.is<ext::json::object>()){
			const auto& map = jval.asObject();
			if(const auto itr = map.find(ext::json::keys::Typename); itr != map.end()){
				return ext::reflect::tryConstruct<T>(itr->second.as<ext::json::string>());
			}
		}
 		return nullptr;
	}


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
		constexpr static void with(PassType val, DataPassType src){
			if constexpr (isWriting){
				JsonFieldIOCallable::write<I>(val, src);
			}else{
				JsonFieldIOCallable::read<I>(val, src);
			}
		}

		template <std::size_t I>
		constexpr static void read(PassType val, DataPassType src);

		template <std::size_t I>
		constexpr static void write(PassType val, DataPassType src);
	};

	template <typename T>
	void writeToJson(ext::json::JsonValue& jval, const T& val){
		using RawT = std::decay_t<T>;
		if constexpr (jsonDirectSerializable<RawT>){
			Core::IO::JsonSerializator<RawT>::write(jval, val);
		}else if constexpr (ext::reflect::ClassField<RawT>::defined){
			ext::reflect::ClassField<RawT>::template fieldEach<Core::IO::JsonFieldIOCallable<RawT, true>>(val, jval);
		}else{
			(void)TriggerFailure<std::decay_t<T>>{};
		}
	}

	template <typename T>
	void readFromJson(const ext::json::JsonValue& jval, T& val){
		using RawT = std::decay_t<T>;

		if constexpr (jsonDirectSerializable<RawT>){
			Core::IO::JsonSerializator<RawT>::read(jval, val);
		}else if constexpr (ext::reflect::ClassField<RawT>::defined){
			ext::reflect::ClassField<RawT>::template fieldEach<Core::IO::JsonFieldIOCallable<RawT, false>>(val, jval);
		}else{
			(void)TriggerFailure{};
		}
	}
}

//TODO should this thing be here???
export namespace ext::json{
	template <typename T>
	ext::json::JsonValue getJsonOf(T&& val){
		ext::json::JsonValue jval{};
		Core::IO::writeToJson(jval, std::forward<T>(val));

		return jval;
	}

	template <typename T>
	T& getValueTo(T& val, const ext::json::JsonValue& jval){
		Core::IO::readFromJson(jval, val);
		return val;
	}

	template <typename T>
		requires std::is_default_constructible_v<T>
	T getValueFrom(const ext::json::JsonValue& jval){
		T t{};
		Core::IO::readFromJson(jval, t);
		return t;
	}

	template <typename T>
	void append(ext::json::JsonValue& jval, const std::string_view key, const T& val){
		jval.asObject().insert_or_assign(key, ext::json::getJsonOf(val));
	}

	template <typename T>
	void push_back(ext::json::JsonValue& jval, const T& val){
		jval.asArray().push_back(ext::json::getJsonOf(val));
	}

	template <typename T>
	void read(const ext::json::JsonValue& jval, const std::string_view key, T& val){
		ext::json::getValueTo(val, jval.asObject().at(key));
	}

	template <typename T>
		requires std::is_copy_assignable_v<T>
	void read(const ext::json::JsonValue& jval, const std::string_view key, T& val, const T& defValue){
		auto& map = jval.asObject();
		if(const auto itr = map.find(key); itr != map.end()){
			ext::json::getValueTo(val, itr->second);
		}else{
			val = defValue;
		}
	}
}

export namespace Core::IO{
	template <typename T>
	constexpr bool jsonSerializable = requires(T& val){
		ext::json::getJsonOf(val);
		ext::json::getValueTo(val, ext::json::JsonValue{});
	};

	template <typename T, bool isWriting>
	template <std::size_t I>
	constexpr void JsonFieldIOCallable<T, isWriting>::read(PassType val, DataPassType src){
		using Field = typename ClassField::template FieldAt<I>;
		using FieldClassInfo = typename Field::ClassInfo;

		if(src.getTag() != ext::json::object)return; //TODO throw maybe?

		if constexpr(Field::getSrlType != ext::reflect::SrlType::disable){
			auto& member = val.*Field::mptr;
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
				Core::IO::fromByte(member, ext::base64::decode<std::vector<char>>(jval.as<ext::json::string>()));
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
	constexpr void JsonFieldIOCallable<T, write>::write(PassType val, DataPassType src){
		using Field = typename ClassField::template FieldAt<I>;
		using FieldClassInfo = typename Field::ClassInfo;

		if constexpr(Field::getSrlType != ext::reflect::SrlType::disable){
			constexpr std::string_view fieldName = Field::getName;
			constexpr auto srlType = ext::conditionalVal<
				FieldClassInfo::srlType == ext::reflect::SrlType::depends,
				Field::getSrlType, FieldClassInfo::srlType>;

			(void)src.asObject(); //TODO is this really good?

			if constexpr (srlType == ext::reflect::SrlType::json){
				src.append(fieldName, ext::json::getJsonOf(std::forward<PassType>(val).*Field::mptr));
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