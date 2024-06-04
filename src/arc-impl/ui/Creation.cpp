module UI.Creation;

import UI.Styles;
import UI.RegionDrawable;

void UI::Create::LineCreater::operator()(ImageRegion& image) const{
	image.setEmptyDrawer();
	image.color = defColor;
	image.setDrawable(TextureNineRegionDrawable{&UI::Styles::tex_elem_s1_back});
	image.scaling = Align::Scale::stretch;
}

void UI::Create::LineCreater::operator()(LayoutCell& cell) const{
	if(clearMargin)cell.setMargin(0.f);
}
