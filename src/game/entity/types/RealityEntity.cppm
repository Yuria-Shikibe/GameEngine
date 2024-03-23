module;

export module Game.Entity.RealityEntity;

import Concepts;

export import Game.Entity;
export import Game.Entity.Drawable;
export import Game.Entity.Healthed;
export import Game.Entity.Factional;
export import Game.Entity.PosedEntity;
export import Game.Entity.PhysicsAttribute;
export import Game.Entity.Controller;
export import Game.Faction;

import Game.Entity.Collision;

import Geom.Shape.RectBox;
import Geom.Shape.Rect_Orthogonal;
import Geom.Matrix3D;
import Geom.Vector2D;
import Geom;

import RuntimeException;

import Math;

import std;

export namespace Game {
	class BasicRealityEntity : public DrawableEntity, public PosedEntity, public Healthed, public Factional {
		virtual void init(){

		}
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
		mutable std::unordered_map<const RealityEntity*, CollisionData> intersectedPointWith{};
		mutable std::mutex intersectionLock{};

		/**
		 * \brief This *is(or at least should, children should process its collision by its parent) relative to the global reference frame!
		 */
		Game::HitBox hitBox{};

		/**
		 * @brief CCD usage;
		 */
		Geom::Shape::OrthoRectFloat maxBound{};

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

			hitBox.updateHitbox(trans);
		}

		virtual void targetUpdated(){

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

		[[nodiscard]] virtual Geom::Vec2 transformToSuper(Geom::Vec2 vec) const {
			vec *= localToSuper;
			return vec;
		}

		[[nodiscard]] virtual Geom::Vec2 transformToGlobal(Geom::Vec2 vec) const {
			const auto* current = this;

			while(current != nullptr) {
				vec = current->transformToSuper(vec);
				current = current->parent;
			}

			return vec;
		}

		[[nodiscard]] virtual float transformToSuperAngle(const float ang) const {
			return ang + trans.rot;
		}

		[[nodiscard]] virtual float transformToGlobalAngle(float ang) const {
			const auto* current = this;

			while(current != nullptr) {
				ang = current->transformToSuperAngle(ang);
				current = current->parent;
			}

			return ang;
		}

		[[nodiscard]] const Geom::Shape::OrthoRectFloat& getDrawBound() const override {
			return maxBound;
		}

		virtual void updateTransition(){
			(void)localToSuper.translateTo(trans);
		}

		void update(const float deltaTick) override {
			controller->update();
			updateMovement(deltaTick);

			updateTransition();

			// checkStateValid(this);
		}

		void fetchToHitbox(){
			trans.pos = hitBox.trans.pos;
		}

		virtual void updateCollision(const float delatTick) {
			if(!enablePhysics() || intersectedPointWith.empty())return;

			fetchToHitbox();

			velocityCollision = velocity;
			for(auto& [entity, point] : intersectedPointWith) {
				calCollideTo(entity, point, delatTick);
			}

			fetchToHitbox();
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
			intersection.lerp(hitBox.trans.pos, 0.0001f);
		}

		virtual void calCollideTo(const Game::RealityEntity* object, CollisionData data, const float delatTick) {
			//Pull in to correct calculation

			if(object->isOverrideCollisionTo(this)) {
				const_cast<Game::RealityEntity*>(object)->overrideCollisionTo(this, data.intersection);
				return;
			}

			intersectionCorrection(data.intersection);

			if(!hitBox.contains(data.intersection))return;

			using Geom::Vec2;

			//TODO collision corretion
			// if(velocityCollision > object->velocityCollision && object->hitBox.contains(hitBox.transition.displacement)){
			// 	constexpr float PullBackScl = 1.5f;
			// 	hitBox.transition.displacement.add((hitBox.transition.displacement - object->hitBox.transition.displacement).normalize().scl(delatTick * PullBackScl * hitBox.sizeVec2.length()));
			// }

			if(!hitBox.contains(data.intersection)) {
				throw ext::RuntimeException{"Intersection Out Of Bound!"};
			}

			//Origin Point To Hit Point
			const Vec2 dstToSubject = data.intersection - hitBox.trans.pos;
			const Vec2 dstToObject  = data.intersection - object->hitBox.trans.pos;

			const Vec2 subjectVel = collideVelAt(dstToSubject);
			const Vec2 objectVel = object->collideVelAt(dstToObject);

			const Vec2 relVel = objectVel - subjectVel;

			//TODO overlap quit process
			if(relVel.isZero())return;

			const Vec2 collisionNormalVec = -object->hitBox.getAvgEdgeNormal(data);//Geom::avgEdgeNormal(intersection, object->hitBox).normalize();
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
					auto correction = (hitBox.trans.pos - object->hitBox.trans.pos).setLength2(hitBox.getAvgSizeSqr());
					hitBox.trans.pos.add(correction.scl(0.05f));
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
			hitBox.genContinousRectBox(velocity, delta);
			hitBox.updateHitbox(trans);

			maxBound = hitBox.maxBound;
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

			trans.pos.add(velocity, delta);
			trans.rot += angularVelocity * delta;
			trans.rot = std::fmod(trans.rot, Math::DEG_FULL);

			angularAcceleration = 0;
			acceleration.setZero();

			updateHitbox(delta);
		}

		void addIntersection(const RealityEntity* object, const CollisionData intersection) const {
			if(object == nullptr)throw ext::NullPointerException{"Null Entity!"};

			std::lock_guard guard{intersectionLock};
			intersectedPointWith.insert_or_assign(object, intersection);
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
		/**
		 * @brief WARNING: this function may change the position of an entity enabling CCD!!!
		 */
		static bool exactInterscet(const RealityEntity* subject, const RealityEntity* object) {
			const bool needInterscetPointCalculation_subject = subject->requiresCollisionIntersection();
			const bool needInterscetPointCalculation_object = subject->requiresCollisionIntersection();

			CollisionData data = subject->hitBox.collideWithExact(object->hitBox, needInterscetPointCalculation_object || needInterscetPointCalculation_subject);

			if(data.valid()){
				if(needInterscetPointCalculation_subject){
					subject->addIntersection(object, data);
				}

				// if(needInterscetPointCalculation_object){
				// 	data.swapIndex();
				// 	object->addIntersection(subject, data);
				// }

				return true;
			}

			return false;
		}

		static const Geom::Shape::OrthoRectFloat& getHitBoound(const RealityEntity* reality_entity) {
			return reality_entity->hitBox.maxBound;
		}

		static bool roughInterscet(const RealityEntity* subject, const RealityEntity* object) {
			if(Math::abs(subject->layer - object->layer) > subject->collisionThickness + subject->collisionThickness)return false;
			if(subject->deletable() || object->deletable() || subject == object)return false;
			if(subject->ignoreCollisionTo(object) || object->ignoreCollisionTo(subject))return false;
			// if(subject->intersectedPointWith.contains(object))return false;
			return subject->hitBox.collideWithRough(object->hitBox);
		}

		static bool pointInterscet(const RealityEntity* subject, const Geom::Vec2 point) {
			return subject->hitBox.contains(point);
		}

		static bool within(const float dst, const RealityEntity* subject, const RealityEntity* object) {
			return subject->trans.pos.within(object->trans.pos, dst);
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

			if(std::isnan(subject->angularVelocity) || std::isinf(subject->angularVelocity))
				throw ext::RuntimeException{"Invalid Entitiy State!"};

			if(std::isnan(subject->angularAcceleration) || std::isinf(subject->angularAcceleration))
				throw ext::RuntimeException{"Invalid Entitiy State!"};
		}
	};
}