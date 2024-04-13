module UI.ScrollPane;

import UI.Drawer;
import Graphic.Draw;

import UI.Root;
import Graphic.Color;
import Math;

using UI::Root;

void UI::ScrollBarDrawer::operator()(const ScrollPane* pane) const {
	Graphic::Draw::color(barColor);

	if(pane->enableHorizonScroll()) {
		region.render_RelativeExter(pane->getHoriBarRect().copy().shrink(margin).moveY(pane->getBorder().bottom * -offsetScl.x));
	}

	if(pane->enableVerticalScroll()) {
		region.render_RelativeExter(pane->getVertBarRect().copy().shrink(margin).moveX(pane->getBorder().right * offsetScl.y));
	}
}

void UI::ScrollPane::applyDefDrawer(){
	Group::applyDefDrawer();
	scrollBarDrawer = &UI::defScrollBarDrawer;
}

void UI::ScrollPane::drawBase() const{
	Widget::drawBase();
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
