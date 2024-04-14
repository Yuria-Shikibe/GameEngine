module UI.Label;

import Graphic.Draw;

void UI::Label::drawContent() const {
	Graphic::Draw::mixColor(color);
	glyphLayout->render(maskOpacity);

	// Graphic::Draw::tint(Graphic::Colors::YELLOW, .35f);
	// Graphic::Draw::Line::setLineStroke(1.25f);
	// Graphic::Draw::Line::rectOrtho(glyphLayout->getDrawBound(), true, glyphLayout->offset);
	//
	// Graphic::Draw::tint(Graphic::Colors::ACID, .35f);
	// Graphic::Draw::Line::setLineStroke(1.25f);
	// Graphic::Draw::Line::rectOrtho(getValidBound(), true, getAbsSrc());
}



