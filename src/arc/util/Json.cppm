module;

export module ext.Json;

import std;
import ext.Heterogeneous;
import ext.Encoding;
import ext.StringParse;
import MetaProgramming;

using namespace ext::string;

namespace ext::json{
	bool hasMeaning(const char c){
		return !std::isspace(c);
	}
}

export namespace ext::json{
	using JsonInteger = std::int64_t;
	using JsonFloat = std::double_t;

	template <typename T>
	using JsonScalarType = std::conditional_t<std::same_as<T, bool>, bool, std::conditional_t<std::is_floating_point_v<T>, JsonFloat, JsonInteger>>;

	namespace keys{
		/**
		 * @brief Indeicate this json info refer to a Polymorphic Class
		 */
		constexpr std::string_view Typename = "$ty"; //type
		constexpr std::string_view Tag = "$t"; //tag
		constexpr std::string_view ID = "$i"; //id
		constexpr std::string_view Data = "$d"; //data
		constexpr std::string_view Version = "$v"; //version
		constexpr std::string_view Pos = "$p"; //position
		constexpr std::string_view Size2D = "$s2"; //size 2D
		constexpr std::string_view Size = "$s1"; //size 1D
		constexpr std::string_view Bound = "$b"; //bound

		constexpr std::string_view Key = "$k"; //key
		constexpr std::string_view Value = "$v"; //value

		constexpr std::string_view First = "$f"; //first
		constexpr std::string_view Secound = "$s"; //second
	}

	struct IllegalJsonSegment final : std::exception{
		IllegalJsonSegment() = default;

		explicit IllegalJsonSegment(char const* _Message)
			: exception{_Message}{}

		IllegalJsonSegment(char const* _Message, int i)
			: exception{_Message, i}{}

		explicit IllegalJsonSegment(exception const& _Other)
			: exception{_Other}{}
	};

	inline JsonInteger parseInt(const std::string_view str, const int base = 10){
		// int& errno_ref = errno; // Nonzero cost, pay it once
		const char* ptr = str.data();
		char* endPtr{nullptr};
		// errno_ref = 0;
		const long _Ans = std::strtol(ptr, &endPtr, base);

		if(ptr == endPtr){
			throw IllegalJsonSegment("invalid stoi argument");
		}

		// if(errno_ref == ERANGE){
		// 	throw IllegalJsonSegment("stoi argument out of range");
		// }

		return _Ans;
	}

	inline JsonFloat parseFloat(const std::string_view str){
		// int& errno_ref = errno; // Nonzero cost, pay it once
		const char* ptr = str.data();
		char* endPtr{nullptr};
		// errno_ref = 0;
		const auto ans = std::strtof(ptr, &endPtr);

		if(ptr == endPtr){
			throw IllegalJsonSegment("invalid stoi argument");
		}

		// if(errno_ref == ERANGE){
		// 	throw IllegalJsonSegment("stoi argument out of range");
		// }

		return ans;
	}


	enum struct JsonValueTag : std::size_t{
		arithmetic_int,
		arithmetic_float,
		boolean,
		string,
		array,
		object
	};

	using enum JsonValueTag;

	class JsonValue{
#define TypeGroup JsonInteger, JsonFloat, bool, std::string, Array, Object

	public:
		using Object = StringMap<JsonValue>;
		using Array = std::vector<JsonValue>;

		template <typename T>
		static constexpr std::size_t typeIndex = uniqueTypeIndex_v<
			T, TypeGroup>;

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
			data = val;
		}

		template <JsonValueTag tag>
		bool is() const noexcept{
			return getTag() == tag;
		}

		template <typename T>
			requires validType<T>
		bool is() const noexcept{
			return getTagIndex() == typeIndex<T>;
		}

		void setData(const bool val){
			data = val;
		}

		[[nodiscard]] constexpr std::size_t getTagIndex() const noexcept{
			return data.index();
		}

		template <JsonValueTag tag>
			requires requires{ requires static_cast<size_t>(tag) < std::variant_size_v<decltype(data)>; }
		[[nodiscard]] constexpr decltype(auto) as(){
			return std::get<static_cast<size_t>(tag)>(data);
		}

		template <JsonValueTag tag>
			requires requires{ requires static_cast<size_t>(tag) < std::variant_size_v<decltype(data)>; }
		[[nodiscard]] constexpr decltype(auto) as() const{
			return std::get<static_cast<size_t>(tag)>(data);
		}

		auto& asObject(){
			if(getTag() != object){
				setData(ext::StringMap<JsonValue>{});
			}

			return std::get<Object>(data);
		}

		auto& asObject() const {
			return std::get<Object>(data);
		}

		auto& asArray(){
			if(getTag() != array){
				setData(std::vector<JsonValue>{});
			}

			return std::get<Array>(data);
		}

		auto& asArray() const {
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
			if(getTag() != object)return; //TODO throw maybe?
			if constexpr (std::same_as<T, JsonValue>){
				asObject().insert_or_assign(name, std::forward<T>(val));
			}else{
				asObject().insert_or_assign(name, JsonValue{std::forward<T>(val)});
			}
		}

		template <typename T>
			requires std::same_as<std::decay_t<T>, JsonValue> || validType<T>
		void append(std::string&& name, T&& val){
			if(getTag() != object)return; //TODO throw maybe?
			if constexpr (std::same_as<T, JsonValue>){
				asObject().insert_or_assign(std::move(name), std::forward<T>(val));
			}else{
				asObject().insert_or_assign(std::move(name), JsonValue{std::forward<T>(val)});
			}
		}

		template <typename T>
			requires std::same_as<T, JsonValue> || validType<T>
		void push_back(T&& val){
			if(getTag() != array)return; //TODO throw maybe?
			if constexpr (std::same_as<T, JsonValue>){
				asArray().push_back(std::forward<T>(val));
			}else{
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

		template <JsonValueTag t>
			requires requires{ requires static_cast<size_t>(t) < std::variant_size_v<decltype(data)>; }
		[[nodiscard]] constexpr TypeAt<static_cast<size_t>(t)>* tryGetValue() noexcept{
			return std::get_if<static_cast<size_t>(t)>(&data);
		}

		template <JsonValueTag t>
			requires requires{ requires static_cast<size_t>(t) < std::variant_size_v<decltype(data)>; }
		[[nodiscard]] constexpr const TypeAt<static_cast<size_t>(t)>* tryGetValue() const noexcept{
			return std::get_if<static_cast<size_t>(t)>(&data);
		}

		/**
		 * @brief
		 * @param view
		 * @return KeyName
		 */
		std::string parseKV(const std::string_view view){
			bool escapeTheNext{false};
			bool parsingString{false};

			auto colonPos = FailedPos;
			auto nameBegin = FailedPos;
			auto nameEnd = FailedPos;

			auto dataBegin = FailedPos;
			auto dataEnd = FailedPos;

			for(int pos = 0; pos < view.size(); ++pos){
				if(escapeTheNext){
					escapeTheNext = false;
					continue;
				}

				const char character = view[pos];

				if(character == '\\'){
					escapeTheNext = true;
					continue;
				}

				if(character == '"'){
					if(!parsingString){
						parsingString = true;
						if(nameBegin == FailedPos) nameBegin = pos + 1;
					} else{
						parsingString = false;
						if(nameEnd == FailedPos) nameEnd = pos;
					}
				}

				if(colonPos == FailedPos){
					if(!parsingString && character == ':'){
						colonPos = pos;
						continue;
					}
				} else{
					if(hasMeaning(character)){
						dataBegin = pos;
						break;
					}
				}
			}

			if(dataBegin != FailedPos){
				for(int pos = view.size() - 1; pos >= 0; --pos){
					if(hasMeaning(view[pos])){
						dataEnd = pos + 1;
						break;
					}
				}
			}


			if(colonPos == FailedPos || !(nameBegin < nameEnd && nameEnd < colonPos && colonPos < dataBegin && dataBegin
				< dataEnd)){
				throw IllegalJsonSegment{"Illegal Json Seg"};
			}

			const auto arg = view.substr(dataBegin, dataEnd - dataBegin);

			setValueBy(arg);

			return static_cast<std::string>(view.substr(nameBegin, nameEnd - nameBegin));
		}

		/**
		 * @brief
		 * @param view {<Begin -- View -- End>}
		 */
		void parseObject(const std::string_view view){
			const auto args = splitByComma(view.substr(1, view.size() - 2));

			ext::StringMap<JsonValue> values(args.size());

			for(const auto arg : args){
				JsonValue value{};
				auto key = value.parseKV(arg);

				values.insert_or_assign(std::move(key), std::move(value));
			}

			data.emplace<decltype(values)>(std::move(values));
		}

		/**
		 * @brief
		 * @param view [<Begin -- View -- End>]
		 */
		void parseArray(const std::string_view view){
			const auto args = splitByComma(view.substr(1, view.size() - 2));

			std::vector<JsonValue> values(args.size());

			for(auto [index, jsonValue] : values | std::ranges::views::enumerate){
				jsonValue.setValueBy(args.at(index));
			}

			data.emplace<decltype(values)>(std::move(values));
		}

		void setValueBy(const std::string_view valueView){
			if(valueView.empty()){
				throw IllegalJsonSegment{"Json with empty value string"};
			}
			if(valueView.front() == '{'){
				if(valueView.back() != '}'){
					throw IllegalJsonSegment{"Losing '}' At Json Object Back"};
				}

				parseObject(valueView);
				return;
			}

			if(valueView.front() == '['){
				if(valueView.back() != ']'){
					throw IllegalJsonSegment{"Losing ']' At Json Array Back"};
				}

				parseArray(valueView);
				return;
			}

			if(valueView.front() == '"'){
				if(valueView.back() != '"' || valueView.size() == 1){
					throw IllegalJsonSegment{"Losing '""' At Json String Back"};
				}

				data.emplace<std::string>(static_cast<std::string>(valueView.substr(1, valueView.size() - 2)));
				return;
			}

			if(valueView.front() == 't'){
				data.emplace<bool>(true);
				return;
			}

			if(valueView.front() == 'f'){
				data.emplace<bool>(false);
				return;
			}

			if(
				std::ranges::any_of(valueView, [](const char c){
					return
						c == '.' ||
						c == 'f' ||
						c == 'F' ||
						c == 'e' ||
						c == 'E';
				})){
				data.emplace<JsonFloat>(parseFloat(valueView));

				return;
			}

			if(valueView.front() == '0'){
				if(valueView.size() >= 2){
					switch(valueView[1]){
						case 'x' : data.emplace<JsonInteger>(parseInt(valueView.substr(2), 16));
							break;
						case 'b' : data.emplace<JsonInteger>(parseInt(valueView.substr(2), 2));
							break;
						default : data.emplace<JsonInteger>(parseInt(valueView.substr(1), 8));
							break;
					}
				} else{
					data.emplace<JsonInteger>(0);
				}
			} else{
				data.emplace<JsonInteger>(parseInt(valueView));
			}
		}

		void print(std::ostream& os, const bool flat = false, const bool noSpace = false, const unsigned padSize = 1, const unsigned depth = 1) const{
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

				default : os << "Undefined";
			}
		}

		friend std::ostream& operator<<(std::ostream& os, const JsonValue& obj){
			obj.print(os);

			return os;
		}
	};


	class Json{
		JsonValue root{};
		std::string strData{};

		void updateJsonData(){
			flattenString();
			root.parseObject(strData);
		}

	public:
		Json() = default;

		explicit Json(const JsonValue& root)
			: root{root}{}

		explicit Json(const std::string& strData)
			: strData{strData}{
			updateJsonData();
		}

		explicit Json(std::string&& strData)
			: strData{std::move(strData)}{
			updateJsonData();
		}

		/**
		 * @brief This is not always valid!
		 * @return
		 */
		[[nodiscard]] constexpr const std::string& getStrData() const{ return strData; }

		[[nodiscard]] constexpr const JsonValue& getData() const noexcept{ return root; }

		[[nodiscard]] constexpr JsonValue& getData() noexcept{ return root; }

		void parse(const std::string& str){
			strData = str;
			updateJsonData();
		}

		void parse(std::string&& str){
			strData = std::move(str);
			updateJsonData();
		}

		constexpr void flattenString(){
			strData = strData | std::ranges::views::filter([](const char c){ return c != '\n'; }) | std::ranges::to<
				std::string>();
		}

		void updateString(){
			std::ostringstream ss{};

			root.print(ss);

			strData = std::move(ss).str();
		}
	};
}


export
template <>
struct ::std::formatter<ext::json::JsonValue>{
	constexpr auto parse(std::format_parse_context& context) const{
		return context.begin();
	}

	bool flat{ false };
	bool noSpace{ false };
	int pad{2};

	constexpr auto parse(std::format_parse_context& context)
	{
		auto it = context.begin();
		if (it == context.end())
			return it;

		if (*it == 'n'){
			noSpace = true;
			++it;
		}

		if (*it == 'f'){
			flat = true;
			++it;
		}

		if (*it >= '0' && *it <= '9'){
			pad = *it - '0';
			++it;
		}

		if (it != context.end() && *it != '}')
			throw std::format_error("Invalid format");

		return it;
	}

	auto format(const ext::json::JsonValue& json, auto& context) const{
		std::ostringstream ss{};
		json.print(ss, flat, noSpace);

		return std::format_to(context.out(), "{}", ss.str());
	}
};
