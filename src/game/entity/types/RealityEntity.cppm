module;

export module Game.Entity.RealityEntity;

import Concepts;

import Game.Pool;

import Game.Entity;
import Game.Entity.BaseEntity;
import Game.Entity.Drawable;
import Game.Entity.Healthed;
import Game.Entity.Factional;
import Game.Entity.Pos;
import Game.Entity.Collidable;

import Game.Faction;
import Geom.Shape.RectBox;
import Geom.Matrix3D;
import Geom.Vector2D;

import <memory>;
import <ranges>;
import <unordered_map>;

export namespace Game {
	/**
	 * \brief
	 * RealityEntity should exist in Reality,
	 * which means that they should be drawn in the screen with real-space coords and has valid boxes.
	 * Objects such as a spacecraft, crews, asteroids, bullets can be made a RealityEntity.
	 *
	 * So normally they should be destructable, or at least hittable.
	 *
	 * TODO z layer necessity?
	 */
	class RealityEntity : public DrawableEntity, public PosedEntity, public Healthed, public Factional, public Collidable {
	public:
		/** \brief relative to its parent frame.*/
		// using PosedEntity::position;

		float inertialMass = 1;

		Geom::Vec2 acceleration{};
		Geom::Vec2 velocity{};
		float angularVelocity{};
		float angularAcceleration{};

		/** \brief Transform this entity from local to its parent or global.*/
		Geom::Matrix3D localToSuper{};

		/**
		 * \brief Children Objects should only be accessed from its parent object!
		 * also its lifetime should be the same as its parent object!
		 * normally they shouldn't has their own hitbox globally valid.
		 */
		std::unordered_map<IDType, std::unique_ptr<RealityEntity>> childrenObjects{};
		RealityEntity* parent{nullptr};

		/**
		 * \brief This *is(or at least should, children should process its collision by its parent) relative to the global reference frame!
		 */
		Geom::RectBox hitBox{};

		virtual RealityEntity* addChild(const IDType id, std::unique_ptr<RealityEntity>&& ptr) {
			return childrenObjects.insert_or_assign(id, std::forward<std::unique_ptr<RealityEntity>>(ptr)).first->second.get();
		}

		template <Concepts::Derived<RealityEntity> T, Concepts::Invokable<void(T&)> Func, typename ...Args>
		void emplaceChild(IDType id, Func&& func = nullptr, Args... args){
			auto t = *addChild(id, std::make_unique<T>(args...));
			if constexpr(!std::is_null_pointer_v<Func>) {
				if(func) {
					func(t);
				}
			}
		}

		virtual RealityEntity* addChild(std::unique_ptr<RealityEntity>&& ptr) {
			const IDType id = ptr->getID();
			return addChild(id, std::forward<std::unique_ptr<RealityEntity>>(ptr));
		}

		[[nodiscard]] bool rootEntity() const {
			return parent == nullptr;
		}

		Geom::Vec2& transformToSuper(Geom::Vec2& vec) const {
			vec *= localToSuper;
			return vec;
		}

		Geom::Vec2& transformToGlobal(Geom::Vec2& vec) const {
			vec *= localToSuper;

			auto current = this;

			while(current) {
				parent->transformToGlobal(vec);
				current = current->parent;
			}

			return vec;
		}

		virtual void drawChildren() {
			for(const auto& entity : childrenObjects | std::ranges::views::values) {
				entity->draw();
			}
		}

		~RealityEntity() override = default;

		static const Geom::Shape::OrthoRectFloat& getHitBoound(const RealityEntity& reality_entity) {
			return reality_entity.hitBox.maxOrthoBound;
		}
	};
}
