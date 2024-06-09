export module ext.json.io;

export import ext.Json;
import ext.MetaProgramming;
import ext.Owner;
import ext.Base64;
import ext.StaticReflection;
import ext.Heterogeneous;

import std;

namespace ext::json{
	export
	template <typename T>
		requires !std::is_pointer_v<T>
	struct JsonSerializator{
		static void write(ext::json::JsonValue& jsonValue, const T& data) = delete;

		static void read(const ext::json::JsonValue& jsonValue, T& data) = delete;
	};

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

export namespace ext::json{
	struct JsonSerializateException final : std::exception{
		JsonSerializateException() = default;

		explicit JsonSerializateException(char const* Message)
			: exception{Message}{}

		JsonSerializateException(char const* Message, const int i)
			: exception{Message, i}{}

		explicit JsonSerializateException(exception const& Other)
			: exception{Other}{}
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
		[[nodiscard]] static ext::Owner<T*> generate(const ext::json::JsonValue& jval) noexcept(std::same_as<void, T>){
			if(jval.is<ext::json::object>()){
				auto& map = jval.asObject();
				if(const auto itr = map.find(ext::json::keys::Typename); itr != map.end() && itr->second.is<ext::json::string>()){
					return static_cast<T*>(ext::reflect::tryConstruct(itr->second.as<ext::json::string>()));
				}
			}

			return nullptr;
		}

		template <typename T>
		[[nodiscard]] static ext::Owner<T*> generate_noCheck(const ext::json::JsonValue& jval){
			return static_cast<T*>(ext::reflect::tryConstruct(jval.asObject().at(ext::json::keys::Typename).as<ext::json::string>()));
		}

		[[nodiscard]] std::type_index getTypeIndex() const noexcept{return typeid(*this);}
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
			ext::json::JsonSerializator<RawT>::write(jval, val);
		}else if constexpr (ext::reflect::ClassField<RawT>::defined){
			ext::reflect::ClassField<RawT>::template fieldEach<ext::json::JsonFieldIOCallable<RawT, true>>(val, jval);
		}else{
			(void)TriggerFailure<std::decay_t<T>>{};
		}
	}

	template <typename T>
	void readFromJson(const ext::json::JsonValue& jval, T& val){
		using RawT = std::decay_t<T>;

		if constexpr (jsonDirectSerializable<RawT>){
			ext::json::JsonSerializator<RawT>::read(jval, val);
		}else if constexpr (ext::reflect::ClassField<RawT>::defined){
			ext::reflect::ClassField<RawT>::template fieldEach<ext::json::JsonFieldIOCallable<RawT, false>>(val, jval);
		}else{
			(void)TriggerFailure{};
		}
	}

	template <typename T>
	ext::json::JsonValue getJsonOf(T&& val){
		ext::json::JsonValue jval{};
		ext::json::writeToJson(jval, std::forward<T>(val));

		return jval;
	}

	template <typename T>
	T& getValueTo(T& val, const ext::json::JsonValue& jval){
		ext::json::readFromJson(jval, val);
		return val;
	}

	template <typename T>
		requires std::is_default_constructible_v<T>
	T getValueFrom(const ext::json::JsonValue& jval){
		T t{};
		ext::json::readFromJson(jval, t);
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

export namespace ext::json{
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
				(void)TriggerFailure<T>{};
				//Core::IO::fromByte(member, ext::base64::decode<std::vector<char>>(jval.as<ext::json::string>()));
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
				(void)TriggerFailure<T>{};
				// ext::json::JsonValue value{};
				//
				// auto data = Core::IO::toByte(val);
				// value.setData<std::string>(ext::base64::encode<std::string>(data));
				// src.append(fieldName, std::move(value));
			}else if constexpr (srlType == ext::reflect::SrlType::binary_byMember){
				//TODO binary IO support
				(void)TriggerFailure<T>{};
			}else{
				(void)TriggerFailure<T>{};
			}
		}
	}


	template <>
	struct ext::json::JsonSerializator<std::string_view>{
		static void write(ext::json::JsonValue& jsonValue, const std::string_view data){
			jsonValue.setData<std::string>(static_cast<std::string>(data));
		}

		static void read(const ext::json::JsonValue& jsonValue, std::string_view& data) = delete;
	};

	template <>
	struct ext::json::JsonSerializator<std::string>{
		static void write(ext::json::JsonValue& jsonValue, const std::string& data){
			jsonValue.setData(data);
		}

		static void read(const ext::json::JsonValue& jsonValue, std::string& data){
			data = jsonValue.as<std::string>();
		}
	};

	template <std::ranges::sized_range Cont>
		requires !std::is_pointer_v<std::ranges::range_value_t<Cont>> && ext::json::jsonSerializable<
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

	template <typename V, bool overwirteOnly = false>
		requires !std::is_pointer_v<V> && std::is_default_constructible_v<V>
	struct JsonSrlContBase_string_map{
		//OPTM using array instead of object to be the KV in json?
		static void write(ext::json::JsonValue& jsonValue, const ext::StringHashMap<V>& data){
			auto& val = jsonValue.asObject();
			val.reserve(data.size());

			for(auto& [k, v] : data){
				val.insert_or_assign(k, ext::json::getJsonOf(v));
			}
		}

		static void read(const ext::json::JsonValue& jsonValue, ext::StringHashMap<V>& data){
			if(auto* ptr = jsonValue.tryGetValue<ext::json::object>()){
				data.reserve(ptr->size());

				for(const auto& [k, v] : *ptr){
					if constexpr (overwirteOnly){
						V* d = data.tryFind(k);
						if(d){
							ext::json::getValueTo(*d, v);
						}
					}else{
						data.insert_or_assign(std::string(k), ext::json::getValueFrom<V>(v));
					}

				}
			}
		}
	};
}