//
// Created by Matrix on 2024/1/4.
//

export module Game.Entity.Turrets;

export import Game.Entity.RealityEntity;

import Geom.Vector2D;
import Math;
import GL.Texture.TextureRegionRect;

export namespace Game{
	class TurretEntity;

	struct TurretDrawer {

	};

	struct TurretTrait {
		virtual ~TurretTrait() = default;
		/**
		 * @brief In Degree
		 */
		float maxRotateSpeed{5};
		float rotateAccel{1.25f};

		float shootAngleTolerance{20};

		float initialDirection{0};
		float rotateLimitation{-1};

		float reloadTime{60};

		bool reloadAlways{false};

		float zOffset = 0;

		[[nodiscard]] bool hasRotateLimitation() const {
			return rotateLimitation > 0 && rotateLimitation < 180;
		}

		virtual void update(TurretEntity* turret) const = 0;

		virtual void draw(const TurretEntity* turret) const = 0;

		virtual void shoot(TurretEntity* turret, RealityEntity* shooter) const = 0;

		virtual void init(TurretEntity* turret) const;

	};

	class TurretEntity : public BaseEntity {
	protected:
		const TurretTrait* trait{nullptr};
		Geom::Vec2 targetPosition{};

		float rotateSpeed{0};

		float reload{0};

		float expectedRotaton{0};

		bool firing{false};

		bool quitBattle{true};

		RealityEntity* parent{nullptr};

	public:
		Geom::Vec2 relativePosition{};

		void update(const float deltaTick) override{
			firing &= !quitBattle;

			updatePosition();
			updateExpectedRotate();
			updateRotate(deltaTick);
			updateReload(deltaTick);

			trait->update(this);
		}

		void updateExpectedRotate(){
			if(quitBattle){
				expectedRotaton = parent->transformToGlobalAngle(trait->initialDirection);
			}else{
				expectedRotaton = (targetPosition - trans.pos).angle();
			}
		}

		void updateRotate(const float deltaTick){
			if(!Math::Angle::within(expectedRotaton, trans.rot, .0005f)){
				rotateSpeed = Math::approach(rotateSpeed, trait->maxRotateSpeed, trait->rotateAccel * deltaTick);
				trans.rot = Math::Angle::moveToward(trans.rot, expectedRotaton, rotateSpeed * deltaTick);

				if(trait->hasRotateLimitation()){
					trans.rot = Math::Angle::clampRange(trans.rot, trait->initialDirection, trait->rotateLimitation);
				}
			}else{
				trans.rot = expectedRotaton;
			}
		}

		void updateReload(const float delta){
			if(shouldReload()){
				if(reload >= trait->reloadTime){
					if(shouldShoot()){
						shoot();
						reload = 0;
					}else{
						reload = trait->reloadTime;
					}
				}else{
					reload += delta * getReloadSpeedMultipler();
				}
			}
		}

		void updatePosition(){
			trans.pos = relativePosition;
			trans.pos = parent->transformToSuper(trans.pos);
		}

		void shoot(){
			trait->shoot(this, parent);
		}

		[[nodiscard]] bool shouldShoot() const{
			return firing && Math::Angle::within(expectedRotaton, trans.rot, trait->shootAngleTolerance);
		}

		[[nodiscard]] float getReloadSpeedMultipler() const{
			return 0.35f + 0.65f * getHealthRatio();
		}

		void activateFiring(){
			quitBattle = false;
			firing = true;
		}

		[[nodiscard]] bool shouldReload() const{
			return trait->reloadAlways || firing;
		}

		void draw() const override{
			trait->draw(this);
		}

		void drawDebug() const override{
			draw();
		}

		void init() override{

		}

		void init(const TurretTrait* const trait, RealityEntity* parent){
			setTrait(trait);
			trait->init(this);
			this->parent = parent;

			init();
		}

		void setTargetPosition(const Geom::Vec2 targetPosition){
			this->targetPosition = targetPosition;
		}

		void setTrait(const TurretTrait* const trait){
			this->trait = trait;
		}

		[[nodiscard]] const TurretTrait* getTrait() const{
			return trait;
		}
	};


	void TurretTrait::init(TurretEntity* turret) const{
		turret->trans.rot = initialDirection;
	}
}
