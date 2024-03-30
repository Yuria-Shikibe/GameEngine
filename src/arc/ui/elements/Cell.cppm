module;

export module UI.Cell;

import UI.Align;
import Concepts;
import UI.Elem;
import std;

export namespace UI {
	/**
	 * @code
	 *	  Inner   BOUND  Exter
	 *             |
	 *   Margin <--|--> Pad
	 *             |
	 * @endcode
	 */
	struct LayoutCell {
	protected:
		void changed() const {
			if(item) {
				item->changed();
			}
		}
	public:
		//Weak Reference Object
		Elem* item{nullptr};

		Rect allocatedBound{};

		Align::Spacing margin{};

		Align::Spacing pad{};

		//scales are done after margin, so it calculates the size shrunk after margin
		Align::Spacing scale{0, 1.0, 0, 1.0};


		Align::Mode align = Align::Mode::bottom_left;

		/**
		 * \brief When true, the cell will expand parent group when the room isn't enough.
		 */
		bool modifyParentX{false};
		bool modifyParentY{false};

		bool scaleRelativeToParentX{true};
		bool scaleRelativeToParentY{true};

		[[nodiscard]] bool isEndRow() const {
			return item->isEndingRow();
		}

		LayoutCell& lineFeed(){
			item->setEndRow(true);

			return *this;
		}

		LayoutCell& setSize(const float w, const float h){
			item->setSize(w, h);

			return *this;
		}

		LayoutCell& setSize(const float s){
			item->setSize(s);

			return *this;
		}

		LayoutCell& setHeight(float h){
			item->setHeight(h);

			return *this;
		}

		LayoutCell& setWidth(const float w){
			item->setWidth(w);

			return *this;
		}

		LayoutCell& wrapX() {
			changed();
			scaleRelativeToParentX = false;
			modifyParentX = false;
			return *this;
		}

		LayoutCell& wrapY() {
			changed();
			scaleRelativeToParentY = false;
			modifyParentY = false;
			return *this;
		}

		LayoutCell& wrap() {
			changed();
			scaleRelativeToParentX = false;
			scaleRelativeToParentY = false;
			return *this;
		}

		LayoutCell& fillParentX() {
			changed();
			scaleRelativeToParentX = true;
			return *this;
		}

		LayoutCell& fillParentY() {
			changed();
			scaleRelativeToParentY = true;
			return *this;
		}

		LayoutCell& fillParent() {
			changed();
			scaleRelativeToParentX = true;
			scaleRelativeToParentY = true;
			return *this;
		}

		LayoutCell& setMargin(const float left, const float right, const float top, const float bottom) {
			margin = {left, right, bottom, top};
			changed();

			return *this;
		}

		LayoutCell& setMargin(const float val) {
			margin.set(val);
			changed();

			return *this;
		}

		LayoutCell& setMargin(const Align::Spacing margin) {
			if(this->margin != margin){
				this->margin = margin;
				changed();
			}

			return *this;
		}

		LayoutCell& setPad(const Align::Spacing pad) {
			if(this->pad != pad){
				this->pad = pad;
				changed();
			}

			return *this;
		}


		LayoutCell& expandX(const bool val = true) {
			val ? wrapX() : fillParentX();
			modifyParentX = val;
			return *this;
		}

		LayoutCell& expandY(const bool val = true) {
			val ? wrapY() : fillParentY();
			modifyParentY = true;
			return *this;
		}

		LayoutCell& expand(const bool valX = true, const bool valY = true) {
			expandX(valX);
			expandY(valY);
			return *this;
		}

		LayoutCell& applyLayout(const LayoutCell& other) {
			if(this == &other) return *this;
			changed();
			const auto cur = item;
			*this = other;
			item = cur;
			return *this;
		}

		LayoutCell& setAlign(const Align::Mode align = Align::Mode::center) {\
			if(align == this->align)return *this;
			changed();
			this->align = align;

			return *this;
		}

		LayoutCell& setSrcScale(const float xScl, const float yScl, const bool move = true) {
			changed();

			if(move) {
				const float dstX = getHoriScale();
				const float dstY = getVertScale();

				scale.left = xScl;
				scale.bottom = yScl;

				scale.right = xScl + dstX;
				scale.top = yScl + dstY;
			}else {
				scale.left = xScl;
				scale.bottom = yScl;
			}

			return *this;
		}

		LayoutCell& setEndScale(const float xScl, const float yScl) {
			changed();
			scale.right = xScl;
			scale.top = yScl;

			return *this;
		}

		LayoutCell& setSizeScale(const float xScl, const float yScl, const Align::Mode align = Align::Mode::center, const bool needClearRelaMove = true) {
			changed();
			if(align & Align::Mode::top) {
				scale.bottom = scale.top - yScl;
			}else if(align & Align::Mode::bottom){
				scale.top = scale.bottom + yScl;
			}else { //centerY
				scale.top = 0.5f + yScl * 0.5f;
				scale.bottom = 0.5f - yScl * 0.5f;
			}

			if(align & Align::Mode::right) {
				scale.left = scale.right - xScl;
			}else if(align & Align::Mode::left){
				scale.right = scale.left + xScl;
			}else { //centerX
				scale.right = 0.5f + xScl * 0.5f;
				scale.left = 0.5f - xScl * 0.5f;
			}

			changed();
			if(needClearRelaMove)this->clearRelativeMove();

			return *this;
		}

		LayoutCell& clearRelativeMove() {
			changed();
			scale.right -= scale.left;
			scale.top -= scale.bottom;

			scale.left = scale.bottom = 0;

			return *this;
		}

		[[nodiscard]] float getCellWidth() const {return allocatedBound.getWidth();}
		[[nodiscard]] float getCellHeight() const {return allocatedBound.getHeight();}
		[[nodiscard]] float getScaledCellHeight() const {return getVertScale() * getCellHeight();}
		[[nodiscard]] float getScaledCellWidth() const {return getHoriScale() * getCellWidth();}

		[[nodiscard]] float getExpectedItemHeight() const{
			return item->getHeight();
		}
		[[nodiscard]] float getExpectedItemWidth() const{
			return item->getWidth();
		}


		[[nodiscard]] float getHoriScale() const {return scale.right - scale.left;}
		[[nodiscard]] float getVertScale() const {return scale.top - scale.bottom;}

		[[nodiscard]] float getMarginHori() const {return margin.getMarginWidth();}
		[[nodiscard]] float getMarginVert() const {return margin.getMarginHeight();}

		[[nodiscard]] float getPadHori() const {return pad.getMarginWidth();}
		[[nodiscard]] float getPadVert() const {return pad.getMarginHeight();}

		void applySizeToItem(){ // NOLINT(*-make-member-function-const)
			const float width = (scaleRelativeToParentX ? allocatedBound.getWidth() : item->getWidth());
			const float height = (scaleRelativeToParentY ? allocatedBound.getHeight() : item->getHeight());

			//Modify item size
			item->setSize(width * getHoriScale(), height * getVertScale());

			//Apply Expansion
			if(modifyParentX) {
				allocatedBound.setWidth(item->getWidth());
			}else{
				// allocatedBound.setShorterWidth(item->getWidth());
			}

			if(modifyParentY) {
				allocatedBound.setHeight(item->getHeight());
			}else{
				// allocatedBound.setShorterHeight(item->getHeight());
			}

			item->getBoundRef().addSize(-getMarginHori(), -getMarginVert());
			item->changed();

			allocatedBound.addSize(getPadHori(), getPadVert());
		}

		void applyAlignToItem(const Rect bound) const {
			Rect& itemBound = item->getBoundRef();
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

		//Invoke this after all cell bound has been arranged.
		void applyPosToItem(Elem* parent) const {
			applyAlignToItem(allocatedBound);

			float xSign = 0;
			float ySign = 0;

			if(align & Align::Mode::top) {
				ySign = -1;
			}else if(align & Align::Mode::bottom){
				ySign = 1;
			}

			if(align & Align::Mode::right) {
				xSign = -1;
			}else if(align & Align::Mode::left){
				xSign = 1;
			}

			const bool left   = xSign == 1;
			const bool bottom = ySign == 1;

			const float xMove = xSign * ((left   ? (pad.left + margin.left + parent->getMargin().left) : (pad.right + margin.right + parent->getMargin().right)) + getCellWidth() * scale.left);
			const float yMove = ySign * ((bottom ? (pad.bottom + margin.bottom + parent->getMargin().bottom) : (pad.top + margin.top + parent->getMargin().top)) + getCellHeight() * scale.bottom);

			item->getBoundRef().move(xMove, yMove);

			//TODO align...

			item->calAbsoluteSrc(parent);
		}

		[[nodiscard]] bool isIgnoreLayout() const {
			return item->isIgnoreLayout();
		}

		template <Concepts::Derived<Elem> T>
		T& as() {//TODO static cast maybe??
			return dynamic_cast<T&>(*item);
		}
	};
}
