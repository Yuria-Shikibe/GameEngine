//
// Created by Matrix on 2024/5/14.
//

export module UI.Format;

import std;

export namespace UI::Fmt{
	constexpr auto FailPos = std::string_view::npos;
	using std::operator""sv;
	std::string keyType(const std::string_view key){
		if(const auto pos = key.find("keyboard_"sv); pos != FailPos){
			auto rest = key.substr(pos + "keyboard_"sv.size());
			return std::format("{}$<sub>$<c#[e7915f]><Key>$<\\sub>", rest);
		}

		if(const auto pos = key.find("mouse_"sv); pos != FailPos){
			auto rest = key.substr(pos + "mouse_"sv.size());
			return std::format("{}$<sub>$<c#[d7c7ff]><Mouse>$<\\sub>", rest);
		}

		return "Unknown: " + std::string(key);
	}
}
