module;

export module UI.Styles;

import UI.ElemDrawer;

import Geom.Shape.Rect_Orthogonal;
import GL.Texture.TextureRegionRect;
import GL.Texture.TextureNineRegion;
import Graphic.TextureAtlas;
import Graphic.Color;
import std;

export namespace UI::Styles {
	GL::TextureNineRegion
	tex_elem1_egde,
	tex_elem1_back,

	tex_elem2_egde,
	tex_elem2_back,

	tex_elem3_egde,
	tex_elem3_back,

	tex_elem_s1_egde,
	tex_elem_s1_back
	;

	std::unique_ptr<UI::Drawable>
		emptyDrawable{std::make_unique<UI::Drawable>()},
		drawable_elem1_egde{nullptr},
		drawable_elem1_back{nullptr},
		drawable_elem2_egde{nullptr},
		drawable_elem2_back{nullptr},
		drawable_elem3_egde{nullptr},
		drawable_elem3_back{nullptr},
		drawable_elem_s1_egde{nullptr},
		drawable_elem_s1_back{nullptr}
	;

	UI::DrawPair emptyPair = {emptyDrawable.get(), Graphic::Colors::CLEAR};

	UI::DrawPair drawpair_elem1_egde_white;
	UI::DrawPair drawpair_elem1_egde_sky;
	UI::DrawPair drawpair_elem1_back_white;
	UI::DrawPair drawpair_elem1_back_sky;

	UI::DrawPair drawpair_elem2_egde_white;
	UI::DrawPair drawpair_elem2_egde_sky;
	UI::DrawPair drawpair_elem2_back_white;
	UI::DrawPair drawpair_elem2_back_sky;

	UI::DrawPair drawpair_elem3_egde_white;
	UI::DrawPair drawpair_elem3_egde_sky;
	UI::DrawPair drawpair_elem3_back_white;
	UI::DrawPair drawpair_elem3_back_sky;

	UI::DrawPair drawpair_elem_s1_egde_white;
	UI::DrawPair drawpair_elem_s1_egde_sky;
	UI::DrawPair drawpair_elem_s1_back_white;
	UI::DrawPair drawpair_elem_s1_back_sky;

	std::unique_ptr<UI::UIStyle>
		style_elem1{std::make_unique<UI::UIStyle>()},
		style_elem2{std::make_unique<UI::UIStyle>()},
		style_elem3{std::make_unique<UI::UIStyle>()},
		style_elem_s1{std::make_unique<UI::UIStyle>()}
	;

	std::unique_ptr<UI::ElemDrawer>
		drawer_elem1{nullptr},
		drawer_elem2{nullptr},
		drawer_elem3{nullptr},
		drawer_elem_s1{nullptr}
	;

	void load(Graphic::TextureAtlas& atlas) {
		auto&& applyMargin_16 = [](const std::unique_ptr<UI::UIStyle>& style) {
			style->margin_bottom = style->margin_top = style->margin_left = style->margin_right = 16.0f;
		};

		auto&& applyMargin_8 = [](const std::unique_ptr<UI::UIStyle>& style) {
			style->margin_bottom = style->margin_top = style->margin_left = style->margin_right = 8.0f;
		};

		{
			tex_elem1_egde = GL::TextureNineRegion{atlas.find("ui-elem1-edge"), {16, 16, 64, 64}};
			tex_elem1_back = GL::TextureNineRegion{atlas.find("ui-elem1-back"), {16, 16, 64, 64}};

			drawable_elem1_egde = std::make_unique<UI::TextureNineRegionDrawable>(&tex_elem1_egde);
			drawable_elem1_back = std::make_unique<UI::TextureNineRegionDrawable>(&tex_elem1_back);

			drawpair_elem1_egde_white = {drawable_elem1_egde.get(), Graphic::Colors::WHITE};
			drawpair_elem1_egde_sky = {drawable_elem1_egde.get(), Graphic::Colors::AQUA_SKY};

			drawpair_elem1_back_white = {drawable_elem1_back.get(), Graphic::Color{Graphic::Colors::WHITE}.setA(0.13f)};
			drawpair_elem1_back_sky = {drawable_elem1_back.get(), Graphic::Color{Graphic::Colors::AQUA_SKY}.setA(0.13f)};

			style_elem1->base = emptyPair;
			style_elem1->edge = drawpair_elem1_egde_sky;
			style_elem1->inbound = drawpair_elem1_egde_white;
			style_elem1->pressed = drawpair_elem1_back_white;
			style_elem1->disabled = emptyPair;

			applyMargin_16(style_elem1);

			drawer_elem1 = std::make_unique<UI::StyleDrawer>(style_elem1.get());
		}

		{
			tex_elem2_egde = GL::TextureNineRegion{atlas.find("ui-elem2-edge"), {16, 16, 64, 64}};
			tex_elem2_back = GL::TextureNineRegion{atlas.find("ui-elem2-back"), {16, 16, 64, 64}};

			drawable_elem2_egde = std::make_unique<UI::TextureNineRegionDrawable>(&tex_elem2_egde);
			drawable_elem2_back = std::make_unique<UI::TextureNineRegionDrawable>(&tex_elem2_back);

			drawpair_elem2_egde_white = {drawable_elem2_egde.get(), Graphic::Colors::WHITE};
			drawpair_elem2_egde_sky = {drawable_elem2_egde.get(), Graphic::Colors::AQUA_SKY};

			drawpair_elem2_back_white = {drawable_elem2_back.get(), Graphic::Color{Graphic::Colors::WHITE}.setA(0.23f)};
			drawpair_elem2_back_sky = {drawable_elem2_back.get(), Graphic::Color{Graphic::Colors::AQUA_SKY}.setA(0.23f)};

			style_elem2->base = emptyPair;
			style_elem2->edge = drawpair_elem2_egde_sky;
			style_elem2->inbound = drawpair_elem2_egde_white;
			style_elem2->pressed = drawpair_elem2_back_white;
			style_elem2->disabled = emptyPair;

			applyMargin_16(style_elem2);

			drawer_elem2 = std::make_unique<UI::StyleDrawer>(style_elem2.get());
		}

		{
			tex_elem3_egde = GL::TextureNineRegion{atlas.find("ui-elem3-edge"), {16, 16, 64, 64}};
			tex_elem3_back = GL::TextureNineRegion{atlas.find("ui-elem3-back"), {16, 16, 64, 64}};

			drawable_elem3_egde = std::make_unique<UI::TextureNineRegionDrawable>(&tex_elem3_egde);
			drawable_elem3_back = std::make_unique<UI::TextureNineRegionDrawable>(&tex_elem3_back);

			drawpair_elem3_egde_white = {drawable_elem3_egde.get(), Graphic::Colors::WHITE};
			drawpair_elem3_egde_sky = {drawable_elem3_egde.get(), Graphic::Colors::AQUA_SKY};

			drawpair_elem3_back_white = {drawable_elem3_back.get(), Graphic::Color{Graphic::Colors::WHITE}.setA(0.33f)};
			drawpair_elem3_back_sky = {drawable_elem3_back.get(), Graphic::Color{Graphic::Colors::AQUA_SKY}.setA(0.33f)};

			style_elem3->base = emptyPair;
			style_elem3->edge = drawpair_elem3_egde_sky;
			style_elem3->inbound = drawpair_elem3_egde_white;
			style_elem3->pressed = drawpair_elem3_back_white;
			style_elem3->disabled = emptyPair;

			applyMargin_16(style_elem3);

			drawer_elem3 = std::make_unique<UI::StyleDrawer>(style_elem3.get());
		}

		{
			tex_elem_s1_egde = GL::TextureNineRegion{atlas.find("ui-elem-s1-edge"), {12, 12, 8, 8}};
			tex_elem_s1_back = GL::TextureNineRegion{atlas.find("ui-elem-s1-back"), {12, 12, 8, 8}};

			drawable_elem_s1_egde = std::make_unique<UI::TextureNineRegionDrawable>(&tex_elem_s1_egde);
			drawable_elem_s1_back = std::make_unique<UI::TextureNineRegionDrawable>(&tex_elem_s1_back);

			drawpair_elem_s1_egde_white = {drawable_elem_s1_egde.get(), Graphic::Colors::WHITE};
			drawpair_elem_s1_egde_sky = {drawable_elem_s1_egde.get(), Graphic::Colors::AQUA_SKY};

			drawpair_elem_s1_back_white = {drawable_elem_s1_back.get(), Graphic::Color{Graphic::Colors::WHITE}.setA(0.13f)};
			drawpair_elem_s1_back_sky = {drawable_elem_s1_back.get(), Graphic::Color{Graphic::Colors::AQUA_SKY}.setA(0.13f)};

			style_elem_s1->base = emptyPair;
			style_elem_s1->edge = drawpair_elem_s1_egde_sky;
			style_elem_s1->inbound = drawpair_elem_s1_egde_white;
			style_elem_s1->pressed = drawpair_elem_s1_back_sky;
			style_elem_s1->disabled = emptyPair;

			applyMargin_8(style_elem_s1);

			drawer_elem_s1 = std::make_unique<UI::StyleDrawer>(style_elem_s1.get());;
		}

		UI::defDrawer = drawer_elem_s1.get();
	}
}
