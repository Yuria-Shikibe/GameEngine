module UI.Label;

import Graphic.Draw;

void UI::Label::drawContent() const {
	Graphic::Draw::mixColor(color);
	Graphic::Draw::mixMulAlpha(maskOpacity);

	glyphLayout->render();

	Graphic::Draw::tint(Graphic::Colors::YELLOW, .35f);
	Graphic::Draw::Line::setLineStroke(1.25f);
	Graphic::Draw::Line::rectOrtho(glyphLayout->bound, true, glyphLayout->offset);
}



