module UI.Elem;

import UI.Group;
import UI.Drawer;
import UI.Root;
import Graphic.Draw;
import ext.RuntimeException;
import Core;

void UI::Elem::passSound(const SoundSource sound) const{
	root->handleSound(sound);
}

bool UI::Elem::layout_tryFillParent() noexcept{
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

void UI::Elem::drawBase(const Rect rect) const{
	drawer->drawBackground(rect);
}

void UI::Elem::draw() const {
	if(!visiable)return;

	if(parent) {
		maskOpacity = parent->maskOpacity;
	}


	Graphic::Draw::Overlay::mixColor();
	Graphic::Draw::Overlay::color(color, color.a * maskOpacity * selfMaskOpacity);

	drawContent();
	drawStyle();

	Graphic::Draw::Overlay::color();
}

void UI::Elem::applyDefDrawer() noexcept{
	setDrawer(UI::defDrawer);
}

void UI::Elem::drawStyle() const {
	drawer->drawStyle(this);
}

UI::Elem* UI::Elem::getParent() const {
	return parent;
}

void UI::Elem::setDrawer(ElemDrawer* drawer){
	this->drawer = drawer;

	if(drawer) {
		drawer->applyToElem(this);
	}else{
		throw ext::NullPointerException{"Drawer Should Never Be Null!"};
	}
}

void UI::Elem::setEmptyDrawer(){
	setDrawer(&UI::emptyDrawer);
}

UI::Elem* UI::Elem::setParent(Elem* const parent) {
	Elem* former = parent;
	this->parent = parent;
	setRoot(parent->root);

	return former;
}

void UI::Elem::callRemove() {
	if(parent != nullptr) {
		parent->postRemove(this);
	}

	if(tooltipHandle){
		root->tooltipManager.forceDrop(tooltipHandle);
	}
}

void UI::Elem::setFocusedKey(const bool focus) noexcept{
	if(!isFocusedKeyInput() && !focus)return;
	this->root->currentInputFocused = focus ? this : nullptr;
}

void UI::Elem::setFocusedScroll(const bool focus) noexcept{
	if(!isFocusedScroll() && !focus)return;
	this->root->currentScrollFocused = focus ? this : nullptr;
}

void UI::Elem::changed(const ChangeSignal direction, const ChangeSignal removal) noexcept{
	lastSignal = lastSignal + direction - removal;
	if(parent && lastSignal & ChangeSignal::notifyParentOnly)parent->changed(ChangeSignal::notifySelf);
}

void UI::Elem::postChanged() noexcept{
	if(lastSignal & ChangeSignal::notifySelf){
		notifyLayoutChanged();
	}

	lastSignal = ChangeSignal::notifyNone;
}

bool UI::Elem::isInbound(const Geom::Vec2 screenPos) const noexcept{
	if(touchbility == TouchbilityFlags::disabled && !tooltipbuilder)return false;
	if(parent != nullptr && !parent->hintInbound_validToParent(screenPos))return false;
	return screenPos.x >= absoluteSrc.x && screenPos.y >= absoluteSrc.y && screenPos.x <= absoluteSrc.x + bound.getWidth() && screenPos.y <= absoluteSrc.y + bound.getHeight();
}

bool UI::Elem::isFocusedKeyInput() const noexcept{
	return this->root->currentInputFocused == this;
}

bool UI::Elem::isFocusedScroll() const noexcept{
	return this->root->currentScrollFocused == this;
}

bool UI::Elem::isCursorInbound() const noexcept{
	return this->root->currentCursorFocus == this;
}

void UI::Elem::releaseAllFocus() const noexcept{
	if(!root)return;

	dropTooltip();

	if(isFocusedKeyInput())root->currentInputFocused = nullptr;
	if(isFocusedScroll())root->currentScrollFocused = nullptr;
	if(isCursorInbound())root->currentCursorFocus = nullptr;
}

bool UI::Elem::hasTooltip() const{
	if(!root)return false;

	return root->tooltipManager.hasTooltipInstanceOf(this);
}

void UI::Elem::dropTooltip(const bool instant) const{
	if(hasTooltip()){
		root->tooltipManager.dropCurrentAt<1>(this, instant);
	}
}

Geom::Vec2 UI::Elem::getCursorPos() const noexcept{
	return root ? root->cursorPos : Geom::Vec2{};
}


bool UI::Elem::keyDown(const int code, const int action, const int mode) const{
	return root->keyDown(code, action, mode);
}

void UI::Elem::buildTooltip(){
	if(!root)return;
	updateHoverTableHandle(root->tooltipManager.tryObtain(this));
}

std::string_view UI::Elem::getBundleEntry(const std::string_view key, const bool fromUICategory) const{
	return getBundles(fromUICategory)[key];
}

Assets::Bundle& UI::Elem::getBundles(const bool fromUICategory) const{
	if(root && fromUICategory){
		return root->uiBasicBundle;
	}else{
		return Core::bundle;
	}
}
