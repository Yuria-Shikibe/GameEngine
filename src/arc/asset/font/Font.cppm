//
// Created by Matrix on 2023/11/27.
//

module;

#include <ft2build.h>
#include <functional>
#include <freetype/freetype.h>

export module Font;

import <vector>;
import <memory>;
import <string>;
import <fstream>;
import <unordered_map>;
import GL.Texture.Texture2D;
import GL.Texture.TextureRegionRect;
import GL.Texture.TextureRegion;
import Geom.Shape.Rect_Orthogonal;
import Geom.Vector2D;
import Graphic.Pixmap;
import RuntimeException;
import File;
import Event;
import Core;

using namespace Geom;

export namespace Font {
	struct LoadParams;

	FT_Library freeTypeLib;

	constexpr std::string_view regular = "regular";
	constexpr std::string_view bold = "bold";
	constexpr std::string_view italic = "italic";
	constexpr std::string_view bold_italic = "bold_italic";

	constexpr std::string_view data_suffix = ".bin";
	constexpr std::string_view tex_suffix  = ".png";

	std::vector<LoadParams> loadTargets{};

	void writeIntoPixmap(const FT_Bitmap& map, unsigned char* data) {
		for(size_t size = 0; size < map.width * map.rows; ++size) {
			data[size * 4 + 0] = 0xff;
			data[size * 4 + 1] = 0xff;
			data[size * 4 + 2] = 0xff;
			data[size * 4 + 3] = map.buffer[size];
		}
	}

	enum class FontLoadState {
		begin, end,
		max
	};

	Event::EventManager_Quick<FontLoadState, FontLoadState::max> fontLoadListeners{};

	struct LoadParams {
		FT_Int loadFlags = FT_LOAD_RENDER;
		FT_UInt height = 48;

		unsigned char expectedVersion = 0;

		std::string variation = static_cast<std::string>(regular);
		std::string fontStemName{};

		OS::File fontFile{};
		OS::File rootCacheDir{};

		std::vector<FT_ULong> segments{};

		[[nodiscard]] std::string fullname() const {
			return fontFile.stem() + "-" + variation;
		}

		std::function<bool(FT_Face)> loader = [](const FT_Face face) {
			return FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		};
	};

	struct FontData_Preload{
		FT_ULong charCode{0};
		FT_Glyph_Metrics matrices{};
		Shape::OrthoRectUInt box{};
		Graphic::Pixmap bitmap{};

		[[nodiscard]] FontData_Preload(const FT_UInt width, const FT_UInt height) : box(width, height), bitmap(width, height) {

		}

		[[nodiscard]] explicit FontData_Preload(const FT_ULong charCode, const FT_GlyphSlot glyph) : charCode(charCode),  box(glyph->bitmap.width, glyph->bitmap.rows), bitmap(glyph->bitmap.width, glyph->bitmap.rows), matrices(glyph->metrics){ // NOLINT(*-misplaced-const)
			writeIntoPixmap(glyph->bitmap, bitmap.getData());
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

	struct FontData {
		std::unordered_map<FT_ULong, Shape::OrthoRectUInt> boxes{};
		std::unordered_map<FT_ULong, std::pair<GL::TextureRegionRect, FT_Glyph_Metrics>> charRegions{};

		[[nodiscard]] explicit FontData(const size_t size) {
			charRegions.reserve(size);
			boxes.reserve(size);
		};
	};

	//single font data, may have multi types like [regular, bold, italic...]
	class Font {
	public:
		OS::File fontDir{};
		std::string fontName{"null"};
		std::unordered_map<std::string, std::unique_ptr<FontData>> datas;

		FontData* defaultFont{nullptr}; //Should be regular

		explicit Font(const std::string& name, const std::unordered_map<std::string, std::unique_ptr<FontData>>& datas) : fontName(name), datas(datas) {
			if(
				const auto itr = datas.find(static_cast<std::string>(regular));
				itr != datas.end()
			) {
				defaultFont = itr->second.get();
			}else {
				defaultFont = datas.begin()->second.get();
			}
		}
	};

	//this will contain all the fonts with a single texture, for fast batch process
	class FontsManager {
		std::unique_ptr<GL::Texture2D> fontTexture{nullptr};
	};

	std::fstream obtainStream(const OS::File& file) {
		return std::fstream{file.absolutePath(), std::ios::binary | std::ios::in | std::ios::out};
	}

	void readCacheVersion(std::istream& stream, unsigned char& version) {
		stream.read(reinterpret_cast<char*>(&version), sizeof(version));
	}

	void saveCacheVersion(std::ostream& stream, const unsigned char& version) {
		stream.write(reinterpret_cast<const char*>(&version), sizeof(version));
	}

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

	std::unique_ptr<FontData> loadFont(LoadParams& params) {
		const std::string fontFullName = params.fullname();
		const auto fontCacheDir = params.rootCacheDir.subFile(params.fontStemName);

		bool needCache = false;

		if(!fontCacheDir.exist()) {
			needCache = true;
			fontCacheDir.createDirQuiet();
		}

		FT_Face face;
		if(FT_New_Face(freeTypeLib, params.fontFile.absolutePath().string().data(), 0, &face)) {
			goto exit;
		}

		//Correct the style name if necessary
		params.fontStemName = face->family_name;
		params.variation = face->style_name;

		OS::File dataFile = fontCacheDir.subFile(fontFullName + static_cast<std::string>(data_suffix));
		OS::File texFile  = fontCacheDir.subFile(fontFullName + static_cast<std::string>(tex_suffix ));

		auto fstream = obtainStream(dataFile);

		if(dataFile.exist()) {
			unsigned char version = 0;
			fstream.read(reinterpret_cast<char*>(&version), sizeof(version));

			if(version != params.expectedVersion) {
				needCache = true;
			}
		}else {
			dataFile.createFileQuiet();
			needCache = true;
		}

		std::vector<FontData_Preload> fontDatas{};
		FT_UInt width = 0;
		FT_UInt height = 0;
		size_t size = 0;
		Graphic::Pixmap maxMap{};

		if(needCache) {
			FT_Set_Pixel_Sizes(face, 0, params.height);

			FT_UInt currentWidth = 0;
			FT_UInt maxHeight = 0;

			for(size_t t = 0; t < params.segments.size() / 2; ++t) {
				for(FT_ULong i = params.segments[t * 2]; i <= params.segments[t * 2 + 1]; ++i) {
					if(FT_Load_Char(face, i, params.loadFlags))goto exit;

					if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
						if (params.loader(face)) {
							goto exit;
						}
					}

					fontDatas.emplace_back(i, face->glyph);
					fontDatas.back().box.move(currentWidth, 0);

					maxHeight = std::max(maxHeight, face->glyph->bitmap.rows);
					currentWidth += face->glyph->bitmap.width;
				}
			}

			size = fontDatas.size();
			width = currentWidth;
			height = maxHeight;

			saveCacheData(fstream, width, height, size);

			maxMap.create(width, height);

			currentWidth = 0;
			for(const auto& data : fontDatas) {
				data.write(fstream);
				maxMap.set(data.bitmap, currentWidth, 0);
				currentWidth += data.bitmap.getWidth();
			}
		}else{ //Load from cache
			readCacheData(fstream, width, height, size);

			fontDatas.resize(size);

			for(size_t i = 0; i < size; ++i) {
				fontDatas[i].read(fstream);
			}

			maxMap = Graphic::Pixmap{texFile};
		}

		auto* fontLoadout = new FontData{size};

		for(auto& data: fontDatas) {
			fontLoadout->boxes.emplace(data.charCode, data.box);
			fontLoadout->charRegions.emplace(data.charCode, std::make_pair(TextureRegionRect{}, data.matrices));

			//Dont init it right now, the regions should be set at last!
		}

		return std::unique_ptr<FontData>(fontLoadout);

		exit: {
			throw ext::RuntimeException{"Font Load Failed : " + params.fullname()};
		}
	}

	void loadLib() {
		if(FT_Init_FreeType(&freeTypeLib)) {
			throw ext::RuntimeException{"Unable to initialize FreeType Library!"};
		}
	}

	void checkUpdate() {

	}

	void load() { //Dose not support hot load!
		//FT Lib init;

		loadLib();

		//User Customized fonts to load

		fontLoadListeners.fire(FontLoadState::begin);

		//If a font doesn't have cache, then cache it; or if the cache is too old, then recache;



		//If one font cache updated, then recache the total cache;


		//FontsManager obtain the texture from the total cache; before this no texture should be generated!


		//FontsManager init its member's TextureRegion data;


		//Release resouces

		fontLoadListeners.fire(FontLoadState::end);

		//Load End
	}
}