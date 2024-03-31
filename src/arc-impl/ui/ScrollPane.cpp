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

void UI::ScrollPane::drawContent() const{
	Graphic::Batch::flush();

	const auto lastRect = GL::getScissorRect();

	const Geom::OrthoRectInt clip{
		Math::round<int>(absoluteSrc.x + border.left), Math::round<int>(absoluteSrc.y + horiBarStroke() + border.bottom),
	Math::round<int>(this->getContentWidth()), Math::round<int>(this->getContentHeight())
	};

	const auto count = GL::getSrhinkCount();
	GL::enable(GL::Test::SCISSOR);

	if(!count)GL::forceSetScissor(clip);
	GL::scissorShrinkBegin();
	GL::setScissor(clip);

	drawChildren();
	Graphic::Batch::flush();

	GL::forceSetScissor(lastRect);
	GL::scissorShrinkEnd();
	if(!count)GL::disable(GL::Test::SCISSOR);

	scrollBarDrawer->operator()(this);
}
