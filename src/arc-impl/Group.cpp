module UI.Group;

void UI::Group::setRoot(Root* const root) {
	Elem::setRoot(root);

	std::for_each(std::execution::par_unseq, children.begin(), children.end(), [root](auto& elem) {
		elem->setRoot(root);
	});
}

void UI::Group::modifyAddedChildren(Elem* elem) {
	childrenCheck(elem);
	elem->setParent(this);
	elem->changed();
	changed();
	elem->setRoot(root);
}
