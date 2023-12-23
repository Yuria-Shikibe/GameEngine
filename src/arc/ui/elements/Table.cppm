module;

export module UI.Table;
import UI.Group;
import <vector>;
import <memory_resource>;
import <array>;
import <algorithm>;
import <execution>;

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
			return std::tie(lhs.align, lhs.allocatedBound, lhs.marginLeft, lhs.marginRight, lhs.marginBottom,
			                lhs.marginTop, lhs.padLeft, lhs.padRight, lhs.padBottom, lhs.padTop, lhs.srcxScale,
			                lhs.srcyScale, lhs.endxScale, lhs.endyScale, lhs.modifyParentX, lhs.modifyParentY,
			                lhs.scaleRelativeToParentX, lhs.scaleRelativeToParentY) == std::tie(
				       rhs.align, rhs.allocatedBound, rhs.marginLeft, rhs.marginRight, rhs.marginBottom, rhs.marginTop,
				       rhs.padLeft, rhs.padRight, rhs.padBottom, rhs.padTop, rhs.srcxScale, rhs.srcyScale,
				       rhs.endxScale, rhs.endyScale, rhs.modifyParentX, rhs.modifyParentY, rhs.scaleRelativeToParentX,
				       rhs.scaleRelativeToParentY);
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

	class Table : public Group{
	public:
		size_t rowsCount = 0;
		size_t maxElemPerRow = 0;

		[[nodiscard]] Table() {
			touchbility = TouchbilityFlags::childrenOnly;

			color.a = 0.5f;
		}

		LayoutCell defaultCellLayout{nullptr};

		std::vector<LayoutCell> cells{};

		bool relativeLayoutFormat = true;

		//TODO mess
		void layoutRelative() {
			if(cells.empty() || std::ranges::all_of(std::as_const(cells), [](const LayoutCell& cell) {
				return cell.ignore();
			}))return;

			size_t curLayoutRows = rows();

			if(!cells.back().endRow()) {
				curLayoutRows++;
			}

			//Register Row Max width / height
			size_t curElemPerRow = 0;

			float currentColunmMaxWidth = 0.0f;


			float currentRowMaxHeight = 0.0f;
			float totalRowMaxHeight = 0.0f;

			for(const auto& cell : cells) {
				if(!cell.scaleRelativeToParentX) {
					currentColunmMaxWidth += cell.widthScale() * cell.item->getWidth();
				}

				if(cell.ignore()) {
					if(cell.endRow())goto endRow;
				}

				curElemPerRow++;

				if(!cell.scaleRelativeToParentY) {
					currentRowMaxHeight = std::fmaxf(cell.heightScale() * cell.item->getHeight(), currentRowMaxHeight);
				}

				if(cell.endRow())goto endRow;

				continue;

				endRow:

				totalRowMaxHeight += currentRowMaxHeight;
				bound.setLargerWidth(currentColunmMaxWidth);
				currentRowMaxHeight = currentColunmMaxWidth = 0.0f;

				if(curElemPerRow > maxElemPerRow) {
					maxElemPerRow = curElemPerRow;
				}

				curElemPerRow = 0;
			}

			if(curElemPerRow > maxElemPerRow) {
				maxElemPerRow = curElemPerRow;
			}

			totalRowMaxHeight += currentRowMaxHeight;

			bound.setLargerWidth(currentColunmMaxWidth);
			bound.setLargerHeight(totalRowMaxHeight);

			//Split all into boxes
			//TODO should cells have their own column or row data?
			int curX{0};
			int curY{0};

			//TODO is this necessary?
			std::vector<float> maxSizeArr(columns() + curLayoutRows);

			const float spacingX = bound.getWidth()  / static_cast<float>(columns());
			const float spacingY = bound.getHeight() / static_cast<float>(curLayoutRows);

			for(auto& cell : cells) {
				if(cell.ignore()) {
					if(cell.endRow()) {
						curY++;
						curX = 0;
					}
					continue;
				}

				cell.allocatedBound.setSize(spacingX, spacingY);

				cell.allocatedBound.setSrc(spacingX * curX, spacingY * (curLayoutRows - curY - 1));

				cell.applySize();

				maxSizeArr[curLayoutRows + curX] = std::fmaxf(maxSizeArr[curLayoutRows + curX], cell.getCellWidth());
				maxSizeArr[curY] = std::fmaxf(maxSizeArr[curY], cell.getCellHeight());

				curX++;
				if(cell.endRow()) {
					curY++;
					curX = 0;
				}
			}

			curX = curY = 0;

			for(auto& cell : cells) {
				if(cell.ignore()) {
					if(cell.endRow()) {
						curY++;
						curX = 0;
					}
					continue;
				}

				cell.allocatedBound.setSrc(maxSizeArr[curLayoutRows + curX] * curX, maxSizeArr[curY] * (curLayoutRows - curY - 1));

				cell.applyPos(this);

				curX++;
				if(cell.endRow()) {
					curY++;
					curX = 0;
				}
			}

			setSize(
				std::accumulate(maxSizeArr.begin() + curLayoutRows, maxSizeArr.end(), 0.0f),
				std::accumulate(maxSizeArr.begin(), maxSizeArr.begin() + curLayoutRows, 0.0f)
			);
		}

		void layoutIrrelative() {
			for(auto& cell : cells) {
				if(cell.ignore())continue;

				cell.allocatedBound = bound;

				cell.applySize();
				cell.applyPos(this);
			}
		}

		void layout() override {

			layout_fillParent();

			layoutChanged = false;

			if(relativeLayoutFormat) {
				layoutRelative();
			}else {
				layoutIrrelative();
			}

			layoutChildren();

			layoutChanged = false;
		}

		[[nodiscard]] size_t rows() const {
			return rowsCount;
		}

		[[nodiscard]] size_t columns() const {
			return maxElemPerRow;
		}

		void calAbsolute(Elem* parent) override {
			Elem::calAbsolute(parent);
			calAbsoluteChildren();
		}

		template <Concepts::Derived<Elem> T, Concepts::Invokable<void(T*)> Func = nullptr_t>
		LayoutCell& add(Func&& func = nullptr) {
			T* elem = new T;
			LayoutCell& cell = add(elem);
			cell.applyLayout(defaultCellLayout);

			if(func) {
				func(elem);
			}

			return cell;
		}

		template <Concepts::Derived<Elem> T, Concepts::Invokable<void(T*)> Func = nullptr_t>
		LayoutCell& add(const size_t depth = std::numeric_limits<size_t>::max(), Func&& func = nullptr) {
			T* elem = new T;
			LayoutCell& cell = add(elem, depth);
			cell.applyLayout(defaultCellLayout);

			if(func) {
				func(elem);
			}

			return cell;
		}

		LayoutCell& add(Elem* elem, const size_t depth = std::numeric_limits<size_t>::max()) { // NOLINT(*-non-const-parameter)
			addChildren(elem, depth);
			if(elem->endingRow())rowsCount++;
			return cells.emplace_back(elem).applyLayout(defaultCellLayout);
		}

		void removePosted() override {
			if(toRemove.empty() || children.empty())return;
			const auto&& itrCell = std::remove_if(std::execution::par_unseq, cells.begin(), cells.end(), [this](const LayoutCell& cell) {
				return toRemove.contains(cell.item);
			});

			if(itrCell == cells.end())return;

			const auto&& itr = std::remove_if(std::execution::par_unseq, children.begin(), children.end(), [this](const std::unique_ptr<Elem>& ptr) {
				return toRemove.contains(ptr.get());
			});

			cells.erase(itrCell);
			children.erase(itr);
			toRemove.clear();
		}

		void endRow() {
			if(!relativeLayoutFormat || children.empty())return;

			rowsCount++;
			children.back()->setEndingRow(true);
		}

		void update(const float delta) override {
			//TODO move this into listener
			if(layoutChanged) {
				layout();
			}

			Elem::update(delta);
		}
	};
}


