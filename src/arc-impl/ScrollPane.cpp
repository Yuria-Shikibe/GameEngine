module;

module UI.ScrollPane;

import UI.Root;

using UI::Root;

void UI::ScrollerDrawer::operator()(const ScrollPane* pane) const {
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

void UI::ScrollPane::draw() const {
	drawBackground();

	Graphic::Draw::flush();

	GL::enable(GL::Test::SCISSOR);

	GL::scissor(absoluteSrc.x, absoluteSrc.y + horiBarStroke(), getWidth() - vertBarStroke(), getHeight());
	drawChildren();

	Graphic::Draw::flush();
	GL::disable(GL::Test::SCISSOR);

	scrollBarDrawer->operator()(this);
}
