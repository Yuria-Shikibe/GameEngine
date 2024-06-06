export module ext.Base64;

import std;

using uint8 = unsigned char;
using uint32 = unsigned long;

export namespace ext::base64{
	constexpr uint8 alphabet_map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	constexpr uint8 reverse_map[] = {
			255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
			255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62, 255, 255, 255, 63,
			52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255, 255, 255, 255, 255,
			255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
			15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255,
			255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
			41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 255, 255, 255, 255, 255
		};

	struct bad_code final : std::exception{
		bad_code() = default;

		explicit bad_code(char const* _Message)
			: exception{_Message}{}

		bad_code(char const* _Message, const int i)
			: exception{_Message, i}{}

		explicit bad_code(exception const& _Other)
			: exception{_Other}{}
	};

	template<typename RetContainer, std::ranges::sized_range Range>
		requires requires(Range range, RetContainer cont){
			range.operator[](0);
			requires sizeof(std::ranges::range_value_t<Range>) == 1;
			cont.push_back(uint8{});
		}
	[[nodiscard]] constexpr RetContainer encode(Range&& toEncode){
		RetContainer encoded{};

		if constexpr (requires (RetContainer cont){
			cont.reserve(std::size_t{});
		}){
			encoded.reserve(std::ranges::size(toEncode) * 4);
		}

		size_t i;
		for(i = 0; i + 3 <= std::ranges::size(toEncode); i += 3){
			encoded.push_back(alphabet_map[toEncode[i] >> 2]);
			encoded.push_back(alphabet_map[toEncode[i] << 4 & 0x30 | toEncode[i + 1] >> 4]);
			encoded.push_back(alphabet_map[toEncode[i + 1] << 2 & 0x3c | toEncode[i + 2] >> 6]);
			encoded.push_back(alphabet_map[toEncode[i + 2] & 0x3f]);
		}

		if(i < std::ranges::size(toEncode)){
			if(const size_t tail = std::ranges::size(toEncode) - i; tail == 1){
				encoded.push_back(alphabet_map[toEncode[i] >> 2]);
				encoded.push_back(alphabet_map[toEncode[i] << 4 & 0x30]);
				encoded.push_back('=');
				encoded.push_back('=');
			} else{
				encoded.push_back(alphabet_map[toEncode[i] >> 2]);
				encoded.push_back(alphabet_map[toEncode[i] << 4 & 0x30 | toEncode[i + 1] >> 4]);
				encoded.push_back(alphabet_map[toEncode[i + 1] << 2 & 0x3c]);
				encoded.push_back('=');
			}
		}

		return encoded;
	}

	template<typename RetContainer, std::ranges::sized_range Range>
	requires requires(Range range, RetContainer cont){
		range.operator[](0);
		requires sizeof(std::ranges::range_value_t<Range>) == 1;
		cont.push_back(uint8{});
	}
	[[nodiscard]] constexpr RetContainer decode(Range&& toDecode){
		if(std::ranges::size(toDecode) & 0x03 != 0){
			throw bad_code{"Decode Failed"};
		}

		RetContainer plain{};
		if constexpr(requires(RetContainer cont){
			cont.reserve(std::size_t{});
		}){
			plain.reserve(std::ranges::size(toDecode) / 4);
		}

		uint8 quad[4];
		for(uint32 i = 0; i < std::ranges::size(toDecode); i += 4){
			for(uint32 k = 0; k < 4; k++){
				quad[k] = reverse_map[toDecode[i + k]];
			}

			if(!(quad[0] < 64 && quad[1] < 64)){
				throw bad_code{"Decode Failed"};
			}

			plain.push_back(quad[0] << 2 | quad[1] >> 4);

			if(quad[2] >= 64) break;
			if(quad[3] >= 64){
				plain.push_back(quad[1] << 4 | quad[2] >> 2);
				break;
			}
			plain.push_back(quad[1] << 4 | quad[2] >> 2);
			plain.push_back(quad[2] << 6 | quad[3]);
		}

		return plain;
	}
}
