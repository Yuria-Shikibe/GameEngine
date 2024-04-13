//
// Created by Matrix on 2024/4/11.
//

export module UI.HoverTableManager;

export import UI.Table;
export import UI.Elem;
import UI.Action.Actions;
import UI.SeperateDrawable;
import Concepts;
import Geom.Vector2D;
import Heterogeneous;
import Concepts;

import std;

export namespace UI{
	class HoverTableManager{
		std::deque<std::unique_ptr<Table>> droppedTables{};

		//TODO using stack to support multiple hover=table
		std::vector<std::pair<std::unique_ptr<Table>, const Widget*>> focusTableStack{};
		// std::unique_ptr<Table> currentFocus{};

		bool followCursor{false};

		Align::Mode align{Align::Mode::top_left};

		Geom::Vec2 cursorPos{};
		// Geom::Vec2 offset{};

		const Widget* lastRequester{nullptr};
		Root* root{nullptr};
		friend class Root;

		int nextPopCount = 0;

		void drop(std::unique_ptr<Table>&& element, const bool instantDrop = false);

	public:
		HoverTableManager() = default;

		explicit HoverTableManager(Root* root)
			: root{root}{}

		struct TableDeleter{
			HoverTableManager& manager;

			void operator()(Widget* elem) const{

				manager.nextPopCount++;
			}

		} deleter{*this};

		void dropCurrentAt(const Widget* where, const bool instantDrop = false);

		/**
		 * @brief
		 * @param consumer
		 * @param func Table Init func
		 * @param followCursor
		 * @param additionalOffset
		 * @return Used as a release handle, should avoid using it to access members deferly
		 */
		Table* obtain(const Widget* consumer, Concepts::Invokable<void(Table&)> auto&& func, const bool followCursor = false, const Geom::Vec2 additionalOffset = {}){
			if(getCurrentFocus() != consumer){
				dropCurrentAt(consumer);
			}

			auto ptr = std::make_unique<Table>();
			func(*ptr);

			// updateCurrentPosition();
			ptr->setRoot(root);
			ptr->maskOpacity = 0.0f;
			ptr->pushAction<Actions::AlphaMaskAction>(5.0f, 1.0f);
			ptr->layout();
			ptr->setDropFocusAtCursorQuitBound(true);
			updateCurrentPosition(ptr.get());

			lastRequester = consumer;
			this->followCursor = followCursor;

			focusTableStack.emplace_back(std::move(ptr), consumer);

			return ptr.get();
		}

		Table* obtain(const Widget* consumer){
			if(consumer && consumer->getHoverTableBuilder() &&
				obtainValid(
					consumer,
					consumer->getHoverTableBuilder().minHoverTime,
					consumer->getHoverTableBuilder().useStaticTime)
			){
				return obtain(
					consumer,
					consumer->getHoverTableBuilder().builder,
					consumer->getHoverTableBuilder().followCursor,
					consumer->getHoverTableBuilder().offset);
			}

			return nullptr;
		}

		[[nodiscard]] bool obtainValid(const Widget* lastRequester, const float minHoverTime = 45.0f, const bool useStaticTime = true) const;

		void releaseFocus(const Table* table = nullptr){
			dropCurrentAt(table);
		}

		void update(float delta);

		[[nodiscard]] bool isCursorInbound() const{
			auto* top = getCurrentFocus();
			return top && top->isInbound(cursorPos);
		}

		[[nodiscard]] Table* getCurrentFocus() const{
			return focusTableStack.empty() ? nullptr : focusTableStack.back().first.get();
		}

		[[nodiscard]] const Widget* getCurrentConsumer() const{
			return focusTableStack.empty() ? nullptr : focusTableStack.back().second;
		}

		void updateCurrentPosition(UI::Table* table) const{
			const auto offset = Align::getOffsetOf(align, table->getBound());
			table->setSrc(offset + cursorPos + Geom::Vec2{-4.0f, 4.0f});

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

		void render() const{
			for (const auto& dropped : droppedTables){
				dropped->draw();
			}

			for(auto& element : focusTableStack | std::ranges::views::elements<0>){
				element->draw();
			}
		}

		void forceDrop(Table* handle){
			if(!handle)return;

			dropCurrentAt(handle, true);

			std::erase_if(droppedTables, [handle](const decltype(droppedTables)::value_type& ptr){
				return ptr.get() == handle;
			});

			lastRequester = getCurrentConsumer();
		}

		void clear(){
			focusTableStack.clear();
			droppedTables.clear();
			followCursor = false;
			nextPopCount = 0;
		}

		[[nodiscard]] bool isOccupied(const Widget* widget) const{
			return widget == lastRequester;
		}

		[[nodiscard]] auto getDrawSeq() const {
			auto dropped = droppedTables | std::ranges::views::transform(&std::unique_ptr<Table>::get) | std::ranges::to<std::vector<const Table*>>();

			auto focused =  focusTableStack
					| std::ranges::views::elements<0>
					| std::ranges::views::transform(&std::unique_ptr<Table>::get);

			return std::make_pair(dropped, focused);
		}
	};
}
