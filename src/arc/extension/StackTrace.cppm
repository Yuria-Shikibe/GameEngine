//
// Created by Matrix on 2023/11/20.
//
module ;

export module StackTrace;

import std;

export namespace ext{
	void getStackTraceBrief(std::stringstream&ss, const bool jumpUnSource = true, const bool showExec = false, const int skipNative = 3){
		ss << "\n--------------------- Stack Trace Begin:\n\n";

		const auto currentStacktrace = std::stacktrace::current();

		int index = 0;
		for (const auto& entry : currentStacktrace) {
			if(entry.source_file().empty() && jumpUnSource) continue;
			if(entry.description().find("invoke_main") != std::string::npos && skipNative)break;

			index++;

			if(skipNative > 0 && skipNative >= index){
				continue;
			}

#ifdef _DEBUG
			auto exePrefix = entry.description().find_first_of('!');

			exePrefix = exePrefix == std::string::npos && showExec ? 0 : exePrefix + 1;

			auto charIndex = entry.description().find_first_of("+0x");

			std::filesystem::path src{entry.source_file()};

			std::string fileName = src.filename().string();

			if(!std::filesystem::directory_entry{src}.exists() || fileName.find('.') == std::string::npos) {
				fileName.insert(0, "<").append(">");
			}

			// std::string functionName = entry.description().substr(index, charIndex - index);
			std::string functionPtr = entry.description().substr(charIndex + 1);

			ss << std::left << "[SOURCE   FILE] : " << fileName << '\n';
			ss << std::left << "[FUNC     NAME] : " << entry.description().substr(exePrefix) << '\n';
			ss << std::left << "[FUNC END LINE] : " << entry.source_line() << " [FUNC PTR]: " << std::dec << functionPtr << "\n\n";
#else
			auto index = entry.description().find_first_of('!');

			index = (index == std::string::npos && showExec) ? 0 : (index + 1);

			auto charIndex = entry.description().find_first_of("+0x");

			std::filesystem::path path{entry.source_file()};

			ss << std::left << "[FILE]: " << path.filename().string() << '\n';
			ss << std::left << "[DESC]: " << entry.description().substr(index, charIndex) << '\n';
			ss << std::left << "[LINE]: " << entry.source_line() << " [At]: " << std::dec << entry.description().substr(charIndex + 1) << "\n\n";
#endif
		}

		ss << "--------------------- Stack Trace End:\n\n";
	}
}