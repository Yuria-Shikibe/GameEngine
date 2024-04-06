module UI.Root;

import UI.Drawer;
import std;
import Core;

UI::Root::Root(): root(std::make_unique<UI::Table>()){
	// NOLINT(*-use-equals-default)
	root->setSrc(0.0f, 0.0f);
	root->setAbsSrc(Geom::ZERO);
	root->relativeLayoutFormat = false;
	root->setTouchbility(UI::TouchbilityFlags::childrenOnly);
	root->setRoot(this);
	root->setDrawer(&UI::emptyDrawer);
	root->setBorder({marginX, marginX, marginY, marginY});
	root->name = "UI Root";
	root->defaultCellLayout.setMargin(marginX, marginX, marginY, marginY);

	registerCtrl();
}

void UI::Root::update(const float delta){
	bool stop = false;

	Elem* last = nullptr;

	iterateAll_DFS(root.get(), stop, [this, &last](Elem* elem) mutable{
		if(elem->isInteractable() && elem->isInbound(cursorPos)){
			last = elem;
		}

		return !elem->touchDisabled();
	});

	determinShiftFocus(last);

	onDragUpdate();

	root->update(delta);
}

void UI::Root::determinShiftFocus(const Elem* newFocus){
	if(newFocus == nullptr){
		if(currentCursorFocus != nullptr){
			if(currentCursorFocus->needSetMouseUnfocusedAtCursorOutOfBound()){
				setEnter(nullptr);
			} else if(pressedMouseButtons.none()){
				setEnter(nullptr);
			}
		}
	} else{
		if(currentCursorFocus != nullptr){
			if(pressedMouseButtons.none()){
				setEnter(newFocus);
			}
		} else{
			setEnter(newFocus);
		}
	}
}

void UI::Root::resize(const unsigned w, const unsigned h){
	width = static_cast<float>(w);
	height = static_cast<float>(h);
	root->setSize(static_cast<float>(w), static_cast<float>(h));

	//TODO apply margin with FBO, not directly
	// projection.setOrthogonal(-marginX, -marginY, static_cast<float>(w) + marginX * 2.0f, static_cast<float>(h) + marginY * 2.0f);
	projection.setOrthogonal(0, 0, static_cast<float>(w), static_cast<float>(h));
}

void UI::Root::render() const{
	root->draw();
}

void UI::Root::onDoubleClick(const int id, int mode){
	if(currentCursorFocus == nullptr) return;
	doubleClickAction.set(cursorPos, id);
	currentCursorFocus->getInputListener().fire(doubleClickAction);
}

void UI::Root::onPress(const int id, int mode){
	if(currentCursorFocus == nullptr) return;
	pressAction.set(cursorPos, id);
	cursorPressedBeginPos = cursorPos;
	currentCursorFocus->getInputListener().fire(pressAction);
	pressedMouseButtons[id] = true;
}

void UI::Root::onRelease(const int id, int mode){
	if(currentCursorFocus == nullptr){
		pressedMouseButtons[id] = false;
		return;
	}
	releaseAction.set(cursorPos, id);
	currentCursorFocus->getInputListener().fire(releaseAction);
	pressedMouseButtons[id] = false;
}

void UI::Root::onScroll() const{
	if(currentScrollFocused == nullptr) return;
	scrollAction.set(mouseScroll);
	currentScrollFocused->getInputListener().fire(scrollAction);
}

void UI::Root::disable(){
	root->setVisible(false);
	root->setTouchbility(TouchbilityFlags::disabled);
	allHidden = true;
}

void UI::Root::enable(){
	root->setVisible(true);
	root->setTouchbility(TouchbilityFlags::childrenOnly);
	allHidden = false;
}

bool UI::Root::onDrag(const int id, int mode) const{
	return pressedMouseButtons[id] && currentCursorFocus != nullptr;
}

void UI::Root::onDragUpdate() const{
	if(currentCursorFocus == nullptr) return;
	for(int i = 0; i < Ctrl::MOUSE_BUTTON_COUNT; ++i){
		dragAction.begin = cursorPressedBeginPos;
		dragAction.end = cursorPos;

		if(onDrag(i)){
			dragAction.set(cursorVel, i);
			currentCursorFocus->getInputListener().fire(dragAction);
		}
	}
}

bool UI::Root::keyDown(const int code, const int action, const int mode) const{
	return uiInput->keyGroup.get(code, action, mode);
}

void UI::Root::registerCtrl() const{
	uiInput->registerKeyBind({{Ctrl::KEY_BACKSPACE, Ctrl::Act_Press}, {Ctrl::KEY_BACKSPACE, Ctrl::Act_Repeat}}, [this]{
		if(textInputListener){
			textInputListener->informBackSpace(0);
		}
	});

	uiInput->registerKeyBind({{Ctrl::KEY_DELETE, Ctrl::Act_Press}, {Ctrl::KEY_DELETE, Ctrl::Act_Repeat}}, [this]{
		if(textInputListener){
			textInputListener->informDelete(0);
		}
	});

	uiInput->registerKeyBind({{Ctrl::KEY_Z, Ctrl::Act_Press, Ctrl::Mode_Ctrl}, {Ctrl::KEY_Z, Ctrl::Act_Repeat, Ctrl::Mode_Ctrl}}, [this]{
		if(textInputListener){
			textInputListener->informDo();
		}
	});

	uiInput->registerKeyBind({{Ctrl::KEY_ENTER, Ctrl::Act_Press}, {Ctrl::KEY_ENTER, Ctrl::Act_Repeat}}, [this]{
		if(textInputListener){
			textInputListener->informEnter(0);
		}
	});

	uiInput->registerKeyBind({{Ctrl::KEY_Z, Ctrl::Act_Press, Ctrl::Mode_Ctrl_Shift}, {Ctrl::KEY_Z, Ctrl::Act_Repeat, Ctrl::Mode_Ctrl_Shift}}, [this]{
		if(textInputListener){
			textInputListener->informUndo();
		}
	});

	uiInput->registerKeyBind(Ctrl::KEY_A, Ctrl::Act_Press, Ctrl::Mode_Ctrl, [this]{
		if(textInputListener){
			textInputListener->informSelectAll();
		}
	});

	uiInput->registerKeyBind(Ctrl::KEY_C, Ctrl::Act_Press, Ctrl::Mode_Ctrl, [this]{
		if(textInputListener){
			Core::platform->setClipboard(textInputListener->getClipboardCopy());
		}
	});

	uiInput->registerKeyBind(Ctrl::KEY_V, Ctrl::Act_Press, Ctrl::Mode_Ctrl, [this]{
		if(textInputListener){
			textInputListener->informClipboardPaste(Core::platform->getClipboard());
		}
	});

	uiInput->registerKeyBind(Ctrl::KEY_X, Ctrl::Act_Press, Ctrl::Mode_Ctrl, [this]{
		if(textInputListener){
			Core::platform->setClipboard(textInputListener->getClipboardClip());
		}
	});
}

void UI::Root::setEnter(const Elem* elem){
	if(elem == currentCursorFocus) return;

	if(currentCursorFocus != nullptr){
		exboundAction.set(cursorPos);
		currentCursorFocus->getInputListener().fire(exboundAction);
	}

	currentCursorFocus = elem;

	if(currentCursorFocus != nullptr){
		inboundAction.set(cursorPos);
		currentCursorFocus->getInputListener().fire(inboundAction);
	}
}
