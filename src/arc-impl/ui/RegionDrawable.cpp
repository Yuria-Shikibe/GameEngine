module UI.RegionDrawable;

import Graphic.Draw;

using namespace Graphic;

void UI::Icon::draw(const Geom::OrthoRectFloat rect) const{
	Draw::Overlay::Fill::rectOrtho(wrapper, rect);
}

void UI::UniqueRegionDrawable::draw(const Geom::OrthoRectFloat rect) const{
	Draw::Overlay::Fill::rectOrtho(wrapper, rect);
}
