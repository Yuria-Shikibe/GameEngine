module UI.Dialog;

import UI.Root;

void UI::Dialog::initContent(){
	if(isRootDialog())return;

	content.setRoot(parentDialog->content.getRoot());

	resizeContent();
}

void UI::Dialog::resizeContent(){
	if(fillScreen){
		const auto* root = content.getRoot();

		content.setSize(root->width - margin.getWidth(), root->height - margin.getHeight());
		content.changed(ChangeSignal::notifyAll);
		content.postChanged();

		content.setSrc(margin.bot_lft());
	}

	content.layout();
	content.calAbsoluteSrc(nullptr);
}

void UI::Dialog::resize(){
	if(showing){
		resizeContent();
	}else if(hasChild()){
		childDialog->resize();
	}
}
