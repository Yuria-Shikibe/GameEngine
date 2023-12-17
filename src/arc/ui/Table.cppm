module;

export module UI.Group;

import UI.Elem;
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

		Align::Mode align = Align::Mode::center;

		[[nodiscard]] bool endRow() const {
			return item->endingRow();
		}

		LayoutCell& setAlign(const Align::Mode align = Align::Mode::center) {
			this->align = align;

			return *this;
		}

		[[nodiscard]] explicit LayoutCell(Elem* const item)
			: item(item) {
		}

		[[nodiscard]] float getWidth() const {return allocatedBound.getWidth();}
		[[nodiscard]] float getHeight() const {return allocatedBound.getHeight();}

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
			item->getBound().setSrc(allocatedBound);
			item->getBound().move(allocatedBound.getWidth() * srcxScale + padLeft + marginLeft, allocatedBound.getHeight() * srcyScale + padBottom + marginBottom);

			//TODO align...

			item->calAbsolute(parent);
		}

		template <Concepts::Derived<Elem> T>
		T& as() {
			return reinterpret_cast<T&>(*item);
		}
	};

	class LayoutableGroup : public Elem{
	public:
		size_t rowsCount = 0;
		size_t maxElemPerRow = 0;

		std::vector<LayoutCell> cells{};

		void layout() override {
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

				maxColumnWidth[curX] = std::max(maxColumnWidth[curX], cell.getWidth());
				maxRowHeight[curY] = std::max(maxRowHeight[curX], cell.getHeight());

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
			//Cell Allocation Done
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
			if(children.empty())return;

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


