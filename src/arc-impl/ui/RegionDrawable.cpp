module UI.RegionDrawable;

import Graphic.Draw;

using namespace Graphic;

void UI::UniqueRegionDrawable::draw(const Geom::OrthoRectFloat rect) const{
	Draw::rectOrtho(&wrapper, rect);
}
