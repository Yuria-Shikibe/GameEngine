module;

export module Assets.TexturePacker;

import Assets.Loader;
import Math.StripPacker2D;
import Graphic.Pixmap;
import Geom.Shape.Rect_Orthogonal;
import GL.Texture.TextureRegionRect;
import GL.Texture.Texture2D;
import GL;
import Async;
import OS.Handler;
import RuntimeException;
import File;
import OS;
import NativeUtil;
import <algorithm>;
import <string>;
import <vector>;
import <fstream>;
import <functional>;
import <ranges>;
import <unordered_map>;
import <unordered_set>;

using Geom::Shape::OrthoRectUInt;
using namespace Graphic;

export namespace Assets {
	using PixmapModifer = std::function<void(Graphic::Pixmap& modifier)>;
	struct  TexturePackData{
		OrthoRectUInt bound{};
		GL::TextureRegionRect textureRegion{};
		Graphic::Pixmap pixmap{};
		OS::File sourceFile{};

		int pageID{0};

		PixmapModifer modifer{nullptr};

		[[nodiscard]] bool valid() const {
			return textureRegion.getData();
		}

		void write(std::ofstream& stream) const {
			stream.write(reinterpret_cast<const char*>(&pageID), sizeof(pageID));
			stream.write(reinterpret_cast<const char*>(&bound), sizeof(bound));
		}

		[[nodiscard]] bool read(std::ifstream& stream, const std::vector<std::unique_ptr<GL::Texture2D>>& textures){
			stream.read(reinterpret_cast<char*>(&pageID), sizeof(pageID));
			stream.read(reinterpret_cast<char*>(&bound), sizeof(bound));

			if(pageID > textures.size())return false;

			const auto tex = textures[pageID].get();
			if(bound.getEndX() > tex->getWidth() || bound.getEndY() > tex->getHeight())return false;

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

		[[nodiscard]] bool loadPixmap(){ // NOLINT(*-make-member-function-const)
			if(hasFile()) {
				pixmap.loadFrom(sourceFile);
			}

			if(hasPixelData()) {
				if(modifer)modifer(pixmap);
			}

			bound.set(0, 0, pixmap.getWidth(), pixmap.getHeight());

			return hasPixelData();
		}

		[[nodiscard]] explicit TexturePackData(const OS::File& source, const PixmapModifer& modifer = nullptr)
			: sourceFile(source), modifer(modifer)  {
		}

		[[nodiscard]] explicit TexturePackData(const Graphic::Pixmap& pixmap, const PixmapModifer& modifer = nullptr)
			: pixmap(pixmap), modifer(modifer)  {
		}

		[[nodiscard]] explicit TexturePackData(Graphic::Pixmap&& pixmap, const PixmapModifer& modifer = nullptr)
			: pixmap(std::move(pixmap)), modifer(modifer) {
		}
	};

	enum class PackState {
		polling, loading, packing, savingCache, readingCache, done
	};


	class TexturePackPage : public ext::ProgressTask<void, Assets::AssetsTaskHandler>{
	protected:
		static constexpr int TotalWeight = 10;

		std::vector<Graphic::Pixmap> mergedMaps{};
		OS::File cacheDir{};
		std::unordered_map<std::string, TexturePackData> packData{};
		PackState state{PackState::polling};

	public:
		OrthoRectUInt texMaxBound{2048, 2048};
		std::string_view pageName{};

		std::vector<std::unique_ptr<GL::Texture2D>> textures{};

		bool forcePack = false;

		bool dynamic{false};

		[[nodiscard]] TexturePackPage(const OS::File& cacheDir, const std::string_view pageName,
			const OrthoRectUInt& texMaxBound, const bool forcePack)
			: cacheDir(cacheDir),
			pageName(pageName),
			texMaxBound(texMaxBound),
			forcePack(forcePack) {
		}

		[[nodiscard]] const std::unordered_map<std::string, TexturePackData>& getData() const {
			return packData;
		}

		[[nodiscard]] std::unordered_map<std::string, TexturePackData>& getData() {
			return packData;
		}

		[[nodiscard]] TexturePackPage(const OS::File& cacheDir, const std::string_view pageName,
		                              const OrthoRectUInt& texMaxBound)
			: TexturePackPage(cacheDir, pageName, texMaxBound, false){
		}

		[[nodiscard]] TexturePackPage(const OS::File& cacheDir, const std::string_view pageName) :
			TexturePackPage(cacheDir, pageName, {GL::getMaxTextureSize(), GL::getMaxTextureSize()}, false){
		}

		[[nodiscard]] PackState getState() const {
			return state;
		}

		GL::TextureRegionRect* pushRequest(const OS::File& file, const PixmapModifer& modifer = nullptr) {
			return &packData.try_emplace(file.stem(), file, modifer).first->second.textureRegion;
		}

		GL::TextureRegionRect* pushRequest(const std::string& name, const OS::File& file, const PixmapModifer& modifer = nullptr) {
			return &packData.try_emplace(name, file, modifer).first->second.textureRegion;
		}

		GL::TextureRegionRect* pushRequest(const std::string& name, const Graphic::Pixmap& pixmap, const PixmapModifer& modifer = nullptr) {
			return &packData.try_emplace(name, pixmap, modifer).first->second.textureRegion;
		}

		GL::TextureRegionRect* pushRequest(const std::string& name, Graphic::Pixmap&& pixmap, const PixmapModifer& modifer = nullptr) {
			return &packData.try_emplace(name, std::move(pixmap), modifer).first->second.textureRegion;
		}

		GL::TextureRegionRect* overwriteRequest(const std::string& name, Graphic::Pixmap&& pixmap, const PixmapModifer& modifer = nullptr) {
			TexturePackData data{std::move(pixmap), modifer};
			packData.insert_or_assign(name, data);
			return &data.textureRegion;
		}

		TexturePackData* findPackData(const std::string& name) {
			const auto itr = packData.find(name);
			return itr == packData.end() ? nullptr : &itr->second;
		}

		[[nodiscard]] const TexturePackData* findPackData(const std::string& name) const {
			const auto itr =  packData.find(name);
			return itr == packData.end() ? nullptr : &itr->second;
		}

		[[nodiscard]] OS::File getDataFile() const {
			return cacheDir.subFile(static_cast<std::string>(pageName) + ".bin");
		}

		[[nodiscard]] bool hasCache() const {
			return getDataFile().exist();
		}

		void loadRequest() {
			for(auto& [name, data] : packData) {
				if(!data.loadPixmap()) {
					throw ext::RuntimeException{"Cannot Read Pixmap Data: " + name};
				}
			}
		}

		void load() {
			taskProgress = 0;
			state = PackState::readingCache;

			if(
				//Repack if needed, notice that here actually try read cache!
				forcePack || !(hasCache() && readCache())
			) {
				taskProgress = 0;

				setProgress(TotalWeight, 0, 0);

				state = PackState::loading;
				loadRequest();
				setProgress(TotalWeight, 1, 0);

				state = PackState::packing;
				pack();
				setProgress(TotalWeight, 2, 1);

				state = PackState::savingCache;
				saveCache();
				setProgress(TotalWeight, 6, 3);

				state = PackState::readingCache;
				postToHandler(std::bind(&TexturePackPage::apply, this)).get();
				setProgress(TotalWeight, 1, 9);
			}

			state = PackState::done;
			setDone();
			done = true;

			mergedMaps.clear();
		}

		[[nodiscard]] std::future<void> launch(const std::launch policy) override {
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

		void apply() {
			for(auto& map : mergedMaps) {
				textures.push_back(std::make_unique<GL::Texture2D>(map.getWidth(), map.getHeight(), map.release()));
			}

			for(auto& data : packData | std::views::values) {
				data.textureRegion.setData(textures[data.pageID].get());

				data.textureRegion.fetchIntoCurrent(data.bound);
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

			std::ofstream stream{getDataFile().path(), std::ios::binary | std::ios::out};


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
			std::ifstream stream{getDataFile().path(), std::ios::binary | std::ios::in};

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
				if(!data.read(stream, textures)) {
					return false;
				}
			}

			return true;
		}

		/**
		 * \brief After this function, modifies to pixmaps wont work anymore.
		 */
		void pack() {
			std::vector<TexturePackData*> all{};

			std::ranges::transform(packData | std::ranges::views::values, std::back_inserter(all),
			                       [](auto& data)->TexturePackData*{ return &data; });

			loadRemains(std::move(all), 0);
		}


	protected:
		static OrthoRectUInt& transformBound(TexturePackData& d) {
			return d.bound;
		}

		template <Concepts::Iterable<TexturePackData*> Range>
		void loadRemains(Range&& remains, const int currentID) {
			if(remains.empty())return;
			setProgress(TotalWeight, 2, 1, packData.size() - remains.size(), packData.size());

			const std::function obtainer = &TexturePackPage::transformBound;

			Math::StripPacker2D<TexturePackData, unsigned int> packer{obtainer};

			packer.push(remains);

			packer.setMaxSize(texMaxBound.getWidth(), texMaxBound.getHeight());
			packer.sortDatas();
			packer.process();
			const OrthoRectUInt r = packer.resultBound();
			mergeTexture(packer.packed, r.getWidth(), r.getHeight(), currentID);
			loadRemains(std::move(packer.remains()), currentID + 1);
		}

		void mergeTexture(const std::vector<TexturePackData*>& packedDatas, const unsigned int width, const unsigned int height, const int id) {
			Graphic::Pixmap& mergedMap = mergedMaps.emplace_back();

			mergedMap.create(std::max(width, 2u), std::max(height, 2u));

			for(const auto& data : packedDatas) {
				mergedMap.set(data->pixmap, data->bound.getSrcX(), data->bound.getSrcY());
				data->pixmap.free();
			}
		}
	};
}
