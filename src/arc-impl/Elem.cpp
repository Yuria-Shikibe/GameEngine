module UI.Elem;

import UI.ElemDrawer;
import UI.Root;
import Core;

using UI::Root;

UI::Elem::Elem() {
	// setRoot(Core::uiRoot);
	if(!drawer)drawer = UI::defDrawer.get();
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
