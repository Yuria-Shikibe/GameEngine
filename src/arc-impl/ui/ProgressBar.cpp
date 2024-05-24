module UI.ProgressBar;

import UI.Drawer;

void UI::ProgressBar::drawContent() const{

	if(approachScope == ApproachScope::drawing){
		updateProgress();
	}

	barDrawer->draw(this);
}

void UI::ProgressBar::applyDefDrawer() noexcept{
	Elem::applyDefDrawer();
	barDrawer = &defProgressBarDrawer;
}
