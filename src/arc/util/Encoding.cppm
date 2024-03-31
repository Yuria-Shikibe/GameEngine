module;

#if defined(_WIN32) || defined(_WIN64)
#define WIN_SYS
#include <Windows.h>
#else

#endif

export module Encoding;

import std;

export namespace ext{
	template <typename ToType>
	using MultiByteBuffer = std::array<ToType, 4 / sizeof(ToType)>;

	using CharBuffer = std::array<char, 4>;

	unsigned int convertTo(const CharBuffer charCodes){
		unsigned int buffer{};
#ifdef WIN_SYS
		MultiByteToWideChar(CP_UTF8, 0, charCodes.data(), 4, reinterpret_cast<wchar_t*>(buffer), 1);
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

	constexpr int getCharCodeLength(const char code){
		//TODO check if this code is really a unicode character
		constexpr char Mask = 0b0000'0001;
		if(code >> 7 & Mask){
			if(code >> 6 & Mask){
				if(code >> 5 & Mask){
					if(code >> 4 & Mask){
						return 4;
					}
					return 3;
				}
				return 2;
			}
			return 1;
		}
		return 1;
	}
}

export std::ostream& operator<<(std::ostream& stream, const ext::CharBuffer buffer){
	// stream.write(buffer.data(), 2);

	stream << std::string{buffer.data()};
	return stream;
}
