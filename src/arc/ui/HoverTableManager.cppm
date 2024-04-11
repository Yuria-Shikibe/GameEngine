//
// Created by Matrix on 2024/4/11.
//

export module UI.HoverTableManager;

export import UI.Table;
export import UI.Elem;
import UI.Action.Actions;
import Concepts;
import Geom.Vector2D;
import Heterogeneous;
import Concepts;

import std;

export namespace UI{
	class HoverTableManager{
		std::deque<std::unique_ptr<Table>> droppedTables{};
		std::unique_ptr<Table> currentFocus{};

		bool followCursor{false};

		Align::Mode align{Align::Mode::top_left};

		Geom::Vec2 cursorPos{};
		// Geom::Vec2 offset{};

		const Elem* lastRequester{nullptr};
		Root* root{nullptr};
		friend class Root;

		int nextPopCount = 0;

	public:
		HoverTableManager() = default;

		explicit HoverTableManager(Root* root)
			: root{root}{}

		struct TableDeleter{
			HoverTableManager& manager;

			void operator()(Elem* elem) const{

				manager.nextPopCount++;
			}

		} deleter{*this};

		void dropCurrent(){
			if(!currentFocus)return;
			lastRequester = nullptr;

			currentFocus->pushAction<Actions::AlphaMaskAction>(15.0f, 0.0f);
			currentFocus->pushAction<Actions::RunnableAction<Elem, TableDeleter>>(deleter);

			droppedTables.push_front(std::move(currentFocus));
		}

		/**
		 * @brief
		 * @param consumer
		 * @param func Table Init func
		 * @param followCursor
		 * @param additionalOffset
		 * @return Used as a release handle, should avoid using it to access members deferly
		 */
		Table* obtain(const Elem* consumer, Concepts::Invokable<void(Table&)> auto&& func, const bool followCursor = false, const Geom::Vec2 additionalOffset = {}){
			dropCurrent();

			currentFocus = std::make_unique<Table>();
			func(*currentFocus);

			// updateCurrentPosition();
			currentFocus->setRoot(root);
			currentFocus->maskOpacity = 0.0f;
			currentFocus->pushAction<Actions::AlphaMaskAction>(10.0f, 1.0f);
			currentFocus->layout();

			lastRequester = consumer;
			this->followCursor = followCursor;

			return currentFocus.get();
		}

		Table* obtain(const Elem* consumer){
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

		[[nodiscard]] bool obtainValid(const Elem* lastRequester, const float minHoverTime = 45.0f, const bool useStaticTime = true) const;

		void releaseFocus(const Table* table = nullptr){
			if(currentFocus == nullptr)return;
			if(table == nullptr || table == currentFocus.get()){
				dropCurrent();
			}
		}

		void update(const float delta);

		void updateCurrentPosition() const{
			const auto offset = Align::getOffsetOf(align, currentFocus->getBound());
			currentFocus->setSrc(offset + cursorPos + Geom::Vec2{-4.0f, 4.0f});

			currentFocus->calAbsoluteSrc(nullptr);
		}

		void renderBase() const{
			if(currentFocus){
				currentFocus->drawBase();
			}

			for (const auto& dropped : droppedTables){
				dropped->drawBase();
			}
		}

		void render() const{
			for (const auto& dropped : droppedTables){
				dropped->draw();
			}

			if(currentFocus){
				currentFocus->draw();
			}
		}

		void forceDrop(Table* handle){
			if(!handle)return;

			if(currentFocus.get() == handle){
				currentFocus.reset();
			}else{
				std::erase_if(droppedTables, [handle](const decltype(droppedTables)::value_type& ptr){
					return ptr.get() == handle;
				});
			}
		}

		void clear(){
			currentFocus.reset();
			droppedTables.clear();
			followCursor = false;
			nextPopCount = 0;
		}
	};
}
