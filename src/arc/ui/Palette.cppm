//
// Created by Matrix on 2024/5/7.
//

export module UI.Palette;

export import Graphic.Color;

export namespace UI::Pal{
	using namespace Graphic::Colors;
	using Graphic::Color;

	constexpr Color KEY_WORD{ 0xE89267FF };
	constexpr Color THEME = AQUA_SKY.createLerp(Graphic::Colors::LIGHT_GRAY, 0.276f).setA(0.875f);;

}
