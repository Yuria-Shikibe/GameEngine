//
// Created by Matrix on 2024/1/1.
//

export module Game.Entity.Bullet;

export import Game.Entity.RealityEntity;

export import Game.Attributes.DamageTrait;

import Game.Entity.EntityManager;
import Game.Entity.Collision;
// import GL.Texture.TextureRegionRect;
import Graphic.TextureAtlas;
import OS.File;
import Geom.Vector2D;
import Graphic.Trail;

import ext.Timer;

import Math.Timed;

export namespace Game{
	class Bullet;

	struct BulletTrait {
		DamageComposition initDamage{};
		float maximumLifetime{60.0f};
		float initSpeed{200.0f};

		float trailUpdateSpacing = 0.5f;
		std::size_t trailLength = 20;

		virtual ~BulletTrait() = default;

		virtual void update(Bullet& bullet) const = 0;

		virtual void onShoot(Bullet& bullet) const;

		virtual void hit(Bullet& bullet, RealityEntity& entity, Geom::Vec2 actualPosition) const = 0;

		virtual void despawn(Bullet& bullet) const = 0;

		virtual void draw(const Bullet& bullet) const = 0;

		[[nodiscard]]
		virtual bool despawnable(const Bullet& bullet) const;

		virtual void loadTexture(const OS::File& dir, const Graphic::TextureAtlas& atlas){

		}

		virtual void init(){

		}

		virtual float getMaximumRange(){
			return 0;
		}
	};

	class Bullet : public Game::RealityEntity {
	public:
		const BulletTrait* trait{nullptr};

		Math::Timed life{};

		DamageComposition damage{};

		ext::Timer<> timer{};
		Graphic::Trail trail{20};

		Game::RealityEntity* shooter{nullptr};

		bool removeable{false};
	public:

		float getTrailLifetime() const{
			const float velo = vel.vec.length();
			const float dst = trail.getDst();
			if(velo == 0.0f) [[unlikely]] return dst * 5.0f;
			return dst / velo;
		}

		void activate() override{
			RealityEntity::activate();

			trait->onShoot(*this);
		}

		bool isOverrideCollisionTo(const Game::RealityEntity* object) const override{
			return true;
		}

		bool ignoreCollisionTo(const Game::RealityEntity* object) const override{
			return removeable || object == shooter || object->getFaction()->isAllyTo(faction);
		}

		void overrideCollisionTo(Game::RealityEntity* object, const Geom::Vec2 intersection) override{
			if(removeable)return;

			updateEnd();
			trail.update(intersection.x, intersection.y);

			trait->hit(*this, *object, intersection);
			object->dealDamage(damage);
			removeable = true;
		}

		void updateMovement(const float delta) override{
			accel.vec.clampMax(accelerationLimit);
			accel.rot = Math::clampRange(accel.rot, angularAccelerationLimit);

			{
				//TODO pre global force field process.
				//TODO drag should be applied by things like a global force field.
				vel.rot = Math::lerp(vel.rot, 0, 0.005f * delta);
				vel.vec.lerp(Geom::ZERO, 0.005f * delta);
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

		void updateEnd(){
			trail.update(trans.vec.x, trans.vec.y);
		}

		void update(const float dt) override{
			if(removeable){
				deactivate();
				return;
			}

			RealityEntity::update(dt);

			life.time += dt;
			if(life.time >= life.lifetime){
				life.time = life.lifetime;
				removeable = true;
				despawn();
			}

			timer.run(trait->trailUpdateSpacing, dt, [this]{
				trail.update(trans.vec.x, trans.vec.y);
			});


			if(trait->despawnable(*this))despawn();
		}

		void calCollideTo(const Game::RealityEntity* object, CollisionData intersection, const float delatTick) override{

		}

		Geom::OrthoRectFloat getDrawBound() const override{
			const auto trailBound = trail.getBound();
			if(trailBound.area() == 0)return maxBound;
			return maxBound.copy().expandBy(trailBound);
		}

		void despawn(){
			removeable = true;
			updateEnd();
			trait->despawn(*this);
		}

		void updateCollision(const float deltaTick) override {
			intersectedPointWith.clear();
			EntityManage::realEntities.quadTree->intersectAny(this);

			RealityEntity::updateCollision(deltaTick);
		}

		void draw() const override{
			trait->draw(*this);
		}

		void drawDebug() const override{

		}
	};

	void BulletTrait::onShoot(Bullet& bullet) const{
		bullet.damage = initDamage;
		bullet.life.lifetime = maximumLifetime;
		bullet.vel.vec.setLength(initSpeed);
	}

	bool BulletTrait::despawnable(const Bullet& bullet) const{
		return bullet.vel.vec.isZero(0.005f);
	}
}
