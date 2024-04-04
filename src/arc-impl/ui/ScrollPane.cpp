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
		Graphic::Draw::rectOrtho(Graphic::Draw::contextTexture, pane->getHoriBarRect());
	}

	if(pane->enableVerticalScroll()) {
		Graphic::Draw::rectOrtho(Graphic::Draw::contextTexture, pane->getVertBarRect());
	}
}

void UI::ScrollPane::drawContent() const{
	Graphic::Batch::flush();

	const auto lastRect = GL::getScissorRect();

	const Geom::OrthoRectInt clip{
		Math::floorPositive(absoluteSrc.x + border.left), Math::floorPositive(absoluteSrc.y + horiBarStroke() + border.bottom),
	Math::ceilPositive(getContentWidth()), Math::ceilPositive(getContentHeight())
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
