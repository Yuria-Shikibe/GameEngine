//
// Created by Matrix on 2023/11/27.
//

module;

#include <ft2build.h>
#include <freetype/freetype.h>

export module GlyphArrangment;

import Graphic.Color;
import Geom.Shape.Rect_Orthogonal;
import Geom.Vector2D;
import GL.Texture.TextureRegionRect;
import GL.Texture.TextureRegion;
import GL.Texture.Texture2D;
import <memory>;
import <unordered_map>;

export namespace Font{
	struct GlyphData {
		FT_ULong charCode{0};
		Geom::Shape::OrthoRectInt rect{};
		FT_Glyph_Metrics matrices{};

		//Support more datas...

		std::unique_ptr<unsigned char[]> data{nullptr};

		[[nodiscard]] GlyphData() = default;

		[[nodiscard]] GlyphData(const FT_ULong charCode, Geom::Shape::OrthoRectInt rect, unsigned char* dataRaw)
			: charCode(charCode),
			  rect(std::move(rect)), data(dataRaw) {
		}

		[[nodiscard]] GlyphData(const FT_ULong charCode, const int width, const int height, const FT_Glyph_Metrics& m, unsigned char* dataRaw)
			: charCode(charCode),
			  rect(width, height), matrices(m), data(dataRaw){
		}

		[[nodiscard]] GlyphData(const FT_ULong charCode, const int width, const int height)
			: charCode(charCode),
			  rect(width, height){
		}
	};

	struct GlyphTexPack {
		GL::Texture2D texture{};
		std::unordered_map<FT_ULong, std::pair<GL::TextureRegionRect, FT_Glyph_Metrics>> charRegions{};

		[[nodiscard]] GlyphTexPack() = default;

		[[nodiscard]] GlyphTexPack(GL::Texture2D&& texture2D, const size_t size){
			texture = std::move(texture2D);
			charRegions.reserve(size);
		}
	};

	struct GlyphRenderData {
		Geom::Shape::OrthoRectFloat vertPosition{};
		Graphic::Color color{};
	};

	class GlyphSequence {
	protected:
		std::vector<std::pair<GlyphRenderData, GL::TextureRegionRect>> arrangement{};
		Geom::Vector2D lastPosition{};

	public:
		virtual void push(const FT_ULong charCore, const Graphic::Color& color = Graphic::Colors::WHITE, const) {

		}
	};

	// std::unordered_map<std::string, >
}

