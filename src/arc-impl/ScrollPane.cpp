module;

module UI.ScrollPane;

import UI.Root;
import Graphic.Color;

using UI::Root;

void UI::ScrollerDrawer::operator()(const ScrollPane* pane) const {
	Graphic::Draw::color(Graphic::Colors::GRAY);

	if(pane->enableHorizonScroll()) {
		Graphic::Draw::rect(
			pane->drawSrcX() + pane->margin_bottomLeft.x + pane->horiScrollRatio() * (pane->getWidth() - pane->marginWidth() - pane->horiBarLength()),
			pane->drawSrcY() + pane->margin_bottomLeft.y,
			pane->horiBarLength(),
			pane->horiBarStroke()
		);
	}

	if(pane->enableVerticalScroll()) {
		Graphic::Draw::rect(
			pane->drawSrcX() + pane->getWidth() - pane->margin_topRight.x,
			pane->drawSrcY() + pane->margin_bottomLeft.y + pane->vertScrollRatio() * (pane->getHeight() - pane->marginHeight() - pane->vertBarSLength()),
			-pane->vertBarStroke(),
			pane->vertBarSLength()
		);
	}
}

void UI::ScrollPane::drawContent() const {
	Graphic::Draw::flush();

	GL::enable(GL::Test::SCISSOR);

	GL::scissor(Math::round<int>(absoluteSrc.x + margin_bottomLeft.x), Math::round<int>(absoluteSrc.y + horiBarStroke() + margin_bottomLeft.y), Math::round<int>(getWidth() - vertBarStroke() - marginWidth()), Math::round<int>(getHeight() - marginHeight()));

	drawChildren();

	Graphic::Draw::flush();
	GL::disable(GL::Test::SCISSOR);

	scrollBarDrawer->operator()(this);
}
