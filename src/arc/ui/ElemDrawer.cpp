import UI.ElemDrawer;

import Graphic.Color;
import Graphic.Draw;

void UI::EdgeDrawer::drawBackground(const float x, const float y, const float width, const float height, Graphic::Color& color, const float opacity) const {
	if(inbound) {
		color.mul(1.1f).lerp(Graphic::Colors::WHITE, 0.3f);
		Graphic::Draw::color(color);
		Graphic::Draw::alpha(0.2f);
		Graphic::Draw::rect(Graphic::Draw::defaultTexture, x, y, width, height);
	}


	Graphic::Draw::color(color);
	Graphic::Draw::setLineStroke(5.0f);
	Graphic::Draw::alpha(color.a * opacity);
	Graphic::Draw::rectLine(x, y, width, height);

	Graphic::Draw::reset();
}