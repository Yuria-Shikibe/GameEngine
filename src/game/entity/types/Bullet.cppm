//
// Created by Matrix on 2024/1/1.
//

export module Game.Entity.Bullet;

export import Game.Entity.RealityEntity;

export import Game.Settings.DamageTrait;

import Game.Entity.EntityManager;



export namespace Game{
	class Bullet;

	struct BulletTrait {
		DamageComposition initDamage{};
		virtual ~BulletTrait() = default;

		virtual void update(Bullet& bullet) const = 0;

		virtual void init(Bullet& bullet) const = 0;

		virtual void hit(Bullet& bullet, RealityEntity& entity, Geom::Vec2 actualPosition) const = 0;

		virtual void despawn(Bullet& bullet) const = 0;

		virtual void draw(const Bullet& bullet) const = 0;
	};

	class Bullet : public Game::RealityEntity {
	public:
		const BulletTrait* trait{nullptr};

		DamageComposition damage{};

		Game::RealityEntity* shooter{nullptr};

		bool removeable{false};
	public:

		void activate() override{
			RealityEntity::activate();

			trait->init(*this);
		}

		bool isOverrideCollisionTo(const Game::RealityEntity* object) const override{
			return true;
		}

		bool ignoreCollisionTo(const Game::RealityEntity* object) const override{
			return object == shooter || object->getFaction()->isAllyTo(faction);
		}

		void overrideCollisionTo(Game::RealityEntity* object, const Geom::Vec2 intersection) override{
			if(removeable)return;
			trait->hit(*this, *object, intersection);
			object->dealDamage(damage);
			removeable = true;
		}

		void update(const float dt) override{
			if(removeable)deactivate();

			RealityEntity::update(dt);

			if(velocity.isZero(0.005f))remove();
		}

		void remove(){
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
}
