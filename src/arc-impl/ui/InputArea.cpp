module UI.InputArea;

import Graphic.Draw;
import UI.Root;

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

	if(!glyphLayout->empty()){
		const Geom::Vec2 off = glyphLayout->getDrawOffset();

		for (const auto & caret : carets){
			unsigned curRow = caret.dataBegin ? caret.dataBegin->getRow() : std::numeric_limits<unsigned>::max();
			Geom::Vec2 sectionBegin{caret.getDrawPos(false).scl(glyphLayout->getScale())};
			Geom::Vec2 sectionEnd{};

			for(auto data : std::span{caret.dataBegin, caret.dataEnd}){
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
			sectionEnd.x = caret.getDrawPos().x * glyphLayout->getScale();
			Graphic::Draw::color(caret.selectionColor, 0.65f);
			Graphic::Draw::rectOrtho(Graphic::Draw::getDefaultTexture(), rect.setVert(sectionBegin + off, sectionEnd + off));
		}

		glyphLayout->render();
	}


	Graphic::Draw::color();
	Graphic::Draw::Line::setLineStroke(2.0f);
	if(isTextFocused() && Math::cycleStep<75, 40>(time)){
		for (const auto & caret : carets){
			Geom::Vec2 src = caret.getDrawPos() * glyphLayout->getScale() + glyphLayout->offset;
			src.y -= glyphLayout->getDrawBound().getHeight();
			Graphic::Draw::Line::line(src, {src.x, src.y + caret.getHeight() * glyphLayout->getScale()});
		}
	}
}
