module;

#include <cerrno>

export module ext.Json;

import std;
import ext.Heterogeneous;
import ext.MetaProgramming;

export namespace ext::json{
	class JsonValue;

	using Integer = std::int64_t;
	using Float = std::double_t;
	using Object = StringMap<JsonValue>;
	using Array = std::vector<JsonValue>;

	template <typename T>
	using JsonScalarType = std::conditional_t<std::same_as<T, bool>, bool, std::conditional_t<
		                                          std::is_floating_point_v<T>, Float, Integer>>;

	namespace keys{
		/**
		 * @brief Indeicate this json info refer to a Polymorphic Class
		 */
		constexpr std::string_view Typename = "$ty"; //type
		constexpr std::string_view Tag = "$t";       //tag
		constexpr std::string_view ID = "$i";        //id
		constexpr std::string_view Data = "$d";      //data
		constexpr std::string_view Version = "$v";   //version
		constexpr std::string_view Pos = "$p";       //position
		constexpr std::string_view Size2D = "$s2";   //size 2D
		constexpr std::string_view Size = "$s1";     //size 1D
		constexpr std::string_view Bound = "$b";     //bound

		constexpr std::string_view Key = "$k";   //key
		constexpr std::string_view Value = "$v"; //value

		constexpr std::string_view First = "$f";   //first
		constexpr std::string_view Secound = "$s"; //second
	}


	struct IllegalJsonSegment final : std::exception{
		IllegalJsonSegment() = default;

		explicit IllegalJsonSegment(char const* Message)
			: exception{Message}{}

		IllegalJsonSegment(char const* Message, const int i)
			: exception{Message, i}{}

		explicit IllegalJsonSegment(exception const& Other)
			: exception{Other}{}
	};

	inline Integer parseInt(const std::string_view str, const int base = 10){
		int& errno_ref = errno; // Nonzero cost, pay it once
		const char* ptr = str.data();
		char* endPtr{nullptr};
		errno_ref = 0;
		const long ans = std::strtol(ptr, &endPtr, base);

		if(ptr == endPtr){
			throw IllegalJsonSegment("invalid stoi argument");
		}

		if(errno_ref == ERANGE){
			throw IllegalJsonSegment("stoi argument out of range");
		}

		return ans;
	}

	inline Float parseFloat(const std::string_view str){
		int& errno_ref = errno; // Nonzero cost, pay it once
		const char* ptr = str.data();
		char* endPtr{nullptr};
		errno_ref = 0;
		const auto ans = std::strtof(ptr, &endPtr);

		if(ptr == endPtr){
			throw IllegalJsonSegment("invalid stoi argument");
		}

		if(errno_ref == ERANGE){
			throw IllegalJsonSegment("stoi argument out of range");
		}

		return ans;
	}


	enum struct JsonValueTag : std::size_t{
		null,
		arithmetic_int,
		arithmetic_float,
		boolean,
		string,
		array,
		object
	};

	using enum JsonValueTag;

	class JsonValue{
#define TypeGroup std::nullptr_t, Integer, Float, bool, std::string, Array, Object

	public:
		template <typename T>
		static constexpr std::size_t typeIndex = uniqueTypeIndex_v<T, TypeGroup>;

		template <typename T>
		static constexpr bool validType = requires{
			requires ext::containedWith<std::decay_t<T>, TypeGroup>();
		};

	private:
		using VariantTypeTuple = std::tuple<TypeGroup>;
		std::variant<TypeGroup> data{};

	public:
		template <std::size_t index>
		using TypeAt = std::tuple_element_t<index, VariantTypeTuple>;

		static constexpr auto VariantSize = std::variant_size_v<decltype(data)>;

		JsonValue() = default;

		template <typename T>
			requires validType<T> && !std::is_arithmetic_v<T>
		explicit JsonValue(T&& val){
			this->setData(std::forward<T>(val));
		}

		template <typename T>
			requires std::is_arithmetic_v<T> || std::same_as<T, bool>
		explicit JsonValue(const T val){
			this->setData<JsonScalarType<T>>(val);
		}

		explicit JsonValue(const std::string_view val){
			this->setData(val);
		}

		void setData(const std::string_view str){
			data = static_cast<std::string>(str);
		}

		template <typename T>
			requires validType<T> && !std::is_arithmetic_v<T>
		void setData(T&& val){
			data = std::forward<T>(val);
		}

		template <typename T>
			requires validType<JsonScalarType<T>> && std::is_arithmetic_v<T>
		void setData(const T val){
			data = static_cast<JsonScalarType<T>>(val);
		}

		template <JsonValueTag tag>
		[[nodiscard]] bool is() const noexcept{
			return getTag() == tag;
		}

		template <typename T>
			requires validType<T>
		[[nodiscard]] bool is() const noexcept{
			return getTagIndex() == typeIndex<T>;
		}

		void setData(const bool val){
			data = val;
		}

		[[nodiscard]] constexpr std::size_t getTagIndex() const noexcept{
			return data.index();
		}

		JsonValue& operator[](const std::string_view key){
			if(!is<object>()){
				throw IllegalJsonSegment{"Illegal access on a non-object jval!"};
			}

			return asObject().at(key);
		}

		const JsonValue& operator[](const std::string_view key) const{
			if(!is<object>()){
				throw IllegalJsonSegment{"Illegal access on a non-object jval!"};
			}

			return asObject().at(key);
		}

		template <typename T>
			requires JsonValue::validType<T> || (std::is_arithmetic_v<T> && JsonValue::validType<JsonScalarType<T>>)
		[[nodiscard]] constexpr decltype(auto) as(){
			if constexpr(std::is_arithmetic_v<T>){
				return static_cast<T>(std::get<JsonScalarType<T>>(data));
			} else{
				return std::get<T>(data);
			}
		}

		template <typename T>
			requires JsonValue::validType<T> || (std::is_arithmetic_v<T> && JsonValue::validType<JsonScalarType<T>>)
		[[nodiscard]] constexpr decltype(auto) as() const{
			if constexpr(std::is_arithmetic_v<T>){
				return static_cast<T>(std::get<JsonScalarType<T>>(data));
			} else{
				return std::get<T>(data);
			}
		}

		template <JsonValueTag tag>
		[[nodiscard]] constexpr decltype(auto) as(){
			return std::get<static_cast<size_t>(tag)>(data);
		}

		template <JsonValueTag tag>
		[[nodiscard]] constexpr decltype(auto) as() const{
			return std::get<static_cast<size_t>(tag)>(data);
		}

		auto& asObject(){
			if(getTag() != object){
				setData(ext::StringMap<JsonValue>{});
			}

			return std::get<Object>(data);
		}

		[[nodiscard]] auto& asObject() const{
			return std::get<Object>(data);
		}

		auto& asArray(){
			if(getTag() != array){
				setData(std::vector<JsonValue>{});
			}

			return std::get<Array>(data);
		}

		[[nodiscard]] auto& asArray() const{
			return std::get<Array>(data);
		}

		// ReSharper disable CppDFAUnreachableCode
		// ReSharper disable CppDFAConstantConditions
		template <typename T>
			requires std::same_as<std::decay_t<T>, JsonValue> || validType<T>
		void append(const char* name, T&& val){
			this->append(std::string{name}, std::forward<T>(val));
		}

		template <typename T>
			requires std::same_as<std::decay_t<T>, JsonValue> || validType<T>
		void append(const std::string_view name, T&& val){
			if(getTag() != object) return; //TODO throw maybe?
			if constexpr(std::same_as<T, JsonValue>){
				asObject().insert_or_assign(name, std::forward<T>(val));
			} else{
				asObject().insert_or_assign(name, JsonValue{std::forward<T>(val)});
			}
		}

		template <typename T>
			requires std::same_as<std::decay_t<T>, JsonValue> || validType<T>
		void append(std::string&& name, T&& val){
			if(getTag() != object) return; //TODO throw maybe?
			if constexpr(std::same_as<T, JsonValue>){
				asObject().insert_or_assign(std::move(name), std::forward<T>(val));
			} else{
				asObject().insert_or_assign(std::move(name), JsonValue{std::forward<T>(val)});
			}
		}

		template <typename T>
			requires std::same_as<T, JsonValue> || validType<T>
		void push_back(T&& val){
			if(getTag() != array) return; //TODO throw maybe?
			if constexpr(std::same_as<T, JsonValue>){
				asArray().push_back(std::forward<T>(val));
			} else{
				asArray().push_back(JsonValue{std::forward<T>(val)});
			}
		}

		// ReSharper restore CppDFAConstantConditions
		// ReSharper restore CppDFAUnreachableCode


		[[nodiscard]] JsonValueTag getTag() const noexcept{ return JsonValueTag{getTagIndex()}; }

		friend bool operator==(const JsonValue& lhs, const JsonValue& rhs){
			return lhs.getTagIndex() == rhs.getTagIndex()
			       && lhs.data == rhs.data;
		}

		friend bool operator!=(const JsonValue& lhs, const JsonValue& rhs){ return !(lhs == rhs); }

		template <typename T>
			requires validType<T>
		JsonValue& operator=(T&& val){
			this->setData<T>(std::forward<T>(val));
			return *this;
		}


		template <typename T>
			requires validType<T>
		[[nodiscard]] constexpr T* tryGetValue() noexcept{
			return std::get_if<T>(&data);
		}

		template <typename T>
			requires validType<T>
		[[nodiscard]] constexpr const T* tryGetValue() const noexcept{
			return std::get_if<T>(&data);
		}

		template <JsonValueTag tag>
		[[nodiscard]] constexpr TypeAt<static_cast<size_t>(tag)>* tryGetValue() noexcept{
			return std::get_if<static_cast<size_t>(tag)>(&data);
		}

		template <JsonValueTag tag>
		[[nodiscard]] constexpr const TypeAt<static_cast<size_t>(tag)>* tryGetValue() const noexcept{
			return std::get_if<static_cast<size_t>(tag)>(&data);
		}

		//TODO better format
		void print(std::ostream& os, const bool flat = false, const bool noSpace = false, const unsigned padSize = 1,
			const unsigned depth = 1) const{
			const std::string pad(flat ? 0 : depth * padSize, ' ');
			const std::string_view endRow =
				flat
					? noSpace
						  ? ""
						  : " "
					: "\n";

			switch(getTag()){
				case arithmetic_int :{
					os << std::to_string(as<arithmetic_int>());
					break;
				}

				case arithmetic_float :{
					os << std::to_string(as<arithmetic_float>());
					break;
				}

				case boolean :{
					os << (as<boolean>() ? "true" : "false");
					break;
				}

				case string :{
					os << '"' << as<string>() << '"';
					break;
				}

				case array :{
					os << '[' << endRow;
					const auto data = as<array>();

					if(!data.empty()){
						int index = 0;
						for(; index < data.size() - 1; ++index){
							os << pad;
							data[index].print(os, flat, noSpace, padSize, depth + 1);
							os << ',' << endRow;
						}

						os << pad;
						data[index].print(os, flat, noSpace, padSize, depth + 1);
						os << endRow;
					}

					os << std::string_view(pad).substr(0, pad.size() - 4) << ']';
					break;
				}

				case object :{
					os << '{' << endRow;

					const auto data = as<object>();

					int count = 0;
					for(const auto& [k, v] : data){
						os << pad << '"' << k << '"' << (noSpace ? ":" : " : ");
						v.print(os, flat, noSpace, padSize, depth + 1);
						count++;
						if(count == data.size()) break;
						os << ',' << endRow;
					}

					os << endRow;
					os << std::string_view(pad).substr(0, pad.size() - 4) << '}';
					break;
				}

				default : os << "null";
			}
		}

		friend std::ostream& operator<<(std::ostream& os, const JsonValue& obj){
			obj.print(os);

			return os;
		}
	};
}

namespace ext::json{
	constexpr std::string_view ArraySignature = std::string_view{"$arr"};

	constexpr std::string_view trim(const std::string_view view, const std::string_view def = {"null"}){
		constexpr std::string_view Empty = " \t\n\r\f\v";
		const auto first = view.find_first_not_of(Empty);

		if(first == std::string_view::npos){
			return {};
		}

		const auto last = view.find_last_not_of(Empty);
		return view.substr(first, last - first + 1);
	}

	struct Scope{
		// std::vector<std::pair<std::size_t, std::size_t>> jumpPoints{};
		std::string_view code{};

		std::vector<std::pair<std::string_view, Scope>> args{};

		[[nodiscard]] bool isLeaf() const{
			return args.empty();
		}

		[[nodiscard]] bool isArray_OnGenerate() const{
			return code == ArraySignature;
		}

		[[nodiscard]] bool isArray() const{
			return !code.empty() && code.front() == '[';
		}

		[[nodiscard]] bool isObject() const{
			return !code.empty() && code.front() == '{';
		}
	};

	static JsonValue parseBasicValue(const std::string_view view){

		if(view.empty()){
			throw IllegalJsonSegment{"Json with empty value string"};
		}

		const auto frontChar = view.front();

		switch(frontChar){
			case '"' :{
				if(view.back() != '"' || view.size() == 1)
					throw IllegalJsonSegment{
						"Losing '\"' At Json String Back"
					};

				return JsonValue{std::string(view.substr(1, view.size() - 2))};
			}

			case 't' : return JsonValue(true);
			case 'f' : return JsonValue(false);
			case 'n' : return JsonValue(nullptr);
			default : break;
		}

		if(view.find_first_of(".fFeE") != std::string_view::npos){
			return JsonValue(parseFloat(view));
		}

		if(frontChar == '0'){
			if(view.size() >= 2){
				switch(view[1]){
					case 'x' : return JsonValue(parseInt(view.substr(2), 16));
					case 'b' : return JsonValue(parseInt(view.substr(2), 2));
					default : return JsonValue(parseInt(view.substr(1), 8));
				}
			} else{
				return JsonValue(static_cast<Integer>(0));
			}
		} else{
			return JsonValue(parseInt(view));
		}
	}

	JsonValue splitTextToScope(std::string_view text){
		static constexpr auto Invalid = std::string_view::npos;

		bool escapeTheNext{false};
		bool parsingString{false};

		struct LayerData{
			std::size_t layerBegin{};
			JsonValue value{};

			std::size_t layerLastSplit{};
			JsonValue* last{};

			void processData(const std::size_t index, const std::string_view text){
				if(layerLastSplit != Invalid){
					const auto lastCode = trim(text.substr(layerLastSplit + 1, index - layerLastSplit - 1));

					if(lastCode.empty())return;

					if(value.is<object>()){
						last->operator=(parseBasicValue(lastCode));
					}

					if(value.is<array>()){
						value.asArray().push_back(parseBasicValue(lastCode));
					}
				}

				layerLastSplit = index;
			}
		};

		std::stack<LayerData> layers{};

		for(const auto [index, character] : text | std::ranges::views::enumerate){
			if(escapeTheNext){
				escapeTheNext = false;
				continue;
			}

			if(character == '\\'){
				escapeTheNext = true;
				continue;
			}

			if(character == '"'){
				if(!parsingString){
					parsingString = true;
				} else{
					parsingString = false;
				}
			}

			if(parsingString) continue;

			switch(character){
				case ':' :{
					auto& [begin, layer, split, last] = layers.top();
					auto lastKey = trim(text.substr(split + 1, index - split - 1));
					last = &layer.asObject().insert_or_assign(lastKey.substr(1, lastKey.size() - 2), JsonValue{nullptr}).first->second;

					split = index;

					break;
				}

				case '[' :{
					layers.emplace(index, JsonValue{Array{}}, index);
					break;
				}

				case '{' :{
					layers.emplace(index, JsonValue{Object{}}, index);
					break;
				}

				case ']' :
				case '}' :{
					auto lastLayer = std::move(layers.top());
					layers.pop();

					lastLayer.processData(index, text);

					if(layers.empty()) return std::move(lastLayer.value);

					auto& [pos, ly, split, last] = layers.top();

					if(ly.is<object>()){
						last->operator=(std::move(lastLayer.value));
					}

					if(ly.is<array>()){
						ly.asArray().push_back(std::move(lastLayer.value));
					}

					split = Invalid;

					break;
				}

				case ',' :{
					layers.top().processData(index, text);
				}
				default : break;
			}
		}

		std::unreachable();
	}
}

export namespace ext::json{
	struct Parser{
		static JsonValue parse(const std::string_view view){
			return splitTextToScope(view);
		}
	};
}

export
template <>
struct ::std::formatter<ext::json::JsonValue>{
	constexpr auto parse(std::format_parse_context& context) const{
		return context.begin();
	}

	bool flat{false};
	bool noSpace{false};
	int pad{2};

	constexpr auto parse(std::format_parse_context& context){
		auto it = context.begin();
		if(it == context.end()) return it;

		if(*it == 'n'){
			noSpace = true;
			++it;
		}

		if(*it == 'f'){
			flat = true;
			++it;
		}

		if(*it >= '0' && *it <= '9'){
			pad = *it - '0';
			++it;
		}

		if(it != context.end() && *it != '}') throw std::format_error("Invalid format");

		return it;
	}

	auto format(const ext::json::JsonValue& json, auto& context) const{
		std::ostringstream ss{};
		json.print(ss, flat, noSpace);

		return std::format_to(context.out(), "{}", ss.str());
	}
};
