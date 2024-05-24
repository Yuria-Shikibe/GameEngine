module UI.TextWidget;

import Graphic.Draw;

void UI::TextWidget::drawContent() const{
	Elem::drawContent();

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