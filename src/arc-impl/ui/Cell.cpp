module UI.Cell;

bool UI::LayoutCell::applySizeToItem(){ // NOLINT(*-make-member-function-const)
	const Geom::Vec2 originalSize = allocatedBound.getSize();

	float width = (scaleRelativeToParentX ? allocatedBound.getWidth() : item->getWidth());
	float height = (scaleRelativeToParentY ? allocatedBound.getHeight() : item->getHeight());

	//Apply Expansion
	if(modifyParentX) {
		width = Math::max(allocatedBound.getWidth(), item->getWidth());
		allocatedBound.setWidth(width * getHoriScale());
	}else{
		// allocatedBound.setShorterWidth(item->getWidth());
	}

	if(modifyParentY) {
		height = Math::max(allocatedBound.getHeight(), item->getHeight());
		allocatedBound.setHeight(height * getVertScale());
	}else{
		// allocatedBound.setShorterHeight(item->getHeight());
	}

	if(scaleRelativeToParentX){
		item->bound.setWidth(width * getHoriScale());
	}

	if(scaleRelativeToParentY){
		item->bound.setHeight(height * getVertScale());
	}

	item->bound.addSize(-getMarginHori(), -getMarginVert());
	item->changed(ChangeSignal::notifySubs);
	item->layout();

	allocatedBound.addSize(getPadHori(), getPadVert());

	return allocatedBound.getSize() != originalSize;
}

void UI::LayoutCell::applyAlignToItem(const Rect bound) const{
	item->bound.setSrc(Align::getOffsetOf(align, item->getBound(), bound));
}

void UI::LayoutCell::applyPosToItem(Widget* parent) const{
	applyAlignToItem(allocatedBound);

	float xMove{};
	float yMove{};

	if(align & Align::Mode::top) {
		yMove = -(pad.top + margin.top + getCellHeight() * scale.getSrcY());
	}else if(align & Align::Mode::bottom){
		yMove = pad.bottom + margin.bottom + getCellHeight() * scale.getSrcY();
	}

	if(align & Align::Mode::right) {
		xMove = -(pad.right + margin.right + getCellWidth() * scale.getSrcX());
	}else if(align & Align::Mode::left){
		xMove = pad.left + margin.left + getCellWidth() * scale.getSrcX();
	}


	item->bound.move(xMove, yMove);
	//TODO align...

	item->calAbsoluteSrc(parent);
}
