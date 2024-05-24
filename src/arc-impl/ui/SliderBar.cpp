module UI.SliderBar;

import UI.Drawer;

void UI::SliderBar::applyDefDrawer() noexcept{
	Elem::applyDefDrawer();
	barDrawer = &defSlideBarDrawer;
}

void UI::SliderBar::drawContent() const{
	barDrawer->draw(this);
}
