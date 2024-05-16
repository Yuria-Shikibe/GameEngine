//
// Created by Matrix on 2024/4/6.
//

export module Font.UnicodeRefParser;

import std;
import ext.Encoding;
import OS.File;

export namespace Font{
	std::vector<unsigned> genRefTable(const OS::File& reference){
		const std::string src = reference.quickRead();
		std::vector<unsigned> dest{};
		dest.reserve(src.size() * 4);

		const size_t totalSize = src.size();
		for(size_t index = 0; index < totalSize; ++index){
			const char cur = src.at(index);
			if(cur == '\n' || cur == '\r' || cur == ' ')continue;
			unsigned int charCode = static_cast<unsigned char>(cur);
			const int charCodeLength = ext::getUnicodeLength<int>(cur);

			if(charCodeLength > 1 && index + charCodeLength <= totalSize){
				charCode = ext::convertTo(src.data() + index, charCodeLength);
			}

			dest.push_back(charCode);

			index += charCodeLength - 1;
		}

		return dest;
	}
}
