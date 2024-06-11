module;


export module Game.Entity.EntityMap;

export import Game.Entity;
import ext.Container.ObjectPool;
import ext.Event;
import ext.Concepts;
import std;

export namespace Game {
	template <Concepts::Derived<Entity> T>
	class EntityMap : public RemoveCallalble{

	public:
		~EntityMap() override = default;

		using StoreType = std::shared_ptr<T>;
		using ValueType = T;

		struct AddEvent final : ext::EventType {T* entity{nullptr};};
		struct RemoveEvent final : ext::EventType {T* entity{nullptr};};

	protected:
		AddEvent addEvent{};
		RemoveEvent removeEvent{};
		std::unordered_set<T*> toRemove{};

	public:
		std::unordered_map<IDType, std::shared_ptr<T>> idMap{std::unordered_map<IDType, std::shared_ptr<T>>{5000}};

		ext::EventManager groupListener{
			ext::typeIndexOf<AddEvent>(),
			ext::typeIndexOf<RemoveEvent>()
		};

		std::mutex removeLock{};
		std::mutex addLock{};

		[[nodiscard]] EntityMap() = default;


		virtual void add(std::shared_ptr<T> t) {
			std::lock_guard guard{addLock};
			if(const auto pair = idMap.try_emplace(t->getID(), t); pair.second) {
				addEvent.entity = t.get();
				groupListener.fire(addEvent);
				t->registerGroup(this);
			}else {
				//If insert doesn't happen, says the id crushed, should show a warning or sth
				//TODO warning or exception
			}
		}

		virtual void rerove(T& t) {
			if(const auto itr = idMap.find(t.getID()); itr != idMap.end()) {
				removeEvent.entity = &t;
				groupListener.fire(removeEvent);

				idMap.erase(itr);
			}else {
				//TODO this entity doesn't exist in current group, show a warning or throw an exception.
			}
		}

		virtual void postRemove(const IDType id) {
			this->postRemove(idMap.at(id).get());
		}

		virtual void cancelRemove(const IDType id) {
			this->cancelRemove(idMap.at(id).get());
		}

		virtual void postRemove(T* entity) {
			std::lock_guard lk{removeLock};
			toRemove.insert(entity);
		}

		void postRemovePrimitive(::Game::Entity* entity) override {
			if(T* t = dynamic_cast<T*>(entity)) {
				this->postRemove(t);
			}else {
				//TODO debug throw
			}
		}

		// void postAddPrimitive(Game::Entity* entity) override {
		//
		// }

		virtual void cancelRemove(T* entity) {
			toRemove.erase(entity);
		}

		virtual void updateMain(const float delta) {
			processRemoves();
			this->each([delta](std::shared_ptr<T>& t){
				if(!t->isSleeping())t->update(delta);
			});
		}

		void processRemoves() {
			if(toRemove.empty())return;
			for(T* entity : toRemove) {
				this->rerove(*entity);
			}
			toRemove.clear();
		}

		void each(Concepts::Invokable<void(std::shared_ptr<T>&)> auto&& func) {
			auto range = idMap | std::ranges::views::values;
			std::for_each(std::execution::par, range.begin(), range.end(), std::forward<decltype(func)>(func));
		}

		void clear() {
			idMap.clear();
		}
	};
}
