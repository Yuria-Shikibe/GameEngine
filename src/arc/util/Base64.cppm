module;

export module Base64;

import <cstring>;
import <cassert>;
import <string_view>;
import <vector>;

export namespace ext{
	typedef unsigned char  uint8;
	typedef unsigned long uint32;

	constexpr uint8 alphabet_map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	constexpr uint8 reverse_map[] ={
	     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62, 255, 255, 255, 63,
	     52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255, 255, 255, 255, 255,
	     255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255,
	     255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 255, 255, 255, 255, 255
	};

	void base64_encode(const std::string_view text, std::vector<uint8>& encode){
	    size_t i;
	    for (i = 0; i + 3 <= text.size(); i += 3){
	    	encode.push_back(alphabet_map[text[i]>>2]);
	    	encode.push_back(alphabet_map[text[i]<<4&0x30|text[i+1]>>4]);
	    	encode.push_back(alphabet_map[text[i+1]<<2&0x3c|text[i+2]>>6]);
	    	encode.push_back(alphabet_map[text[i+2]&0x3f]);
	    }

	    if (i < text.size()){
		    if(const size_t tail = text.size() - i; tail == 1){
		    	encode.push_back(alphabet_map[text[i]>>2]);
		    	encode.push_back(alphabet_map[text[i]<<4&0x30]);
		    	encode.push_back('=');
		    	encode.push_back('=');
	        }else{//tail==2
	        	encode.push_back(alphabet_map[text[i]>>2]);
	        	encode.push_back(alphabet_map[text[i]<<4&0x30|text[i+1]>>4]);
	        	encode.push_back(alphabet_map[text[i+1]<<2&0x3c]);
	        	encode.push_back('=');
	        }
	    }
	}

	std::vector<uint8> encode(const std::string_view toEncode) {
		std::vector<uint8> encoded{};
		encoded.reserve(toEncode.size() * 4);
		base64_encode(toEncode, encoded);
		return encoded;
	}


	uint32 base64_decode(const uint8 *code, const uint32 code_len, uint8 *plain)
	{
		if(code_len&0x03 != 0) {
			throw std::exception{"Cannot Decode!"};
		}

		uint32 j = 0;
		uint8 quad[4];
		for (uint32 i = 0; i < code_len; i+=4)
		{
			for (uint32 k = 0; k < 4; k++)
			{
				quad[k] = reverse_map[code[i+k]];
			}

			assert(quad[0]<64 && quad[1]<64);

			plain[j++] = (quad[0]<<2)|(quad[1]>>4);

			if (quad[2] >= 64)break;
			if (quad[3] >= 64){
				plain[j++] = (quad[1]<<4)|(quad[2]>>2);
				break;
			}
			plain[j++] = (quad[1]<<4)|(quad[2]>>2);
			plain[j++] = (quad[2]<<6)|quad[3];
		}
		return j;
	}

	 // int main(void)
	 // {
	 //     char input[256];
	 //     while (true){
	 //     printf("Please input what you want to decode: ");
	 //     scanf("%s", input);
	 //     uint8 *text = (uint8 *)input;
	 //     uint32 text_len = (uint32)strlen((char *)text);
	 //     uint8 buffer[1024],buffer2[4096];
	 //
	 //     uint32 size = base64_decode(text, text_len, buffer);
	 //     buffer[size] = 0;
	 //     printf("Decoded content: %s\n", buffer);
	 //     size = base64_encode(buffer, size, buffer2);
	 //     buffer2[size] = 0;
	 //     printf("Confirmation of the original content: %s\n", buffer2);
	 //     }
	 //     return 0;
	 // }

}
