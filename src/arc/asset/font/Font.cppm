module;

#include <ft2build.h>
#include <freetype/freetype.h>

export module Font;

import std;

import Async;
import Assets.Loader;

import GL.Texture.Texture2D;
import GL.Texture.TextureRegionRect;
import GL.Texture.TextureRegion;
import Assets.TexturePacker;
import Geom.Rect_Orthogonal;
import Geom.Vector2D;
import Graphic.Pixmap;
import RuntimeException;
import OS.File;
import Image;
import Event;
import Math;

using namespace Geom;
using Graphic::Pixmap;

namespace Font {
	void exitLoad(const std::string& fontName) {
		throw ext::RuntimeException{"Font Load Failed : " + fontName};
	}

	std::fstream obtainStream(const OS::File& file) {
		return std::fstream{file.absolutePath(), std::ios::binary | std::ios::in | std::ios::out};
	}

	void readCacheVersion(std::istream& stream, unsigned char& version) {
		stream.read(reinterpret_cast<char*>(&version), sizeof(version));
	}

	void readCacheVersion(std::istream&& stream, unsigned char& version) {
		stream.read(reinterpret_cast<char*>(&version), sizeof(version));
	}

	void saveCacheVersion(std::ostream& stream, const unsigned char& version) {
		stream.write(reinterpret_cast<const char*>(&version), sizeof(version));
	}

	//Just use 'size', not EOF, just my preference...
	void readCacheData(std::istream& stream, FT_UInt& width, FT_UInt& height, size_t& size) {
		stream.read(reinterpret_cast<char*>(&width ), sizeof(width ));
		stream.read(reinterpret_cast<char*>(&height), sizeof(height));
		stream.read(reinterpret_cast<char*>(&size  ), sizeof(size  ));
	}

	void saveCacheData(std::ostream& stream, const FT_UInt width, const FT_UInt height, const size_t size) {
		stream.write(reinterpret_cast<const char*>(&width ), sizeof(width ));
		stream.write(reinterpret_cast<const char*>(&height), sizeof(height));
		stream.write(reinterpret_cast<const char*>(&size  ), sizeof(size  ));
	}

	void writeIntoPixmap(const FT_Bitmap& map, unsigned char* data) {
		for(auto size = 0; size < map.width * map.rows; ++size) {
			//Normal
			data[size * 4 + 0] = 0xff;
			data[size * 4 + 1] = 0xff;
			data[size * 4 + 2] = 0xff;
			data[size * 4 + 3] = map.buffer[size];
		}
	}

	constexpr unsigned int TexturePackGap{1};
}

export namespace Font {
	using CharCode = unsigned int;
	FT_Library GlobalFreeTypeLib{};

	enum struct Style : unsigned char{
		null    = 0x00'00,
		regualr = 0x00'01,
		bold    = 0x00'02,
		italic  = 0x00'04,
	};

	constexpr std::string_view regular{"Regular"};
	constexpr std::string_view bold{"Bold"};
	constexpr std::string_view italic{"Italic"};
	constexpr std::string_view data_suffix{".bin"};
	constexpr std::string_view tex_suffix{".png"};

	unsigned char getStyleID(const std::string_view str) {
		unsigned char id = 0x00;

		if(str.contains(regular)) {
			id |= static_cast<unsigned char>(Style::regualr);
		}

		if(str.contains(bold   )) {
			id |= static_cast<unsigned char>(Style::bold   );
		}

		if(str.contains(italic )) {
			id |= static_cast<unsigned char>(Style::italic );
		}

		return id;
	}

	void loadLib() {
		if(FT_Init_FreeType(&GlobalFreeTypeLib)) {
			throw ext::RuntimeException{"Failed to initialize FreeType Library!"};
		}
	}

	struct CharData {
		GL::TextureRegionRect region{};
		FT_Glyph_Metrics matrices{};
		OrthoRectUInt charBox{};

		[[nodiscard]] CharData(const FT_Glyph_Metrics& matrices, const OrthoRectUInt charBox)
			: matrices(matrices),
			  charBox(charBox) {
		}

		[[nodiscard]] CharData() = default;
	};

	struct FontData {
		OrthoRectUInt box{};
		std::unordered_map<CharCode, CharData> charDatas{};
		float lineSpacingMin{-1};

		std::unique_ptr<Pixmap> fontPixmap{nullptr};

		[[nodiscard]] FontData() = default;

		[[nodiscard]] explicit FontData(OrthoRectUInt&& box, const size_t size, Pixmap&& pixmap) : box(std::move(box)){
			fontPixmap = std::make_unique<Pixmap>(std::forward<Pixmap>(pixmap));
			charDatas.reserve(size);
		}

		void write(std::ostream& ostream) const{
			const size_t size = charDatas.size();
			ostream.write(reinterpret_cast<const char*>(&size), sizeof(size));

			for(const auto& [key, value]: charDatas) {
				ostream.write(reinterpret_cast<const char*>(&key		   ), sizeof(key           ));
				ostream.write(reinterpret_cast<const char*>(&value.matrices), sizeof(value.matrices));
				ostream.write(reinterpret_cast<const char*>(&value.charBox ), sizeof(value.charBox ));
			}
		}

		void read(std::istream& istream){
			size_t size = 0;
			istream.read(reinterpret_cast<char*>(&size), sizeof(size));
			charDatas.reserve(size);

			for(size_t i = 0; i < size; ++i) {
				CharData value{};
				CharCode key;
				istream.read(reinterpret_cast<char*>(&key		    ), sizeof(key           ));
				istream.read(reinterpret_cast<char*>(&value.matrices), sizeof(value.matrices));
				istream.read(reinterpret_cast<char*>(&value.charBox ), sizeof(value.charBox ));

				charDatas.insert(std::make_pair(std::move(key), std::move(value)));
			}
		}
	};

	const CharData emptyCharData{};

	struct FontFlags {
		static constexpr auto styleOffset = 16;
		static constexpr auto familyOffset = 8;
		static constexpr auto fontOffset = 0;

		OS::File fontFile{};
		std::vector<Math::Section<CharCode>> segments{};
		FT_Int loadFlags = FT_LOAD_RENDER; //
		FT_UInt size = 48;

		unsigned char version = 0;


		FT_Face face{nullptr};
		unsigned char expectedVersion = 0;

		std::string styleName = static_cast<std::string>(regular);
		std::string familyName{};

				/**
		 * \brief 8 for style and 255 for id should be enough !
		 * \code
		 *	    0000_0000  0000_0000  0000_0000  0000_0000
		 *	               |          |          |
		 *	               Style      Family ID  Font ID
		 *
		 * \endcode
		 */
		FT_UInt internalID{0}; //This should be assigned by internal operation!

		std::unique_ptr<FontData> data{nullptr};

		std::function<bool(FT_Face)> loader = nullptr; /*[](const FT_Face face) {
			return FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		};*/

		const FontFlags* fallback{nullptr};

		[[nodiscard]] FontFlags(
			const OS::File& fontFile,
			const std::vector<Math::Section<CharCode>>& segments,
			const FT_Int loadFlags = FT_LOAD_RENDER,
			const FT_UInt height = 48,
			const std::function<bool(FT_Face)>& loader = nullptr
		)
			: fontFile(fontFile),
			  segments(segments),
			  loadFlags(loadFlags),
			  size(height),
			  loader(loader) {
			familyName = fontFile.filename();
		}

		[[nodiscard]] FontFlags(
			OS::File&& fontFile,
			const std::vector<Math::Section<CharCode>>& segments,
			const FT_Int loadFlags = FT_LOAD_RENDER,
			const FT_UInt height = 48,
			const std::function<bool(FT_Face)>& loader = nullptr
		)
			: fontFile(std::forward<OS::File>(fontFile)),
			  segments(segments),
			  loadFlags(loadFlags),
			  size(height),
			  loader(loader) {
			familyName = fontFile.filename();
		}

		FontFlags* tryLoad(const CharCode charCode){
			if(!face) { //Fall back may roll to font that doesn't need cache, just load its face
				if(FT_New_Face(GlobalFreeTypeLib, fontFile.absolutePath().string().data(), 0, &face)) {
					exitLoad(fullname());
				}
			}

			FT_Set_Pixel_Sizes(face, 0, size);

			if(const auto state = FT_Load_Char(face, charCode, loadFlags); state != 0) {
				if(state == FT_Err_Invalid_Glyph_Index) {
					if(fallback){ //Seriously I don't want to use const cast
						return const_cast<FontFlags*>(fallback)->tryLoad(charCode);
					}

					const std::string str = FT_Error_String(state);
					exitLoad(fullname() + " | " + str);
				}
			}

			return this;
		}

		[[nodiscard]] bool containsData(const CharCode charCode) const {
			if(data) {
				if(data->charDatas.contains(charCode))return true;
			}

			if(fallback) {
				return fallback->containsData(charCode);
			}

			return false;
		}

		[[nodiscard]] const CharData* getCharData(const CharCode charCode) const {
			if(data) {
				if(const auto itr = data->charDatas.find(charCode); itr != data->charDatas.end()) {
					return &itr->second;
				}
			}

			if(fallback) {
				return fallback->getCharData(charCode);
			}

			return nullptr;
		}

		[[nodiscard]] const FontFlags* getFallback() const {
			return fallback;
		}

		void setFallback(const FontFlags* const fallback) {
			this->fallback = fallback;
		}

		[[nodiscard]] std::string fullname() const {
			return fontFile.stem() + "-" + styleName;
		}

		[[nodiscard]] OS::File fontCacheDir(const OS::File& rootCacheDir) const {
			return rootCacheDir.subFile(familyName);
		}

		[[nodiscard]] OS::File dataFile(const OS::File& cacheDir) const {
			return cacheDir.subFile(fullname() + "-" + std::to_string(size) + static_cast<std::string>(data_suffix));
		}

		[[nodiscard]] OS::File texFile(const OS::File& cacheDir) const {
			return cacheDir.subFile(fullname() + "-" + std::to_string(size) + static_cast<std::string>(tex_suffix ));
		}

		template <typename ...T>
			requires (std::same_as<Style, T> && ...)
		[[nodiscard]] bool styleOf(T ...styles) const {
			return ((static_cast<unsigned char>(styles) & style()) && ...);
		}

		[[nodiscard]] unsigned char style() const {
			return static_cast<unsigned char>(internalID >> styleOffset);
		}

		[[nodiscard]] unsigned char familyID() const {
			return static_cast<unsigned char>(internalID >> familyOffset);
		}

		[[nodiscard]] unsigned char fontID() const {
			return static_cast<unsigned char>(internalID >> fontOffset);
		}

		[[nodiscard]] FT_UInt familyFallback() const {
			constexpr FT_UInt mask = ~(0x0000'00ff << styleOffset); //0x0000'0000__0000'00ff__0000'0000__0000'0000
			return internalID & mask | static_cast<FT_UInt>(Style::regualr) << styleOffset;
		}

		void freeFontFace() {
			if(!face)FT_Done_Face(face);
			face = nullptr;
		}

		~FontFlags() {
			if(!face)FT_Done_Face(face);
			face = nullptr;
		}
	};

	//this will contain all the fonts with a single texture, for fast batch process
	struct FontAtlas {
	protected:
		std::unique_ptr<GL::Texture2D> fontTexture{nullptr};
		std::unordered_map<FT_UInt, std::set<FT_UInt>> supportedFonts{};
		std::unordered_map<FT_UInt, std::unique_ptr<FontFlags>> fonts{}; //Access it by FontFlags.internalID

	public:
		[[nodiscard]] FontAtlas() = default;

		[[nodiscard]] explicit FontAtlas(Graphic::Pixmap& textureBitmap, std::vector<std::unique_ptr<FontFlags>>& fontsRaw){
			fontTexture.reset(new GL::Texture2D(textureBitmap.getWidth(), textureBitmap.getHeight(), std::move(textureBitmap).release()));

			fontTexture->setScale(GL::TexParams::mipmap_linear_linear);

			const OrthoRectUInt bound{fontTexture->getWidth(), fontTexture->getHeight()};

			fonts.reserve(fontsRaw.size());
			for(auto& value: fontsRaw) {
				//Register valid chars
				for (const auto [from, to] : value->segments){
					for(CharCode i = from; i <= to; ++i) {
						supportedFonts[i].insert(value->internalID);
					}
				}

				//Set font char texture region uv
				for(auto& charData : value->data->charDatas | std::ranges::views::values) {
					charData.region.setData(*fontTexture);

					charData.region.fetchInto(charData.charBox, bound);
					charData.region.flipY();
					if(value->data->lineSpacingMin < 0) {
						value->data->lineSpacingMin = std::max(static_cast<float>(charData.charBox.getHeight()), value->data->lineSpacingMin);
					}
				}

				//The pixmap data for a single font wont be needed anymore in all cases, release it if possible;
				if(value->data->fontPixmap)value->data->fontPixmap->free();

				fonts[value->internalID] = std::move(value);
			}
		}

		[[nodiscard]] const GL::Texture2D* getTexture() const {
			return fontTexture.get();
		}

		[[nodiscard]] const FontFlags* obtain(const FontFlags* const flag) const {
			return obtain(flag->internalID);
		}

		[[nodiscard]] const FontFlags* obtain(const FT_UInt id) const {
			if(const auto itr = fonts.find(id); itr != fonts.end()) {
				return itr->second.get();
			}

			return nullptr;
		}

		[[nodiscard]] bool contains(const FT_UInt id, const CharCode charCode) const {
			return supportedFonts.at(charCode).contains(id);
		}

		[[nodiscard]] const CharData* getCharData(const FT_UInt id, const CharCode charCode) const {
			return contains(id, charCode) ? &fonts.at(id)->data->charDatas.at(charCode) : nullptr;
		}
	};

	struct FontData_Preload{
		CharCode charCode{0};
		FT_Glyph_Metrics matrices{};
		OrthoRectUInt box{};
		Graphic::Pixmap pixmap{};

		[[nodiscard]] FontData_Preload() = default;

		[[nodiscard]] FontData_Preload(const CharCode charCode, const FT_GlyphSlot glyph) : // NOLINT(*-misplaced-const)
			charCode(charCode),
			matrices(glyph->metrics),
			box(glyph->bitmap.width, glyph->bitmap.rows),
			pixmap(glyph->bitmap.width, glyph->bitmap.rows)
		{
			if(glyph->bitmap.buffer) {
				writeIntoPixmap(glyph->bitmap, pixmap.getData());
			}
		}

		void write(std::ostream& ostream) const{
			ostream.write(reinterpret_cast<const char*>(&charCode), sizeof(charCode));
			ostream.write(reinterpret_cast<const char*>(&box	 ), sizeof(box     ));
			ostream.write(reinterpret_cast<const char*>(&matrices), sizeof(matrices));
		}

		void read(std::istream& istream){
			istream.read(reinterpret_cast<char*>(&charCode), sizeof(charCode));
			istream.read(reinterpret_cast<char*>(&box	  ), sizeof(box     ));
			istream.read(reinterpret_cast<char*>(&matrices), sizeof(matrices));
		}
	};

	class FontManager final : public ext::ProgressTask<void, Assets::AssetsTaskHandler>{
	public:
		bool quickInit = false;
		std::vector<std::unique_ptr<FontFlags>> flags{};
		Event::CycleSignalManager fontLoadListeners{};
		OS::File rootCacheDir{};
		std::unique_ptr<FontAtlas> manager{nullptr};
		Assets::TexturePackPage* fontAtlas{nullptr};

		[[nodiscard]] FontManager() = default;

		[[nodiscard]] explicit FontManager(OS::File&& root_cache_dir) :
			rootCacheDir(std::move(root_cache_dir)) {
		}

		[[nodiscard]] explicit FontManager(const OS::File& root_cache_dir) :
			rootCacheDir(root_cache_dir) {
		}

		FontFlags* registerFont(FontFlags* fontFlags) {
			flags.emplace_back(fontFlags);
			return fontFlags;
		}

		[[nodiscard]] bool valid() const {
			return manager != nullptr;
		}

	protected:
		void loadFont(FontFlags& params) const {
			const std::string fontFullName = params.fullname();

			if(!params.face)exitLoad(fontFullName);

			const auto fontCacheDir = params.fontCacheDir(rootCacheDir);

			bool needCache = false;

			if(!fontCacheDir.exist()) {
				needCache = true;
				fontCacheDir.createDirQuiet();
			}

			OS::File dataFile = params.dataFile(fontCacheDir);
			OS::File texFile  = params.texFile (fontCacheDir);

			std::fstream fstream;

			if(dataFile.exist()) {
				unsigned char version = 0;
				fstream = obtainStream(dataFile);
				readCacheVersion(fstream, version);

				if(version != params.expectedVersion) {
					needCache = true;
				}
			}else {
				dataFile.createFileQuiet();
				needCache = true;
				fstream = obtainStream(dataFile);
			}

			std::vector<FontData_Preload> fontDatas{};
			FT_UInt width = 0;
			FT_UInt height = 0;
			size_t size = 0;

			Graphic::Pixmap maxMap{};

			if(needCache){
				FT_UInt currentWidth = 0;
				FT_UInt maxHeight = 0;

				for (const auto [from, to] : params.segments){
					for(CharCode i = from; i <= to; ++i) {
						const FontFlags& valid = * params.tryLoad(i);

						if(!valid.face->glyph) {
							exitLoad(fontFullName);
						}

						if (params.loader && valid.face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
							if (params.loader(valid.face)) {
								exitLoad(fontFullName);
							}
						}

						auto& cur = fontDatas.emplace_back(i, valid.face->glyph);
						cur.box.move(currentWidth, 0);

						//TODO font linefeed to support many glyphs
						//TODO uses texture packer instead?
						maxHeight = std::max(maxHeight, valid.face->glyph->bitmap.rows);
						currentWidth += valid.face->glyph->bitmap.width + TexturePackGap;
					}
				}

				//TODO customized glyph support
				if(auto itr = std::ranges::find(fontDatas, '_', &FontData_Preload::charCode);
					itr != fontDatas.end()){
					if(!std::ranges::contains(fontDatas, ' ', &FontData_Preload::charCode)){
						FontData_Preload spaceData{};
						spaceData.box = itr->box;
						spaceData.box.setSrcX(currentWidth);
						spaceData.box.setWidth(itr->box.getWidth());
						spaceData.matrices = itr->matrices;
						spaceData.charCode = ' ';
						spaceData.pixmap.create(spaceData.box.getWidth(), spaceData.box.getHeight());
						spaceData.pixmap.clear();

						currentWidth += spaceData.box.getWidth() + TexturePackGap;

						fontDatas.push_back(std::move(spaceData));

						if(!std::ranges::contains(fontDatas, '\n', &FontData_Preload::charCode)){
							FontData_Preload lineFeedData{};
							lineFeedData.box = spaceData.box;
							lineFeedData.box.setSrcX(TexturePackGap);
							lineFeedData.box.setWidth(1);
							lineFeedData.matrices = spaceData.matrices;
							lineFeedData.matrices.width = TexturePackGap;
							lineFeedData.matrices.horiBearingX = 0;
							lineFeedData.matrices.horiAdvance = 0;

							lineFeedData.charCode = '\n';
							lineFeedData.pixmap.create(lineFeedData.box.getWidth(), lineFeedData.box.getHeight());
							lineFeedData.pixmap.clear();

							currentWidth += TexturePackGap;

							fontDatas.push_back(std::move(lineFeedData));
						}

						if(!std::ranges::contains(fontDatas, '\t', &FontData_Preload::charCode)){
							FontData_Preload lineFeedData{};
							lineFeedData.box = spaceData.box;
							lineFeedData.box.setSrcX(TexturePackGap);
							lineFeedData.box.setWidth(1);
							lineFeedData.matrices = spaceData.matrices;
							lineFeedData.matrices.width *= 4;
							lineFeedData.matrices.horiAdvance *= 4;

							lineFeedData.charCode = '\t';
							lineFeedData.pixmap.create(lineFeedData.box.getWidth(), lineFeedData.box.getHeight());
							lineFeedData.pixmap.clear();

							currentWidth += TexturePackGap;

							fontDatas.push_back(std::move(lineFeedData));
						}
					}
				}


				size = fontDatas.size();
				width = currentWidth;
				height = maxHeight;

				saveCacheVersion(fstream, params.version);
				saveCacheData(fstream, width, height, size);

				maxMap.create(width, height);

				currentWidth = 0;
				for(const auto& data : fontDatas) {
					data.write(fstream);

					maxMap.set(data.pixmap, currentWidth, 0);
					currentWidth += data.pixmap.getWidth() + TexturePackGap;
				}

				maxMap.write(texFile, true);
			}else{ //Load from cache
				//The version has been read during the check!
				readCacheData(fstream, width, height, size);

				fontDatas.resize(size);

				for(size_t i = 0; i < size; ++i) {
					fontDatas[i].read(fstream);
				}

				maxMap.loadFrom(texFile);
			}

			params.data.reset(new FontData{Geom::OrthoRectUInt{maxMap.getWidth(), maxMap.getHeight()}, size, std::move(maxMap)});

			for(auto& data: fontDatas) {
				params.data->charDatas.emplace(data.charCode, CharData{data.matrices, data.box});

				//Dont init it right now, the regions should be set at last!
			}
		}

		[[nodiscard]] bool checkCahce() const {
			bool requiresRecache = false;

			for(const auto& params: flags) {
				if(!params->face) {
					FT_Face face;

					if(FT_New_Face(GlobalFreeTypeLib, params->fontFile.absolutePath().string().data(), 0, &face)) {
						exitLoad(params->fullname());
					}

					params->face = face;

					//Correct the style name if necessary
					params->familyName = params->face->family_name;
					params->styleName = params->face->style_name;
				}

				if(requiresRecache)continue;

				const auto&& cacheDir = params->fontCacheDir(rootCacheDir);
				cacheDir.createDirQuiet();
				// ReSharper disable once CppTooWideScopeInitStatement
				const auto&& dataFile = params->dataFile(cacheDir);

				if(!dataFile.exist()) {
					requiresRecache = true;
				}else{
					unsigned char version = 0;
					readCacheVersion(obtainStream(dataFile), version);
					if(version != params->expectedVersion) {
						requiresRecache = true;
					}
				}
			}

			return requiresRecache;
		}

		[[nodiscard]] Graphic::Pixmap merge(bool requiresRecache) const {
			Graphic::Pixmap mergedMap{};

			const OS::File dataFile = rootCacheDir.subFile("merged.bin");
			const OS::File texFile  = rootCacheDir.subFile("merged.png");

			FT_UInt totalWidth = 0, totalHeight = 0;
			size_t size = 0;
			size_t totalCharCount = 0; //This is inaccurate!
			if(!dataFile.exist())dataFile.createFileQuiet(true);
			auto&& stream = obtainStream(dataFile);
			constexpr std::hash<std::string> hasher{};

			//If a font doesn't have cache, then cache it; or if the cache is too old, then recache;
			cache:
			if(requiresRecache) {
				size = flags.size();
				saveCacheData(stream, totalWidth, totalHeight, size);

				//Generate FontDatas from loadTargets
				for(const auto& params : flags) {
					loadFont(*params);

					totalWidth = std::max(params->data->box.getWidth(), totalWidth);
					totalHeight += params->data->box.getHeight() + TexturePackGap;
					const size_t hash = hasher(params->fullname());
					stream.write(reinterpret_cast<const char*>(&hash), sizeof(hash));
				}

				//Resize the merge map
				mergedMap.create(totalWidth, totalHeight);

				FT_UInt currentHeightOffset = 0;
				for(const auto& flag : flags) {
					const auto& data = flag->data;

					mergedMap.set(*data->fontPixmap, 0, currentHeightOffset);

					data->box.setSrcY(currentHeightOffset);
					for(auto& charData : data->charDatas | std::ranges::views::values) {
						charData.charBox.move(0, currentHeightOffset);
					}

					currentHeightOffset += data->box.getHeight() + TexturePackGap;

					totalCharCount = std::max(totalCharCount, data->charDatas.size());

					data->write(stream);
				}

				mergedMap.write(texFile, true);
			}else{
				//
				readCacheData(stream, totalWidth, totalHeight, size);

				//Check cache is right
				if(size != flags.size()) {
					requiresRecache = true;
					goto cache; //Recache if the size cannot match...
				}

				for(size_t i = 0; i < size; ++i) {
					size_t hash = 0;
					stream.read(reinterpret_cast<char*>(&hash), sizeof(hash));
					if(hasher(flags[i]->fullname()) != hash) {
						requiresRecache = true;
						goto cache; //Recache if the font cannot match...
					}
				}
				//End Chec

				//Load font data from bin file
				for(size_t i = 0; i < size; ++i) {
					auto& data = flags[i]->data;
					data.reset(new FontData);
					data->read(stream);
					totalCharCount = std::max(totalCharCount, data->charDatas.size());
				}

				//Load font tex from png file
				mergedMap.loadFrom(texFile);
			}

			return mergedMap;
		}

		void assignID() const {
			std::vector<std::string> loadedFamily;
			for(size_t i = 0; i < flags.size(); ++i) {
				// ReSharper disable once CppUseStructuredBinding
				auto& flag = *flags[i];

				const auto itr = std::ranges::find(loadedFamily, flag.familyName);
				const size_t dst = itr - loadedFamily.cbegin();

				if(itr == loadedFamily.end()){
					loadedFamily.push_back(flag.familyName);
				}

				flag.internalID |= static_cast<unsigned char>(i) << FontFlags::fontOffset;
				flag.internalID |= static_cast<unsigned char>(dst) << FontFlags::familyOffset;
				flag.internalID |= getStyleID(flag.styleName) << FontFlags::styleOffset;
			}
		}

	public:
		void load() { //Dose not support hot load!
			//User Customized fonts to load;
			fontLoadListeners.fire(Event::begin);

			//Init face data;
			const bool requiresRecache = checkCahce();
			setProgress(0.15f);

			//Cache prepare
			Graphic::Pixmap&& mergedMap = merge(requiresRecache);
			setProgress(0.65f);

			assignID();
			setProgress(0.8f);

			//TODO is this necessary?
			for (const auto& flag : flags) {
				flag->freeFontFace();
			}

			//Now only [loadTargets, mergedMap] matters. All remain operations should be done in the memory;
			//FontsManager obtain the texture from the total cache; before this no texture should be generated!
			if(quickInit) {
				manager = std::make_unique<FontAtlas>(mergedMap, flags);
			}else {
				postToHandler([&mergedMap, this] {
					manager = std::make_unique<FontAtlas>(mergedMap, flags);
				}).get();
			}


			setProgress(0.95f);


			//FontsManager init its member's TextureRegion data;

			//...

			//Release resouces

			fontLoadListeners.fire(Event::end);
			setProgress(1.0f);

			setDone();

			//Load End
		}

	public:
		std::unique_ptr<FontAtlas> getManager() && {
			return std::move(manager);
		}

		[[nodiscard]] std::future<void> launch(const std::launch policy) override{
			return std::async(policy, &FontManager::load, this);
		}

		[[nodiscard]] std::string_view getTaskName() const override {
			return "Loading Fonts.";
		}

		FontManager(const FontManager& other) = delete;

		FontManager(FontManager&& other) noexcept = delete;

		FontManager& operator=(const FontManager& other) = delete;

		FontManager& operator=(FontManager&& other) noexcept = delete;
	};
}
