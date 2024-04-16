module UI.Widget;

import UI.Group;
import UI.Drawer;
import UI.Root;
import Graphic.Draw;
import ext.RuntimeException;
import Core;

bool UI::Widget::layout_tryFillParent() {
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

void UI::Widget::drawBase() const{
	if(!visiable)return;

	if(parent) {
		maskOpacity = parent->maskOpacity;
	}

	drawer->drawBackground(this);
}

void UI::Widget::draw() const {
	if(!visiable)return;

	if(parent) {
		maskOpacity = parent->maskOpacity;
	}


	drawStyle();
	Graphic::Draw::mixColor();
	Graphic::Draw::color(color, color.a * maskOpacity * selfMaskOpacity);

	drawContent();
	Graphic::Draw::color();
}

void UI::Widget::applyDefDrawer(){
	setDrawer(UI::defDrawer);
}

void UI::Widget::drawStyle() const {
	drawer->drawStyle(this);
}

UI::Group* UI::Widget::getParent() const {
	return parent;
}

void UI::Widget::setDrawer(WidgetDrawer* drawer) {
	this->drawer = drawer;

	if(drawer) {
		drawer->applyToElem(this);
	}
}

void UI::Widget::setEmptyDrawer(){
	setDrawer(&UI::emptyDrawer);
}

UI::Group* UI::Widget::setParent(Group* const parent) {
	Group* former = parent;
	this->parent = parent;
	setRoot(parent->root);

	return former;
}

void UI::Widget::callRemove() {
	if(parent != nullptr) {
		parent->postRemove(this);
	}

	if(hoverTableHandle){
		root->tooltipManager.forceDrop(hoverTableHandle);
	}
}

void UI::Widget::setFocusedKey(const bool focus) const {
	if(!isFocusedKeyInput() && !focus)return;
	this->root->currentInputFocused = focus ? const_cast<Widget*>(this)  : nullptr;
}

void UI::Widget::setFocusedScroll(const bool focus){
	if(!isFocusedScroll() && !focus)return;
	this->root->currentScrollFocused = focus ? this : nullptr;
}

void UI::Widget::postChanged(){
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

bool UI::Widget::isInbound(const Geom::Vec2 screenPos){
	if(touchbility == TouchbilityFlags::disabled && !tooltipbuilder)return false;
	if(parent != nullptr && !parent->hintInbound_validToParent(screenPos))return false;
	return screenPos.x > absoluteSrc.x && screenPos.y > absoluteSrc.y && screenPos.x < absoluteSrc.x + bound.getWidth() && screenPos.y < absoluteSrc.y + bound.getHeight();
}

bool UI::Widget::isFocusedKeyInput() const {
	return this->root->currentInputFocused == this;
}

bool UI::Widget::isFocusedScroll() const {
	return this->root->currentScrollFocused == this;
}

bool UI::Widget::isCursorInbound() const {
	return this->root->currentCursorFocus == this;
}

void UI::Widget::releaseAllFocus() const {
	if(!root)return;
	if(root->tooltipManager.getLastRequester() == this){
		root->tooltipManager.dropCurrentAt(nullptr, true);
	}
	if(isFocusedKeyInput())root->currentInputFocused = nullptr;
	if(isFocusedScroll())root->currentScrollFocused = nullptr;
	if(isCursorInbound())root->currentCursorFocus = nullptr;
}

bool UI::Widget::keyDown(const int code, const int action, const int mode) const{
	return root->keyDown(code, action, mode);
}

void UI::Widget::buildTooltip(){
	if(!root)return;
	updateHoverTableHandle(root->tooltipManager.tryObtain(this));
}
