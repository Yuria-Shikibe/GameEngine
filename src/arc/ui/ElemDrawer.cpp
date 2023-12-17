import UI.ElemDrawer;

import Graphic.Color;
import Graphic.Draw;

void UI::EdgeDrawer::drawBackground(const float x, const float y, const float width, const float height, const Graphic::Color& color, const float opacity) const {
	Graphic::Draw::color(color);
	Graphic::Draw::setLineStroke(5.0f);
	Graphic::Draw::alpha(color.a * opacity);
	Graphic::Draw::rectLine(x, y, width, height);
}