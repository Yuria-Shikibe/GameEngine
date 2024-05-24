module UI.InputArea;

import Graphic.Draw;
import UI.Root;
import UI.Palette;

bool UI::InputArea::isTextFocused() const{
	return root->textInputListener == this;
}

void UI::InputArea::setTextFocused(){
	root->setTextFocus(this);
}

void UI::InputArea::setTextUnfocused() const{
	root->setTextFocus(nullptr);
}

void UI::InputArea::drawContent() const{
	Graphic::Draw::alpha();

	if(showingHintText){
		Graphic::Draw::mixColor(UI::Pal::GRAY.copy().mul(color));
	}else{
		Graphic::Draw::mixColor(color);
	}

	if(!glyphLayout->empty()){
		const Geom::Vec2 off = glyphLayout->getDrawOffset();

		for (const auto & caret : carets){
			auto [from, to] = Math::Section{caret.dataBegin, caret.dataEnd}.ordered();

			unsigned curRow = from ? from->getRow() : std::numeric_limits<unsigned>::max();
			Geom::Vec2 sectionBegin{from->getBoundSrc().scl(glyphLayout->getScale())};
			Geom::Vec2 sectionEnd{};

			for(auto& data : std::ranges::subrange{from, to}){
				if(sectionBegin.isNaN()){
					sectionBegin = data.getBoundSrc().scl(glyphLayout->getScale());
				}

				sectionEnd = data.getBoundEnd().scl(glyphLayout->getScale());

				if(curRow != data.getRow() || data.isEndRow()){
					curRow = data.getRow();
					Rect rect{};
					if(data.isEndRow()){
						curRow++;
						sectionEnd.set(glyphLayout->getRawBound().getWidth(), data.getBoundEnd().y).scl(glyphLayout->getScale());
					}
					Graphic::Draw::color(caret.selectionColor, 0.65f);
					Graphic::Draw::rectOrtho(Graphic::Draw::getDefaultTexture(), rect.setVert(sectionBegin + off, sectionEnd + off));
					sectionBegin.setNaN();
				}
			}

			Rect rect{};
			sectionEnd.x = to->getBoundSrc().x * glyphLayout->getScale();
			Graphic::Draw::color(caret.selectionColor, 0.65f);
			Graphic::Draw::rectOrtho(Graphic::Draw::getDefaultTexture(), rect.setVert(sectionBegin + off, sectionEnd + off));
		}
	}

	glyphLayout->render(maskOpacity);

	if(isTextFull()){
		Graphic::Draw::mixColor(Pal::RED_DUSK);
	}else if(isTextNearlyFull()){
		Graphic::Draw::mixColor(Pal::KEY_WORD);
	}else{
		Graphic::Draw::mixColor();
	}
	Graphic::Draw::Line::setLineStroke(2.0f);
	if(isTextFocused() && Math::cycleStep<75, 40>(insertLineTimer)){
		for (const auto & caret : carets){
			Graphic::Draw::color(caret.caretColor);
			const Geom::Vec2 src = caret.getDrawPos() * glyphLayout->getScale() + glyphLayout->offset;
			Graphic::Draw::Line::line(src, {src.x, src.y + caret.getHeight() * glyphLayout->getScale()});
		}
	}

	Graphic::Draw::color();

}
