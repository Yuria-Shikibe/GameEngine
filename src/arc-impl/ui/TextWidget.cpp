module UI.TextWidget;

import Graphic.Draw;

void UI::TextWidget::drawContent() const{
	Elem::drawContent();

	Graphic::Draw::Overlay::mixColor(color);
	glyphLayout->render(maskOpacity);

	// Graphic::Draw::Overlay::tint(Graphic::Colors::YELLOW, .35f);
	// Graphic::Draw::Overlay::Line::setLineStroke(1.25f);
	// Graphic::Draw::Overlay::Line::rectOrtho(glyphLayout->getDrawBound(), true, glyphLayout->offset);
	//
	// Graphic::Draw::Overlay::tint(Graphic::Colors::ACID, .35f);
	// Graphic::Draw::Overlay::Line::setLineStroke(1.25f);
	// Graphic::Draw::Overlay::Line::rectOrtho(getValidBound(), true, getAbsSrc());
}
