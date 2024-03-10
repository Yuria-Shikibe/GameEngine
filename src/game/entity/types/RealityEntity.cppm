module;

export module Game.Entity.RealityEntity;

import Concepts;

import Game.Entity;
import Game.Entity.Drawable;
import Game.Entity.Healthed;
import Game.Entity.Factional;
import Game.Entity.PosedEntity;
import Game.Entity.PhysicsAttribute;
import Game.Entity.Controller;
import Game.Faction;

import Geom.Shape.RectBox;
import Geom.Shape.Rect_Orthogonal;
import Geom.Matrix3D;
import Geom.Vector2D;
import Geom;

import RuntimeException;

import Math;

import <algorithm>;
import <execution>;
import <memory>;
import <ranges>;
import <unordered_map>;

export namespace Game {
	class BasicRealityEntity : public DrawableEntity, public PosedEntity, public Healthed, public Factional {

	};
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
	class RealityEntity : public BasicRealityEntity {
	public:
		static constexpr float angularVelocityLimit = 180;
		static constexpr float angularAccelerationLimit = 90;
		static constexpr float accelerationLimit = 8000;
		static constexpr float speedLimit = 10000;

		//Physics Attribute
		float collisionThickness = 0;
		PhysicsAttribute_Rigid physicsBody{};

		std::unique_ptr<Controller> controller{std::make_unique<Controller>(this)};

		Geom::Vec2 acceleration{};
		Geom::Vec2 velocity{};
		Geom::Vec2 velocityCollision{};

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
		mutable std::mutex intersectionLock{};

		/**
		 * \brief This *is(or at least should, children should process its collision by its parent) relative to the global reference frame!
		 */
		mutable Geom::RectBox hitBox{};

		/**
		 * @brief CCD usage;
		 */
		Geom::Shape::OrthoRectFloat maxBound{};

		struct Trace {
			std::vector<Geom::QuadBox> contiounsTraces{20};
			using sizeType = decltype(contiounsTraces)::size_type;
			std::atomic<sizeType> clampSize{0};

			void clampTo(sizeType index){
				++index;
				if(index> clampSize)return;
				clampSize = index;
			}

			void resize(){
				clampSize = contiounsTraces.size();
			}

			[[nodiscard]] sizeType size() const{
				return clampSize;
			}

			[[nodiscard]] bool empty() const{
				return clampSize == 0;
			}
		};

		mutable Trace trace{};

		float CCD_activeThresholdSpeed2 = 250; //TODO also relative to size and frames!

		[[nodiscard]] virtual bool enableCCD() const {
			const float len2 = velocity.length2();

			return len2 >  CCD_activeThresholdSpeed2/* || len2 > hitBox.sizeVec2.length2() * 0.5f*/;
		}

		[[nodiscard]] virtual bool requiresCollisionIntersection() const {
			return true;
		}

		[[nodiscard]] virtual bool enablePhysics() const {
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

			const auto* current = this;

			while(current != nullptr) {
				parent->transformToGlobal(vec);
				current = current->parent;
			}

			return vec;
		}


		[[nodiscard]] const Geom::Shape::OrthoRectFloat& getDrawBound() const override {
			return maxBound;
		}

		void update(const float deltaTick) override {
			controller->update();
			updateMovement(deltaTick);

			// checkStateValid(this);
		}

		virtual void updateCollision(const float delatTick) {
			if(!enablePhysics() || intersectedPointWith.empty())return;

			if(enableCCD()) { //Pull Back
				position = hitBox.originPoint;
			}

			velocityCollision = velocity;
			for(auto& [entity, point] : intersectedPointWith) {
				calCollideTo(entity, point, delatTick);
			}

			position = hitBox.originPoint;
		}

		[[nodiscard]] virtual bool selectable() const {
			return false;
		}

		virtual void assignController() const {

		}

		virtual bool isOverrideCollisionTo(const Game::RealityEntity* object) const {
			return false;
		}

		virtual void overrideCollisionTo(Game::RealityEntity* object, Geom::Vec2 intersection) {

		}

		[[nodiscard]] constexpr float getRotationalInertia() const {
			return hitBox.getRotationalInertia(physicsBody.inertialMass, physicsBody.rotationalInertiaScale);
		}

		[[nodiscard]] constexpr Geom::Vec2 collideVelAt(Geom::Vec2 dst) const {
			return velocityCollision + dst.cross(angularVelocity * Math::DEGREES_TO_RADIANS);
		}

		/**
		 * \param object To collide with, when [this] as the true object
		 */
		virtual bool ignoreCollisionTo(const Game::RealityEntity* object) const {
			return !enablePhysics() || physicsBody.inertialMass / object->physicsBody.inertialMass < 0.0005f;
		}

		/*virtual*/ void intersectionCorrection(Geom::Vec2& intersection) const {
			intersection.lerp(hitBox.originPoint, 0.0001f);
		}

		/*virtual*/ void calCollideTo(const Game::RealityEntity* object, Geom::Vec2 intersection, const float delatTick) {
			//Pull in to correct calculation

			if(object->ignoreCollisionTo(this))return;

			if(object->isOverrideCollisionTo(this)) {
				const_cast<Game::RealityEntity*>(object)->overrideCollisionTo(this, intersection);
				return;
			}

			intersectionCorrection(intersection);

			if(!hitBox.contains(intersection))return;

			using Geom::Vec2;


			if(velocityCollision > object->velocityCollision && object->hitBox.contains(hitBox.originPoint)){
				constexpr float PullBackScl = 1.5f;
				hitBox.originPoint.add((hitBox.originPoint - object->hitBox.originPoint).normalize().scl(delatTick * PullBackScl * hitBox.sizeVec2.length()));
			}

			if(!hitBox.contains(intersection)) {
				throw ext::RuntimeException{"Intersection Out Of Bound!"};
			}

			//Origin Point To Hit Point
			const Vec2 dstToSubject = intersection - hitBox.originPoint;
			const Vec2 dstToObject  = intersection - object->hitBox.originPoint;

			const Vec2 subjectVel = collideVelAt(dstToSubject);
			const Vec2 objectVel = object->collideVelAt(dstToObject);

			const Vec2 relVel = objectVel - subjectVel;

			//TODO overlap quit process
			if(relVel.isZero())return;

			const Vec2 collisionNormalVec = -Geom::avgEdgeNormal(intersection, object->hitBox).normalize();
			Vec2 collisionTangentVec = collisionNormalVec.copy().rotateRT_counterclockwise();

			// collisionTangentVec.setZero();

			if(collisionTangentVec.dot(relVel) < 0)collisionTangentVec.inv();

			// position += collisionNormalVec * 200;
			// return;

			Vec2 relVelNormal{relVel};
			relVelNormal.project(collisionNormalVec);

			const float scale = 1 / physicsBody.inertialMass + 1 / object->physicsBody.inertialMass;

			const float subjectRotationalInertia = this->getRotationalInertia();
			const float objectRotationalInertia = object->getRotationalInertia();

			Vec2 impulseNormal{relVelNormal};
			impulseNormal *= -(1 + physicsBody.restitution) / (scale +
				Math::sqr(dstToObject.cross(collisionNormalVec)) / objectRotationalInertia +
				Math::sqr(dstToSubject.cross(collisionNormalVec)) / subjectRotationalInertia);

			const Vec2 relVelTangent = relVel - relVelNormal;
			const Vec2 impulseTangent = -relVelTangent.sign().cross(std::min(
				physicsBody.frictionCoefficient * impulseNormal.length(),
				relVelTangent.length() / (scale +
				Math::sqr(dstToObject.cross(collisionTangentVec)) / objectRotationalInertia +
				Math::sqr(dstToSubject.cross(collisionTangentVec)) / subjectRotationalInertia)
			));

			Vec2 additional = collisionNormalVec * impulseNormal.length() + collisionTangentVec * impulseTangent.length();

			float veloAddScale = 0.65f;

			if(relVel.dot(additional) < 0){
				additional.inv();
				if(velocityCollision.length() * physicsBody.inertialMass < object->velocityCollision.length() * object->physicsBody.inertialMass) {
					auto correction = (hitBox.originPoint - object->hitBox.originPoint).setLength2(hitBox.sizeVec2.length());
					hitBox.originPoint.add(correction.scl(0.05f));
					velocity.add(correction.scl(0.185f));
				}
			}

			// additional.limit2(40 * inertialMass * inertialMass);

			acceleration += additional / physicsBody.inertialMass;


			velocity.add(additional * (delatTick * veloAddScale / physicsBody.inertialMass));
			//
			angularAcceleration += dstToSubject.cross(additional) / subjectRotationalInertia * Math::RADIANS_TO_DEGREES;
		}

		virtual void updateHitbox(const float delta) {
			hitBox.originPoint = position;
			hitBox.rotation = rotation;

			if(enableCCD()) {
				//TODO box rotation support
				Geom::genContinousRectBox(trace.contiounsTraces, velocity, delta, hitBox);
				maxBound = Geom::maxContinousBoundOf(trace.contiounsTraces);
				trace.resize();
			}else {
				maxBound = hitBox.maxOrthoBound;
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

		void addIntersection(const RealityEntity* object, Geom::Vec2 intersection) const {
			if(object == nullptr)throw ext::NullPointerException{"Null Entity!"};

			std::lock_guard guard{intersectionLock};
			intersectedPointWith.emplace(object, intersection);
		}

		void positionCorrectionCCD(const Geom::Vec2 lastPosition_V0) const {
			hitBox.originPoint += lastPosition_V0 - hitBox.v0;
		}

		virtual void drawChildren() {
			for(const auto& entity : childrenObjects | std::ranges::views::values) {
				entity->draw();
			}
		}

		virtual bool processCollisionWithChildren() {
			return false;
		}

		//TODO abstract these to other classes
		//TODO is this a good idea? this actually modifies many state of the entities
		static bool exactInterscet(const RealityEntity* subject, const RealityEntity* object) {
			const bool needInterscetPointCalculation_subject = subject->requiresCollisionIntersection();
			const bool needInterscetPointCalculation_object = subject->requiresCollisionIntersection();

			const bool subjectEnablesCCD = subject->enableCCD() && !subject->trace.empty();
			const bool objectEnablesCCD = object->enableCCD() && !object->trace.empty();

			//TODO why this performace so bad? Debug Mode Reason?
			// if(const auto itr = object->intersectedPointWith.find(subject); itr != object->intersectedPointWith.end()) {
			// 	if(needInterscetPointCalculation_subject) {
			// 		subject->intersectedPointWith.emplace(object, itr->second);
			// 	}
			//
			// 	return true;
			// }

			//Both enable CCD
			if(subjectEnablesCCD && objectEnablesCCD) {
				for(int i = 0; i < subject->trace.size(); ++i) {
					const int objectIndex = //The subject's size may shrink, cause the ratio larger than 1, resulting in array index out of bound
						Math::min(Math::floor(static_cast<float>(i) * static_cast<float>(object->trace.size()) / static_cast<float>(subject->trace.size())), Math::max(0, static_cast<int>(object->trace.size() - 1)));


					if(subject->trace.contiounsTraces.at(i).overlapExact(object->trace.contiounsTraces.at(objectIndex),
						subject->hitBox.normalU, subject->hitBox.normalV,
						object->hitBox.normalU, object->hitBox.normalV
					)) {
						if(needInterscetPointCalculation_subject || needInterscetPointCalculation_object) {
							const auto intersection = Geom::rectAvgIntersection(subject->trace.contiounsTraces.at(i), object->trace.contiounsTraces.at(objectIndex));
							if(needInterscetPointCalculation_subject) {
								subject->addIntersection(object, intersection);
							}
						}

						subject->trace.clampTo(i);
						object->trace.clampTo(objectIndex);

						subject->positionCorrectionCCD(subject->trace.contiounsTraces.at(i).v0);
						object->positionCorrectionCCD(object->trace.contiounsTraces.at(objectIndex).v0);

						return true;
					}
				}

				return false;
			}

			//One side enables CCD
			const bool enableCCD = subjectEnablesCCD xor objectEnablesCCD;
			bool swapped = false;
			if(enableCCD && !subjectEnablesCCD) {
				swapped = true;
				std::swap(subject, object);
			}

			if(enableCCD) {
				//Subject active CCD!
				for(size_t index = 0; index < subject->trace.size(); ++index){
					auto& currentBox = subject->trace.contiounsTraces.at(index);

					if(currentBox.maxOrthoBound.overlap(object->maxBound) && object->hitBox.overlapExact(currentBox, subject->hitBox.normalU, subject->hitBox.normalV)) {
						if(needInterscetPointCalculation_subject || needInterscetPointCalculation_object) {
							const Geom::Vec2 intersection = Geom::rectAvgIntersection(currentBox, object->hitBox);

							const auto *const subject_ = swapped ? object : subject;
							const auto *const object_  = swapped ? subject : object;

							if(needInterscetPointCalculation_subject) {
								subject_->addIntersection(object_, intersection);
							}
						}

						subject->trace.clampTo(index);

						subject->positionCorrectionCCD(currentBox.v0);

						return true;
					}
				}

				return false;
			}

			//None enable CCD
			if(subject->hitBox.overlapExact(object->hitBox)) {
				if(needInterscetPointCalculation_subject || needInterscetPointCalculation_object) {
					const Geom::Vec2 intersection = Geom::rectAvgIntersection(subject->hitBox, object->hitBox);

					if(needInterscetPointCalculation_subject) {
						subject->addIntersection(object, intersection);
					}
				}
				return true;
			}

			return false;
		}

		static const Geom::Shape::OrthoRectFloat& getHitBoound(const RealityEntity* reality_entity) {
			if(reality_entity->enableCCD()) {
				return reality_entity->maxBound;
			}
			return reality_entity->hitBox.maxOrthoBound;
		}

		static bool roughInterscet(const RealityEntity* subject, const RealityEntity* object) {
			if(Math::abs(subject->layer - object->layer) > subject->collisionThickness + subject->collisionThickness)return false;
			if(subject->deletable() || object->deletable() || subject == object)return false;
			return subject->hitBox.overlapRough(object->hitBox);
		}

		static bool pointInterscet(const RealityEntity* subject, const Geom::Vec2 point) {
			return subject->hitBox.contains(point);
		}

		static bool within(const float dst, const RealityEntity* subject, const RealityEntity* object) {
			return subject->position.within(object->position, dst);
		}

		static void checkStateValid(const RealityEntity* subject) {
			// return;
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