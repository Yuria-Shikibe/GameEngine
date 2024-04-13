module UI.Group;

void UI::Group::modifyAddedChildren(Widget* elem) {
	childrenCheck(elem);
	elem->setParent(this);
	changed(ChangeSignal::notifyAll);
	elem->setRoot(root);
}
