module UI.Elem;

import UI.ElemDrawer;
import UI.Root;
import Core;

using UI::Root;
using UI::Elem;

UI::Elem::~Elem() {
	setUnfocused();
}

UI::Elem::Elem() {
	setRoot(Core::uiRoot);
	if(!drawer)setDrawer(UI::defDrawer.get());
}

void UI::Elem::drawBackground() const {
	drawer->drawBackground(this);
}

void Elem::setDrawer(ElemDrawer* drawer) {
	this->drawer = drawer;

	if(drawer && drawer->style) {
		margin_bottomLeft.x = drawer->style->margin_left;
		margin_bottomLeft.y = drawer->style->margin_bottom;

		margin_topRight.x = drawer->style->margin_right;
		margin_topRight.y = drawer->style->margin_top;

		changed();
	}
}

void UI::Elem::setFocusedKey(const bool focus) const {
	if(!isFocusedKey() && !focus)return;
	this->root->currentInputFocused = focus ? const_cast<Elem*>(this)  : nullptr;
}

void Elem::setFocusedScroll(const bool focus) const {
	if(!isFocusedScroll() && !focus)return;
	this->root->currentScrollFocused = focus ? const_cast<Elem*>(this) : nullptr;
}

bool UI::Elem::isFocusedKey() const {
	return this->root->currentInputFocused == this;
}

bool UI::Elem::isFocusedScroll() const {
	return this->root->currentScrollFocused == this;
}

bool UI::Elem::cursorInbound() const {
	return this->root->currentCursorFocus == this;
}

void Elem::setUnfocused() const {
	if(isFocusedKey())root->currentInputFocused = nullptr;
	if(isFocusedScroll())root->currentScrollFocused = nullptr;
}
