module;

export module Game.Entity.RealityEntity;

import ext.Concepts;

export import Game.Entity.BaseEntity;
export import Game.Entity.PhysicsAttribute;
export import Game.Entity.Controller;
export import Game.Faction;

import Game.Entity.Collision;

import Geom.Shape.RectBox;
import Geom.Rect_Orthogonal;
import Geom.Matrix3D;
import Geom.Vector2D;
import Geom;

import ext.RuntimeException;

import Math;

import std;

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
	class RealityEntity : public BaseEntity {
	public:
		static constexpr float angularVelocityLimit = 0.75f;
		static constexpr float angularAccelerationLimit = 0.05f;
		static constexpr float accelerationLimit = 8000;
		static constexpr float speedLimit = 10000;

	protected:
		Geom::Vec2 velocityCollision{};

		/**
		 * @brief CCD usage;
		 */
		Geom::OrthoRectFloat maxBound{};

	public:

		//Physics Attribute
		float collisionThickness = 1.0f;
		PhysicsAttribute_Rigid physicsBody{};

		std::unique_ptr<Controller> controller{std::make_unique<Controller>(this)};

		/** \brief Transform this entity from local to its parent or global.*/
		Geom::Matrix3D localToSuper{};

		/**
		 * \brief Children Objects should only be accessed from its parent object!
		 * also its lifetime should be the same as its parent object!
		 * normally they shouldn't has their own hitbox globally valid.
		 */
		std::unordered_map<IDType, std::unique_ptr<RealityEntity>> childrenObjects{};
		RealityEntity* parent{nullptr};

		//TODO no mutable
		/**
		 * \brief Entity collided with self. Modifiy physics should only done to self as the const said.
		 */
		mutable std::unordered_map<const RealityEntity*, CollisionData> intersectedPointWith{};

		//TODO using shared_mtx maybe
		mutable std::mutex intersectionLock{};

		/**
		 * \brief This *is(or at least should, children should process its collision by its parent) relative to the global reference frame!
		 */
		Game::HitBox hitBox{};

		float CCD_activeThresholdSpeedSqr = 250; //TODO also relative to size and frames!

		[[nodiscard]] virtual bool enableCCD() const {
			const float len2 = vel.vec.length2();

			return len2 >  CCD_activeThresholdSpeedSqr/* || len2 > hitBox.sizeVec2.length2() * 0.5f*/;
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

		virtual void targetUpdated(){}

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

		[[nodiscard]] Geom::OrthoRectFloat getDrawBound() const override {
			return maxBound;
		}

		virtual void updateTransition(){
			(void)localToSuper.setTranslation(trans);
		}

		void update(const Core::Tick deltaTick) override {
			controller->update();
			updateMovement(deltaTick);

			updateTransition();

			// checkStateValid(this);
		}

		void fetchToHitbox(){
			trans.vec = hitBox.trans.vec;
		}

		virtual void updateCollision(const float delatTick) {
			if(!enablePhysics() || intersectedPointWith.empty())return;

			fetchToHitbox();

			velocityCollision = vel.vec;
			for(auto& [entity, point] : intersectedPointWith) {
				calCollideTo(entity, point, delatTick);
			}

			fetchToHitbox();
		}

		[[nodiscard]] virtual bool selectable() const noexcept{
			return false;
		}

		virtual void assignController() const {

		}

		virtual bool isOverrideCollisionTo(const Game::RealityEntity* object) const {
			return false;
		}

		virtual void overrideCollisionTo(Game::RealityEntity* object, Geom::Vec2 intersection) {}

		[[nodiscard]] constexpr float getRotationalInertia() const {
			return hitBox.getRotationalInertia(physicsBody.inertialMass, physicsBody.rotationalInertiaScale);
		}

		[[nodiscard]] constexpr Geom::Vec2 collideVelAt(const Geom::Vec2 dst) const {
			return velocityCollision + dst.cross(vel.rot * Math::DEGREES_TO_RADIANS);
		}

		/**
		 * \param object To collide with, when [this] as the true object
		 */
		virtual bool ignoreCollisionTo(const Game::RealityEntity& object) const {
			return !enablePhysics() || physicsBody.inertialMass / object.physicsBody.inertialMass < 0.0005f;
		}

		/*virtual*/ void intersectionCorrection(Geom::Vec2& intersection) const {
			intersection.lerp(hitBox.trans.vec, 0.0001f);
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
			const Vec2 dstToSubject = data.intersection - hitBox.trans.vec;
			const Vec2 dstToObject  = data.intersection - object->hitBox.trans.vec;

			const Vec2 subjectVel = collideVelAt(dstToSubject);
			const Vec2 objectVel = object->collideVelAt(dstToObject);

			const Vec2 relVel = objectVel - subjectVel;

			//TODO overlap quit process
			if(relVel.isZero())return;

			const Vec2 collisionNormalVec = -object->hitBox.getAvgEdgeNormal(data);//Geom::avgEdgeNormal(intersection, object->hitBox).normalize();
			Vec2 collisionTangentVec = collisionNormalVec.copy().rotateRT_counterClockwise();

			// collisionTangentVec.setZero();

			if(collisionTangentVec.dot(relVel) < 0)collisionTangentVec.reverse();

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
				additional.reverse();
				if(velocityCollision.length() * physicsBody.inertialMass < object->velocityCollision.length() * object->physicsBody.inertialMass) {
					auto correction = (hitBox.trans.vec - object->hitBox.trans.vec).setLength2(hitBox.getAvgSizeSqr());
					hitBox.trans.vec.add(correction.scl(0.05f));
					vel.vec.add(correction.scl(0.185f));
				}
			}

			// additional.limit2(40 * inertialMass * inertialMass);

			accel.vec += additional / physicsBody.inertialMass;


			vel.vec.add(additional * (delatTick * veloAddScale / physicsBody.inertialMass));
			//
			accel.rot += dstToSubject.cross(additional) / subjectRotationalInertia * Math::RADIANS_TO_DEGREES;
		}

		virtual void updateHitbox(const float delta) {
			hitBox.genContinousRectBox(vel.vec, delta);
			hitBox.updateHitbox(trans);

			maxBound = hitBox.maxBound;
		}

		virtual void updateMovement(const float delta) {
			accel.vec.clampMax(accelerationLimit);
			accel.rot = Math::clampRange(accel.rot, angularAccelerationLimit);

			{
				//TODO pre global force field process.
				//TODO drag should be applied by things like a global force field.
				vel.rot = Math::lerp(vel.rot, 0, 0.0075f * delta);
				vel.vec.lerp(Geom::ZERO, 0.075f * delta);
			}

			//Loacl process
			vel.vec.mulAdd(accel.vec, delta);
			vel.rot += accel.rot * delta;

			vel.rot = Math::clampRange(vel.rot, angularVelocityLimit);
			vel.vec.clampMax(speedLimit);

			trans.vec.mulAdd(vel.vec, delta);

			if(controller->moveCommand.rotateActivated()){
				trans.rot = Math::Angle::moveToward_signed(
					trans.rot, controller->moveCommand.expectedFaceAngle,
					vel.rot * delta, 2.0f * delta, [this]{
						vel.rot = 0;
					});
			}else{
				trans.rot += vel.rot * delta;
			}

			trans.rot = Math::Angle::getAngleInPi2(trans.rot);

			accel.setZero();

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

		float getYawAngle() const noexcept{
			return vel.vec.angle() - trans.rot;
		}

		//TODO abstract these to other classes
		//TODO is this a good idea? this actually modifies many state of the entities
		/**
		 * @brief WARNING: this function may change the position of an entity enabling CCD!!!
		 */
		static bool exactInterscet(const RealityEntity& subject, const RealityEntity& object) {
			const bool needInterscetPointCalculation_subject = subject.requiresCollisionIntersection();
			const bool needInterscetPointCalculation_object = subject.requiresCollisionIntersection();

			CollisionData data = subject.hitBox.collideWithExact(object.hitBox, needInterscetPointCalculation_object || needInterscetPointCalculation_subject);

			if(data.valid()){
				if(needInterscetPointCalculation_subject){
					subject.addIntersection(&object, data);
				}

				// if(needInterscetPointCalculation_object){
				// 	data.swapIndex();
				// 	object->addIntersection(subject, data);
				// }

				return true;
			}

			return false;
		}

		static const Geom::OrthoRectFloat& getHitBoound(const RealityEntity& reality_entity) {
			return reality_entity.hitBox.maxBound;
		}

		static bool roughInterscet(const RealityEntity& subject, const RealityEntity& object) {
			if(Math::abs(subject.zLayer - object.zLayer) > subject.collisionThickness + subject.collisionThickness)return false;
			if(subject.deletable() || object.deletable() || &subject == &object)return false;
			if(subject.ignoreCollisionTo(object) || object.ignoreCollisionTo(subject))return false;
			// if(subject->intersectedPointWith.contains(object))return false;
			return true;
		}

		static bool pointInterscet(const RealityEntity& subject, const Geom::Vec2 point) {
			return subject.hitBox.contains(point);
		}

		static bool within(const float dst, const RealityEntity& subject, const RealityEntity& object) {
			return subject.trans.vec.within(object.trans.vec, dst);
		}

		static void checkStateValid(const RealityEntity& subject) {
			// return;
#ifndef _DEBUG
			return;
#endif

			if(subject.accel.vec.isNaN() || subject.accel.vec.isInf())
				throw ext::RuntimeException{"Invalid Entitiy State!"};

			if(subject.vel.vec.isNaN() || subject.vel.vec.isInf())
				throw ext::RuntimeException{"Invalid Entitiy State!"};

			if(std::isnan(subject.vel.rot) || std::isinf(subject.vel.rot))
				throw ext::RuntimeException{"Invalid Entitiy State!"};

			if(std::isnan(subject.accel.rot) || std::isinf(subject.accel.rot))
				throw ext::RuntimeException{"Invalid Entitiy State!"};
		}
	};
}