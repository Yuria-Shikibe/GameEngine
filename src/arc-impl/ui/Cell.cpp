module UI.Cell;

bool UI::LayoutCell::applySizeToItem(){ // NOLINT(*-make-member-function-const)
	const Geom::Vec2 originalSize = allocatedBound.getSize();

	Geom::Vec2 exportSize{};

	if(hasDefWidth()){
		exportSize.x = defSize.x;
	}

	if(hasDefHeight()){
		exportSize.y = defSize.y;
	}

	//Apply Expansion
	if(modifyParentX) {
		const float width = Math::max(allocatedBound.getWidth(), exportSize.x);
		allocatedBound.setWidth(width * getHoriScale());
	}else{
		// allocatedBound.setShorterWidth(item->getWidth());
	}

	if(modifyParentY) {
		const float height = Math::max(allocatedBound.getHeight(), exportSize.y);
		allocatedBound.setHeight(height * getVertScale());
	}else{
		// allocatedBound.setShorterHeight(item->getHeight());
	}

	if(scaleRelativeToParentX || isSlaveX() || hasRatioFromHeight()){
		exportSize.x = allocatedBound.getWidth() * getHoriScale();
	}

	if(scaleRelativeToParentY || isSlaveY() || hasRatioFromWidth()){
		exportSize.y = allocatedBound.getHeight() * getVertScale();
	}

	// exportSize.clampX(0, allocatedBound.getWidth());
	// exportSize.clampY(0, allocatedBound.getHeight());
	// item->maximumSize = allocatedBound.getSize();
	item->setWidth_Quiet(Math::clampPositive(exportSize.x - getMarginHori()));
	item->setHeight_Quiet(Math::clampPositive(exportSize.y - getMarginVert()));

	// const Geom::Vec2 currentSize = item->bound.getSize();
	// if(currentSize == Geom::maxVec2<float>){

	// }

	item->changed(ChangeSignal::notifySubs, ChangeSignal::notifyParentOnly);

	return allocatedBound.getSize() != originalSize;
}

void UI::LayoutCell::applyAlignToItem(const Rect bound) const{
	item->bound.setSrc(Align::getOffsetOf(align, item->getBound(), bound));
}

void UI::LayoutCell::applyPosToItem(Elem* parent) const{
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

	item->layout();

	applyAlignToItem(allocatedBound);

	item->bound.move(xMove, yMove);
	//TODO align...
	item->calAbsoluteSrc(parent);
}
