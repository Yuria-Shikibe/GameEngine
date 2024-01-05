module UI.Elem;

import UI.Group;
import UI.ElemDrawer;
import UI.Root;
import RuntimeException;
import Core;

UI::Elem::~Elem() {
	setUnfocused();
}

UI::Elem::Elem() {
	if(!drawer)setDrawer(UI::defDrawer);
}

bool UI::Elem::layout_fillParent() {
	if(parent) {
		if(const Rect rect = parent->getFilledChildrenBound(this); rect != bound) {
			bound = rect;
			return true;
		}
	}

	return false;
}

void UI::Elem::draw() const {
	if(!visiable)return;

	if(parent) {
		maskOpacity *= parent->maskOpacity;
	}

	drawBackground();
	drawContent();

	maskOpacity = 1.0f;
}

void UI::Elem::drawBackground() const {
	drawer->drawBackground(this);
}

UI::Group* UI::Elem::getParent() const {
	return parent;
}

void UI::Elem::setDrawer(ElemDrawer* drawer) {
	this->drawer = drawer;

	if(drawer) {
		drawer->applyToElem(this);
	}
}

UI::Group* UI::Elem::setParent(Group* const parent) {
	Group* former = parent;
	this->parent  = parent;
	setRoot(parent->root);

	return former;
}

UI::Elem& UI::Elem::prepareRemove() {
	if(parent == nullptr) {
		throw ext::NullPointerException{"This Elem: " + name + " Doesn't Have A Parent!"};
	}
	parent->postRemove(this);

	return *this;
}

void UI::Elem::setFocusedKey(const bool focus) const {
	if(!isFocusedKey() && !focus)return;
	this->root->currentInputFocused = focus ? const_cast<Elem*>(this)  : nullptr;
}

void UI::Elem::setFocusedScroll(const bool focus) const {
	if(!isFocusedScroll() && !focus)return;
	this->root->currentScrollFocused = focus ? const_cast<Elem*>(this) : nullptr;
}

void UI::Elem::changed() const {
	layoutChanged = true;
	if(parent)parent->changed();
}

bool UI::Elem::inbound(const Geom::Vec2& screenPos) const {
	if(touchbility == TouchbilityFlags::disabled)return false;
	if(parent != nullptr && !parent->inbound_validToParent(screenPos))return false;
	return screenPos.x > absoluteSrc.x && screenPos.y > absoluteSrc.y && screenPos.x < absoluteSrc.x + bound.getWidth() && screenPos.y < absoluteSrc.y + bound.getHeight();
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

void UI::Elem::setUnfocused() const {
	if(isFocusedKey())root->currentInputFocused = nullptr;
	if(isFocusedScroll())root->currentScrollFocused = nullptr;
}
