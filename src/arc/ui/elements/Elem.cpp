import UI.Root;
import UI.Elem;
import Core;

UI::Elem::Elem() {
	// setRoot(Core::uiRoot);

	inputListener.on<UI::CurosrInbound>([this](const auto& e) {
		drawer->inbound = true;
	});

	inputListener.on<UI::CurosrExbound>([this](const auto& e) {
		drawer->inbound = false;
	});
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
