module UI.ElemDrawer;

import UI.Elem;
import Graphic.Color;
import Graphic.Draw;

void UI::EdgeDrawer::drawBackground(const UI::Elem* elem) const {
	elem->tempColor = elem->color;
	Graphic::Color& color = elem->tempColor;

	if(elem->cursorInbound()) {
		color.mul(1.1f).lerp(Graphic::Colors::WHITE, 0.3f);
		Graphic::Draw::color(color);
		Graphic::Draw::alpha(0.2f);
		Graphic::Draw::rect(Graphic::Draw::defaultTexture, elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());
	}


	Graphic::Draw::color(color);
	Graphic::Draw::setLineStroke(5.0f);
	Graphic::Draw::alpha(color.a * elem->maskOpacity);
	Graphic::Draw::rectLine(elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());

	Graphic::Draw::reset();
}