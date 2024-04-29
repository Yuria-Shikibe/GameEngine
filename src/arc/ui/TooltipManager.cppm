//
// Created by Matrix on 2024/4/11.
//

export module UI.TooltipManager;

export import UI.Table;
export import UI.Widget;
import UI.Action.Actions;
import UI.SeperateDrawable;
import Concepts;
import Geom.Vector2D;
import ext.Heterogeneous;
import Concepts;

import std;

export namespace UI{
	class TooltipManager{
		std::deque<std::unique_ptr<Table>> droppedTables{};

		//TODO using stack to support multiple hover=table
		std::vector<std::pair<std::unique_ptr<Table>, const Widget*>> focusTableStack{};

		Geom::Vec2 cursorPos{};

		const Widget* lastRequester{nullptr};
		Root* root{nullptr};
		friend class Root;

		int nextPopCount = 0;

		void drop(std::unique_ptr<Table>&& element, const bool instantDrop = false);

	public:
		TooltipManager() = default;

		explicit TooltipManager(Root* root)
			: root{root}{}


		struct TableDeleter{
			TooltipManager& manager;

			void operator()(Widget* elem) const{
				manager.nextPopCount++;
			}
		} deleter{*this};


		[[nodiscard]] const Widget* getLastRequester() const{ return lastRequester; }

		void dropCurrentAt(const Widget* where = nullptr, const bool instantDrop = false);

		/**
		 * @brief
		 * @param consumer
		 * @return Used as a release handle, should avoid using it to access members deferly
		 */
		Table* obtainFromWedget(const Widget* consumer){
			if(getCurrentFocus() != consumer){
				dropCurrentAt(consumer);
			}

			auto ptr = std::make_unique<Table>();
			consumer->getTooltipBuilder().builder(*ptr);

			// updateCurrentPosition();
			ptr->setRoot(root);
			ptr->maskOpacity = 0.0f;
			ptr->pushAction<Actions::AlphaMaskAction>(5.0f, 1.0f);
			ptr->layout();
			ptr->setDropFocusAtCursorQuitBound(true);

			lastRequester = consumer;
			updateCurrentPosition(ptr.get());

			focusTableStack.emplace_back(std::move(ptr), consumer);

			return ptr.get();
		}

		Table* tryObtain(const Widget* consumer);

		[[nodiscard]] bool obtainValid(const Widget* lastRequester, const float minHoverTime = 45.0f,
		                               const bool useStaticTime = true) const;

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

		[[nodiscard]] const Widget* getCurrentConsumer() const{
			return focusTableStack.empty() ? nullptr : focusTableStack.back().second;
		}

		void updateCurrentPosition(Table* table) const{
			if(!lastRequester) return;

			const auto offset = getOffsetOf(lastRequester->getTooltipBuilder().tooltipSrcAlign,
			                                       table->getBound());
			Geom::Vec2 followOffset{};

			switch(lastRequester->getTooltipBuilder().followTarget){
				//TODO align apply
				case FollowTarget::cursor : followOffset.set(-6.0f, 6.0f).inv().add(cursorPos);
					break;
				case FollowTarget::none : followOffset.set(-6.0f, 6.0f).add(cursorPos);
					break;
				case FollowTarget::parent :{
					if(lastRequester){
						followOffset = getVert(lastRequester->getTooltipBuilder().followTargetAlign,
						                              lastRequester->getBound().setSrc(0, 0));
						followOffset += lastRequester->getAbsSrc();
					}
				}

				default : break;
			}

			table->setSrc(followOffset + offset);

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

			lastRequester = getCurrentConsumer();
		}

		void clear(){
			focusTableStack.clear();
			droppedTables.clear();
			nextPopCount = 0;
			// lastRequester = nullptr;
		}

		[[nodiscard]] bool isOccupied(const Widget* widget) const{
			return widget == lastRequester;
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
