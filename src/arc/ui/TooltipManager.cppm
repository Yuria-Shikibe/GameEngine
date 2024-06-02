//
// Created by Matrix on 2024/4/11.
//

export module UI.TooltipManager;

export import UI.Table;
export import UI.Elem;
import UI.Action.Actions;
import UI.SeperateDrawable;
import ext.Concepts;
import Geom.Vector2D;
import ext.Heterogeneous;
import ext.Concepts;

import std;

export namespace UI{
	class TooltipManager{
		std::deque<std::unique_ptr<Table>> droppedTables{};

		//TODO using stack to support multiple hover=table
		std::vector<std::pair<std::unique_ptr<Table>, const Elem*>> focusTableStack{};

		Geom::Vec2 cursorPos{};

		/** @brief Nullable */
		const Elem* lastConsumer{nullptr};
		Root* root{nullptr};
		friend class Root;

		std::unordered_set<Elem*> toErase{};

		void drop(std::unique_ptr<Table>&& element, const bool instantDrop = false);

	public:
		TooltipManager() = default;

		explicit TooltipManager(Root* root)
			: root{root}{}


		struct TableDeleter{
			TooltipManager& manager;

			void operator()(Elem* elem) const{
				manager.toErase.insert(elem);
			}
		} deleter{*this};


		[[nodiscard]] const Elem* getLastRequester() const{ return lastConsumer; }

		[[nodiscard]] bool hasTooltipInstanceOf(const Elem* consumer) const{
			return std::ranges::find(focusTableStack | std::ranges::views::elements<1>, consumer).base() != focusTableStack.end();
		}

		/**
		 * @brief
		 * @tparam findIndex 0 -> Tooltip, 1 -> Consumer
		 * @param where Tooltip / Consumer
		 * @param instantDrop
		 */
		template <std::size_t findIndex = 0>
			requires (findIndex <= 1)
		void dropCurrentAt(const Elem* where = nullptr, const bool instantDrop = false){
			if(focusTableStack.empty())return;

			lastConsumer = nullptr;

			if(where == nullptr){
				for(auto& element : focusTableStack | std::ranges::views::elements<0> | std::ranges::views::reverse){
					drop(std::move(element), instantDrop);
				}

				focusTableStack.clear();

				lastConsumer = getTopConsumer();
			}

			decltype(focusTableStack)::iterator begin{};

			if constexpr (findIndex == 0){
				begin = std::ranges::find(focusTableStack | std::ranges::views::elements<findIndex>, where, &std::unique_ptr<Table>::get).base();
			}else{
				begin = std::ranges::find(focusTableStack | std::ranges::views::elements<findIndex>, where).base();
			}

			const auto end = focusTableStack.end();

			for(auto&& element : std::ranges::subrange{begin, end} | std::ranges::views::elements<0> | std::ranges::views::reverse){
				drop(std::move(element), instantDrop);
			}

			focusTableStack.erase(begin, end);

			lastConsumer = getTopConsumer();
		}

		/**
		 * @brief
		 * @param builder
		 * @param consumer
		 * @return Used as a release handle, should avoid using it to access members deferly
		 */
		Table* generateTooltip(const std::function<void(Table&)>& builder, const Elem* consumer = nullptr){
			if(getCurrentFocus() != consumer){
				dropCurrentAt(consumer);
			}

			auto ptr = std::make_unique<Table>();
			builder(*ptr);

			// updateCurrentPosition();
			ptr->setRoot(root);
			ptr->maskOpacity = 0.0f;
			ptr->pushAction<Actions::AlphaMaskAction>(5.0f, 1.0f);
			ptr->layout();
			ptr->setDropFocusAtCursorQuitBound(true);

			updateCurrentPosition(ptr.get());

			lastConsumer = consumer;
			focusTableStack.emplace_back(std::move(ptr), consumer);

			return ptr.get();
		}

		Table* tryObtain(const Elem* consumer);

		[[nodiscard]] bool obtainValid(const Elem* consumer) const;

		void releaseFocus(const Table* table = nullptr){
			dropCurrentAt(table);
		}

		void update(float delta);

		[[nodiscard]] bool isCursorInbound() const{
			auto* top = getCurrentFocus();
			return top && top->isInbound(cursorPos);
		}

		Table* findFocus(){
			for(const auto& table : focusTableStack | std::ranges::views::keys | std::ranges::views::reverse){
				if(table->isInbound(cursorPos)){
					return table.get();
				}
			}

			return nullptr;
		}

		[[nodiscard]] Table* getCurrentFocus() const{
			return focusTableStack.empty() ? nullptr : focusTableStack.back().first.get();
		}

		[[nodiscard]] const Elem* getTopConsumer() const{
			return focusTableStack.empty() ? nullptr : focusTableStack.back().second;
		}

		void updateCurrentPosition(Table* table) const{
			Geom::Vec2 followOffset{};

			if(!lastConsumer){
				//Follow cursor by default
				followOffset.set(-6.0f, 6.0f).reverse().add(cursorPos);
			}else{
				const auto offset =
					getOffsetOf(lastConsumer->getTooltipBuilder().tooltipSrcAlign, table->getBound());

				switch(lastConsumer->getTooltipBuilder().followTarget){
					//TODO align apply
					case TooltipFollowTarget::cursor : followOffset.set(-6.0f, 6.0f).reverse().add(cursorPos);
					break;
					case TooltipFollowTarget::none : followOffset.set(-6.0f, 6.0f).add(cursorPos);
					break;
					case TooltipFollowTarget::parent :{
						if(lastConsumer){
							followOffset = getVert(lastConsumer->getTooltipBuilder().followTargetAlign,
														  lastConsumer->getBound().setSrc(0, 0));
							followOffset += lastConsumer->getAbsSrc();
						}
					}

					default : break;
				}

				followOffset += offset;
			}

			table->setSrc(followOffset);

			table->calAbsoluteSrc(nullptr);
		}

		void renderBase() const{
			for(auto& element : droppedTables){
				element->drawBase();
			}

			for(auto& element : focusTableStack | std::ranges::views::elements<0>){
				element->drawBase();
			}
		}

		constexpr bool focusEmpty() const noexcept{
			return focusTableStack.empty();
		}

		void render() const{
			for(const auto& dropped : droppedTables){
				dropped->draw();
			}

			for(auto& element : focusTableStack | std::ranges::views::elements<0>){
				element->draw();
			}
		}

		void forceDrop(Table* handle){
			if(!handle) return;

			dropCurrentAt(handle, true);

			std::erase_if(droppedTables, [handle](const decltype(droppedTables)::value_type& ptr){
				return ptr.get() == handle;
			});

			lastConsumer = getTopConsumer();
		}

		/**
		 * @brief
		 * @return true if drop happens
		 */
		bool dropBack(){
			while(!focusEmpty() && !focusTableStack.back().first->isVisiable()){
				auto [handle, consumer] = std::move(focusTableStack.back());
				focusTableStack.pop_back();
				drop(std::move(handle), true);
			}

			if(!focusEmpty()){
				auto [handle, consumer] = std::move(focusTableStack.back());
				focusTableStack.pop_back();
				drop(std::move(handle));
				lastConsumer = getTopConsumer();

				return true;
			}

			lastConsumer = getTopConsumer();

			return false;
		}

		void clear() noexcept{
			focusTableStack.clear();
			droppedTables.clear();
			toErase.clear();
			// lastRequester = nullptr;
		}


		[[nodiscard]] bool isOccupied(const Elem* widget) const noexcept{
			return widget == lastConsumer;
		}

		template <Concepts::Invokable<void(Table&)> Func>
		void each(Func&& func){
			for (auto& droppedTable : droppedTables){
				func(*droppedTable);
			}

			for (auto& droppedTable : focusTableStack | std::ranges::views::elements<0>){
				func(*droppedTable);
			}
		}

		[[nodiscard]] auto getDrawSeq() const{
			auto dropped = droppedTables | std::ranges::views::transform(&std::unique_ptr<Table>::get);

			auto focused = focusTableStack
				| std::ranges::views::elements<0>
				| std::ranges::views::transform(&std::unique_ptr<Table>::get);

			return std::make_pair(dropped, focused);
		}
	};
}
