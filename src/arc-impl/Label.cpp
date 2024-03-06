module UI.Label;

import Graphic.Draw;

void UI::Label::drawContent() const {
	Graphic::Draw::mixColor(color);
	Graphic::Draw::mixMulAlpha(maskOpacity);

	glyphLayout->render();
}



