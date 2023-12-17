import UI.Root;
import UI.Elem;

void UI::Elem::setFocused(bool focus){
	this->root->currentInputFocused = this;
}

bool UI::Elem::isFocused() const {
	return this->root->currentInputFocused == this;
}
