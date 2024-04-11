module UI.Elem;

import UI.Group;
import UI.Drawer;
import UI.Root;
import RuntimeException;
import Core;

bool UI::Elem::layout_tryFillParent() {
	if(parent) {
		if(const Rect rect = parent->getFilledChildrenBound(this); rect != bound) {
			setSrc(rect.getSrcX(), rect.getSrcY());
			setSize(rect.getWidth(), rect.getHeight());
			overrideChanged(false);
			changed(ChangeSignal::notifySubs);
			return true;
		}
	}

	return false;
}

void UI::Elem::drawBase() const{
	if(!visiable)return;

	if(parent) {
		maskOpacity = parent->maskOpacity;
	}

	drawer->drawBackground(this);
}

void UI::Elem::draw() const {
	if(!visiable)return;

	if(parent) {
		maskOpacity = parent->maskOpacity;
	}

	drawStyle();
	drawContent();
}

void UI::Elem::applyDefDrawer(){
	setDrawer(UI::defDrawer);
}

void UI::Elem::drawStyle() const {
	drawer->drawStyle(this);
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

void UI::Elem::setEmptyDrawer(){
	setDrawer(&UI::emptyDrawer);
}

UI::Group* UI::Elem::setParent(Group* const parent) {
	Group* former = parent;
	this->parent  = parent;
	setRoot(parent->root);

	return former;
}

void UI::Elem::callRemove() {
	if(parent != nullptr) {
		parent->postRemove(this);
	}

	if(hoverTableHandle){
		root->hoverTableManager.forceDrop(hoverTableHandle);
	}
}

void UI::Elem::setFocusedKey(const bool focus) const {
	if(!isFocusedKeyInput() && !focus)return;
	this->root->currentInputFocused = focus ? const_cast<Elem*>(this)  : nullptr;
}

void UI::Elem::setFocusedScroll(const bool focus){
	if(!isFocusedScroll() && !focus)return;
	this->root->currentScrollFocused = focus ? this : nullptr;
}

void UI::Elem::postChanged(){
	if(lastSignal & ChangeSignal::notifySelf){
		layoutChanged = true;
	}

	if(lastSignal & ChangeSignal::notifyParentOnly){
		if(parent){
			parent->changed(lastSignal);
			parent->postChanged();
		}
	}

	lastSignal = ChangeSignal::notifyNone;
}

bool UI::Elem::isInbound(const Geom::Vec2 screenPos){
	if(touchbility == TouchbilityFlags::disabled)return false;
	if(parent != nullptr && !parent->hintInbound_validToParent(screenPos))return false;
	return screenPos.x > absoluteSrc.x && screenPos.y > absoluteSrc.y && screenPos.x < absoluteSrc.x + bound.getWidth() && screenPos.y < absoluteSrc.y + bound.getHeight();
}

bool UI::Elem::isFocusedKeyInput() const {
	return this->root->currentInputFocused == this;
}

bool UI::Elem::isFocusedScroll() const {
	return this->root->currentScrollFocused == this;
}

bool UI::Elem::isCursorInbound() const {
	return this->root->currentCursorFocus == this;
}

void UI::Elem::setUnfocused() const {
	if(isFocusedKeyInput())root->currentInputFocused = nullptr;
	if(isFocusedScroll())root->currentScrollFocused = nullptr;
}

bool UI::Elem::keyDown(const int code, const int action, const int mode) const{
	return root->keyDown(code, action, mode);
}
