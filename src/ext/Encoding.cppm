module;

#if defined(_WIN32) || defined(_WIN64)
#define WIN_SYS
#include <Windows.h>
#else

#endif

export module ext.Encoding;

import std;

export namespace ext{
	template <typename ToType>
	using MultiByteBuffer = std::array<ToType, 4 / sizeof(ToType)>;

	using CharBuffer = std::array<char, 4>;

	unsigned int convertTo(const char* charCodes, const int size = 4){
		unsigned int buffer{};
#ifdef WIN_SYS
		MultiByteToWideChar(CP_UTF8, 0, charCodes, size, reinterpret_cast<wchar_t*>(&buffer), sizeof(unsigned int) / sizeof(wchar_t));
#endif
		return buffer;
	}

	template <typename ToType, typename FromType>
	MultiByteBuffer<ToType> convertTo(FromType charCode){
		MultiByteBuffer<ToType> buffer{};
#ifdef WIN_SYS
		int failed{};

		WideCharToMultiByte(
			CP_UTF8, 0,
			reinterpret_cast<wchar_t*>(&charCode), 1,
			reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(ToType),
			nullptr, &failed);

		if(failed){
			//TODO warning or exception here
		}
#endif
		return buffer;
	}

	template <typename ToType, typename FromType>
	void convertTo(FromType charCode, MultiByteBuffer<ToType> buffer){
#ifdef WIN_SYS
		int failed{};

		WideCharToMultiByte(
			CP_UTF8, 0,
			reinterpret_cast<wchar_t*>(&charCode), 1,
			reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(ToType),
			nullptr, &failed);

		if(failed){
			//TODO warning or exception here
		}
#endif
	}

	template <typename FromType, typename ToType>
	std::basic_string<ToType> convertTo(const std::basic_string_view<FromType> src){
		std::basic_string<ToType> str{};

		str.reserve(str.size() * (sizeof(FromType) / sizeof(ToType)));

		for(const auto value : src){
			str.append_range(convertTo<char32_t, char>(value));
		}

		return str;
	}

	/**
	 * @tparam To Dest Char Type
	 * @param from Src String [ASCII Only]
	 */
	template <typename To>
	std::basic_string<To> convertTo(const std::string_view from){
		std::basic_string<To> to{};
		to.reserve(from.size());
		std::ranges::copy(from, std::back_inserter(to));
		return to;
	}

	/**
	 * @tparam To Dest Char Type
	 * @param from Src String [ASCII Only]
	 */
	std::wstring convertToWide(const std::string_view from){
		std::wstring to{};
		to.reserve(from.size());
		std::ranges::copy(from, std::back_inserter(to));
		return to;
	}

	constexpr bool isUnicodeHead(const char code){
		constexpr char Mask = 0b0000'0001;

		// 0b'10...
		if((code >> 7 & Mask) == 1 && (code >> 6 & Mask) == 0){
			return false;
		}

		return true;
	}

	/**
	 * @brief Warning: this function assume that inItr is always derreferenceable
	 * @param inItr Search Pos
	 */
	[[nodiscard]] std::string::iterator gotoUnicodeHead(std::string::iterator inItr){
		while(!isUnicodeHead(inItr.operator*())){
			--inItr;
		}

		return inItr;
	}

	[[nodiscard]] std::string_view::iterator gotoUnicodeHead(std::string_view::iterator inItr){
		while(!isUnicodeHead(inItr.operator*())){
			--inItr;
		}

		return inItr;
	}

	/**
	 * @brief
	 * @param code Unicode Head
	 * @return 0 if this code is not a unicode head
	 */
	template <std::integral Ret = unsigned>
	constexpr Ret getUnicodeLength(const char code){
		//TODO check if this code is really a unicode character
		constexpr char Mask = 0b0000'0001;

		if(code >> 7 & Mask){ //0b'1...
			if(code >> 6 & Mask){ //0b'11...
				if(code >> 5 & Mask){ //0b'111...
					if(code >> 4 & Mask){ //0b'1111...
						if(code >> 3 & Mask){
							return 0; //0b'11111... ,Undefined
						}
						//0b'11110... ,4
						return 4;
					}
					//0b'1110... ,3
					return 3;
				}
				//0b'110... ,2
				return 2;
			}
			//0b'10... ,Not Head
			return 0;
		}

		//0b'0... ,ASCII
		return 1;
	}
}

export std::ostream& operator<<(std::ostream& stream, const ext::CharBuffer buffer){
	// stream.write(buffer.data(), 2);

	stream << std::string{buffer.data(), ext::getUnicodeLength(buffer.front())};
	return stream;
}

export
template <>
struct std::formatter<ext::CharBuffer> {
	static constexpr auto parse(auto& ctx){return ctx.begin();}

	template<class FmtContext>
	static typename FmtContext::iterator format(const ext::CharBuffer buffer, FmtContext& ctx){
		std::ostringstream out{};
		out << std::string_view{buffer.data(), ext::getUnicodeLength(buffer.front())};

		return std::ranges::copy(std::move(out).str(), ctx.out()).out;
	}
};

