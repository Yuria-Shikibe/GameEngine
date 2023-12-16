module;

export module UI.Group;

export import UI.Elem;
import <vector>;
import Geom.Shape.Rect_Orthogonal;
import Align;

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
		Elem* item{nullptr};

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

		// LayoutCell& setAlign(const Align::Mode align = Align::Mode::center) {
		// 	this->align = align;
		//
		// 	return *this;
		// }

		[[nodiscard]] explicit LayoutCell(Elem* const item)
			: item(item) {
		}
	};

	class LayoutableGroup : public Elem{
	public:
		float margin{0.0f};

		std::vector<Rect> rowBounds{};
		std::vector<LayoutCell> cells{};

		void layout() override {
			rowBounds.clear();
			Rect rowBound{};

			//Register Row Max width / height
			for(const auto& child : children) {
				rowBound.addSize(child->getBound().getWidth(), 0);
				rowBound.setLargerHeight(child->getBound().getHeight());

				if(child->endingRow()) {
					rowBounds.push_back({std::move(rowBound)});
				}
			}
		}

		[[nodiscard]] size_t rows() const {
			return rowBounds.size();
		}

		virtual LayoutCell& addChildren(Elem* elem) {
			children.push_back(elem);
			return cells.emplace_back(elem);
		}

		void draw() const override {
			drawChildren();
		}

		void update(const float delta) override {

		}
	};
}


