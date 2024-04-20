module;

export module Graphic.TextureAtlas;

import GL.Texture.TextureRegionRect;
import GL.Texture.Texture2D;
import GL.TextureArray;
import Assets.TexturePacker;
import OS.File;

import ext.RuntimeException;

import ext.Heterogeneous;

// import Event;

import std;

export namespace Graphic {
	namespace Page {
		constexpr std::string_view UI = "ui";
		constexpr std::string_view Game = "game";

		constexpr std::string_view extensionOf() {
			return "invalid";
			//TODO finish this if support extension contents.
		}
	}

	class TextureAtlas {
	protected:
		std::unordered_map<std::string_view, Assets::TexturePackPage> pages{};
		ext::StringMap<GL::TextureRegionRect*> regions{};

		ext::StringMap<std::unique_ptr<GL::Texture2DArray>> textureGroups{};

		const GL::TextureRegionRect* fallbackTextureRegion{nullptr};

		Assets::TexturePackPage* contextPage{nullptr};

	public:
		template <Concepts::InvokeNullable<void(GL::Texture2DArray*)> Func = std::nullptr_t>
		void bindTextureArray(std::string_view mergedPageName, const std::initializer_list<std::string_view> compPageNames, Func&& func = nullptr){
			std::unordered_map<const GL::Texture*, const GL::Texture2DArray*> textureReplaceMap{};

			const std::vector toMergePageNames = compPageNames;
			const std::string_view standardPageName = toMergePageNames.front();

			auto& mainPage = getPage(standardPageName);

			/*
			 * |--------| Arr1, Arr2, ...
			 * | Page00 | tex0, tex1, ...
			 * | Page01 | tex0, tex1, ...
			 * | Page02 | tex0, tex1, ...
			 */
			std::vector<std::vector<const GL::Texture2D*>> pages(toMergePageNames.size());

			//Build up pages layout
			for(int curPageID = 0; curPageID < toMergePageNames.size(); ++curPageID){
				const auto& currentPageName = toMergePageNames.at(curPageID);

				std::ranges::transform(
					this->getPage(currentPageName).getTextures(),
					std::back_inserter(pages.at(curPageID)),
					[](const std::unique_ptr<GL::Texture2D>& tex){return tex.get();}
				);
			}

			//Create texture array
			for(int texID = 0; texID < mainPage.getTextures().size(); ++texID){
				auto [rstItr, success] = textureGroups.try_emplace(std::format("{}-{}", mergedPageName, texID), std::make_unique<GL::Texture2DArray>());
				auto& curArray = rstItr->second;

				if constexpr(!std::same_as<Func, std::nullptr_t>){
					func(curArray.get());
				}

				std::vector<const GL::Texture2D*> texture2Ds(compPageNames.size());

				for(int page = 0; page < texture2Ds.size(); ++page){
					texture2Ds.at(page) = pages.at(page).at(texID);
				}

				curArray->init(texture2Ds);

				textureReplaceMap.insert_or_assign(mainPage.getTextures().at(texID).get(), curArray.get());
			}

			for(auto& data : mainPage.getData() | std::ranges::views::values){
				const GL::Texture2DArray* textureArray = textureReplaceMap.at(data.textureRegion.getData());
				data.textureRegion.setData(textureArray);
			}

			//TODO move this other place
			// for(const auto pageTex : pages | std::ranges::views::join){
			// 	const_cast<GL::Texture2D*>(pageTex)->freeGpuData();
			// }
		}

		[[nodiscard]] decltype(textureGroups)& getTextureGroups() {
			return textureGroups;
		}

		const GL::TextureRegionRect* load(const OS::File& file, const Assets::PixmapModifer& modifer = nullptr) const { // NOLINT(*-use-nodiscard)
			return contextPage->pushRequest(file, modifer);
		}

		const GL::TextureRegionRect* load(const std::string_view pageName, const OS::File& file, const Assets::PixmapModifer& modifer = nullptr){
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

		Assets::TexturePackPage* registerAttachmentPage(const std::string_view pageName, const Assets::TexturePackPage* target) {
			if(!target){
				throw ext::NullPointerException{std::format("{}: {}", "Register Attachment Page Failed", pageName)};
			}

			Assets::TexturePackPage* page = registerPage(pageName, target->getCacheDir());
			page->linkTarget = target;

			return page;
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
		const GL::TextureRegionRect* find(const std::string_view regionName) {
			if(const auto itr = regions.find(regionName); itr != regions.end()) {
				return itr->second;
			}

			return fallbackTextureRegion;
		}

		[[nodiscard]] std::unordered_map<std::string_view, Assets::TexturePackPage>& getPages(){
			return pages;
		}

		[[nodiscard]] ext::StringMap<GL::TextureRegionRect*>& getRegions(){
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
