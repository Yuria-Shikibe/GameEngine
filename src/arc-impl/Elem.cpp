import UI.Root;
import UI.Elem;
import Core;

UI::Elem::Elem() {
	// setRoot(Core::uiRoot);
}

void UI::Elem::drawBackground() const {
	drawer->drawBackground(this);
}

void UI::Elem::setFocused(const bool focus) {
	this->root->currentInputFocused = focus ? this : nullptr;
}

bool UI::Elem::isFocused() const {
	return this->root->currentInputFocused == this;
}

bool UI::Elem::cursorInbound() const {
	return this->root->currentCursorFocus == this;
}
