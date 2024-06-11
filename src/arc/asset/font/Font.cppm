module;

#include <ft2build.h>
#include <freetype/freetype.h>

export module Font;

import std;

import GL.Texture.TextureRegion;
import Geom.Rect_Orthogonal;
import Geom.Vector2D;
import Graphic.Pixmap;
import ext.RuntimeException;
import OS.File;
import Math;
import ext.Heterogeneous;

using namespace Geom;
using Graphic::Pixmap;

namespace Font {
	void exitLoad(const std::string& fontName) {
		throw ext::RuntimeException{"Font Load Failed : " + fontName};
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
	using FontID = FT_UInt;

	constexpr CharCode ErrorFallbackCode = 0xF0000;

	FT_Library GlobalFreeTypeLib{nullptr};

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

	void loadGlobalFreeTypeLib() {
		if(FT_Init_FreeType(&GlobalFreeTypeLib)) {
			throw ext::RuntimeException{"Failed to initialize FreeType Library!"};
		}
	}

	struct CharData {
		FT_Glyph_Metrics glyphMatrices{};
		GL::TextureRegion* region{};

		[[nodiscard]] CharData() = default;

		[[nodiscard]] explicit CharData(const FT_Glyph_Metrics& glyphMatrices)
			: glyphMatrices{glyphMatrices}{}

		template <Concepts::Number T>
		[[nodiscard]] constexpr Geom::Vector2D<T> getSize() const{
			return {normalizeLen<T>(glyphMatrices.width), normalizeLen<T>(glyphMatrices.height)};
		}
	};

	//TODO Should this be exported?
	struct FontData_Preload{
		CharCode charCode{0};
		mutable CharData charData{};
		Graphic::Pixmap pixmap{};

		[[nodiscard]] FontData_Preload() = default;

		[[nodiscard]] FontData_Preload(const CharCode charCode, const FT_GlyphSlot glyph) : // NOLINT(*-misplaced-const)
			charData(glyph->metrics),
			charCode(charCode),
			pixmap(glyph->bitmap.width, glyph->bitmap.rows)
		{
			if(glyph->bitmap.buffer) {
				writeIntoPixmap(glyph->bitmap, pixmap.data());
			}
		}

		FontData_Preload(const CharCode charCode, const FT_Glyph_Metrics& glyphMatrices, Graphic::Pixmap&& pixmap)
			: charCode{charCode},
			  charData{glyphMatrices},
			  pixmap{std::move(pixmap)}{}

		[[nodiscard]] Geom::Point2U getSize() const{
			return {normalizeLen<unsigned>(charData.glyphMatrices.width), normalizeLen<unsigned>(charData.glyphMatrices.height)};
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



namespace Font{
	export
	struct FontData {
		std::unordered_map<CharCode, CharData> charDatas{};
		float lineSpacingDef{-1};

		explicit FontData(const std::unordered_map<CharCode, FontData_Preload>& datas){
			for (auto&& data : datas | std::views::values){
				charDatas.try_emplace(data.charCode, data.charData);
			}
		}
	};

	export
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

	export
	constexpr CharData emptyCharData{};

	export
	struct FontFace {
		static constexpr auto styleOffset = 16;
		static constexpr auto familyOffset = 8;
		static constexpr auto fontOffset = 0;

		OS::File fontFile{};
		std::vector<CharCode> segments{};
		FT_Int loadFlags = FT_LOAD_RENDER;
		FT_UInt size = 48;

		unsigned char version{};
		unsigned char expectedVersion{};

		FT_Face face{nullptr};

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
		FontID fullID{0}; //This should be assigned by internal operation!

		std::unique_ptr<FontData> data{nullptr};
		std::vector<CustomeCharData> customeCharDatas{};
		std::function<bool(FT_Face)> loader = nullptr;

		const FontFace* fallback{nullptr};

		[[nodiscard]] FontFace(
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

		[[nodiscard]] FontFace(
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
						return const_cast<FontFace*>(fallback)->tryLoad(charCode);
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

		[[nodiscard]] const FontFace* getFallback() const {
			return fallback;
		}

		void setFallback(const FontFace* const fallback) {
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
			return static_cast<unsigned char>(fullID >> styleOffset);
		}

		[[nodiscard]] unsigned char familyID() const {
			return static_cast<unsigned char>(fullID >> familyOffset);
		}

		[[nodiscard]] unsigned char fontID() const {
			return static_cast<unsigned char>(fullID >> fontOffset);
		}

		[[nodiscard]] FontID familyFallback() const {
			constexpr FontID mask = static_cast<FontID>(~(0x0000'00ff << styleOffset)); //0x0000'0000__0000'00ff__0000'0000__0000'0000
			return fullID & mask | static_cast<FontID>(Style::regualr) << styleOffset;
		}

		void freeFontFace() {
			if(!face)FT_Done_Face(face);
			face = nullptr;
		}

		FontFace(const FontFace& other) = delete;

		FontFace(FontFace&& other) noexcept = delete;

		FontFace& operator=(const FontFace& other) = delete;

		FontFace& operator=(FontFace&& other) noexcept = delete;

		~FontFace() {
			if(!face)FT_Done_Face(face);
			face = nullptr;
		}
	};
}

