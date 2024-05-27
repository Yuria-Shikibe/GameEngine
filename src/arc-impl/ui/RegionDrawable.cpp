module UI.RegionDrawable;

import Graphic.Draw;

using namespace Graphic;

void UI::UniqueRegionDrawable::draw(const Geom::OrthoRectFloat rect) const{
	Draw::Overlay::Fill::rectOrtho(&wrapper, rect);
}
