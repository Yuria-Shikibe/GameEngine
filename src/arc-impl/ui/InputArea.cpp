module UI.InputArea;

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
