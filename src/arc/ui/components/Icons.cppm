export module UI.Icons;

import std;
import ext.Heterogeneous;

import Graphic.TextureAtlas;

export import UI.RegionDrawable;
export import :Assets;

export namespace UI::Icons{
	namespace FileType{
		//TODO move this to other place;
		constexpr std::string_view Audio = "audio";
		constexpr std::string_view Code = "code";
		constexpr std::string_view Image = "image";
		constexpr std::string_view Text = "txt";
		constexpr std::string_view Unknown = "unknown";
	}


	ext::StringMap<std::string_view> categoryMap{
			{".json", FileType::Code},
			{".yaml", FileType::Code},
			{".cppm", FileType::Code},
			{".cpp", FileType::Code},
			{".h", FileType::Code},
			{".hpp", FileType::Code},
			{".c", FileType::Code},
			{".ixx", FileType::Code},

			{".txt", FileType::Text},

			{".png", FileType::Image},
			{".jpg", FileType::Image},
			{".jpeg", FileType::Image},
			{".bmp", FileType::Image},

			{".ogg", FileType::Audio},
			{".mp3", FileType::Audio},
			{".wav", FileType::Audio},
		};

	ext::StringMap<std::reference_wrapper<Icon>> linkedIcons{};

	Icon& getIconByFileSuffix(std::string_view suffix){
		const auto categroy = categoryMap.at(suffix, FileType::Unknown);

		return linkedIcons.at(categroy);
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

		std::println(stream, "\texport ext::StringMap<std::reference_wrapper<Icon>> iconMap{{}};");

		for(auto& loadedIcon : loadedIcons){
			std::string name = loadedIcon;
			std::ranges::replace(name, '-', '_');

			std::println(stream, "export Icon {}{{}};", name);
		}


		std::println(stream, "void loadIcons(Graphic::TextureAtlas& atlas){{");
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

	export void load(Graphic::TextureAtlas& atlas){
		loadIcons(atlas);

		linkedIcons.try_emplace(std::string(FileType::Audio), file_music_one);
		linkedIcons.try_emplace(std::string(FileType::Code), file_code_one);
		linkedIcons.try_emplace(std::string(FileType::Text), file_text_one);
		linkedIcons.try_emplace(std::string(FileType::Image), image_files);
		linkedIcons.try_emplace(std::string(FileType::Unknown), file_question);
	}
}
