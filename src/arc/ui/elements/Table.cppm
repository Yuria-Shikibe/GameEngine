module;

export module UI.Table;
export import UI.Group;
export import UI.Elem;
export import UI.Flags;
export import UI.Cell;

import std;

import Geom.Shape.Rect_Orthogonal;
import UI.Align;
import Concepts;
import Geom.Vector2D;

//TODO this is a temp import
import Graphic.Draw;

using Rect = Geom::OrthoRectFloat;

//TODO Using Pool to avoid heap allocation
export namespace UI {
	class Table : public Group{
	public:
		std::vector<unsigned> elemLayoutCountData{};
		Geom::Point2 elemLayoutMaxCount{};

		[[nodiscard]] Table() {
			touchbility = TouchbilityFlags::childrenOnly;
			color.a = 0.5f;
		}

		LayoutCell defaultCellLayout{};

		std::vector<LayoutCell> cells{};

		bool relativeLayoutFormat = true;

		//TODO mess
		void layoutRelative();

		void layoutIrrelative();

		void layout() override {
			layout_tryFillParent();

			layoutChildren();

			if(relativeLayoutFormat) {
				layoutRelative();
			}else {
				layoutIrrelative();
			}

			Group::layout();
		}

		[[nodiscard]] int rows() const {
			return elemLayoutMaxCount.y;
		}

		[[nodiscard]] int columns() const {
			return elemLayoutMaxCount.x;
		}

		template <Concepts::Derived<Elem> T>
		LayoutCell& add(const int depth = std::numeric_limits<int>::max()) {
			LayoutCell& cell = cells.emplace_back(this->addChildren(std::make_unique<T>(), depth));
			cell.applyLayout(defaultCellLayout);

			return cell;
		}

		template <Concepts::Derived<Elem> T>
		LayoutCell& add(Concepts::Invokable<void(T&)> auto&& func, const int depth = std::numeric_limits<int>::max()) {
			LayoutCell& cell = cells.emplace_back(this->addChildren(std::make_unique<T>(), depth));
			cell.applyLayout(defaultCellLayout);

			if constexpr (!std::same_as<decltype(func), std::nullptr_t>) {
				func(cell.as<T>());
			}

			return cell;
		}

		LayoutCell& transferElem(Elem* elem, const size_t depth = std::numeric_limits<size_t>::max()) { // NOLINT(*-non-const-parameter)
			addChildren(elem, depth);
			return cells.emplace_back(LayoutCell{.item = elem}).applyLayout(defaultCellLayout);
		}

		void removePosted() override {
			//TODO the two itr should at the same position...
			if(toRemove.empty() || children.empty())return;

			std::erase_if(cells, [this](const decltype(cells)::value_type& cell) {
				return toRemove.contains(cell.item);
			});
			std::erase_if(children, [this](const decltype(children)::value_type& ptr) {
				return toRemove.contains(ptr.get());
			});

			toRemove.clear();
		}

		void lineFeed() {
			if(!relativeLayoutFormat || children.empty())return;

			elemLayoutMaxCount.y++;
			children.back()->setEndRow(true);
			changed();
		}

		void update(const float delta) override {
			//TODO move this into listener
			if(layoutChanged) {
				layout();
			}

			Group::update(delta);
		}

		void recalculateLayoutSize(){
			elemLayoutMaxCount.setZero();
			elemLayoutCountData.clear();

			auto curLayoutRows =
				std::ranges::count_if(cells, std::identity{}, &LayoutCell::isEndRow);

			//Make Sure There at least exist one row!
			if(!cells.back().isEndRow())curLayoutRows++;
			elemLayoutMaxCount.y = static_cast<int>(curLayoutRows);

			elemLayoutCountData.resize(rows());

			int curElemPerRow = 0;
			int curLine = 0;
			for(const auto& cell : cells) {
				if(!cell.isIgnoreLayout()){
					curElemPerRow++;
				}

				if(cell.isEndRow()){
					elemLayoutMaxCount.maxX(curElemPerRow);
					elemLayoutCountData.at(curLine++) = curElemPerRow;
					curElemPerRow = 0;
				}
			}

			if(!cells.back().isEndRow()){
				elemLayoutMaxCount.maxX(curElemPerRow);
				elemLayoutCountData.at(curLine++) = curElemPerRow;
			}
		}

		void drawContent() const override{
			// for(auto& cell : this->cells){
			// 	Rect rect{};
			// 	rect.setSrc(this->absoluteSrc + cell.allocatedBound.getSrc() + this->border.bot_lft()).setSize(cell.allocatedBound);
			//
			// 	using namespace Graphic;
			// 	Draw::color(Colors::YELLOW);
			// 	Draw::Line::setLineStroke(2.0f);
			// 	Draw::Line::rectOrtho(rect);
			// }

			Group::drawContent();

		}
	};
}


