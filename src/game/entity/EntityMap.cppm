module;


export module Game.Entity.EntityMap;

export import Game.Entity;
import Container.Pool;
import Event;
import Concepts;
import Geom.QuadTree;
import <algorithm>;
import <execution>;
import <functional>;
import <vector>;
import <memory>;
import <ranges>;
import <unordered_map>;
import <unordered_set>;

export namespace Game {
	template <Concepts::Derived<Entity> T>
	class EntityMap{

	public:
		virtual ~EntityMap() = default;

		struct AddEvent final : Event::EventType {T* entity{nullptr};};
		struct RemoveEvent final : Event::EventType {T* entity{nullptr};};

	protected:
		AddEvent addEvent{};
		RemoveEvent removeEvent{};
		std::unordered_set<T*> toRemove{};

	public:
		std::unordered_map<IDType, std::shared_ptr<T>> idMap{std::unordered_map<IDType, std::shared_ptr<T>>{5000}};

		std::unique_ptr<Geom::QuadTreeF<T>> quadTree{nullptr};

		Event::EventManager groupListener{
			Event::indexOf<AddEvent>(),
			Event::indexOf<RemoveEvent>()
		};

		[[nodiscard]] EntityMap() = default;

		virtual void add(std::shared_ptr<T> t) {
			if(const std::pair<auto, bool> pair = idMap.try_emplace(t->getID(), t); pair.second) {
				addEvent.entity = t.get();
				groupListener.fire(addEvent);
				if(quadTree)quadTree->insert(t.get());
			}else {
				//If insert doesn't happen, says the id crushed, should show a warning or sth
				//TODO warning or exception
			}
		}

		virtual void remove(T* t) {
			if(const auto itr = idMap.find(t->getID()); itr != idMap.end()) {
				removeEvent.entity = t;
				groupListener.fire(removeEvent);
				if(quadTree)quadTree->remove(t.get());

				idMap.erase(itr);
			}else {
				//TODO this entity doesn't exist in current group, show a warning or throw an exception.
			}
		}

		virtual void buildTree(const Geom::Shape::OrthoRectFloat& worldBound, const std::function<const Geom::Shape::OrthoRectFloat&(const T&)>& transformer) {
			quadTree = std::make_unique<Geom::QuadTreeF<T>>(worldBound, transformer);
		}

		virtual void resizeTree(const Geom::Shape::OrthoRectFloat& worldBound) {
			if(!quadTree)return;
			quadTree->clear();

			quadTree->setBoundary(worldBound);

			each([this](std::shared_ptr<T>& t) {
				quadTree->insert(t.get());
			});
		}

		virtual void updateTree() {
			if(!quadTree)return;
			quadTree->clearItemsOnly();

			each([this](std::shared_ptr<T>& t) {
				quadTree->insert(t.get());
			});
		}

		virtual void postRemove(const IDType id) {
			postRemove(idMap.at(id).get());
		}

		virtual void cancelRemove(const IDType id) {
			cancelRemove(idMap.at(id).get());
		}

		virtual void postRemove(T* entity) {
			toRemove.insert(entity);
		}

		virtual void cancelRemove(T* entity) {
			toRemove.erase(entity);
		}

		virtual void updateMain(const float delta) {
			auto range = idMap | std::ranges::views::values;
			std::for_each(std::execution::par_unseq, range.begin(), range.end(), [delta](std::shared_ptr<T>& t) {
				if(!t->isSleeping())t->update(delta);
			});
		}

		virtual void processRemoves() {
			if(toRemove.empty())return;
			for(const T* entity : toRemove) {
				remove(entity);
			}
		}

		virtual void render() {

		}

		template <Concepts::Invokable<void(std::shared_ptr<T>&)> Func>
		void each(Func&& func) {
			auto range = idMap | std::ranges::views::values;
			std::for_each(std::execution::par_unseq, range.begin(), range.end(), std::forward<Func>(func));
		}
	};
}
