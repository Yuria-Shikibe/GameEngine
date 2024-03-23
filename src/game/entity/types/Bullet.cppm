//
// Created by Matrix on 2024/1/1.
//

export module Game.Entity.Bullet;

export import Game.Entity.RealityEntity;

export import Game.Settings.DamageTrait;

import Game.Entity.EntityManager;
import Game.Entity.Collision;
// import GL.Texture.TextureRegionRect;
import Graphic.TextureAtlas;
import OS.File;
import Geom.Vector2D;

import Graphic.Timed;

export namespace Game{
	class Bullet;

	struct BulletTrait {
		DamageComposition initDamage{};
		float maximumLifetime{60.0f};
		float initSpeed{20.0f};

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


	};

	class Bullet : public Game::RealityEntity {
	public:
		const BulletTrait* trait{nullptr};

		Graphic::Timed life{};

		DamageComposition damage{};

		Game::RealityEntity* shooter{nullptr};

		bool removeable{false};
	public:

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

			trait->hit(*this, *object, intersection);
			object->dealDamage(damage);
			removeable = true;
		}

		void update(const float dt) override{
			if(removeable){
				deactivate();
				return;
			}

			RealityEntity::update(dt);

			if(trait->despawnable(*this))despawn();
		}

		void calCollideTo(const Game::RealityEntity* object, CollisionData intersection, const float delatTick) override{

		}

		void despawn(){
			removeable = true;
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
	}

	bool BulletTrait::despawnable(const Bullet& bullet) const{
		return bullet.velocity.isZero(0.005f);
	}
}
