module UI.ControlBindTable;

import UI.Root;

void UI::ControlBindTable::beginBind(UI::Button* button, Ctrl::Operation* operation){
	root->inputListener = this;
	currentToBind = operation;
	currentButton = button;

	currentButton->clearChildrenInstantly();
	currentButton->add<UI::Label>([](UI::Label& label){
		label.setTextAlign(Align::Layout::center);
		label.setText(label.getBundleEntry("binding-ctrl"));
		label.getGlyphLayout()->setSCale(0.55f);
		label.setEmptyDrawer();
	}).fillParent();
	currentButton->notifyLayoutChanged();

	currentButton->setTouchbility(TouchbilityFlags::disabled);
}

void UI::ControlBindTable::endBind(){
	if(root->inputListener == this){
		root->inputListener = nullptr;

		currentButton->clearChildrenInstantly();
		buildBindInfo(*currentButton, *currentToBind);
		currentButton->setTouchbility(TouchbilityFlags::enabled);
		currentButton->notifyLayoutChanged();
	}

	currentToBind = nullptr;
	currentButton = nullptr;
}
