module;

export module Graphic.TexturePacker;

import Math.StripPacker2D;
import Graphic.Pixmap;
import Geom.Shape.Rect_Orthogonal;
import GL.Texture.TextureRegionRect;
import GL.Texture.Texture2D;
import GL;
import RuntimeException;
import File;
import <algorithm>;
import <string>;
import <vector>;
import <fstream>;
import <functional>;
import <ranges>;
import <unordered_map>;
import <unordered_set>;

using Geom::Shape::OrthoRectUInt;

export namespace Graphic {
	using PixmapModifer = std::function<void(Graphic::Pixmap& modifier)>;
	struct TexturePackData {
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

		[[nodiscard]] bool loadPixmap(const int margin = 0){ // NOLINT(*-make-member-function-const)
			if(hasFile()) {
				pixmap.loadFrom(sourceFile);
			}

			if(hasPixelData()) {
				if(modifer)modifer(pixmap);
			}

			bound.set(margin, margin, pixmap.getWidth() + margin * 2, pixmap.getHeight() + margin * 2);

			return hasPixelData();
		}

		void correctMargin(const int margin) {
			bound.move(margin, margin);
			bound.addSize(-margin * 2, -margin * 2);
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

	class TexturePackPage {
	protected:
		std::vector<Graphic::Pixmap> mergedMaps{};
		int margin = 0;
		OS::File cacheDir{};
		std::string pageName{};
		std::unordered_map<std::string, TexturePackData> datas{};


	public:
		OrthoRectUInt texMaxBound{2048, 2048};

		std::vector<std::unique_ptr<GL::Texture2D>> textures{};

		bool forcePack = false;
		bool requiresCache = false;

		bool done{false};

		float progress = 0;

		[[nodiscard]] TexturePackPage(const OS::File& cacheDir, const std::string& pageName,
			const OrthoRectUInt& texMaxBound, const bool forcePack)
			: cacheDir(cacheDir),
			pageName(pageName),
			texMaxBound(texMaxBound),
			forcePack(forcePack) {
		}

		[[nodiscard]] TexturePackPage(const OS::File& cacheDir, const std::string& pageName,
			const OrthoRectUInt& texMaxBound)
			: TexturePackPage(cacheDir, pageName, texMaxBound, false){
		}

		[[nodiscard]] TexturePackPage(const OS::File& cacheDir, const std::string& pageName) :
			TexturePackPage(cacheDir, pageName, {GL::getMaxTextureSize(), GL::getMaxTextureSize()}, false){
		}

		void push(const OS::File& file, const PixmapModifer& modifer = nullptr) {
			datas.try_emplace(file.stem(), file, modifer);
		}

		void push(const std::string& name, const Graphic::Pixmap& pixmap, const PixmapModifer& modifer = nullptr) {
			datas.try_emplace(name, pixmap, modifer);
		}

		void push(const std::string& name, Graphic::Pixmap&& pixmap, const PixmapModifer& modifer = nullptr) {
			datas.try_emplace(name, std::move(pixmap), modifer);
		}

		TexturePackData* find(const std::string& name) {
			const auto itr =  datas.find(name);
			return itr == datas.end() ? nullptr : &itr->second;
		}

		[[nodiscard]] const TexturePackData* find(const std::string& name) const {
			const auto itr =  datas.find(name);
			return itr == datas.end() ? nullptr : &itr->second;
		}

		[[nodiscard]] OS::File getDataFile() const {
			return cacheDir.subFile(pageName + ".bin");
		}

		[[nodiscard]] bool hasCache() const {
			return getDataFile().exist();
		}

		void load() {
			//If obtain from cache failed or forced, pack again

			if(
				const bool hasValidCache = hasCache() && readCache();
				forcePack || !hasValidCache
			) {
				requiresCache = true;

				for(auto& [name, data] : datas) {
					if(!data.loadPixmap(margin)) {
						throw ext::RuntimeException{"Cannot Read Pixmap Data: " + name};
					}
				}

				pack();

				saveCache();

				apply();
			}

			done = true;

			mergedMaps.clear();
		}

		void apply() {
			for(auto& map : mergedMaps) {
				textures.push_back(std::make_unique<GL::Texture2D>(map.getWidth(), map.getHeight(), map.release()));
			}

			for(auto& data : datas | std::views::values) {
				data.textureRegion.setData(textures[data.pageID].get());

				//Margin Has Been Corrected In saveCache()
				data.textureRegion.fetchIntoCurrent(data.bound);
			}
		}

		void saveCache() { // NOLINT(*-make-member-function-const)

			if(!requiresCache)return;
			std::ofstream stream{getDataFile().path()};

			const size_t pageSize = mergedMaps.size();
			const size_t dataSize = datas.size();
			stream.write(reinterpret_cast<const char*>(&pageSize), sizeof(pageSize));
			stream.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));

			for(auto& data : datas | std::views::values) {
				data.correctMargin(margin);
				data.write(stream);
			}

			for(int i = 0; i < mergedMaps.size(); ++i) {
				mergedMaps[i].write(cacheDir.subFile(pageName + std::to_string(i) + ".png"), true);
			}

		}

		/**
		 * \brief
		 * \return false if the cache data doesn't match
		 */
		bool readCache() {

			std::ifstream stream{getDataFile().path()};

			size_t pageSize{0}, dataSize{0};
			stream.read(reinterpret_cast<char*>(&pageSize), sizeof(pageSize));
			stream.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

			for(int i = 0; i < pageSize; ++i) {
				textures.emplace_back(std::make_unique<GL::Texture2D>(cacheDir.subFile(pageName + std::to_string(i) + ".png")));
			}

			if(dataSize != datas.size())return false;

			for(auto& data : datas | std::views::values) {
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

			std::ranges::transform(datas, std::back_inserter(all),
			                       [](auto& pair)->TexturePackData*{ return &pair.second; });

			loadRemains(all, 0);
		}


	protected:
		static OrthoRectUInt& transformBound(TexturePackData& d) {
			return d.bound;
		}



		template <Concepts::Iterable<TexturePackData*> Range>
		void loadRemains(const Range& remains, const int currentID) {
			if(remains.empty())return;

			Math::Packer<TexturePackData, unsigned int> packer{transformBound};

			packer.push(remains);

			packer.setMaxSize(texMaxBound.getWidth(), texMaxBound.getHeight());
			packer.begin();
			packer.process();
			const OrthoRectUInt r = packer.resultBound();
			mergeTexture(packer.packed, r.getWidth(), r.getHeight(), currentID);
			loadRemains(packer.remains(), currentID + 1);
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
