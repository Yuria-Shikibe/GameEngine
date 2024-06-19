//
// Created by Matrix on 2024/6/8.
//

export module Assets.Load.TexturePacker;
import std;

import Assets.Load.Core;
import Assets.TexturePage;

import Graphic.Pixmap;
import OS.File;

import Geom.Vector2D;
import Geom.Rect_Orthogonal;

import GL.Texture.Texture2D;
import GL.Texture.TextureRegion;

import ext.json.io;
import ext.json;
export import Core.IO.Specialized;

import ext.Heterogeneous;

import GL;

import Math;
import Math.Algo.TopologicalSort;
import Math.Algo.StripPacker2D;

import ext.RuntimeException;

namespace Assets::Load{
	/**
	 * @brief
	 * PackData Json Structure:
	 * @code
	 *{
	 *	"<page name>" : {
	 *		"version" : xx
	 *		"data" : {
	 *			"<region name>" : {
	 *				//Pack Data
	 *			}
	 *		}
	 *	}
	 *}
	 *
	 * @endcode
	 *
	 */

	struct PackData{
		Graphic::Pixmap bitmapData{};

		Geom::OrthoRectInt region{};
		std::size_t pageID{};

		void copyPositionDataFrom(const PackData& other){
			region = other.region;
			pageID = other.pageID;
		}
	};

	struct PackRawData{
		std::vector<PackData*> fragments{};
		std::size_t pageID{};

		Graphic::Pixmap atlas{};

		[[nodiscard]] PackRawData(std::vector<PackData*>&& datas,
			const Geom::Point2 s, const std::size_t id)
			: fragments(std::move(datas)), pageID(id) {
			atlas.create(Math::max(s.x, 2), Math::max(s.y, 2));
		}

		[[nodiscard]] PackRawData() = default;
	};

	struct PageData{
		TexturePage* page{};
		OS::File cacheDir{};

		LoadTaskHandler handler{};

		ext::StringHashMap<PackData> fragments{};

		const ext::StringHashMap<PageData>* allPages{};

		std::vector<PackRawData> rawAtlases{};
		Geom::OrthoRectInt maxBound{2048, 2048};
		bool fromCache = false;

		[[nodiscard]] OS::File getDataFile() const{
			return cacheDir.subFile(std::format("{}-data.json", page->getName()));
		}

		[[nodiscard]] OS::File getImageFile(std::size_t index) const{
			return cacheDir.subFile(std::format("{}-{}.png", page->getName(), index));
		}

		ext::json::JsonValue pageJsonValue{};

		[[nodiscard]] const TexturePage* getDependency() const noexcept{
			return page->dependency;
		}

		void load(){
			readCache();
			if(isCacheValid()){

				if(!isCacheValid_Post()){
					loadFromPull();
				}else{
					loadFromCache();
				}
			}else{
				loadFromPull();
			}
		}

		void loadFromCache(){
			for(const auto& [index, element] : rawAtlases | std::views::enumerate){
				auto file = getImageFile(index);
				element.atlas.loadFrom(file);
				element.pageID = index;
			}
		}

		void loadFromPull(){
			if(page->dependency){
				loadFromFetch();
			}else{
				pack();
			}

			mergeTexture();
		}

		void loadFromFetch(){
			auto& targetLoadData = allPages->at(page->dependency->getName());
			auto& target = targetLoadData.fragments;
			for (auto& [name, frag] : fragments){
				if(auto itr = target.find(name); itr != target.end()){
					frag.copyPositionDataFrom(itr->second);
				}
			}

			auto all = fragments | std::views::values | std::views::transform([](auto& i){return std::addressof(i);}) | std::ranges::to<std::vector<PackData*>>();

			for (const auto& raw : targetLoadData.rawAtlases){
				decltype(all) currentFragments{};
				auto rst = std::ranges::remove_if(all, std::bind_front(std::equal_to<std::size_t>{}, raw.pageID), &PackData::pageID);
				currentFragments.assign_range(rst);
				all.erase(rst.begin(), rst.end());

				rawAtlases.emplace_back(std::move(currentFragments), raw.atlas.size2D(), raw.pageID);
			}
		}

		void applyTextureToRegion(){
			handler.postTask([this]{
				for (const auto& [index, raw] : rawAtlases | std::ranges::views::enumerate){
					std::println("[load] {}-{} Try obtain GL texture at thread: {}", page->getName(), index, std::this_thread::get_id());
					page->getTextures().emplace_back(
						std::make_unique<GL::Texture2D>(raw.atlas.getWidth(), raw.atlas.getHeight(), raw.atlas.data(), true));
				}
			}).get();

			for (const auto& [name, fragment] : fragments){
				auto& textureRegion = page->getRegions()[name];
				textureRegion.data = page->getTextures().at(fragment.pageID).get();
				textureRegion.fetchIntoCurrent(fragment.region);
			}
		}

		void loadPulledData(){
			for (const auto& bitmapLoadData : page->getRequests() | std::views::values){
				bitmapLoadData->load();
			}

			fragments.reserve(page->getRequests().size());
			for (auto& [name, data] : page->getRequests()){
				auto size = data->bitmapData.size2D();
				fragments.try_emplace(name, PackData{std::move(data->bitmapData), Geom::OrthoRectInt{size}});
			}
		}

		[[nodiscard]] constexpr std::size_t getPageSize() const noexcept{
			return rawAtlases.size();
		}
		/**
		 * \brief After this function, modifies to pixmaps wont work anymore.
		 */
		void pack() {
			rawAtlases.clear();
			auto all = fragments | std::views::values | std::views::transform([](auto& i){return std::addressof(i);}) | std::ranges::to<std::vector<PackData*>>();

			loadRemains(std::move(all), 0);
		}

		void loadRemains(Concepts::Iterable<PackData*> auto&& remains, const int currentPage) {
			if(remains.empty())return;
			// setProgress(TotalProgressWeight, 2, 1, packData.size() - remains.size(), packData.size());
			Math::StripPacker2D<PackData*, int, &PackData::region> packer{remains};

			packer.setMaxSize(maxBound.getWidth(), maxBound.getHeight());
			packer.setMargin(page->getMargin());
			packer.process();
			auto size = packer.getResultSize();

			//Move it
			for (const auto packed : packer.packed){
				packed->pageID = currentPage;
			}

			rawAtlases.emplace_back(std::move(packer.packed), size, currentPage);
			loadRemains(std::move(packer.getRemains()), currentPage + 1);
		}

		void mergeTexture() {
			for(auto& mergeGroup : rawAtlases) {
				Graphic::Pixmap& mergedMap = mergeGroup.atlas;

				for(const auto& data : mergeGroup.fragments) {
					mergedMap.set(data->bitmapData, data->region.getSrcX(), data->region.getSrcY());
				}

				mergeGroup.fragments.clear();
			}
		}

		void readCache(){
			auto data = getDataFile();
			if(data.exist()){
				pageJsonValue = ext::json::parse(data.readString_byLine());
			}
		}


		/**
		 * @brief
		 * @return false if failed.
		 */
		bool isCacheValid_Post(){
			ext::json::JsonSrlContBase_string_map<PackData, true>::read(pageJsonValue.asObject().at(ext::json::keys::Value), fragments);

			if(std::ranges::any_of(fragments | std::ranges::views::values, std::bind_back(std::greater{}, rawAtlases.size()), &PackData::pageID)){
				return false;
			}

			fromCache = true;
			return true;
		}

		void saveCache(){
			if(fromCache)return;
			pageJsonValue.asObject();

			for (const auto& [index, page] : rawAtlases | std::views::enumerate){
				page.atlas.write(getImageFile(index), true);
			}

			pageJsonValue.append(ext::json::keys::Count, static_cast<ext::json::Integer>(rawAtlases.size()));
			pageJsonValue.append(ext::json::keys::Version, static_cast<ext::json::Integer>(page->getVersion()));
			ext::json::JsonSrlContBase_string_map<PackData, true>::write(pageJsonValue.asObject()[ext::json::keys::Value], fragments);

			std::ofstream stream{getDataFile().getPath()};
			std::print(stream, "{}", pageJsonValue);
		}

		//TODO load by cache
		bool isCacheValid(){
			auto& rootMap = pageJsonValue.asObject();

			if(const auto version = rootMap.tryFind(ext::json::keys::Version)){
				if(version->getOr(-1) != page->getVersion())return false;
			}else return false;

			unsigned pageCount = ~0u;
			if(const auto page = rootMap.tryFind(ext::json::keys::Count)){
				pageCount = page->getOr(-1);

				for(unsigned i = 0; i < pageCount; ++i){
					if(!getImageFile(i).exist())return false;
				}
			}else return false;

			rawAtlases.resize(pageCount);

			const auto datas = rootMap.tryFind(ext::json::keys::Value);
			if(!datas)return false;

			auto& valueMap = datas->asObject();

			if(valueMap.size() != page->getRequests().size()){
				return false;
			}

			ext::StringHashSet<> cachedNames{valueMap.size()};

			for (auto& value : valueMap | std::views::keys){
				cachedNames.insert(value);
			}

			for(const auto& name : page->getRequests() | std::views::keys){
				if(const auto itr = cachedNames.find(name); itr != cachedNames.end()){
					cachedNames.erase(itr);
				}else{
					return false;
				}
			}

			if(!cachedNames.empty())return false;



			return true;
		}

	};

	export class TexturePacker : public LoadTask{
		static constexpr int Load_PullWeight = 20;
		static constexpr int Load_MergeWeight = 50;

		ext::StringHashMap<PageData> pagesToPack{};

		ext::json::JsonValue packDataJson{};
		OS::File cacheDir{};

		void execCurrentPhase(){
			switch(handler.getCurrentPhase()){
				case Phase::init:
					for (auto& pageData : pagesToPack | std::views::values){
						pageData.maxBound.setSize(GL::getMaxTextureSize(), GL::getMaxTextureSize());
						pageData.handler = handler;
						pageData.cacheDir = cacheDir;
						pageData.allPages = &pagesToPack;
					}

					progress = .1f;

					break;
				case Phase::pull:
					for (auto& pageData : pagesToPack | std::views::values){
						pageData.loadPulledData();
					}

					progress = .3f;

					break;
				case Phase::load:{
					loadPages();

					for (auto& pageData : pagesToPack | std::views::values){
						pageData.applyTextureToRegion();
					}

					progress = .5f;

					break;
				}
				case Phase::post_load:{
					std::vector<std::future<void>> cacheTasks{};

					for (auto& pageData : pagesToPack | std::views::values){
						cacheTasks.push_back(std::async(&PageData::saveCache, std::move(pageData)));
					}

					try{
						for (std::future<void>& cacheTask : cacheTasks){
							cacheTask.get();

							progress += .5f / static_cast<float>(cacheTasks.size());
						}
					}catch(...){
						handler.throwException(std::current_exception());
					}

					break;
				}
				case Phase::end:
					break;
				case Phase::clear:
					pagesToPack.clear();
					break;
			}
		}

		bool load(){
			if(!cacheDir.exist()){
				if(!cacheDir.createDir(false))
					throw ext::IllegalArguments{std::format("Invalid Cache Dir : {}", cacheDir)};
			}
			while(!handler.stopToken.stop_requested()){
				execCurrentPhase();
				tryArriveAndWait();
			}

			if(pagesToPack.empty())done();
			handler.join();
			return finished;
		}

		void loadPages(){ //TODO optm
			const auto pages = pagesToPack | std::views::values | std::views::transform(&PageData::page) | std::ranges::to<std::vector<TexturePage*>>();
			auto depthMap = Math::get_topological_depth_map(pages, &TexturePage::dependency);

			std::deque<std::vector<PageData*>> loadSections(depthMap.size());

			for (const auto& [page, depth] : depthMap){
				loadSections[depth].push_back(&pagesToPack.at(page->getName()));
			}

			std::vector<std::future<void>> tasksFuture{};

			while(!loadSections.empty()){
				auto top = std::move(loadSections.back());
				loadSections.pop_back();
				if(top.empty())continue;
				tasksFuture.clear();

				for (const auto pageData : top){
					tasksFuture.push_back(std::async(&PageData::load, pageData));
				}


				for (auto& future : tasksFuture){
					try{
						future.get();
					}catch(...){
						handler.throwException(std::current_exception());
					}
				}
			}
		}

		// void loadCacheData_pre(const TexturePage* page){
		// 	auto& rootMap = packDataJson.asObject();
		//
		// 	const OS::File pageDataFile = getDataFileOf(page);
		//
		// 	if(!pageDataFile.exist())return;
		//
		// 	auto json = ext::json::parse(pageDataFile.quickRead());
		//
		// 	rootMap.insert_or_assign(page->name, std::move(json));
		// }

	public:
		[[nodiscard]] TexturePacker() = default;

		TexturePacker(const TexturePacker& other) = delete;

		TexturePacker& operator=(const TexturePacker& other) = delete;

		[[nodiscard]] const OS::File& getCacheDir() const noexcept{ return cacheDir; }

		void setCacheDir(const OS::File& cacheDir){ this->cacheDir = cacheDir; }

		[[nodiscard]] std::future<bool> launch(const std::launch policy) override{
			return std::async(policy, &TexturePacker::load, this);
		}

		void pushPage(TexturePage* page){
			pagesToPack.insert_or_assign(page->getName(), PageData{page});
		}


	};
}

export template<>
	struct ::ext::json::JsonSerializator<Assets::Load::PackData>{
		static void write(ext::json::JsonValue& jsonValue, const Assets::Load::PackData& data){
			ext::json::append(jsonValue, "region", data.region);
			ext::json::append(jsonValue, "page", data.pageID);
		}

		static void read(const ext::json::JsonValue& jsonValue, Assets::Load::PackData& data){
			ext::json::read(jsonValue, "region", data.region, {});
			ext::json::read(jsonValue, "page", data.pageID, 0ull);
		}
	};