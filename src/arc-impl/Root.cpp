module UI.Root;

import UI.ElemDrawer;
import UI.Table;
import <memory>;
import <algorithm>;


UI::Root::Root(): root(std::make_unique<UI::Table>()) { // NOLINT(*-use-equals-default)
	root->setSrc(0.0f, 0.0f);
	root->getAbsSrc().setZero();
	root->relativeLayoutFormat = false;
	root->setTouchbility(UI::TouchbilityFlags::childrenOnly);
	root->setRoot(this);
	root->setDrawer(UI::emptyDrawer.get());
	root->margin_bottomLeft.set(marginX, marginY);
	root->margin_topRight.set(marginX, marginY);
	root->name = "UI Root";
	root->defaultCellLayout.setMargin(marginX, marginX, marginY, marginY);
}


void UI::Root::iterateAll_DFS(Elem* current, bool& shouldStop, const std::function<bool(Elem*)>& func) {
	if (shouldStop)return;

	if (!func(current) || shouldStop)return;

	if(!current->hasChildren())return;

	for (auto& child : *current->getChildren()) {
		iterateAll_DFS(child.get(), shouldStop, func);
	}
}

void UI::Root::update(const float delta) {
	bool stop = false;

	const Elem* last = nullptr;

	iterateAll_DFS(root.get(), stop, [this, &last](const Elem* elem) mutable {
		if(elem->interactive() && elem->inbound(cursorPos)) {
			last = elem;
		}

		return !elem->touchDisabled();
	});

	determinShiftFocus(last);

	onDragUpdate();

	root->update(delta);
}

void UI::Root::determinShiftFocus(const Elem* newFocus) {
	if(newFocus == nullptr) {
		if(currentCursorFocus != nullptr) {
			if(currentCursorFocus->quitMouseFocusAtOutbound()) {
				setEnter(nullptr);
			}else if(pressedMouseButtons.none()){
				setEnter(nullptr);
			}
		}
	}else {
		if(currentCursorFocus != nullptr) {
			if(pressedMouseButtons.none()) {
				setEnter(newFocus);
			}
		}else {
			setEnter(newFocus);
		}
	}
}

void UI::Root::resize(const unsigned w, const unsigned h) {
	width  = static_cast<float>(w);
	height = static_cast<float>(h);
	root->setSize(static_cast<float>(w), static_cast<float>(h));

	//TODO apply margin with FBO, not directly
	// projection.setOrthogonal(-marginX, -marginY, static_cast<float>(w) + marginX * 2.0f, static_cast<float>(h) + marginY * 2.0f);
	projection.setOrthogonal(0, 0, static_cast<float>(w), static_cast<float>(h));
}

void UI::Root::render() const {
	root->draw();
}

void UI::Root::onDoubleClick(const int id) {
	if(currentCursorFocus == nullptr)return;
	doubleClickAction.set(cursorPos, id);
	currentCursorFocus->getInputListener().fire(doubleClickAction);
}

void UI::Root::onPress(const int id) {
	if(currentCursorFocus == nullptr)return;
	pressAction.set(cursorPos, id);
	currentCursorFocus->getInputListener().fire(pressAction);
	pressedMouseButtons[id] = true;
}

void UI::Root::onRelease(const int id) {
	if(currentCursorFocus == nullptr)return;
	releaseAction.set(cursorPos, id);
	currentCursorFocus->getInputListener().fire(releaseAction);
	pressedMouseButtons[id] = false;
}

void UI::Root::onScroll() const {
	if(currentScrollFocused == nullptr)return;
	scrollAction.set(mouseScroll);
	currentScrollFocused->getInputListener().fire(scrollAction);
}

void UI::Root::disable() {
	root->setVisible(false);
	root->setTouchbility(TouchbilityFlags::disabled);
	allHidden = true;
}

void UI::Root::enable() {
	root->setVisible(true);
	root->setTouchbility(TouchbilityFlags::childrenOnly);
	allHidden = false;
}

bool UI::Root::onDrag(const int id) const {
	return pressedMouseButtons[id] && currentCursorFocus != nullptr;
}

void UI::Root::onDragUpdate() const {
	if(currentCursorFocus == nullptr)return;
	for(int i = 0; i < Ctrl::MOUSE_BUTTON_COUNT; ++i) {
		if(onDrag(i)) {
			dragAction.set(cursorVel, i);
			currentCursorFocus->getInputListener().fire(dragAction);
		}
	}
}

void UI::Root::setEnter(const Elem* elem) {
	if(elem == currentCursorFocus)return;

	if(currentCursorFocus != nullptr) {
		exboundAction.set(cursorPos);
		currentCursorFocus->getInputListener().fire(exboundAction);
	}

	currentCursorFocus = elem;

	if(currentCursorFocus != nullptr) {
		inboundAction.set(cursorPos);
		currentCursorFocus->getInputListener().fire(inboundAction);
	}
}
