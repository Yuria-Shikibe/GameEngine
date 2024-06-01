module UI.Cell;

bool UI::LayoutCell::applySizeToItem(){ // NOLINT(*-make-member-function-const)
	const Geom::Vec2 originalSize = allocatedBound.getSize();

	if(hasDefWidth()){
		item->setWidth(defSize.x);
	}

	if(hasDefHeight()){
		item->setHeight(defSize.y);
	}

	//Apply Expansion
	if(modifyParentX) {
		const float width = Math::max(allocatedBound.getWidth(), item->getWidth());
		allocatedBound.setWidth(width * getHoriScale());
	}else{
		// allocatedBound.setShorterWidth(item->getWidth());
	}

	if(modifyParentY) {
		const float height = Math::max(allocatedBound.getHeight(), item->getHeight());
		allocatedBound.setHeight(height * getVertScale());
	}else{
		// allocatedBound.setShorterHeight(item->getHeight());
	}

	if(scaleRelativeToParentX){
		item->setWidth(allocatedBound.getWidth() * getHoriScale());
	}

	if(scaleRelativeToParentY){
		item->setHeight(allocatedBound.getHeight() * getVertScale());
	}


	//TODO uses validsize instead
	item->setSize(
		Math::clampPositive(item->getWidth() - getMarginHori()),
		Math::clampPositive(item->getHeight() - getMarginVert())
	);

	Geom::Vec2 currentSize = item->bound.getSize();
	if(currentSize == Geom::Vec2{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()}){
		item->setMaximumSize(allocatedBound.getSize());
	}

	item->changed(ChangeSignal::notifySubs, ChangeSignal::notifyParentOnly);
	item->layout();

	return allocatedBound.getSize() != originalSize;
}

void UI::LayoutCell::applyAlignToItem(const Rect bound) const{
	item->bound.setSrc(Align::getOffsetOf(align, item->getBound(), bound));
}

void UI::LayoutCell::applyPosToItem(Elem* parent) const{
	applyAlignToItem(allocatedBound);

	float xMove{};
	float yMove{};

	if(align & Align::Layout::top) {
		yMove = -(margin.top + getCellHeight() * scale.getSrcY());
	}else if(align & Align::Layout::bottom){
		yMove = margin.bottom + getCellHeight() * scale.getSrcY();
	}

	if(align & Align::Layout::right) {
		xMove = -(margin.right + getCellWidth() * scale.getSrcX());
	}else if(align & Align::Layout::left){
		xMove = margin.left + getCellWidth() * scale.getSrcX();
	}


	item->bound.move(xMove, yMove);
	//TODO align...

	item->calAbsoluteSrc(parent);
}
