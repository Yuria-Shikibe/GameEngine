module UI.Root;

import Graphic.Draw;
import UI.Drawer;
import std;
import Core;
import OS.ApplicationListenerSetter;

void UI::Root::setRootOf(Elem* widget){
	widget->setRoot(this);
}

UI::Root::Root(): tooltipManager{this}{
	currentScene = scenes.try_emplace(std::string(UI::In_Game), std::make_unique<Scene>()).first->second.get();
	// NOLINT(*-use-equals-default)
	currentScene->setSrc(0.0f, 0.0f);
	currentScene->setAbsSrc(Geom::ZERO);
	currentScene->setLayoutByRelative(false);
	currentScene->setTouchbility(TouchbilityFlags::childrenOnly);
	currentScene->setRoot(this);
	currentScene->PointCheck = 100;
	currentScene->setDrawer(&emptyDrawer);
	currentScene->setBorder(8);
	currentScene->name = "UI Root";

	registerCtrl();

	rootDialog.content.setRoot(this);

	pressedMouseButtons.fill(Ctrl::Mode::Disable);

	OS::setInputMode_Cursor(OS::CursorMode::hidden, Core::platform->window->implHandle);

	Core::input.registerSubInput(&uiInput);
	Core::input.inputKeyListeners.push_back(this);
	Core::input.inputMouseListeners.push_back(this);
}

UI::Root::~Root(){
	Core::input.eraseSubInput(&uiInput);

	std::erase(Core::input.inputKeyListeners, this);
	std::erase(Core::input.inputMouseListeners, this);
}


void UI::Root::drawCursor() const{
	using namespace Graphic;
	const auto& cursor = getCursor(currentCursorType);

	if(Core::input.cursorInbound()){
		//TODO not use normalized draw!
		Draw::Overlay::mixColor();
		Draw::Overlay::color(Colors::WHITE);
		Draw::Overlay::getBatch().beginTempProjection(Geom::MAT3_IDT);
		Draw::Overlay::getBatch().switchBlending(GL::Blendings::Inverse);

		auto [x, y] = Core::renderer->getNormalized(cursorPos);
		cursor.draw(x, y, Core::renderer->getSize());

		Draw::Overlay::getBatch().endTempProjection();
		Draw::Overlay::getBatch().switchBlending();

	}
}

void UI::Root::update(const float delta){
	cursorPos = Core::input.getCursorPos();
	if(currentScene == nullptr)return;

	updateCurrentFocus();

	onDragUpdate();

	currentScene->postChanged();
	currentScene->update(delta);

	tooltipManager.cursorPos = cursorPos;

	if(tooltipManager.isOccupied(currentCursorFocus)){
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

	if(currentCursorFocus){
		if(currentCursorFocus->getTooltipBuilder() && currentCursorFocus->getTooltipBuilder().autoBuild()){
			currentCursorFocus->updateHoverTableHandle(tooltipManager.tryObtain(currentCursorFocus));
		}
		currentCursorType = currentCursorFocus->getCursorType();
	}else{
		currentCursorType = CursorType::regular;
	}

	rootDialog.update(delta);
	tooltipManager.update(delta);

	if(isHidden){
		releaseFocus();
		currentCursorType = CursorType::regular;
	}
}

void UI::Root::determinShiftFocus(Elem* newFocus){

	if(newFocus == nullptr){
		if(currentCursorFocus != nullptr){
			if(currentCursorFocus->shouldDropFocusAtCursorQuitBound() || noMousePress()){
				setEnter(nullptr, currentCursorFocus->isQuietInteractable());
			}
		}
	} else{
		if(currentCursorFocus != nullptr){
			if(noMousePress()){
				setEnter(newFocus, newFocus->isQuietInteractable());
			}
		} else{
			setEnter(newFocus, newFocus->isQuietInteractable());
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

	tooltipManager.clear();
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
	tooltipManager.renderBase();
}

void UI::Root::render_HoverTable() const{
	tooltipManager.render();
}

void UI::Root::onDoubleClick(const int id, const int mode){
	if(currentCursorFocus == nullptr) return;

	doubleClickAction.set(cursorPos, id, mode);
	currentCursorFocus->getInputListener().fire(doubleClickAction);
}

void UI::Root::onPress(const int id, const int mode){
	if(currentCursorFocus == nullptr) return;

	pressAction.set(cursorPos, id, mode);
	cursorPressedBeginPos = cursorPos;
	currentCursorFocus->getInputListener().fire(pressAction);
	pressedMouseButtons[id] = mode;
}

void UI::Root::onRelease(const int id, const int mode){
	if(currentCursorFocus == nullptr){
		Ctrl::Mode::setDisabled(pressedMouseButtons[id]);
		return;
	}

	releaseAction.set(cursorPos, id, mode);
	currentCursorFocus->getInputListener().fire(releaseAction);
	Ctrl::Mode::setDisabled(pressedMouseButtons[id]);
}

void UI::Root::onScroll() const{
	if(currentScrollFocused == nullptr) return;

	scrollAction.set(mouseScroll);
	currentScrollFocused->getInputListener().fire(scrollAction);
}


void UI::Root::onDragUpdate() const{
	if(currentCursorFocus == nullptr) return;

	dragAction.begin = cursorPressedBeginPos;
	dragAction.end = cursorPos;

	for(int i = 0; i < Ctrl::Mouse::Count; ++i){
		if(onDrag(i)){
			dragAction.set(cursorVel, i, pressedMouseButtons[i]);
			currentCursorFocus->getInputListener().fire(dragAction);
		}
	}
}

bool UI::Root::keyDown(const int code, const int action, const int mode) const{
	return uiInput.registerList.get(code, action, mode);
}

void UI::Root::registerCtrl(){
	uiInput.registerBind({{Ctrl::Key::Backspace, Ctrl::Act::Press}, {Ctrl::Key::Backspace, Ctrl::Act::Repeat}}, [this]{
		if(textInputListener){
			textInputListener->informBackSpace(0);
		}
	});

	uiInput.registerBind({{Ctrl::Key::Delete, Ctrl::Act::Press}, {Ctrl::Key::Delete, Ctrl::Act::Repeat}}, [this]{
		if(textInputListener){
			textInputListener->informDelete(0);
		}
	});

	uiInput.registerBind({{Ctrl::Key::Z, Ctrl::Act::Press, Ctrl::Mode::Ctrl}, {Ctrl::Key::Z, Ctrl::Act::Repeat, Ctrl::Mode::Ctrl}}, [this]{
		if(textInputListener){
			textInputListener->informDo();
		}
	});

	uiInput.registerBind({{Ctrl::Key::Enter, Ctrl::Act::Press}, {Ctrl::Key::Enter, Ctrl::Act::Repeat}}, [this]{
		if(textInputListener){
			textInputListener->informEnter(0);
		}
	});

	uiInput.registerBind({{Ctrl::Key::Z, Ctrl::Act::Press, Ctrl::Mode::Ctrl_Shift}, {Ctrl::Key::Z, Ctrl::Act::Repeat, Ctrl::Mode::Ctrl_Shift}}, [this]{
		if(textInputListener){
			textInputListener->informUndo();
		}
	});

	uiInput.registerBind(Ctrl::Key::A, Ctrl::Act::Press, Ctrl::Mode::Ctrl, [this]{
		if(textInputListener){
			textInputListener->informSelectAll();
		}
	});

	uiInput.registerBind(Ctrl::Key::C, Ctrl::Act::Press, Ctrl::Mode::Ctrl, [this]{
		if(textInputListener){
			Core::platform->setClipboard(textInputListener->getClipboardCopy());
		}
	});

	uiInput.registerBind({{Ctrl::Key::V, Ctrl::Act::Press, Ctrl::Mode::Ctrl}, {Ctrl::Key::V, Ctrl::Act::Repeat, Ctrl::Mode::Ctrl}}, [this]{
		if(textInputListener){
			textInputListener->informClipboardPaste(Core::platform->getClipboard());
		}
	});

	uiInput.registerBind(Ctrl::Key::X, Ctrl::Act::Press, Ctrl::Mode::Ctrl, [this]{
		if(textInputListener){
			Core::platform->setClipboard(textInputListener->getClipboardClip());
		}
	});
}

void UI::Root::handleSound(const SoundSource sound){
	Core::audio->play(sound);
}

void UI::Root::setEnter(Elem* elem, const bool quiet){
	if(elem == currentCursorFocus) return;

	if(currentCursorFocus != nullptr){
		exboundAction.set(cursorPos);
		if(!currentCursorFocus->isQuietInteractable())currentCursorFocus->getInputListener().fire(exboundAction);
	}

	currentCursorFocus = elem;

	if(currentCursorFocus != nullptr){
		inboundAction.set(cursorPos);
		if(!quiet)currentCursorFocus->getInputListener().fire(inboundAction);
	}
}
