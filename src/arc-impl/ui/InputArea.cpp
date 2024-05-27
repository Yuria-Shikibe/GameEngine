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
	using namespace Graphic;
	using Draw::Overlay;
	Overlay::alpha();

	if(showingHintText){
		Overlay::mixColor(UI::Pal::GRAY.copy().mul(color));
	}else{
		Overlay::mixColor(color);
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
					Overlay::color(caret.selectionColor, 0.65f);
					Overlay::Fill::rectOrtho(Overlay::getDefaultTexture(), rect.setVert(sectionBegin + off, sectionEnd + off));
					sectionBegin.setNaN();
				}
			}

			Rect rect{};
			sectionEnd.x = to->getBoundSrc().x * glyphLayout->getScale();
			Overlay::color(caret.selectionColor, 0.65f);
			Overlay::Fill::rectOrtho(Overlay::getDefaultTexture(), rect.setVert(sectionBegin + off, sectionEnd + off));
		}
	}

	glyphLayout->render(maskOpacity);

	if(isTextFull()){
		Overlay::mixColor(Pal::RED_DUSK);
	}else if(isTextNearlyFull()){
		Overlay::mixColor(Pal::KEY_WORD);
	}else{
		Overlay::mixColor();
	}
	Overlay::Line::setLineStroke(2.0f);
	if(isTextFocused() && Math::cycleStep<75, 40>(insertLineTimer)){
		for (const auto & caret : carets){
			Overlay::color(caret.caretColor);
			const Geom::Vec2 src = caret.getDrawPos() * glyphLayout->getScale() + glyphLayout->offset;
			Overlay::Line::line(src, {src.x, src.y + caret.getHeight() * glyphLayout->getScale()});
		}
	}

	Overlay::color();

}
