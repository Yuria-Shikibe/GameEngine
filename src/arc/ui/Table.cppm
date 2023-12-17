module;

export module UI.Table;
import UI.Group;
import <vector>;
import <algorithm>;
import Geom.Shape.Rect_Orthogonal;
import Align;
import Concepts;


using Rect = Geom::Shape::OrthoRectFloat;

//TODO Using Pool to avoid heap allocation
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
		bool modifyParentOnExpansionX{false};
		bool modifyParentOnExpansionY{false};

		bool scaleRelativeToParentX{true};
		bool scaleRelativeToParentY{true};

		[[nodiscard]] bool endRow() const {
			return item->endingRow();
		}

		LayoutCell& setAlign(const Align::Mode align = Align::Mode::center) {
			this->align = align;

			return *this;
		}

		LayoutCell& setSrcScale(const float xScl, const float yScl) {
			srcxScale = xScl;
			srcyScale = yScl;

			return *this;
		}

		LayoutCell& setEndScale(const float xScl, const float yScl) {
			endxScale = xScl;
			endyScale = yScl;

			return *this;
		}

		LayoutCell& setSizeScale(const float xScl, const float yScl, Align::Mode align = Align::Mode::center) {
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
			endxScale -= srcxScale;
			endyScale -= srcyScale;

			srcxScale = srcyScale = 0;

			return *this;
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
				itemBound.setSrcY(bound.getSrcX() + (bound.getWidth() - itemBound.getWidth()) * 0.5f);
			}
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
			const float width = (scaleRelativeToParentX ? allocatedBound.getWidth() : item->getBound().getWidth()) - marginHori();
			const float height = (scaleRelativeToParentY ? allocatedBound.getHeight() : item->getBound().getHeight()) - marginVert();

			//Modify item size
			item->setSize(width * widthScale(), height * heightScale());

			//Apply Expansion
			if(modifyParentOnExpansionX) {
				allocatedBound.setLargerWidth(item->getBound().getWidth());
			}

			if(modifyParentOnExpansionY) {
				allocatedBound.setLargerHeight(item->getBound().getHeight());
			}

			allocatedBound.addSize(padHori(), padVert());
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

			item->getBound().move((getCellWidth() * srcxScale + padLeft + marginLeft) * xSign, (getCellHeight() * srcyScale + padBottom + marginBottom) * ySign);

			//TODO align...

			item->calAbsolute(parent);
		}

		template <Concepts::Derived<Elem> T>
		T& as() {
			return reinterpret_cast<T&>(*item);
		}
	};

	class Table : public Group{
	public:
		size_t rowsCount = 0;
		size_t maxElemPerRow = 0;

		std::vector<LayoutCell> cells{};

		bool relativeLayoutFormat = true;

		void layoutRelative() {
			if(cells.empty())return;

			if(!children.back()->endingRow()) {
				endRow();
			}

			if(rows() == 0) {
				rowsCount++;
				children.back()->setEndingRow(true);
			}

			//Register Row Max width / height
			size_t curElemPerRow = 0;
			for(const auto& cell : cells) {
				curElemPerRow++;

				if(cell.endRow()) {
					if(curElemPerRow > maxElemPerRow) {
						maxElemPerRow = curElemPerRow;
					}

					curElemPerRow = 0;
				}
			}

			//Split all into boxes
			//TODO should cells have their own column or row data?
			int curX{0};
			int curY{0};

			auto* maxColumnWidth = new float[columns()]{0};
			auto* maxRowHeight = new float[rows()]{0};

			const float spacingX = bound.getWidth()  / static_cast<float>(columns());
			const float spacingY = bound.getHeight() / static_cast<float>(rows());

			for(auto& cell : cells) {
				cell.allocatedBound.setSize(spacingX, spacingY);

				cell.allocatedBound.setSrc(spacingX * curX, spacingY * (rows() - curY - 1));

				cell.applySize();

				maxColumnWidth[curX] = std::max(maxColumnWidth[curX], cell.getCellWidth());
				maxRowHeight[curY] = std::max(maxRowHeight[curX], cell.getCellHeight());

				curX++;
				if(cell.endRow()) {
					curY++;
					curX = 0;
				}
			}

			curX = curY = 0;

			for(auto& cell : cells) {
				cell.allocatedBound.setSrc(maxColumnWidth[curX] * curX, maxRowHeight[curX] * (rows() - curY - 1));

				cell.applyPos(this);

				curX++;
				if(cell.endRow()) {
					curY++;
					curX = 0;
				}
			}

			delete[] maxColumnWidth;
			delete[] maxRowHeight;
		}

		void layoutIrrelative() {
			for(auto& cell : cells) {
				cell.allocatedBound = bound;

				cell.applySize();
				cell.applyPos(this);
			}
		}

		void layout() override {
			if(relativeLayoutFormat) {
				layoutRelative();
			}else {
				layoutIrrelative();
			}
		}

		[[nodiscard]] size_t rows() const {
			return rowsCount;
		}

		[[nodiscard]] size_t columns() const {
			return maxElemPerRow;
		}

		LayoutCell& add(Elem* elem) {
			addChildren(elem);
			if(elem->endingRow())rowsCount++;
			return cells.emplace_back(elem);
		}

		void endRow() {
			if(!relativeLayoutFormat || children.empty())return;

			rowsCount++;
			children.back()->setEndingRow(true);
		}

		void update(const float delta) override {
			//TODO move this into listener
			if(layoutChanged) {
				layoutChanged = false;
				layout();
			}

			updateChildren(delta);
		}
	};
}


