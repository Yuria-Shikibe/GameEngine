module UI.Root;

import UI.Drawer;
import std;
import Core;

UI::Root::Root(): hoverTableManager{this}{
	currentScene = scenes.try_emplace("default", std::make_unique<Scene>()).first->second.get();
	// NOLINT(*-use-equals-default)
	currentScene->setSrc(0.0f, 0.0f);
	currentScene->setAbsSrc(Geom::ZERO);
	currentScene->setRelativeLayoutFormat(false);
	currentScene->setTouchbility(TouchbilityFlags::childrenOnly);
	currentScene->setRoot(this);
	currentScene->PointCheck = 100;
	currentScene->setDrawer(&emptyDrawer);
	currentScene->setBorder(8);
	currentScene->name = "UI Root";

	registerCtrl();

	rootDialog.content.setRoot(this);
}

void UI::Root::update(const float delta){
	if(currentScene == nullptr)return;

	updateCurrentFocus();

	onDragUpdate();

	currentScene->postChanged();
	currentScene->update(delta);

	hoverTableManager.cursorPos = cursorPos;

	if(hoverTableManager.isOccupied(currentCursorFocus)){
		cursorStrandedTime = cursorInBoundTime = 0.0f;
	}else{
		if(cursorVel.isZero(0.005f)){
			cursorStrandedTime += delta;
		}else{
			cursorStrandedTime = 0.0f;
		}

		if(currentCursorFocus){
			cursorInBoundTime += delta;
		}else{
			cursorInBoundTime = 0.0f;
		}
	}

	if(currentCursorFocus && currentCursorFocus->getHoverTableBuilder()){
		currentCursorFocus->updateHoverTableHandle(hoverTableManager.obtain(currentCursorFocus));
	}

	rootDialog.update(delta);
	hoverTableManager.update(delta);
}

void UI::Root::determinShiftFocus(Widget* newFocus){
	if(newFocus == nullptr){
		if(currentCursorFocus != nullptr){
			if(currentCursorFocus->shouldDropFocusAtCursorQuitBound() || pressedMouseButtons.none()){
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

void UI::Root::resize(const int w, const int h){
	width = static_cast<float>(w);
	height = static_cast<float>(h);
	if(currentScene){
		currentScene->setSize(width, height);
		currentScene->changed(ChangeSignal::notifyAll);
		currentScene->postChanged();
		currentScene->layout();
	}

	determinShiftFocus(nullptr);

	//TODO apply margin with FBO, not directly
	// projection.setOrthogonal(-marginX, -marginY, static_cast<float>(w) + marginX * 2.0f, static_cast<float>(h) + marginY * 2.0f);
	projection.setOrthogonal(0, 0, width, height);

	hoverTableManager.clear();
	rootDialog.resize();
}

void UI::Root::render() const{
	if(currentScene == nullptr)return;
	currentScene->draw();
}

void UI::Root::renderBase() const{
	if(currentScene == nullptr)return;
	currentScene->drawBase();
}

void UI::Root::renderBase_HoverTable() const{
	hoverTableManager.renderBase();
}

void UI::Root::render_HoverTable() const{
	hoverTableManager.render();
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
	isHidden = true;
}

void UI::Root::enable(){
	isHidden = false;
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

	uiInput->registerKeyBind({{Ctrl::KEY_V, Ctrl::Act_Press, Ctrl::Mode_Ctrl}, {Ctrl::KEY_V, Ctrl::Act_Repeat, Ctrl::Mode_Ctrl}}, [this]{
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

void UI::Root::setEnter(Widget* elem){
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
