module;

export module UI.Table;
export import UI.Group;
export import UI.Elem;
export import UI.Flags;
export import UI.Cell;

import std;

import Geom.Rect_Orthogonal;
import UI.Align;
import ext.Concepts;
import Geom.Vector2D;

using Rect = Geom::OrthoRectFloat;

//TODO Using Pool to avoid heap allocation
export namespace UI {
	template <Concepts::Derived<Elem> T>
	struct ElemCreater{
		using ElemType = T;
		void operator()(T& elem) const = delete;
		void operator()(LayoutCell& cell) const{}
	};
	class Table : public Group{
	protected:
		Align::Layout cellAlignMode = Align::Layout::center;
		bool relativeLayoutFormat = true;

		template <typename T>
		LayoutCell& tryAdd(const unsigned depth = std::numeric_limits<unsigned>::max()){
			if constexpr (requires(UI::Root* root){
				T{root};
			}){
				return cells.emplace_back(this->addChildren(std::make_unique<T>(getRoot()), depth));
			}else{
				return cells.emplace_back(this->addChildren(std::make_unique<T>(), depth));
			}
		}

	public:
		std::vector<unsigned> elemLayoutCountData{};
		Geom::Point2 elemLayoutMaxCount{};

		[[nodiscard]] explicit Table(UI::Root* root = nullptr) : Group{root}{
			touchbility = TouchbilityFlags::childrenOnly;
		}

		LayoutCell defaultCellLayout{};

		std::vector<LayoutCell> cells{};

		[[nodiscard]] Align::Layout getCellAlignMode() const{
			return cellAlignMode;
		}

		void setCellAlignMode(const Align::Layout cellAlignMode){
			this->cellAlignMode = cellAlignMode;
		}

		[[nodiscard]] bool isRelativeLayoutFormat() const{ return relativeLayoutFormat; }

		void setLayoutByRelative(const bool relativeLayoutFormat){
			this->relativeLayoutFormat = relativeLayoutFormat;
		}

		//TODO mess
		void layoutRelative();

		void layoutIrrelative();

		void layout() override;

		LayoutCell& getLast(){
			return cells.empty() ? defaultCellLayout : cells.back();
		}

		[[nodiscard]] int rows() const {
			return elemLayoutMaxCount.y;
		}

		[[nodiscard]] int columns() const {
			return elemLayoutMaxCount.x;
		}



		template <typename Creater = ElemCreater<Elem>, Concepts::InvokeNullable<void(typename Creater::ElemType&)> Func = std::nullptr_t>
			requires requires(Creater creater){
				typename Creater::ElemType;
				requires Concepts::Derived<typename Creater::ElemType, Elem>;
				requires Concepts::Derived<Creater, ElemCreater<typename Creater::ElemType>>;
			}
		LayoutCell& emplace(const Creater& creater, Func&& func = nullptr) {
			using T = typename Creater::ElemType;
			static_assert(requires(Creater c, T elem){
				c(elem);
			}, "Creater must have the ability to modify the item to emplace");
			LayoutCell& cell = tryAdd<T>();
			cell.applyLayout(defaultCellLayout);

			creater(cell.as<T>());

			if constexpr (!std::same_as<Func, std::nullptr_t>){
				func(cell.as<T>());
			}

			if constexpr (requires(Creater c, LayoutCell _cell){
				c(_cell);
			}){
				creater(cell);
			}

			return cell;
		}

		template <Concepts::Derived<Elem> T>
		LayoutCell& add(Concepts::Invokable<void(T&)> auto&& func, const unsigned depth = std::numeric_limits<unsigned>::max()) {
			LayoutCell& cell = tryAdd<T>(depth);
			cell.applyLayout(defaultCellLayout);

			func(cell.as<T>());

			return cell;
		}

		template <Concepts::Derived<Elem> T, Concepts::Invokable<void(T&)> Func, typename ...Args>
		LayoutCell& emplace(Func&& func, Args&& ...args) {
			LayoutCell& cell = cells.emplace_back(this->addChildren(std::make_unique<T>(std::forward<Args>(args) ...)));
			cell.applyLayout(defaultCellLayout);

			func(cell.as<T>());

			return cell;
		}

		template <Concepts::Derived<Elem> T, typename ...Args>
		LayoutCell& emplace(Args&& ...args) {
			LayoutCell& cell = cells.emplace_back(this->addChildren(std::make_unique<T>(std::forward<Args>(args) ...)));
			cell.applyLayout(defaultCellLayout);

			return cell;
		}

		LayoutCell& transferElem(Elem* elem, const unsigned depth = std::numeric_limits<unsigned>::max()) { // NOLINT(*-non-const-parameter)
			addChildren(elem, depth);
			return cells.emplace_back(LayoutCell{.item = elem}).applyLayout(defaultCellLayout);
		}

		LayoutCell& transferElem(std::unique_ptr<Elem>&& elem, const unsigned depth = std::numeric_limits<unsigned>::max()) { // NOLINT(*-non-const-parameter)
			auto* ptr = addChildren(std::move(elem), depth);
			return cells.emplace_back(LayoutCell{.item = ptr}).applyLayout(defaultCellLayout);
		}

		constexpr LayoutCell* getCellOf(Elem* elem){
			if(const auto itr = std::ranges::find(cells, elem, &LayoutCell::item); itr != cells.end()){
				return itr.operator->();
			}

			return nullptr;
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

		void clearChildrenInstantly() override{
			Group::clearChildrenInstantly();
			cells.clear();
		}

		void lineFeed() {
			if(!relativeLayoutFormat || children.empty())return;

			elemLayoutMaxCount.y++;
			children.back()->setEndRow(true);
			changed(ChangeSignal::notifyAll);
		}

		void update(const Core::Tick delta) override {
			//TODO move this into listener
			Group::update(delta);
		}

		Rect getFilledChildrenBound(Elem* elem) const noexcept override{
			return elem->getBound();
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

		void drawContent() const override;
	};
}


