module;

module UI.ScrollPane;

import Graphic.Draw;

import UI.Root;
import Graphic.Color;
import Math;

using UI::Root;

void UI::ScrollerDrawer::operator()(const ScrollPane* pane) const {
	Graphic::Draw::color(Graphic::Colors::GRAY);

	if(pane->enableHorizonScroll()) {
		Graphic::Draw::rectOrtho(
			pane->drawSrcX() + pane->getMargin().left + pane->horiScrollRatio() * (pane->getValidWidth() - pane->horiBarLength()),
			pane->drawSrcY() + pane->getMargin().bottom,
			pane->horiBarLength(),
			pane->horiBarStroke()
		);
	}

	if(pane->enableVerticalScroll()) {
		Graphic::Draw::rectOrtho(
			pane->drawSrcX() + pane->getWidth() - pane->getMargin().right,
			pane->drawSrcY() + pane->getMargin().bottom + pane->vertScrollRatio() * (pane->getValidHeight() - pane->vertBarSLength()),
			-pane->vertBarStroke(),
			pane->vertBarSLength()
		);
	}
}

void UI::ScrollPane::drawContent() const {
	Graphic::Batch::flush();

	GL::enable(GL::Test::SCISSOR);

	GL::scissor(Math::round<int>(absoluteSrc.x + border.left), Math::round<int>(absoluteSrc.y + horiBarStroke() + border.bottom), Math::round<int>(getWidth() - vertBarStroke() - getBorderWidth()), Math::round<int>(getHeight() - getBorderHeight()));

	drawChildren();

	Graphic::Batch::flush();
	GL::disable(GL::Test::SCISSOR);

	scrollBarDrawer->operator()(this);
}
