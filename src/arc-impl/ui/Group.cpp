module UI.Group;

void UI::Group::modifyAddedChildren(Elem* elem) {
	childrenCheck(elem);
	elem->setParent(this);
	elem->changed();
	changed();
	elem->setRoot(root);
}
