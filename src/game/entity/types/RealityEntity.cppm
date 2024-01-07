// ReSharper disable CppDFAUnreachableCode
module;

export module Game.Entity.RealityEntity;

import Concepts;

import Game.Pool;

import Game.Entity;
import Game.Entity.BaseEntity;
import Game.Entity.Drawable;
import Game.Entity.Healthed;
import Game.Entity.Factional;
import Game.Entity.PosedEntity;
import Game.Entity.Collidable;
import Game.Faction;

import Geom.Shape.RectBox;
import Geom.Matrix3D;
import Geom.Vector2D;
import Geom;

import RuntimeException;

import <algorithm>;
import <execution>;
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
	class RealityEntity : public DrawableEntity, public PosedEntity, public Healthed, public Factional {
	public:
		static constexpr float angularVelocityLimit = 180;
		static constexpr float angularAccelerationLimit = 90;
		static constexpr float accelerationLimit = 8000;
		static constexpr float speedLimit = 10000;

		float inertialMass = 1000;
		float collideForceScale = 1.0f;

		Geom::Vec2 acceleration{};
		Geom::Vec2 velocity{};
		float rotation{};
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
		 * \brief Entity collided with self. Modifiy physics should only done to self as the const said.
		 */
		mutable std::unordered_map<const RealityEntity*, Geom::Vec2> intersectedPointWith{};

		/**
		 * \brief This *is(or at least should, children should process its collision by its parent) relative to the global reference frame!
		 */
		Geom::RectBox hitBox{};

		/**
		 * @brief CCD usage;
		 */
		Geom::Shape::OrthoRectFloat maxCollisionBound{};
		std::vector<Geom::RectBox_Brief> contiounsTraces{20};
		float CCD_activeThresholdSpeed2 = 250; //TODO also relative to size and frames!

		[[nodiscard]] virtual bool enableCCD() const {
			return velocity.length2() > CCD_activeThresholdSpeed2;
		}

		[[nodiscard]] virtual bool calrectAvgIntersections() const {
			return true;
		}

		[[nodiscard]] virtual bool calPhysicsSimulate() const {
			return true;
		}

		void activate() override {
			Entity::activate();

			hitBox.originPoint = position;
			hitBox.rotation = rotation;
			hitBox.update();
		}

		virtual RealityEntity* addChild(const IDType id, std::unique_ptr<RealityEntity>&& ptr) {
			return childrenObjects.insert_or_assign(id, std::forward<std::unique_ptr<RealityEntity>>(ptr)).first->second.get();
		}

		template <Concepts::Derived<RealityEntity> T, Concepts::Invokable<void(T&)> Func, typename ...Args>
		void emplaceChild(const IDType id, Func&& func = nullptr, Args... args){
			auto t = *this->addChild(id, std::make_unique<T>(args...));
			if constexpr(!std::is_null_pointer_v<Func>) {
				if(func) {
					func(t);
				}
			}
		}

		template <Concepts::Derived<RealityEntity> T, typename ...Args>
		void emplaceChild(const IDType id, Args... args){
			this->addChild(id, std::make_unique<T>(args...));
		}

		virtual RealityEntity* addChild(std::unique_ptr<RealityEntity>&& ptr) {
			const IDType id = ptr->getID();
			return addChild(id, std::forward<std::unique_ptr<RealityEntity>>(ptr));
		}

		[[nodiscard]] bool isRootEntity() const {
			return parent == nullptr;
		}

		virtual Geom::Vec2& transformToSuper(Geom::Vec2& vec) const {
			vec *= localToSuper;
			return vec;
		}

		virtual Geom::Vec2& transformToGlobal(Geom::Vec2& vec) const {
			vec *= localToSuper;

			auto current = this;

			while(current) {
				parent->transformToGlobal(vec);
				current = current->parent;
			}

			return vec;
		}

		void update(float deltaTick) override {
			updateMovement(deltaTick);

			checkStateValid(this);
		}

		virtual void updateCollision(const float delatTick) {
			if(!calPhysicsSimulate() || intersectedPointWith.empty())return;
			for(auto& [entity, point] : intersectedPointWith) {
				calCollideTo(entity, point, delatTick);
			}
		}

		virtual void calCollideTo(const Game::RealityEntity* object, const Geom::Vec2 intersection, const float delatTick) {
			constexpr float scale = 1 / 60.0f;
			constexpr float angularScale = 1 / 40.0f;

			const float massSacle = object->inertialMass / this->inertialMass;

			if(Math::zero(massSacle, 0.005f))return;

			Geom::Vec2 dstTo1 = this->hitBox.originPoint;
			dstTo1 -= intersection;

			Geom::Vec2 dstTo2 = object->hitBox.originPoint;
			dstTo2 -= intersection;

			if(dstTo1.isZero())return;
			if(dstTo2.isZero())return;

			dstTo1.clampMin(8.0f);
			dstTo2.clampMin(8.0f);

			Geom::Vec2 relativeVelo = object->velocity;
			relativeVelo -= this->velocity;
			relativeVelo.add(
				(dstTo2.y * object->angularVelocity - dstTo1.y * this->angularVelocity) * Math::DEGREES_TO_RADIANS * angularScale,
				(dstTo1.x * this->angularVelocity - dstTo2.x * object->angularVelocity) * Math::DEGREES_TO_RADIANS * angularScale
			);

			if(relativeVelo.isZero(0.05f) || relativeVelo.isNaN())return;

			relativeVelo.clampMax(speedLimit * 2);

			const float dot = relativeVelo.dot(dstTo1);

			const float div = std::max(5.0f, (1 + dot + (1 + dstTo2.dot(relativeVelo))) * scale);

			float j = -(1 + this->collideForceScale) * dot / div / this->inertialMass;
			j = Math::clamp(j, -5000.0f, 5000.0f);

			if(hitBox.contains(intersection)) {
				if(enableCCD()) {
					Geom::Vec2 dst = intersection;
					dst -= position;
					const float dstP = dst.projLen(velocity);
					this->position.sub(dst.set(velocity).normalize().scl(dstP));
				}
				this->position.add(object->velocity, delatTick * 0.01f * massSacle);
			}

			this->acceleration.sub(dstTo1, j).add(object->velocity, 0.01f * massSacle);
			this->angularAcceleration += j * dot * angularScale;
		}

		virtual void updateHitbox(const float delta) {
			hitBox.originPoint = position;
			hitBox.rotation = rotation;

			if(enableCCD()) {
				//TODO box rotation support
				Geom::genContinousRectBox(contiounsTraces, velocity, delta, hitBox);
				maxCollisionBound = Geom::maxContinousBoundOf(contiounsTraces);
			}else {
				maxCollisionBound = hitBox.maxOrthoBound;
			}

			hitBox.update();
		}

		virtual void updateMovement(const float delta) {
			acceleration.clampMax(accelerationLimit);
			angularAcceleration = Math::clamp(angularAcceleration, -angularAccelerationLimit, angularAccelerationLimit);

			{
				//TODO pre global force field process.
				//TODO drag should be applied by things like a global force field.
				angularVelocity = Math::lerp(angularVelocity, 0, 0.075f * delta);
				velocity.lerp(Geom::ZERO, 0.075f * delta);
			}

			//Loacl process

			velocity.add(acceleration, delta);
			angularVelocity += angularAcceleration * delta;

			angularVelocity = Math::clamp(angularVelocity, -angularVelocityLimit, angularVelocityLimit);
			velocity.clampMax(speedLimit);

			position.add(velocity, delta);
			rotation += angularVelocity * delta;
			rotation = std::fmod(rotation, Math::DEG_FULL);




			angularAcceleration = 0;
			acceleration.setZero();

			updateHitbox(delta);
		}

		virtual void drawChildren() {
			for(const auto& entity : childrenObjects | std::ranges::views::values) {
				entity->draw();
			}
		}

		virtual bool processCollisionWithChildren() {
			return false;
		}

		static const Geom::Shape::OrthoRectFloat& getHitBoound(const RealityEntity* reality_entity) {
			if(reality_entity->enableCCD()) {
				return reality_entity->maxCollisionBound;
			}
			return reality_entity->hitBox.maxOrthoBound;
		}

		//TODO abstract these to other classes
		static bool exactInterscet(const RealityEntity* subject, const RealityEntity* object) {
			if(subject->deletable() || object->deletable() || subject == object)return false;
			const bool needInterscetPointCalculation = subject->calrectAvgIntersections();
			if(subject->enableCCD() && object->enableCCD()) {
				const float ratio = static_cast<float>(object->contiounsTraces.size()) / static_cast<float>(subject->contiounsTraces.size());

				for(int i = 0; i < subject->contiounsTraces.size(); ++i) {
					const auto progress      = static_cast<float>(i);
					const int objectIndex = Math::floor(progress * ratio);

					if(subject->contiounsTraces.at(i).overlapExact(object->contiounsTraces.at(objectIndex),
						subject->hitBox.normalU, subject->hitBox.normalV,
						object->hitBox.normalU, object->hitBox.normalV
					)) {
						if(needInterscetPointCalculation) {
							subject->intersectedPointWith.insert_or_assign(object, Geom::rectAvgIntersection(subject->contiounsTraces.at(i), object->contiounsTraces.at(objectIndex)));
						}

						return true;
					}
				}

				return false;
			}else{
				bool usingCCD_S = false;
				bool swapped = false;

				if(!subject->enableCCD() && object->enableCCD()) {
					std::swap(subject, object);
					swapped = true;
					usingCCD_S = true;
				}

				if(!usingCCD_S && subject->enableCCD() && !object->enableCCD()) {
					usingCCD_S = true;
				}

				if(usingCCD_S) {
					//Subject active CCD!
					if(needInterscetPointCalculation) {
						return std::ranges::any_of(subject->contiounsTraces, [subject, object, swapped](const decltype(subject->contiounsTraces)::value_type& currentBox) {
							const bool success = currentBox.maxOrthoBound.overlap(object->maxCollisionBound) && object->hitBox.overlapExact(currentBox, subject->hitBox.normalU, subject->hitBox.normalV);
							if(success) {
								if(swapped)object->intersectedPointWith.insert_or_assign(subject, Geom::rectAvgIntersection(currentBox, object->hitBox));
								else subject->intersectedPointWith.insert_or_assign(object, Geom::rectAvgIntersection(currentBox, object->hitBox));
							}

							return success;
						});
					}else {
						return std::ranges::any_of(subject->contiounsTraces, [subject, object](const decltype(subject->contiounsTraces)::value_type& currentBox) {
							return currentBox.maxOrthoBound.overlap(object->maxCollisionBound) && object->hitBox.overlapExact(currentBox, subject->hitBox.normalU, subject->hitBox.normalV);
						});
					}

				}else {
					if(needInterscetPointCalculation) {
						if(subject->hitBox.overlapExact(object->hitBox)) {
							subject->intersectedPointWith.emplace(object, Geom::rectAvgIntersection(subject->hitBox, object->hitBox));
							return true;
						}
						return false;
					}else {
						return subject->hitBox.overlapExact(object->hitBox);
					}
				}
			}
		}

		static bool roughInterscet(const RealityEntity* subject, const RealityEntity* object) {
			return subject->hitBox.overlapRough(object->hitBox);
		}

		static bool within(const float dst, const RealityEntity* subject, const RealityEntity* object) {
			return subject->position.within(object->position, dst);
		}

		static void checkStateValid(const RealityEntity* subject) {
#ifndef _DEBUG
			return;
#endif

			if(subject->acceleration.isNaN() || subject->acceleration.isInf())
				throw ext::RuntimeException{"Invalid Entitiy State!"};

			if(subject->velocity.isNaN() || subject->velocity.isInf())
				throw ext::RuntimeException{"Invalid Entitiy State!"};

			if(subject->position.isNaN() || subject->position.isInf())
				throw ext::RuntimeException{"Invalid Entitiy State!"};

			if(std::isnan(subject->angularVelocity) || std::isinf(subject->angularVelocity))
				throw ext::RuntimeException{"Invalid Entitiy State!"};

			if(std::isnan(subject->angularAcceleration) || std::isinf(subject->angularAcceleration))
				throw ext::RuntimeException{"Invalid Entitiy State!"};

			if(std::isnan(subject->rotation) || std::isinf(subject->rotation))
				throw ext::RuntimeException{"Invalid Entitiy State!"};
		}
	};
}
