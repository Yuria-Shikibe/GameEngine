module;

module UI.ScrollPane;

import UI.Root;

using UI::Root;

void UI::ScrollPane::draw() const {
	maskOpacity = 0.3f;
	Elem::draw();

	porj.setOrthogonal(-scrollOffset.x, -scrollOffset.y, root->getWidth(), root->getHeight());

	Graphic::Draw::beginPorj(porj);
	GL::enable(GL::Test::SCISSOR);

	GL::scissor(absoluteSrc.x, absoluteSrc.y, getWidth(), getHeight());
	Graphic::Draw::rect(absoluteSrc.x, absoluteSrc.y, 8, 8);

	Graphic::Draw::endPorj();
	GL::disable(GL::Test::SCISSOR);
}
