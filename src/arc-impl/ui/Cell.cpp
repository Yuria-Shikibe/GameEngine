module UI.Cell;

void UI::LayoutCell::applyAlignToItem(const Rect bound) const{
	Rect& itemBound = item->bound;
	if(align & Align::Mode::top) {
		itemBound.setSrcY(bound.getEndY() - itemBound.getHeight());
	}else if(align & Align::Mode::bottom){
		itemBound.setSrcY(bound.getSrcY());
	}else { //centerY
		itemBound.setSrcY(bound.getSrcY() + (bound.getHeight() - itemBound.getHeight()) * 0.5f);
	}

	if(align & Align::Mode::right) {
		itemBound.setSrcX(bound.getEndX() - itemBound.getWidth());
	}else if(align & Align::Mode::left){
		itemBound.setSrcX(bound.getSrcX());
	}else { //centerX
		itemBound.setSrcX(bound.getSrcX() + (bound.getWidth() - itemBound.getWidth()) * 0.5f);
	}
}

void UI::LayoutCell::applyPosToItem(Elem* parent) const{
	applyAlignToItem(allocatedBound);

	float xMove{};
	float yMove{};

	if(align & Align::Mode::top) {
		yMove = -(pad.top + margin.top + parent->getBorder().top + getCellHeight() * scale.bottom);
	}else if(align & Align::Mode::bottom){
		yMove = pad.bottom + margin.bottom + parent->getBorder().bottom + getCellHeight() * scale.bottom;
	}

	if(align & Align::Mode::right) {
		xMove = -(pad.right + margin.right + parent->getBorder().right + getCellWidth() * scale.left);
	}else if(align & Align::Mode::left){
		xMove = pad.left + margin.left + parent->getBorder().left + getCellWidth() * scale.left;
	}

	// xMove += getCellWidth() * scale.left;
	// yMove += getCellHeight() * scale.bottom;

	item->bound.move(xMove, yMove);

	//TODO align...

	item->calAbsoluteSrc(parent);
}
