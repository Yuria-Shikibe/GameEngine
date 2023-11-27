//
// Created by Matrix on 2023/11/27.
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

export module FT;

export namespace Font::FT_ {
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
}
