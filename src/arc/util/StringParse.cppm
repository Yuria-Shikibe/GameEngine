//
// Created by Matrix on 2024/4/24.
//

export module ext.StringParse;

import std;

export namespace ext::string{
	constexpr auto FailedPos = std::string_view::npos;

	[[deprecated("Unfinished!")]] void removeAllUnnecessary(std::string& str){
		bool escapeTheNext{false};
		bool parsingString{false};

		for(int pos = 0; pos < str.size(); ++pos){
			if(escapeTheNext){
				escapeTheNext = false;
				continue;
			}

			const char character = str[pos];

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
		}
	}

	std::vector<size_t> getCommaPos(const std::string_view view){
		std::vector<size_t> rowPos{};

		bool escapeTheNext{false};
		bool parsingString{false};

		unsigned int braceCount{0};
		unsigned int bracketCount{0};

		rowPos.push_back(static_cast<std::size_t>(-1));

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
				} else{
					parsingString = false;
				}
			}

			if(parsingString) continue;

			if(character == '\''){
				pos += 2;
				continue;
			}

			switch(character){
				case '{' : braceCount++;
					break;
				case '}' : braceCount--;
					break;
				case '[' : bracketCount++;
					break;
				case ']' : bracketCount--;
					break;
				case ',' :{
					if(!braceCount && !bracketCount){
						rowPos.push_back(pos);
					}
				}
				default : break;
			}
		}

		rowPos.push_back(view.size());

		return rowPos;
	}

	std::vector<std::string_view> splitByComma(const std::string_view view){
		if(view.size() < 2) return {};
		std::vector<std::string_view> splited{};
		const std::vector<size_t> commaPos = getCommaPos(view);

		splited.reserve(commaPos.size());
		for(int i = 0; i < commaPos.size() - 1; ++i){
			const auto begin = commaPos[i];
			const auto end = commaPos[i + 1];

			if(begin == end) continue;

			auto newView = view.substr(begin + 1, end - begin - 1);

			const auto noSpaceBegin = newView.find_first_not_of(' ');
			const auto noSpaceEnd = newView.find_last_not_of(' ');

			if(noSpaceBegin == noSpaceEnd) continue;


			splited.push_back(newView.substr(noSpaceBegin, noSpaceEnd - noSpaceBegin + 1));
		}

		return splited;
	}

	std::vector<std::string_view> getRawAttributes(const std::string_view view){
		if(view.size() < 4) return {};
		std::vector<std::string_view> attribs{};

		bool escapeTheNext{false};
		bool parsingString{false};

		// [ -> 1
		// [[ -> 2
		// ] -> -1
		// ]] -> -2
		char bracketCount{0};

		auto attrBegin = FailedPos;
		auto attrCount = 0;

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
				} else{
					parsingString = false;
				}
			}

			if(parsingString) continue;

			if(bracketCount == 2){
				attrCount++;
			}

			switch(character){
				case '[' :{
					bracketCount++;
					if(bracketCount == 2 && attrBegin == FailedPos){
						attrBegin = pos + 1;
					}
					break;
				}
				case ']' :{
					bracketCount--;
					if(bracketCount == 0 && attrBegin != FailedPos){
						attribs.push_back(view.substr(attrBegin, --attrCount));
						attrCount = 0;
						attrBegin = FailedPos;
					}
					break;
				}
				default :{
					if(bracketCount != 2) bracketCount = 0;
				}
			}
		}


		return attribs;
	}

	/**
	 * @return [ns, args]
	 */
	std::pair<std::string_view, std::string_view> getAttributeNamespace(const std::string_view scope){
		constexpr std::string_view UsingSpc = "using ";
		auto end = scope.find_first_of(':');

		if(end == std::string_view::npos){
			return {"", scope};
		}

		auto nameEnd = end;
		const auto begin = scope.find_first_of(UsingSpc);

		for(auto i = end - 1; i >= begin; i--){
			if(scope.at(i) != ' '){
				nameEnd = i + 1;
			}
		}

		return {scope.substr(begin + UsingSpc.size(), nameEnd - begin), scope.substr(end + 1)};
	}

	struct Attribute{
		std::vector<std::string> all{};
	};

	struct ScopeData{
		std::string scope{};
		std::size_t depth{};
		std::size_t relativeOffset{};

		std::vector<ScopeData> subScopes{};

		Attribute attr{};

		void removeSubScope(){
			std::size_t offsetSub = 0;
			for(const auto& sub : subScopes){
				auto end = scope.cbegin() + static_cast<long long>((sub.relativeOffset + sub.scope.size()));
				int offset{0};
				if(end != scope.cend() && *end == ';'){
					offset++;
				}

				scope.erase(sub.relativeOffset, sub.scope.size() + offset);
				offsetSub += sub.scope.size();
			}
		}

		void parseAttribute(){
			auto rawAttrib = getRawAttributes(scope);
			std::vector<std::string> attribs{rawAttrib.size()};

			std::ostringstream ss{};
			for(const auto attrib : rawAttrib){
				auto [ns, args] = getAttributeNamespace(attrib);
				auto splitedArgs = splitByComma(args);

				if(!ns.empty()){
					for(const auto arg : splitedArgs){
						ss.str("");
						ss << ns << "::" << (arg | std::ranges::views::filter([](const char c){ return c != ' '; }) |
							std::ranges::to<std::string>());
						attribs.push_back(std::move(ss).str());
					}
				} else{
					for(const auto arg : splitedArgs){
						attribs.push_back(
							arg | std::ranges::views::filter([](const char c){ return c != ' '; }) | std::ranges::to<
								std::string>());
					}
				}
			}

			attr.all = std::move(attribs);
		}
	};

	/**
	 * @brief
	 * @param view Rvalue Is Acceptable
	 * @return [pos, depth]
	 */
	std::vector<ScopeData> splitByScope(const std::string_view view){
		std::vector<ScopeData> scope{};

		std::stack<size_t> beginPos{};

		beginPos.push(0);

		bool escapeTheNext{false};
		bool parsingString{false};

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
				} else{
					parsingString = false;
				}
			}

			if(parsingString) continue;

			if(character == '\''){
				pos += 2;
				continue;
			}

			switch(character){
				case '{' : beginPos.push(pos + 1);
					break;
				case '}' :{
					for(int i = 0; i < 2; ++i){
						const auto begin = beginPos.top();
						beginPos.pop();
						const auto count = pos - begin + i;

						if(count == 0) continue;
						auto str = static_cast<std::string>(view.substr(begin, count));
						// str.push_back('\n');

						scope.emplace_back(
							std::move(str),
							beginPos.size(),
							begin
						);
					}

					beginPos.push(pos + 1);

					break;
				}
				case ';' :{
					const auto begin = beginPos.top();
					beginPos.pop();

					const auto count = pos - begin;

					if(count){
						auto str = static_cast<std::string>(view.substr(begin, count));
						// str.push_back('\n');

						scope.emplace_back(
							std::move(str),
							beginPos.size(),
							begin
						);
					}

					beginPos.push(pos + 1);
					break;
				}
				default : break;
			}
		}


		ScopeData* last{&scope.back()};
		std::stack<ScopeData*> directoryStack{};
		directoryStack.push(last);


		for(auto cur = scope.rbegin(); cur != scope.rend(); ++cur){
			const auto& top = directoryStack.top();

			if(cur->depth - 1 == top->depth){
				directoryStack.top()->subScopes.push_back(std::move(*cur));
				last = &directoryStack.top()->subScopes.back();
			} else if(cur->depth > top->depth){
				//Change Directory
				directoryStack.push(last);
				directoryStack.top()->subScopes.push_back(std::move(*cur));
				last = &directoryStack.top()->subScopes.back();
			} else if(cur->depth < top->depth){
				//Fallback Directory
				while(directoryStack.top()->depth != cur->depth - 1){
					directoryStack.pop();
				}

				directoryStack.top()->subScopes.push_back(std::move(*cur));
				last = &directoryStack.top()->subScopes.back();
			} else if(cur->depth == top->depth){
				//Swap Top Directory
				directoryStack.pop();
				if(!directoryStack.empty()){
					directoryStack.top()->subScopes.push_back(std::move(*cur));
					last = &directoryStack.top()->subScopes.back();
				} else{
					last = cur.operator->();
				}

				directoryStack.push(last);
			}
		}

		std::erase_if(scope, [](const ScopeData& s){
			return s.depth != 0;
		});

		return scope;
	}
}
