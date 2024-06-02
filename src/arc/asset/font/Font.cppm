module;

#include <ft2build.h>
#include <freetype/freetype.h>

export module Font;

import std;

import ext.Async;
import Assets.Loader;

import GL.Texture.Texture2D;
import GL.Texture.TextureRegion;
import Assets.TexturePacker;
import Geom.Rect_Orthogonal;
import Geom.Vector2D;
import Graphic.Pixmap;
import ext.RuntimeException;
import OS.File;
import Image;
import Event;
import Math;
import ext.Heterogeneous;

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
	void readCacheDataSize(std::istream& stream, size_t& size) {
		stream.read(reinterpret_cast<char*>(&size  ), sizeof(size  ));
	}

	void saveCacheDataSize(std::ostream& stream, const size_t size) {
		stream.write(reinterpret_cast<const char*>(&size  ), sizeof(size  ));
	}

	void writeIntoPixmap(const FT_Bitmap& map, unsigned char* data) {
		for(unsigned size = 0; size < map.width * map.rows; ++size) {
			//Normal
			data[size * 4 + 0] = 0xff;
			data[size * 4 + 1] = 0xff;
			data[size * 4 + 2] = 0xff;
			data[size * 4 + 3] = map.buffer[size];
		}
	}
}

export namespace Font{
	template <Concepts::Number T = float>
	constexpr T normalizeLen(const long pos) {
		return static_cast<T>(pos >> 6);
	}

	[[nodiscard]] Geom::Point2U getGlyphSize(const FT_Glyph_Metrics& metrics){
		return {normalizeLen<unsigned>(metrics.width), normalizeLen<unsigned>(metrics.height)};
	}

	using CharCode = char32_t;

	constexpr CharCode ErrorFallbackCode = 0xF0000;

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
		FT_Glyph_Metrics glyphMatrices{};
		GL::TextureRegion* region{};

		template <Concepts::Number T>
		[[nodiscard]] constexpr Geom::Vector2D<T> getSize() const{
			return {normalizeLen<T>(glyphMatrices.width), normalizeLen<T>(glyphMatrices.height)};
		}
	};

	//TODO Should this be exported?
	struct FontData_Preload{
		FT_Glyph_Metrics glyphMatrices{};
		CharCode charCode{0};
		Graphic::Pixmap pixmap{};

		mutable GL::TextureRegion* region{};

		[[nodiscard]] FontData_Preload() = default;

		[[nodiscard]] FontData_Preload(const CharCode charCode, const FT_GlyphSlot glyph) : // NOLINT(*-misplaced-const)
			charCode(charCode),
			glyphMatrices(glyph->metrics),
			pixmap(glyph->bitmap.width, glyph->bitmap.rows)
		{
			if(glyph->bitmap.buffer) {
				writeIntoPixmap(glyph->bitmap, pixmap.data());
			}
		}

		FontData_Preload(const FT_Glyph_Metrics& glyphMatrices, const CharCode charCode, Graphic::Pixmap&& pixmap)
			: glyphMatrices{glyphMatrices},
			  charCode{charCode},
			  pixmap{std::move(pixmap)}{}

		[[nodiscard]] Geom::Point2U getSize() const{
			return {normalizeLen<unsigned>(glyphMatrices.width), normalizeLen<unsigned>(glyphMatrices.height)};
		}

		void write(std::ostream& ostream) const{
			ostream.write(reinterpret_cast<const char*>(&charCode), sizeof(charCode));
			ostream.write(reinterpret_cast<const char*>(&glyphMatrices), sizeof(glyphMatrices));
		}

		void read(std::istream& istream){
			istream.read(reinterpret_cast<char*>(&charCode), sizeof(charCode));
			istream.read(reinterpret_cast<char*>(&glyphMatrices), sizeof(glyphMatrices));
		}

		friend bool operator==(const FontData_Preload& lhs, const FontData_Preload& rhs){
			return lhs.charCode == rhs.charCode;
		}

		friend bool operator!=(const FontData_Preload& lhs, const FontData_Preload& rhs){ return !(lhs == rhs); }
	};
}

export
template<>
struct std::hash<Font::FontData_Preload>{
	size_t operator()(const Font::FontData_Preload& d) const noexcept{
		static constexpr std::hash<Font::CharCode> hasher{};

		return hasher(d.charCode);
	}
};

using PreloadDataSet = std::unordered_set<Font::FontData_Preload,
ext::MemberHasher<Font::FontData_Preload, &Font::FontData_Preload::charCode>,
ext::MemberEqualTo<Font::FontData_Preload, &Font::FontData_Preload::charCode>>;

export namespace Font{
	struct FontData {
		std::unordered_map<CharCode, CharData> charDatas{};
		float lineSpacingDef{-1};

		explicit FontData(const PreloadDataSet& datas){
			for (auto&& data : datas){
				charDatas.try_emplace(data.charCode, data.glyphMatrices, data.region);
			}
		}

		void write(std::ostream& ostream) const{
			const size_t size = charDatas.size();
			ostream.write(reinterpret_cast<const char*>(&size), sizeof(size));

			for(const auto& [key, value]: charDatas) {
				ostream.write(reinterpret_cast<const char*>(&key		   ), sizeof(key           ));
				ostream.write(reinterpret_cast<const char*>(&value.glyphMatrices), sizeof(value.glyphMatrices));
			}
		}

		void read(std::istream& istream){
			size_t size = 0;
			istream.read(reinterpret_cast<char*>(&size), sizeof(size));
			charDatas.reserve(size);

			for(size_t i = 0; i < size; ++i) {
				CharCode key;
				istream.read(reinterpret_cast<char*>(&key		    ), sizeof(key           ));
				auto [kv, success] = charDatas.try_emplace(key);

				istream.read(reinterpret_cast<char*>(&kv->second.glyphMatrices), sizeof(kv->second.glyphMatrices));
			}
		}
	};

	struct CustomeCharData{
		CharCode code{};
		Graphic::Pixmap data{};

		bool forceOverride = false;
		CharCode copyTarget{0};
		FT_Glyph_Metrics glyphMatrices{};

		std::function<void(CustomeCharData&)> dataModifier{nullptr};

		[[nodiscard]] bool hasCopyTarget() const{
			return copyTarget != 0;
		}

		void createBlankGlyph(){
			auto [x, y] = getGlyphSize(glyphMatrices);
			data.create(Math::max(x, 1u), Math::max(y, 1u));
		}
	};

	constexpr CharData emptyCharData{};

	struct FontFlags {
		static constexpr auto styleOffset = 16;
		static constexpr auto familyOffset = 8;
		static constexpr auto fontOffset = 0;

		OS::File fontFile{};
		std::vector<CharCode> segments{};
		FT_Int loadFlags = FT_LOAD_RENDER; //
		FT_UInt size = 48;

		unsigned char version{};


		FT_Face face{nullptr};
		unsigned char expectedVersion{};

		std::string styleName{regular};
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

		std::vector<CustomeCharData> customeCharDatas{};

		std::function<bool(FT_Face)> loader = nullptr;

		const FontFlags* fallback{nullptr};



		[[nodiscard]] FontFlags(
			const OS::File& fontFile,
			const std::vector<CharCode>& segments,
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
			genDefNecessaryCharData();
		}

		[[nodiscard]] FontFlags(
			OS::File&& fontFile,
			const std::vector<CharCode>& segments,
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
			genDefNecessaryCharData();
		}


		FT_Face tryLoad(const CharCode charCode){
			if(!face) { //Fall back may roll to font that doesn't need cache, just load its face
				if(FT_New_Face(GlobalFreeTypeLib, fontFile.absolutePath().string().data(), 0, &face)) {
					exitLoad(fullname());
				}
			}

			if(const auto state = FT_Load_Char(face, charCode, loadFlags); state != 0) {
				if(state == FT_Err_Invalid_Glyph_Index) {
					if(fallback){ //Seriously I don't want to use const cast
						return const_cast<FontFlags*>(fallback)->tryLoad(charCode);
					}

					const std::string str = FT_Error_String(state);
					std::println(std::cout, "Exception While Loading Font: {} | {}", fullname(), str);
					return nullptr;
				}
			}

			return face;
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

			if(const auto itr = data->charDatas.find(ErrorFallbackCode); itr != data->charDatas.end()) {
				return &itr->second;
			}

			throw ext::RuntimeException{std::format("Failed To Find Glyph: {}", std::to_string(charCode))};
		}

		void setDefErrorFallback(const Graphic::Pixmap& errorTex){
			customeCharDatas.push_back({
				.code = ErrorFallbackCode,
				.data = errorTex,
				.copyTarget = U'A',
			});
		}

		/**
		 * @brief This function is recommended to invoke to complete charcode like '\n', '\t', ' ', from '_'
		 */
		void genDefNecessaryCharData(){
			customeCharDatas.push_back({
				.code = U' ',
				.copyTarget = U'_',
				.dataModifier = &CustomeCharData::createBlankGlyph
			});

			customeCharDatas.push_back({
				.code = U'\n',
				.copyTarget = U'_',
				.dataModifier = [](CustomeCharData& data){
					data.glyphMatrices.width /= 4;
					data.glyphMatrices.horiBearingX /= 4;
					data.glyphMatrices.horiAdvance /= 4;
					data.createBlankGlyph();
				}
			});

			customeCharDatas.push_back({
				.code = U'\t',
				.copyTarget = U'_',
				.dataModifier = [](CustomeCharData& data){
					data.glyphMatrices.width *= 4;
					data.glyphMatrices.horiAdvance *= 4;
					data.createBlankGlyph();
				}
			});

			customeCharDatas.push_back({
				.code = U'\2',
				.copyTarget = U'_',
				.dataModifier = [](CustomeCharData& data){
					data.glyphMatrices.width /= 4;
					data.glyphMatrices.horiAdvance /= 4;
					data.createBlankGlyph();
				}
			});
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
			constexpr FT_UInt mask = static_cast<FT_UInt>(~(0x0000'00ff << styleOffset)); //0x0000'0000__0000'00ff__0000'0000__0000'0000
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
		std::unordered_map<CharCode, std::set<FT_UInt>> supportedFonts{};
		std::unordered_map<FT_UInt, std::unique_ptr<FontFlags>> fonts{}; //Access it by FontFlags.internalID

	public:
		[[nodiscard]] FontAtlas() = default;

		[[nodiscard]] explicit FontAtlas(std::vector<std::unique_ptr<FontFlags>>& fontsRaw){

			// fontTexture->setScale(GL::TexParams::mipmap_linear_linear);

			fonts.reserve(fontsRaw.size());
			for(auto& value: fontsRaw) {
				//Register valid chars
				for (const auto i : value->segments){
					supportedFonts[i].insert(value->internalID);
				}

				if(value->data->lineSpacingDef < 0) {
					value->data->lineSpacingDef = normalizeLen(value->face->size->metrics.ascender);
				}

				//The pixmap data for a single font wont be needed anymore in all cases, release it if possible;
				// if(value->data->fontPixmap)value->data->fontPixmap->free();

				fonts[value->internalID] = std::move(value);
			}
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

	class FontManager final : public ext::ProgressTask<void, Assets::AssetsTaskHandler>{
	public: //TODO no public
		bool quickInit = false;
		std::vector<std::unique_ptr<FontFlags>> flags{};
		Event::CycleSignalManager fontLoadListeners{};
		OS::File rootCacheDir{};
		std::unique_ptr<FontAtlas> atlas{nullptr};
		Assets::TexturePackPage* texturePage{nullptr};

		[[nodiscard]] FontManager() = default;

		[[nodiscard]] FontManager(OS::File&& root_cache_dir, Assets::TexturePackPage* texturePage) :
			rootCacheDir(std::move(root_cache_dir)), texturePage{texturePage}{
		}

		[[nodiscard]] FontManager(const OS::File& root_cache_dir, Assets::TexturePackPage* texturePage) :
			rootCacheDir(root_cache_dir), texturePage{texturePage}{
		}

		FontFlags* registerFont(FontFlags* fontFlags) {
			flags.emplace_back(fontFlags);
			return fontFlags;
		}

		[[nodiscard]] bool valid() const {
			return atlas != nullptr;
		}

	private:
		void loadFont(FontFlags& params) const{
			const std::string fontFullName = params.fullname();

			if(!params.face)exitLoad(fontFullName);

			const auto fontCacheDir = params.fontCacheDir(rootCacheDir);

			bool needCache = false;

			if(!fontCacheDir.exist()) {
				needCache = true;
				fontCacheDir.createDirQuiet();
			}

			OS::File dataFile = params.dataFile(fontCacheDir);
			//OS::File texFile  = params.texFile (fontCacheDir);

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

			PreloadDataSet fontDatas{};

			// Graphic::Pixmap maxMap{};
			FT_Set_Pixel_Sizes(params.face, 0, params.size);

			if(needCache){
				for (const CharCode i : params.segments){
					FT_Face face = params.tryLoad(i);

					if(face){
						if(!face->glyph) {
							exitLoad(fontFullName);
						}

						//TODO glyph effects
						//TODO should this thing be here???
						if (params.loader && face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
							if (params.loader(face)) {
								exitLoad(fontFullName);
							}
						}

						fontDatas.emplace(i, face->glyph);
					}
				}

				for(auto& customeData : params.customeCharDatas){
					if(!customeData.forceOverride && fontDatas.contains(customeData.code))continue;

					if(customeData.hasCopyTarget()){
						if(auto itr = fontDatas.find(customeData.copyTarget); itr != fontDatas.end()){
							customeData.glyphMatrices = itr->glyphMatrices;
						}
					}

					if(customeData.dataModifier){
						customeData.dataModifier(customeData);
					}

					fontDatas.emplace(customeData.glyphMatrices, customeData.code, std::move(customeData.data));
				}

				for (auto& fontData : fontDatas){
					fontData.region = texturePage->pushRequest(fontFullName + std::to_string(fontData.charCode), fontData.pixmap, [](const Graphic::Pixmap& pixmap){pixmap.flipY();});
				}

				saveCacheVersion(fstream, params.version);
				saveCacheDataSize(fstream, fontDatas.size());

				//maxMap.create(width, height);
				for(const auto& data : fontDatas) {
					data.write(fstream);
				}
			}else{
				size_t size = 0;
				//Load from cache
				//The version has been read during the check!
				readCacheDataSize(fstream, size);
				fontDatas.reserve(size);
				// fontDatas.resize(size);

				for(size_t i = 0; i < size; ++i){
					FontData_Preload data{};
					data.read(fstream);
					data.pixmap.create(1, 1);
					data.region = texturePage->pushRequest(fontFullName + std::to_string(data.charCode), data.pixmap);
					fontDatas.insert(std::move(data));
				}
			}

			params.data = std::make_unique<FontData>(fontDatas);
		}

		void loadAllFace() const {
			for(const auto& params: flags){
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
			}
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

			loadAllFace();
			//Init face data;
			setProgress(0.15f);

			//Cache prepare
			for(const auto& params : flags) {
				loadFont(*params);
			}

			texturePage->setMargin(2);
			texturePage->launch(std::launch::deferred).get();

			setProgress(0.65f);

			assignID();
			setProgress(0.8f);

			//Now only [loadTargets, mergedMap] matters. All remain operations should be done in the memory;
			//FontsManager obtain the texture from the total cache; before this no texture should be generated!
			if(quickInit) {
				atlas = std::make_unique<FontAtlas>(flags);
			}else {
				postToHandler([this] {
					atlas = std::make_unique<FontAtlas>(flags);
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

		std::unique_ptr<FontAtlas> getManager() && {
			return std::move(atlas);
		}

		[[nodiscard]] std::future<void> launch(const std::launch policy) override{
			(void)Task::launch(policy);
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

