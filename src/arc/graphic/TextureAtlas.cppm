module;

export module Graphic.TextureAtlas;

import GL.Texture.TextureRegionRect;
import GL.Texture.Texture2D;
import Assets.TexturePacker;
import OS.File;

import RuntimeException;

// import Event;

import <numeric>;
import <ranges>;
import <sstream>;
import <unordered_map>;
import <string>;

export namespace Graphic {
	namespace Page {
		constexpr std::string_view UI = "ui";
		constexpr std::string_view Game = "game";

		constexpr std::string_view extensionOf() {
			return "invalid";
			//TODO finish this if support extension contents.
		}
	}

	using Assets::PixmapModifer;

	class TextureAtlas {
	protected:
		std::unordered_map<std::string_view, Assets::TexturePackPage> pages{};
		std::unordered_map<std::string, GL::TextureRegionRect*> regions{};

		const GL::TextureRegionRect* fallbackTextureRegion{nullptr};

		Assets::TexturePackPage* contextPage{nullptr};

	public:
		const GL::TextureRegionRect* load(const OS::File& file, const PixmapModifer& modifer = nullptr) const { // NOLINT(*-use-nodiscard)
			return contextPage->pushRequest(file, modifer);
		}

		const GL::TextureRegionRect* load(const std::string_view pageName, const OS::File& file, const PixmapModifer& modifer = nullptr){
			if(const auto itr = pages.find(pageName); itr != pages.end()) {
				return itr->second.pushRequest(file, modifer);
			}

			return nullptr;
		}

		[[nodiscard]] Assets::TexturePackPage& getPage(const std::string_view pageName) {
			if(const auto itr = pages.find(pageName); itr != pages.end()) {
				return itr->second;
			}

			throw ext::IllegalArguments{"Cannot Find Texture Page: " + static_cast<std::string>(pageName)};
		}

		Assets::TexturePackPage* registerPage(const std::string_view pageName, const OS::File& cacheDir) {
			const auto [itr, success] = pages.try_emplace(pageName, pageName, cacheDir);

			if(!success)throw ext::IllegalArguments{"Cannot Register Pages That Has The Same Name: " + static_cast<std::string>(pageName)};

			return &itr->second;
		}

		Assets::TexturePackPage* registerPage(Assets::TexturePackPage&& page) {
			const auto [itr, success] = pages.try_emplace(page.pageName, std::forward<Assets::TexturePackPage>(page));

			if(!success)throw ext::IllegalArguments{"Cannot Register Pages That Has The Same Name: " + static_cast<std::string>(page.pageName)};

			return &itr->second;
		}

		template <typename ...Args>
		Assets::TexturePackPage* registerPage(const std::string_view pageName, Args&&... args) {
			const auto [itr, success] = pages.try_emplace(pageName, pageName, args...);

			if(!success)throw ext::IllegalArguments{"Cannot Register Pages That Has The Same Name: " + static_cast<std::string>(pageName)};

			return &itr->second;
		}

		void flush() {
			size_t size{0};

			for(auto& element : this->pages | std::ranges::views::values) {
				size += element.getData().size();
			}

			regions.reserve(size);

			std::stringstream ss{};

			for(auto& [pageName, page] : pages) {
				for(auto& [dataName, data] : page.getData()) {
					ss.str(std::string{});
					ss << pageName << "-" << dataName;
					regions.emplace(ss.str(), &data.textureRegion);
				}
			}
		}

		[[nodiscard]]
		const GL::TextureRegionRect* find(const std::string& regionName) {
			if(const auto itr = regions.find(regionName); itr != regions.end()) {
				return itr->second;
			}

			return fallbackTextureRegion;
		}

		[[nodiscard]] std::unordered_map<std::string_view, Assets::TexturePackPage>& getPages(){
			return pages;
		}

		[[nodiscard]] std::unordered_map<std::string, GL::TextureRegionRect*>& getRegions(){
			return regions;
		}

		[[nodiscard]] const GL::TextureRegionRect* getFailRegionReplacer() const {
			return fallbackTextureRegion;
		}

		[[nodiscard]] Assets::TexturePackPage* getContextPage() const {
			return contextPage;
		}

		void setFail(const GL::TextureRegionRect* rect) {
			this->fallbackTextureRegion = rect;
		}

		void setContextPage(Assets::TexturePackPage* const context_page) {
			contextPage = context_page;
		}

		void setContextPage(const std::string_view pageName) {
			if(const auto itr = pages.find(pageName); itr != pages.end()) {
				contextPage = &itr->second;
			}else {
				throw ext::IllegalArguments{"Cannot Find Texture Page: " + static_cast<std::string>(pageName)};;
			}
		}
	};
}
