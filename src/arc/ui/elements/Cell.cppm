module;

export module UI.Cell;

import Align;
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
		Align::Mode align = Align::Mode::bottom_left;
		void changed() const {
			if(item) {
				item->changed();
			}
		}
	public:
		//Weak Reference Object
		Elem* item{nullptr};

		Rect allocatedBound{};

		float marginLeft{0};
		float marginRight{0};
		float marginBottom{0};
		float marginTop{0};

		float padLeft{0};
		float padRight{0};
		float padBottom{0};
		float padTop{0};

		//scales are done after margin, so it calculates the size shrunk after margin
		float srcxScale{0.0f};
		float srcyScale{0.0f};

		float endxScale{1.0f};
		float endyScale{1.0f};

		LayoutCell(const LayoutCell& other){std::memcpy(this, &other, sizeof(LayoutCell));}

		LayoutCell(LayoutCell&& other) noexcept{std::memcpy(this, &other, sizeof(LayoutCell));}

		LayoutCell& operator=(const LayoutCell& other) {
			if(this == &other) return *this;
			std::memcpy(this, &other, sizeof(LayoutCell));
			return *this;
		}

		LayoutCell& operator=(LayoutCell&& other) noexcept {
			if(this == &other) return *this;
			std::memcpy(this, &other, sizeof(LayoutCell));
			return *this;
		}

		/**
		 * \brief When true, the cell will expand parent group when the room isn't enough.
		 */
		bool modifyParentX{false};
		bool modifyParentY{false};

		bool scaleRelativeToParentX{true};
		bool scaleRelativeToParentY{true};

		[[nodiscard]] bool endRow() const {
			return item->endingRow();
		}

		friend bool operator==(const LayoutCell& lhs, const LayoutCell& rhs) {
			return lhs.align == rhs.align
			       && lhs.allocatedBound == rhs.allocatedBound
			       && lhs.marginLeft == rhs.marginLeft
			       && lhs.marginRight == rhs.marginRight
			       && lhs.marginBottom == rhs.marginBottom
			       && lhs.marginTop == rhs.marginTop
			       && lhs.padLeft == rhs.padLeft
			       && lhs.padRight == rhs.padRight
			       && lhs.padBottom == rhs.padBottom
			       && lhs.padTop == rhs.padTop
			       && lhs.srcxScale == rhs.srcxScale
			       && lhs.srcyScale == rhs.srcyScale
			       && lhs.endxScale == rhs.endxScale
			       && lhs.endyScale == rhs.endyScale
			       && lhs.modifyParentX == rhs.modifyParentX
			       && lhs.modifyParentY == rhs.modifyParentY
			       && lhs.scaleRelativeToParentX == rhs.scaleRelativeToParentX
			       && lhs.scaleRelativeToParentY == rhs.scaleRelativeToParentY;
		}

		friend bool operator!=(const LayoutCell& lhs, const LayoutCell& rhs) {
			return !(lhs == rhs);
		}

		LayoutCell& wrapX() {
			changed();
			scaleRelativeToParentX = false;
			return *this;
		}

		LayoutCell& wrapY() {
			changed();
			scaleRelativeToParentY = false;
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

		LayoutCell& setMargin(const float left, const float right, const float bottom, const float top) {
			marginLeft = left;
			marginRight = right;
			marginBottom = bottom;
			marginTop = top;

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

		LayoutCell& setSrcScale(const float xScl, const float yScl) {
			changed();
			srcxScale = xScl;
			srcyScale = yScl;

			return *this;
		}

		LayoutCell& setEndScale(const float xScl, const float yScl) {
			changed();
			endxScale = xScl;
			endyScale = yScl;

			return *this;
		}

		LayoutCell& setSizeScale(const float xScl, const float yScl, const Align::Mode align = Align::Mode::center) {
			changed();
			if(align & Align::Mode::top) {
				srcyScale = endyScale - yScl;
			}else if(align & Align::Mode::bottom){
				endyScale = srcyScale + yScl;
			}else { //centerY
				endyScale = 0.5f + yScl * 0.5f;
				srcyScale = 0.5f - yScl * 0.5f;
			}

			if(align & Align::Mode::right) {
				srcxScale = endxScale - xScl;
			}else if(align & Align::Mode::left){
				endxScale = srcxScale + xScl;
			}else { //centerX
				endxScale = 0.5f + xScl * 0.5f;
				srcxScale = 0.5f - xScl * 0.5f;
			}

			return *this;
		}

		LayoutCell& clearRelativeMove() {
			changed();
			endxScale -= srcxScale;
			endyScale -= srcyScale;

			srcxScale = srcyScale = 0;

			return *this;
		}

		[[nodiscard]] explicit LayoutCell(Elem* const item)
			: item(item) {
		}

		[[nodiscard]] float getCellWidth() const {return allocatedBound.getWidth();}
		[[nodiscard]] float getCellHeight() const {return allocatedBound.getHeight();}

		[[nodiscard]] float widthScale() const {return endxScale - srcxScale;}
		[[nodiscard]] float heightScale() const {return endyScale - srcyScale;}

		[[nodiscard]] float marginHori() const {return marginLeft + marginRight;}
		[[nodiscard]] float marginVert() const {return marginBottom + marginTop;}

		[[nodiscard]] float padHori() const {return padLeft + padRight;}
		[[nodiscard]] float padVert() const {return padBottom + padTop;}

		void applySize(){ // NOLINT(*-make-member-function-const)
			const float width = (scaleRelativeToParentX ? allocatedBound.getWidth() : item->getBound().getWidth());
			const float height = (scaleRelativeToParentY ? allocatedBound.getHeight() : item->getBound().getHeight());

			//Modify item size
			item->setSize(width * widthScale(), height * heightScale());

			//Apply Expansion
			if(modifyParentX) {
				allocatedBound.setWidth(item->getBound().getWidth());
			}

			if(modifyParentY) {
				allocatedBound.setHeight(item->getBound().getHeight());
			}

			item->getBound().addSize(-marginHori(), -marginVert());

			allocatedBound.addSize(padHori(), padVert());
		}

		void applyAlign(const Rect& bound) const {
			Rect& itemBound = item->getBound();
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
		void applyPos(Elem* parent) const {
			applyAlign(allocatedBound);

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

			const float xMove = xSign * ((xSign == 1 ? (padLeft + marginLeft) : (padRight + marginRight)) + getCellWidth() * srcxScale);
			const float yMove = ySign * ((ySign == 1 ? (padBottom + marginBottom) : (padTop + marginTop)) + getCellHeight() * srcyScale);

			item->getBound().move(xMove, yMove);

			//TODO align...

			item->calAbsolute(parent);
		}

		[[nodiscard]] bool ignore() const {
			return item->ignoreLayout();
		}

		template <Concepts::Derived<Elem> T>
		T& as() {
			return dynamic_cast<T&>(*item);
		}
	};
}
