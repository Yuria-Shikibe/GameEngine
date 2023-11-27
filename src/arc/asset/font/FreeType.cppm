//
// Created by Matrix on 2023/11/26.
//

module;

#include <ft2build.h>
#include <freetype/freetype.h>

export module Font.FreeType;

import <glad/glad.h>;
import <iostream>;
import <memory>;
import <ranges>;
import <vector>;
import <algorithm>;
import <fstream>;
import <numeric>;
import <stacktrace>;
import <unordered_map>;
import <utility>;

import GlyphArrangment;
import File;
import Geom.Shape.Rect_Orthogonal;
import Geom.Vector2D;
import GL.Texture.Texture2D;
import GL.Texture.TextureRegionRect;
import Graphic.Pixmap;
import Image;
import RuntimeException;
import Math.BinPacker2D;
import Math;

export
/**
 * @brief write cache format:
 * @code
 *		Head[ulong segment count] N
 *		Segments[@link std::pair<@link FT_ULong @endlink,@link FT_ULong @endlink>@endlink] N*
 *
 * @endcode
 */

//TODO this namespace has too many repeated codes, rebuild it if possilbe
namespace Font::FT {
	FT_Library freeTypeLib;

	bool requiresRemerge = false;

	constexpr unsigned char FONT_VERSION = 0;

	constexpr std::string_view DATA_FILE = "pack.bin";
	constexpr std::string_view TEX_FILE = "tex.png";

	void loadLib() {
		if(FT_Init_FreeType(&freeTypeLib)) {
			std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		}
	}

	GlyphTexPack loadFromCache(const OS::File& fontCacheDir) {
		 OS::File dataFile = fontCacheDir.subFile(static_cast<std::string>(DATA_FILE));
		 OS::File texFile = fontCacheDir.subFile(static_cast<std::string>(TEX_FILE));

		 if(!dataFile.exist() || !texFile.exist()) {
		 	throw ext::RuntimeException{"Font Cache Doesn't Exist! : " + fontCacheDir.filename()};
		 }

		std::ifstream fstream{dataFile.absolutePath(), std::ios::binary | std::ios::in | std::ios::out};

		unsigned char version = 0;
		fstream.read(reinterpret_cast<char*>(&version), sizeof(version));

		FT_UInt width, height;
		fstream.read(reinterpret_cast<char*>(&width ), sizeof(width ));
		fstream.read(reinterpret_cast<char*>(&height), sizeof(height));

		size_t size = 0;
		fstream.read(reinterpret_cast<char*>(&size), sizeof(size));

		GlyphTexPack pack{GL::Texture2D{texFile}, size};
		const Geom::Shape::Rect_Orthogonal rectPack{width, height};

		for(size_t i = 0; i < size; ++i) {
			GlyphData quad{};

			//Write tex uv
			fstream.read(reinterpret_cast<char*>(&quad.charCode), sizeof(quad.charCode));
			fstream.read(reinterpret_cast<char*>(&quad.rect	   ), sizeof(quad.rect    ));
			fstream.read(reinterpret_cast<char*>(&quad.matrices), sizeof(quad.matrices));

			GL::TextureRegionRect region{&pack.texture};

			region.fetchInto(quad.rect, rectPack);

			pack.charRegions[quad.charCode] = std::make_pair(region, quad.matrices);
		}

		return pack;
	}

	void loadToCache(const std::vector<FT_ULong>& segments, const OS::File& font, const OS::File& rootCacheDir, const FT_UInt height = 48, const FT_Int loadFlags = FT_LOAD_RENDER) {
		const auto fontCacheDir = rootCacheDir.subFile(font.stem());
		if(!fontCacheDir.exist())fontCacheDir.createDirQuiet();

		OS::File dataFile = fontCacheDir.subFile(static_cast<std::string>(DATA_FILE));
		OS::File texFile = fontCacheDir.subFile(static_cast<std::string>(TEX_FILE));

		//If the data file doesn't exist, means the glyph hasn't been cached
		bool needToRecache = !dataFile.exist();

		if(needToRecache)dataFile.createFileQuiet();

		std::fstream fstream{dataFile.absolutePath(), std::ios::binary | std::ios::in | std::ios::out};

		// Recache if the version cannot match;
		 if(!needToRecache) {
		 	if(!fstream.is_open()) {
		 		goto throwExit;
		 	}

		 	unsigned char version = 0;
		 	fstream.read(reinterpret_cast<char*>(&version), sizeof(version));

		 	needToRecache = version != FONT_VERSION;
		 }

		requiresRemerge |= needToRecache;

		if(needToRecache || true) {
			FT_Face face;
			if(FT_New_Face(freeTypeLib, font.absolutePath().string().data(), 0, &face)) {
				goto throwExit;
			}

			FT_Set_Pixel_Sizes(face, 0, height);

			std::vector<GlyphData> segmentsQuad{};

			for(size_t t = 0; t < segments.size() / 2; ++t) {
				for(FT_ULong i = segments[t * 2]; i <= segments[t * 2 + 1]; ++i) {
					if(FT_Load_Char(face, i, loadFlags)) {
						goto throwExit;
					}

					auto* writeData = new unsigned char[face->glyph->bitmap.width * face->glyph->bitmap.rows * 4];

					for(size_t size = 0; size < face->glyph->bitmap.width * face->glyph->bitmap.rows; ++size) {
						writeData[size * 4 + 0] = 0xff;
						writeData[size * 4 + 1] = 0xff;
						writeData[size * 4 + 2] = 0xff;
						writeData[size * 4 + 3] = face->glyph->bitmap.buffer[size];
					}

					segmentsQuad.emplace_back(
						i,
						static_cast<int>(face->glyph->bitmap.width),
						static_cast<int>(face->glyph->bitmap.rows),
						face->glyph->metrics,
						writeData
					);
				}
			}

			FT_Done_Face(face);

			const FT_UInt totalWidth = std::accumulate(segmentsQuad.begin(), segmentsQuad.end(), 0, [](const FT_UInt v, const GlyphData& d) {
				return v + d.rect.getWidth();
			});

			const Graphic::Pixmap maxMap{totalWidth, height};

			//1. Write font version
			static_assert(std::is_same_v<decltype(FONT_VERSION), const unsigned char>);
			fstream.write(reinterpret_cast<const char*>(&FONT_VERSION), sizeof(FONT_VERSION));

			//2. Write tex size
			static_assert(std::is_same_v<decltype(totalWidth), const FT_UInt>);
			static_assert(std::is_same_v<decltype(height    ), const FT_UInt>);
			fstream.write(reinterpret_cast<const char*>(&totalWidth), sizeof(totalWidth));
			fstream.write(reinterpret_cast<const char*>(&height), sizeof(height));

			//3. Write data size
			const size_t size = segmentsQuad.size();
			static_assert(std::is_same_v<decltype(size), const size_t>);
			fstream.write(reinterpret_cast<const char*>(&size), sizeof(size));

			//4. Write tex uv [*size]
			int currentWidth = 0;
			for(auto& quad: segmentsQuad) {
				quad.rect.setSrcX(currentWidth);

				const Graphic::Pixmap map{
					static_cast<unsigned int>(quad.rect.getWidth()), static_cast<unsigned int>(quad.rect.getHeight()), quad.data.release()
				};

				maxMap.draw(map, currentWidth, 0, false);

				currentWidth += quad.rect.getWidth();

				fstream.write(reinterpret_cast<const char*>(&quad.charCode), sizeof(quad.charCode));
				fstream.write(reinterpret_cast<const char*>(&quad.rect	  ), sizeof(quad.rect	));
				// fstream.write(reinterpret_cast<const char*>(&quad.matrices), sizeof(quad.matrices));
			}

			stbi::setFlipVertically_write(false);

			maxMap.write(texFile, true);

			stbi::setFlipVertically_write(true);
		}

		return;

		throwExit: {
			throw ext::RuntimeException{"Font Load Failed : " + font.filename()};
		}
	}

	void load(const std::vector<FT_ULong>& segments, const OS::File& font, const OS::File& rootCacheDir, const FT_Int loadFlags = FT_LOAD_RENDER, const FT_UInt height = 48) {
		loadToCache(segments, font, rootCacheDir, height, loadFlags);
		loadFromCache(rootCacheDir.subFile(font.stem()));
	}

	void mergeTexture(const std::vector<OS::File>& dirs) {
		std::vector<GlyphTexPack> packs{};
		packs.reserve(dirs.size());
		std::vector<Graphic::Pixmap> cacheTexs{};
		cacheTexs.reserve(dirs.size());

		FT_UInt totalWidth = 0, totalHeight = 0;

		for(size_t i = 0; i < dirs.size(); ++i) {
			const auto& file = dirs.at(i);

			OS::File dataFile = file.subFile(static_cast<std::string>(DATA_FILE));
			OS::File texFile  = file.subFile(static_cast<std::string>(TEX_FILE ));

			if(!dataFile.exist() || !texFile.exist()) {
				throw ext::RuntimeException{"Font Cache Doesn't Exist! : " + file.filename()};
			}

			std::ifstream fstream{dataFile.absolutePath(), std::ios::binary | std::ios::in | std::ios::out};

			unsigned char version = 0;
			fstream.read(reinterpret_cast<char*>(&version), sizeof(version));

			FT_UInt width, height;
			fstream.read(reinterpret_cast<char*>(&width ), sizeof(width ));
			fstream.read(reinterpret_cast<char*>(&height), sizeof(height));

			size_t size = 0;
			fstream.read(reinterpret_cast<char*>(&size), sizeof(size));

			auto t = packs.emplace(packs.begin() + i, GL::Texture2D{}, size);
			cacheTexs.emplace(cacheTexs.begin() + i, texFile);

			GlyphTexPack pack& = *t;
			const Geom::Shape::Rect_Orthogonal rectPack{width, height};

			for(size_t j = 0; j < size; ++j) {
				GlyphData quad{};

				//Write tex uv
				fstream.read(reinterpret_cast<char*>(&quad.charCode), sizeof(quad.charCode));
				fstream.read(reinterpret_cast<char*>(&quad.rect	   ), sizeof(quad.rect    ));
				fstream.read(reinterpret_cast<char*>(&quad.matrices), sizeof(quad.matrices));

				GL::TextureRegionRect region{&pack.texture};

				quad.rect.move(0, totalHeight);

				pack.charRegions[quad.charCode] = std::make_pair(region, quad.matrices);
			}

			totalWidth += width;
			totalHeight += height;
		}

		Graphic::Pixmap totalMap{totalWidth, totalHeight};

		FT_UInt yOffset = 0;

		for(size_t i = 0; i < cacheTexs.size(); ++i) {
			auto& texPack = cacheTexs.at(i);

			totalMap.draw(texPack, 0, yOffset, false);
			packs.at(i).texture = texPack.genTex_move();
		}

		for(const auto& texPack: cacheTexs) {

		}
	}
}

