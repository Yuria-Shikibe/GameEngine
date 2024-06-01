module UI.Group;

import GL;

void UI::Group::drawChildren() const{
	const bool isScissorActivated = GL::getScissorCount();
	for(const auto& elem : children) {
		if(!isScissorActivated || (parent && elem->getBound().setSrc(elem->getAbsSrc()).overlap(parent->getBound().setSrc(parent->getAbsSrc())))){
			elem->draw();
		}
	}
}

void UI::Group::modifyAddedChildren(Elem* elem) {
	childrenCheck(elem);
	elem->setParent(this);
	changed(ChangeSignal::notifyAll);
	elem->setRoot(root);
	notifyLayoutChanged();
}
