module;

export module Graphic.TextureAtlas;

import GL.Texture.TextureRegion;
import GL.Texture.Texture2D;
import GL.TextureArray;
import Assets.TexturePage;
import OS.File;

import ext.RuntimeException;

import ext.Heterogeneous;
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
		ext::StringHashMap<Assets::TexturePage> pages{};

		ext::StringHashMap<GL::TextureRegion*> regions{};
		ext::StringHashMap<std::unique_ptr<GL::Texture2DArray>> textureGroups{};

		GL::TextureRegion* fallbackTextureRegion{nullptr};

		Assets::TexturePage* uiPage{};
	public:
		[[nodiscard]] TextureAtlas() : uiPage{registerPage("ui")}{
			uiPage->setMargin(2);
		}

		[[nodiscard]] Assets::TexturePage* getUIPage() const{ return uiPage; }

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

			for(auto& region : mainPage.getRegions() | std::ranges::views::values){
				const GL::Texture2DArray* textureArray = textureReplaceMap.at(region.data);
				region.data = textureArray;
			}

			//TODO move this other place
			// for(const auto pageTex : pages | std::ranges::views::join){
			// 	const_cast<GL::Texture2D*>(pageTex)->freeGpuData();
			// }
		}

		[[nodiscard]] decltype(textureGroups)& getTextureGroups() {
			return textureGroups;
		}

		const GL::TextureRegion* load(const std::string_view pageName, const OS::File& file){
			if(const auto itr = pages.find(pageName); itr != pages.end()) {
				return itr->second.pushRequest<Assets::FileImportData>(file.stem(), file);
			}

			return nullptr;
		}

		[[nodiscard]] auto& getPage(const std::string_view pageName) {
			if(const auto itr = pages.find(pageName); itr != pages.end()) {
				return itr->second;
			}

			throw ext::IllegalArguments{"Cannot Find Texture Page: " + static_cast<std::string>(pageName)};
		}

		Assets::TexturePage* registerAttachmentPage(const std::string_view pageName, Assets::TexturePage* target) {
			if(!target){
				throw ext::NullPointerException{std::format("{}: {}", "Register Attachment Page Failed", pageName)};
			}

			Assets::TexturePage* page = registerPage(pageName);
			page->dependency = target;

			return page;
		}

		auto* registerPage(const std::string_view pageName) {
			const auto [itr, success] = pages.try_emplace(std::string(pageName), Assets::TexturePage{pageName});
			
			return &itr->second;
		}

		auto* registerPage(Assets::TexturePage&& page) {
			const auto [itr, success] = pages.try_emplace(std::string(page.getName()), std::forward<Assets::TexturePage>(page));

			if(!success)throw ext::IllegalArguments{"Cannot Register Pages That Has The Same Name: " + static_cast<std::string>(page.getName())};

			return &itr->second;
		}

		template <typename ...Args>
		auto* registerPage(const std::string_view pageName, Args&&... args) {
			const auto [itr, success] = pages.try_emplace(pageName, pageName, args...);

			if(!success)throw ext::IllegalArguments{"Cannot Register Pages That Has The Same Name: " + static_cast<std::string>(pageName)};

			return &itr->second;
		}

		void flush() {
			size_t size{0};

			for(auto& element : this->pages | std::ranges::views::values) {
				size += element.getRegions().size();
			}

			regions.reserve(size);

			std::ostringstream ss{};

			for(auto& [pageName, page] : pages) {
				for(auto& [dataName, data] : page.getRegions()) {
					ss.str("");
					ss << pageName << '[' << dataName << ']';
					regions.emplace(std::move(ss).str(), &data);
				}
			}
		}

		[[nodiscard]]
		GL::TextureRegion* find_ui(const std::string_view regionName) const{
			return uiPage->find(regionName, fallbackTextureRegion);
		}

		[[nodiscard]]
		GL::TextureRegion* find(Assets::TexturePage* page, const std::string_view regionName) const{
			return page->find(regionName, fallbackTextureRegion);
		}

		[[nodiscard]]
		GL::TextureRegion* find(const std::string_view pageName, const std::string_view regionName) {
			if(const auto page = pages.find(pageName); page != pages.end()){
				if(const auto itr = page->second.getRegions().find(regionName); itr != page->second.getRegions().end()) {
					return &itr->second;
				}
			}

			return fallbackTextureRegion;
		}

		[[nodiscard]] auto& getPages(){
			return pages;
		}

		[[nodiscard]] ext::StringHashMap<GL::TextureRegion*>& getRegions(){
			return regions;
		}

		[[nodiscard]] const GL::TextureRegion* getFailRegionReplacer() const {
			return fallbackTextureRegion;
		}

		void setFail(GL::TextureRegion* rect) {
			this->fallbackTextureRegion = rect;
		}


	};
}
