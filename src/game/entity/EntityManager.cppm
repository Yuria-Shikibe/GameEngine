export module Game.Entity.EntityManager;

import Game.Entity.DrawMap;
import Game.Entity.EntityMap;
import Game.Entity;
import Game.Entity.RealityEntity;
import Game.Entity.Drawable;
export import Game.Pool;
import ext.Concepts;
import Geom.QuadTree;

import std;

namespace Game::EntityManage {
	std::atomic<IDType> lastID{0};

	IDType allocateID() {
		return lastID++;
	}
}

export namespace Game::EntityManage{
	/** \brief The only place that render will be called*/
	DrawMap drawables{};

	/** \brief The only place that update will be called*/
	EntityMap<::Game::Entity> entities{};

	EntityMap<Game::RealityEntity> realEntities{};

	Geom::QuadTree<RealityEntity, float, true, false> mainTree{};

	void buildTree(const Geom::OrthoRectFloat worldBound) {
		mainTree.setBoundary(worldBound);
	}

	void resizeTree(const Geom::OrthoRectFloat worldBound) {
		mainTree.clear();

		mainTree.setBoundary(worldBound);

		realEntities.each([](const std::shared_ptr<RealityEntity>& t) {
			mainTree.insert(*t);
		});
	}

	void updateTree() {
		mainTree.clearItemsOnly();

		realEntities.each([](const std::shared_ptr<RealityEntity>& t) {
			mainTree.insert(*t);
		});
	}

	void init() {
		buildTree({-10000, -10000, 20000, 20000});
	}

	void postRemove(){
		drawables.processRemoves();
		realEntities.processRemoves();
	}

	void update(const float delta) {
		if(delta != 0.0F){
			entities.updateMain(delta);

			updateTree();

			realEntities.each([delta](const decltype(realEntities)::StoreType& t) {
				t->updateCollision(delta);
			});
		}

		mainTree.intersectRect(drawables.getViewPort(), [](RealityEntity& entity, const Geom::OrthoRectFloat& rect){
			entity.calculateInScreen(rect);
			if(entity.isInScreen()){
				drawables.nextToDraw.push_back(&entity);
			}
		});
	}

	void render() {
		drawables.render();
	}

	void renderDebug() {
		drawables.renderDebug();
	}

	template<Concepts::Derived<Entity> T>
	[[nodiscard]] std::shared_ptr<T> obtain() {
		auto ptr = Pools::entityPoolGroup.obtainRaw<T>();

		ptr->setID(allocateID());
		return std::shared_ptr<T>{ptr, Pools::entityPoolGroup.getPool<T>().getDeleter()};
	}

	template<Concepts::Derived<Entity> T>
	[[nodiscard]] std::unique_ptr<T, typename ext::ObjectPool<T>::Deleter> obtainUnique() {
		auto ptr = Pools::entityPoolGroup.obtainRaw<T>();

		ptr->setID(allocateID());
		return std::unique_ptr<T, typename ext::ObjectPool<T>::Deleter>{ptr, Pools::entityPoolGroup.getPool<T>().getDeleter()};
	}

	template<Concepts::Derived<Entity> T>
	void add(std::shared_ptr<T> entity) {
		entities.add(entity);
		if constexpr(std::is_base_of_v<Game::DrawableEntity, T>)drawables.add(entity);
		if constexpr(std::is_base_of_v<Game::RealityEntity, T>)realEntities.add(entity);

		entity->activate();
	}

	template<Concepts::Derived<Entity> T>
	void postRemove(T* entity) {
		entity->deactivate();

		entities.postRemove(entity);
		if constexpr(std::is_convertible_v<T, Game::DrawableEntity>)drawables.postRemove(entity);
		if constexpr(std::is_convertible_v<T, Game::RealityEntity>)realEntities.postRemove(entity);
	}

	void clear() {
		entities.clear();
		drawables.clear();
		realEntities.clear();
	}
}
