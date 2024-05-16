export module Assets.TexturePacker;

import std;

import Assets.Loader;
import Math.StripPacker2D;
import Graphic.Pixmap;
import Geom.Rect_Orthogonal;
import Geom.Vector2D;

import GL.Texture.Texture2D;
import GL.Texture.TextureRegionRect;

import ext.Async;
import ext.RuntimeException;
import OS.File;
import OS.FileTree;
import OS.Handler;

import Image.Svg;
import ext.Heterogeneous;

export namespace Assets {
	using PixmapModifer = std::function<void(Graphic::Pixmap& modifier)>;

	struct TextureRegionPackData{
		Geom::OrthoRectInt bound{};
		GL::TextureRegionRect textureRegion{};
		Graphic::Pixmap pixmap{};
		OS::File sourceFile{};

		//Uses this for multiple texture region align!
		std::string familyName{};

		int pageID{0};

		bool copyModifier = false;
		PixmapModifer modifer{nullptr};

		[[nodiscard]] bool valid() const {
			return textureRegion.getData();
		}

		void write(std::ofstream& stream) const {
			stream.write(reinterpret_cast<const char*>(&pageID), sizeof(pageID));
			stream.write(reinterpret_cast<const char*>(&bound), sizeof(bound));
		}

		void read(std::ifstream& stream){
			stream.read(reinterpret_cast<char*>(&pageID), sizeof(pageID));
			stream.read(reinterpret_cast<char*>(&bound), sizeof(bound));
		}

		bool adaptTo(const std::vector<std::unique_ptr<GL::Texture2D>>& textures, const int margin = 0) {
			if(pageID > textures.size())return false;

			const auto tex = textures[pageID].get();
			if(bound.getEndX() > tex->getWidth() || bound.getEndY() > tex->getHeight())return false;

			if(margin > 0) {
				bound.addSize(-margin, -margin);
			}

			textureRegion.setData(tex);
			textureRegion.fetchIntoCurrent(bound);

			return true;
		}

		[[nodiscard]] bool hasFile() const {
			return sourceFile.exist();
		}

		[[nodiscard]] bool hasPixelData() const {
			return pixmap.valid();
		}

		[[nodiscard]] bool loadPixmap(const int margin){
			if(!hasPixelData()){
				if(hasFile()) {
					if(sourceFile.extension() == ".svg"){
						pixmap = ext::svgToBitmap(sourceFile);
					}else{
						pixmap.loadFrom(sourceFile);
					}

				}

				//TODO no if branch
				//TODO global search
			}


			if(hasPixelData()) {
				if(modifer)modifer(pixmap);
			}

			bound.set(0, 0, pixmap.getWidth() + margin, pixmap.getHeight() + margin);

			return hasPixelData();
		}

		[[nodiscard]] explicit TextureRegionPackData(const OS::File& source, const PixmapModifer& modifer = nullptr)
			: sourceFile(source), familyName(source.stem()), modifer(modifer)  {
		}

		[[nodiscard]] explicit TextureRegionPackData(const Graphic::Pixmap& pixmap, const PixmapModifer& modifer = nullptr)
			: pixmap(pixmap), modifer(modifer)  {
		}

		[[nodiscard]] explicit TextureRegionPackData(Graphic::Pixmap&& pixmap, const PixmapModifer& modifer = nullptr)
			: pixmap(std::move(pixmap)), modifer(modifer) {
		}

		[[nodiscard]] explicit TextureRegionPackData(const std::string& name, const OS::File& source, const PixmapModifer& modifer = nullptr)
			: sourceFile(source), familyName(name), modifer(modifer) {
		}

		[[nodiscard]] explicit TextureRegionPackData(const std::string& name, Graphic::Pixmap&& pixmap, const PixmapModifer& modifer = nullptr)
			: pixmap(std::move(pixmap)), familyName(name), modifer(modifer) {
		}
	};

	enum class PackState {
		polling, loading, packing, savingCache, readingCache, done
	};

	class TexturePackPage : public ext::ProgressTask<void, Assets::AssetsTaskHandler>{
	protected:
		static constexpr int TotalProgressWeight = 10;

		struct MergeTask {
			std::vector<TextureRegionPackData*> datas{};
			Geom::Vector2D<unsigned> size{};
			unsigned id{0};

			[[nodiscard]] MergeTask(std::vector<TextureRegionPackData*>&& datas,
				const unsigned w, const unsigned h, const unsigned id)
				: datas(std::forward<std::vector<TextureRegionPackData*>>(datas)),
				size(w, h), id(id) {
			}

			[[nodiscard]] MergeTask() = default;
		};

		std::vector<std::unique_ptr<GL::Texture2D>> textures{};
		ext::StringMap<TextureRegionPackData> packData{};

		std::vector<MergeTask> toMerge{};
		std::vector<Graphic::Pixmap> mergedMaps{};
		int margin = 0;

		OS::File cacheDir{};
		OS::FileTree* globalSearchTree{nullptr};

		PackState state{PackState::polling};

	public:
		Geom::OrthoRectInt texMaxBound{0, 0, 2048, 2048};
		std::string pageName{};

		bool forcePack{false};
		bool dynamic{false};
		bool packDone{false};

		const TexturePackPage* linkTarget = nullptr;

		[[nodiscard]] TexturePackPage(const std::string_view pageName, const OS::File& cacheDir,
			const Geom::OrthoRectInt& texMaxBound, const bool forcePack)
			: cacheDir(cacheDir),
			texMaxBound(texMaxBound),
			pageName(pageName),
			forcePack(forcePack) {
		}

		[[nodiscard]] TexturePackPage(const std::string_view pageName, const OS::File& cacheDir,
									  const Geom::OrthoRectInt& texMaxBound)
			: TexturePackPage(pageName, cacheDir, texMaxBound, false){
		}

		[[nodiscard]] TexturePackPage(const std::string_view pageName, const OS::File& cacheDir) :
			TexturePackPage(pageName, cacheDir, {0, 0, GL::getMaxTextureSize(), GL::getMaxTextureSize()}){
		}

		[[nodiscard]] const ext::StringMap<TextureRegionPackData>& getData() const {
			return packData;
		}

		[[nodiscard]] ext::StringMap<TextureRegionPackData>& getData() {
			return packData;
		}

		[[nodiscard]] int getMargin() const{ return margin; }

		void setMargin(const int margin){ this->margin = margin; }

		[[nodiscard]] OS::File getCacheDir() const{
			return cacheDir;
		}

		[[nodiscard]] PackState getState() const {
			return state;
		}

		GL::TextureRegionRect* pushRequest(const OS::File& file, const PixmapModifer& modifer = nullptr) {
			return &packData.try_emplace(file.stem(), file, modifer).first->second.textureRegion;
		}

		GL::TextureRegionRect* pushRequest(const std::string& name, const OS::File& file, const PixmapModifer& modifer = nullptr) {
			return &packData.try_emplace(name, name, file, modifer).first->second.textureRegion;
		}

		GL::TextureRegionRect* pushRequest(const std::string& name, const Graphic::Pixmap& pixmap, const PixmapModifer& modifer = nullptr) {
			return &packData.try_emplace(name, pixmap, modifer).first->second.textureRegion;
		}

		GL::TextureRegionRect* pushRequest(const std::string& name, Graphic::Pixmap&& pixmap, const PixmapModifer& modifer = nullptr) {
			return &packData.try_emplace(name, name, std::move(pixmap), modifer).first->second.textureRegion;
		}

		GL::TextureRegionRect* overwriteRequest(const std::string& name, Graphic::Pixmap&& pixmap, const PixmapModifer& modifer = nullptr) {
			TextureRegionPackData data{name, std::move(pixmap), modifer};
			const auto itr = packData.insert_or_assign(name, data).first;
			return &itr->second.textureRegion;
		}

		TextureRegionPackData* findPackData(const std::string_view name) {
			const auto itr = packData.find(name);
			return itr == packData.end() ? nullptr : &itr->second;
		}

		[[nodiscard]] const TextureRegionPackData* findPackData(const std::string_view name) const {
			const auto itr = packData.find(name);
			return itr == packData.end() ? nullptr : &itr->second;
		}

		[[nodiscard]] const std::vector<std::unique_ptr<GL::Texture2D>>& getTextures() const {
			return textures;
		}

		[[nodiscard]] OS::File getDataFile() const {

			return cacheDir.subFile(static_cast<std::string>(pageName) + ".bin");
		}

		[[nodiscard]] bool hasCache() const {
			return getDataFile().exist();
		}

		void loadRequest() {
			for(auto& [name, data] : packData) {
				if(!data.loadPixmap(margin)) {
					throw ext::RuntimeException{"Cannot Read Pixmap Data: " + name};
				}
			}
		}

		[[nodiscard]] std::future<void> launch(const std::launch policy) override {
			(void)Task::launch(policy);
			return std::async(policy, &TexturePackPage::load, this);
		}

		[[nodiscard]] std::future<void> launch() override {
			return launch(std::launch::async);
		}

		[[nodiscard]] std::string_view getTaskName() const override {
			switch(state) {
				case PackState::readingCache : return "Reading Cache";
				case PackState::loading : return "Loading Pixmaps";
				case PackState::packing : return "Packing TextureRegions";
				case PackState::savingCache : return "Saving Cache";
				case PackState::done : return "Done";
				default: return "Unknown";
			}
		}

		void clearData() {
			mergedMaps.clear();
			toMerge.clear();
		}

		std::future<void> fallbackPost(std::function<void()>&& func) override{
			if(handler){
				return handler->operator()(std::forward<decltype(func)>(func));
			}

			constexpr OS::OSTaskHandler handler{};

			return handler(std::forward<decltype(func)>(func));
		}

	protected:
		static Geom::OrthoRectInt& transformBound(TextureRegionPackData* d) noexcept {
			return d->bound;
		}


		void copyOffset(const TexturePackPage& target) {
			packDone = false;

			while(true) {
				//TODO there should be a better way.
				if(target.done && !target.packDone) {
					handler->operator()(ext::RuntimeException{"Failed To Pack Linked Texture Atlas: " + pageName});
				}
				if(target.packDone)break;
				std::this_thread::sleep_for(std::chrono::milliseconds(250));
			}

			toMerge.resize(target.toMerge.size());

			for(int i = 0; i < toMerge.size(); ++i) {
				const MergeTask& targetMerge = target.toMerge.at(i);
				MergeTask& thisMerge = toMerge.at(i);

				for(const auto& data : targetMerge.datas) {
					auto itr = packData.find(data->familyName);

					if(itr == packData.end()) {
						continue;
						//throw ext::RuntimeException{"Failed To Pack Linked Texture Atlas: " + pageName + "\n Region Doesn't Match: " + data->familyName};
					}

					auto& srcData = itr->second;

					srcData.bound = data->bound;
					if(data->copyModifier)srcData.modifer = data->modifer;

					thisMerge.datas.push_back(&srcData);
				}

				thisMerge.size = targetMerge.size;
				thisMerge.id = targetMerge.id;
			}

			packDone = true;
		}

		void load() {
			taskProgress = 0;
			state = PackState::readingCache;

			if(
				//Repack if needed, notice that here actually try read cache!
				forcePack || !(hasCache() && readCache())
			) {
				taskProgress = 0;

				setProgress(TotalProgressWeight, 0, 0);

				state = PackState::loading;
				loadRequest();
				setProgress(TotalProgressWeight, 1, 0);

				state = PackState::packing;
				if(linkTarget != nullptr) {
					copyOffset(*linkTarget);
				}else {
					pack();
				}

				mergeTexture();

				setProgress(TotalProgressWeight, 2, 1);

				state = PackState::savingCache;
				saveCache();
				setProgress(TotalProgressWeight, 6, 3);

				state = PackState::readingCache;
				postToHandler(std::bind(&TexturePackPage::apply, this)).get();
				setProgress(TotalProgressWeight, 1, 9);
			}

			state = PackState::done;
			setDone();
			done = true;
		}

		void apply() {
			for(auto& map : mergedMaps) {
				textures.push_back(std::make_unique<GL::Texture2D>(map.getWidth(), map.getHeight(), std::move(map).release()));
			}

			for(auto& data : packData | std::views::values) {
				data.adaptTo(textures, margin);
			}
		}

		void saveCache() { // NOLINT(*-make-member-function-const)
			const size_t pageSize = mergedMaps.size();
			const size_t dataSize = packData.size();

			std::vector<std::future<void>> texSaveFutures{};
			texSaveFutures.reserve(pageSize);

			for(int i = 0; i < pageSize; ++i) {
				texSaveFutures.emplace_back(std::async([&pixmap = mergedMaps[i], file = cacheDir.subFile(static_cast<std::string>(pageName) + std::to_string(i) + ".png")] {
					pixmap.write(file, true);
				}));
			}

			std::ofstream stream{getDataFile().getPath(), std::ios::binary | std::ios::out};


			stream.write(reinterpret_cast<const char*>(&pageSize), sizeof(pageSize));
			stream.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));

			for(auto& data : packData | std::views::values) {
				data.write(stream);
			}

			for (std::future<void>& future : texSaveFutures) {
				future.get();
			}
		}

		/**
		 * \brief
		 * \return false if the cache data doesn't match
		 */
		bool readCache() {
			std::ifstream stream{getDataFile().getPath(), std::ios::binary | std::ios::in};

			size_t pageSize{0}, dataSize{0};
			stream.read(reinterpret_cast<char*>(&pageSize), sizeof(pageSize));
			stream.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

			std::vector<std::future<GL::Texture2D>> texFutures{};
			texFutures.reserve(pageSize);

			for(int i = 0; i < pageSize; ++i) {
				texFutures.emplace_back(std::async([file = cacheDir.subFile(static_cast<std::string>(pageName) + std::to_string(i) + ".png")] {
					return GL::Texture2D{file, false};
				}));
			}

			for(int i = 0; i < pageSize; ++i) {
				textures.emplace_back(std::make_unique<GL::Texture2D>(texFutures[i].get()));
				setProgress(0.5f * static_cast<float>(i) / static_cast<float>(pageSize));
			}

			postToHandler([this, pageSize] {
				for(int i = 0; i < pageSize; ++i) {
					textures[i]->init();
				}
			}).get();

			if(dataSize != packData.size())return false;

			for(auto& data : packData | std::views::values) {
				data.read(stream);
				if(!data.adaptTo(textures, margin)) {
					return false;
				}
			}

			return true;
		}

		/**
		 * \brief After this function, modifies to pixmaps wont work anymore.
		 */
		void pack() {
			packDone = false;
			std::vector<TextureRegionPackData*> all{};

			std::ranges::transform(packData, std::back_inserter(all),
								   [](auto& data)->TextureRegionPackData*{ return &data.second; });

			loadRemains(std::move(all), 0);
			packDone = true;
		}

		void loadRemains(Concepts::Iterable<TextureRegionPackData*> auto&& remains, const int currentID) {
			if(remains.empty())return;
			setProgress(TotalProgressWeight, 2, 1, packData.size() - remains.size(), packData.size());

			Math::StripPacker2D<TextureRegionPackData*, int, TexturePackPage::transformBound> packer{};

			packer.push(remains);

			packer.setMaxSize(texMaxBound.getWidth(), texMaxBound.getHeight());
			packer.sortDatas();
			packer.process();
			const Geom::OrthoRectInt r = packer.getResultBound();

			//Move it
			toMerge.emplace_back(std::move(packer.packed), r.getWidth(), r.getHeight(), currentID);
			loadRemains(std::move(packer.getRemains()), currentID + 1);
		}

		void mergeTexture() {
			mergedMaps.resize(toMerge.size());

			for(auto& mergeGroup : toMerge) {
				Graphic::Pixmap& mergedMap = mergedMaps.at(mergeGroup.id);

				mergedMap.create(std::max(mergeGroup.size.x, 2u), std::max(mergeGroup.size.y, 2u));

				for(const auto& data : mergeGroup.datas) {
					mergedMap.set(data->pixmap, data->bound.getSrcX(), data->bound.getSrcY());
					data->pixmap.free();
				}
			}
		}
	};
}
