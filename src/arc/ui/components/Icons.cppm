export module UI.Icons;

import std;

export import UI.RegionDrawable;
export import :Assets;

export namespace UI::Icons{
	Icon& getIconByFileSuffix(std::string_view){
		std::unreachable();
	}
}

export namespace UI::Icons{
	constexpr std::string_view UIPagePrefix = "ui";
	std::vector<std::string> loadedIcons{};

	void registerIconName(const std::string& str){
		loadedIcons.push_back(str);
	}

	void genCode(std::ostream& stream){
		std::println(stream, "export module UI.Icons:Assets;");

		std::println(stream, "export import UI.RegionDrawable;");
		std::println(stream, "import Graphic.TextureAtlas;");
		std::println(stream, "import ext.Heterogeneous;");
		std::println(stream, "import std;");

		std::println(stream, "namespace UI::Icons{{");

		for(auto& loadedIcon : loadedIcons){
			std::string name = loadedIcon;
			std::ranges::replace(name, '-', '_');

			std::println(stream, "export Icon {}{{}};", name);
		}

		std::println(stream, "\texport ext::StringMap<std::reference_wrapper<Icon>> iconMap{{}};");

		std::println(stream, "export void load(Graphic::TextureAtlas& atlas){{");
		std::println(stream, "\tusing namespace std::string_view_literals;");

		for(auto& loadedIcon : loadedIcons){
			std::string name = loadedIcon;
			std::ranges::replace(name, '-', '_');

			std::println(stream, "\t{}.wrapper = *atlas.find(\"{}-{}\");", name, UIPagePrefix, loadedIcon);
			std::println(stream, "\ticonMap.insert_or_assign(\"{}\"sv, {});", loadedIcon, name);
		}

		std::println(stream, "}}");
		std::println(stream, "}}");
		stream.flush();
	}
}