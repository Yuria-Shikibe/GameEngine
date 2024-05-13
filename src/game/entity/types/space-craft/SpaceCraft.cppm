export module Game.Entity.SpaceCraft;

import Game.Entity.RealityEntity;

import Game.Entity.EntityManager;
import Game.Entity.Turrets;
import Graphic.Color;
import Graphic.Draw;
import Math;
import Math.Rand;

import Geom;

import Font.GlyphArrangement;

import ext.RuntimeException;

import std;
import ext.Encoding;

import Game.Graphic.Draw;
import Game.Chamber;
import Game.Chamber.Frame;
import Game.Chamber.FrameTrans;

export import Game.Attributes.ThrusterTrait;

export namespace Game {
	class SpaceCraft;

	struct SpaceCraftTrait {
		ThrusterTrait thrusterTrait{};
		virtual ~SpaceCraftTrait() = default;
		virtual void init(SpaceCraft* entity) const = 0;
		virtual void update(SpaceCraft* entity) const = 0;
		virtual void draw(const SpaceCraft* entity) const = 0;
	};

	/**
	 *	TODO chambers
	 *	TODO crews
	 *	TODO turrets
	 *	TODO armor system
	 *	TODO energy system
	 *	TODO better move impl
	 *	TODO better command impl
	 * @brief
	 */
	class SpaceCraft : public RealityEntity{
	public:
		Geom::Transform chamberTrans{};
		ChamberFrameTrans<SpaceCraft> chambers{};

		const SpaceCraftTrait* trait{nullptr};

		std::shared_ptr<Font::GlyphLayout> coordText = Font::obtainLayoutPtr();

		std::vector<ext::ObjectPool<TurretEntity>::UniquePtr> turretEntities{};

		[[nodiscard]] SpaceCraft() = default;

		void init(const SpaceCraftTrait* const trait){
			this->trait = trait;

			// init();
		}

		void init() override{
			if(!trait){
				throw ext::NullPointerException{"Unable To Find SpaceCraft Trait"};
			}
			auto rand = Math::globalRand;

			const int turrets = rand.random(2, 5);

			turretEntities.reserve(turrets);

			for(int i = 0; i < turrets; ++i){
				turretEntities.push_back(EntityManage::obtainUnique<TurretEntity>());
			}

			for(const auto& turret : turretEntities){
				turret->relativePosition.add(rand.range(40.0f), rand.range(40.0f));
			}
		}

		void setTurretType(const TurretTrait* trait){
			for(const auto& turret : turretEntities){
				turret->init(trait, this);
			}
		}

		void acceptTurretTargets() const{
			if(controller->turretTargets.empty())return;
			for(const auto& turret : turretEntities){
				turret->setTargetPosition(controller->turretTargets.front());
				turret->activateFiring();
			}
		}

		void targetUpdated() override{
			acceptTurretTargets();
		}

		void updateCollision(const float deltaTick) override {
			intersectedPointWith.clear();
			EntityManage::realEntities.quadTree->intersectAll(this);

			RealityEntity::updateCollision(deltaTick);
		}

		void updateMovement(const float delta) override{
			constexpr float maxAngularSpeed = angularVelocityLimit;
			constexpr float maxAngularAccel = angularAccelerationLimit;

			const float powerScale = trait->thrusterTrait.getPowerScale(getYawAngle());

			if(controller->moveCommand.rotateActivated()){
				float angleDst = Math::Angle::angleDst(trans.rot, controller->moveCommand.expectedFaceAngle);
				if(!Math::zero(angleDst, 0.5f)){
					const float curAngularInertial = 0.5f * Math::sqr(this->vel.rot) / maxAngularAccel;
					const float maxAngularInertial = 0.5f * Math::sqr(maxAngularSpeed) / maxAngularAccel;

					const float dstSign = Math::Angle::angleDstSign(trans.rot, controller->moveCommand.expectedFaceAngle);

					if(curAngularInertial < angleDst || Math::diffSign(vel.rot, dstSign)){
						if(Math::diffSign(accel.rot, dstSign)){
							accel.rot = Math::min(maxAngularAccel, 0.5f * Math::sqr(vel.rot) / angleDst) * dstSign * -1;
						}else{
							accel.rot = Math::min(maxAngularAccel, (maxAngularSpeed - Math::abs(vel.rot)) / delta) * dstSign;
						}
					}else{
						if(curAngularInertial > angleDst){
							accel.rot = Math::min(maxAngularAccel, 0.5f * Math::sqr(vel.rot) / angleDst) * dstSign * -1;
						}
					}
				}else{
					accel.rot = 0;
				}
			}else{
				if(!Math::zero(vel.rot, 0.005f)){
					accel.rot = std::copysign(Math::min(maxAngularAccel, Math::abs(vel.rot) / delta), -vel.rot) * angularAccelerationLimit;
				}
			}

			if(controller->moveCommand.moveActivated()){
				const Vec2 dest = controller->moveCommand.nextDest();
				if(controller->moveCommand.requiresMovement(dest)){
					//TODO perform according to moveCommand.expected velocity
					constexpr float tolerance = 20.0f;
					auto dir = (dest - trans.vec);

					const float dst2 = dir.length2();
					if(dst2 < Math::sqr(tolerance)){
						accel.vec = dir.setLength2(-1.5f * (1 - dst2 / Math::sqr(tolerance))) * powerScale;
					}else{
						accel.vec.approach(dir.setLength2(2.25f), delta * powerScale);
					}
				}
			}

			accel.vec.clampMax(accelerationLimit * powerScale);
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
			vel.vec.clampMax(speedLimit * powerScale);

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

		void calculateInScreen(const Geom::OrthoRectFloat& viewport) override{
			RealityEntity::calculateInScreen(viewport);

			chambers.updateDrawTarget(viewport);
		}

		void update(const float deltaTick) override {
			RealityEntity::update(deltaTick);

			// auto t = chamberTrans | trans;
			chambers.setLocalTrans(chamberTrans | trans);

			if(const auto t = trans.vec.copy().toAbs(); t.x > 50000 || t.y > 50000) {
				deactivate();
			}

			if(health < 0){
				deactivate();
			}

			for(const auto& turretEntity : turretEntities){
				turretEntity->update(deltaTick);
			}
		}

		[[nodiscard]] bool selectable() const noexcept override{
			return true;
		}

		void assignController() const override{

		}

		void draw() const override {
			trait->draw(this);
			Game::Draw::chamberFrame(*this, chambers);
		}

		void drawDebug() const override {
			Game::Draw::chamberFrameTile(chambers);

			GL::setDepthMask(false);
			Font::defGlyphParser->parseWith(coordText, std::format("$<scl#[0.85]>$<color#[eeeeeeff]>Health: {:.1f}", this->health));
			coordText->offset.set(this->trans.vec).add(maxBound.getWidth() * 0.55f, maxBound.getHeight() * 0.55f);
			coordText->setAlign(Align::Mode::bottom_left);
			coordText->render();
			// Graphic::Batch::flush();
			GL::setDepthMask(true);

			Graphic::Draw::alpha();
			Graphic::Draw::color(Graphic::Colors::RED);

			for(const auto& data : intersectedPointWith | std::ranges::views::values) {
				Graphic::Draw::rectOrtho(data.intersection.x - 2, data.intersection.y - 2, 4, 4);
			}

			Graphic::Draw::Line::setLineStroke(1.0f);
			Graphic::Draw::color(Graphic::Colors::MAGENTA);
			Graphic::Draw::Line::lineAngle(trans.vec.x, trans.vec.y, trans.rot, std::sqrt(hitBox.getAvgSizeSqr()));

			Graphic::Draw::Line::setLineStroke(2.0f);

			for (auto& boxData : hitBox.hitBoxGroup){
				constexpr Graphic::Color colors[]{Graphic::Colors::ROYAL, Graphic::Colors::PINK, Graphic::Colors::GREEN, Graphic::Colors::PURPLE};
				auto& cur = boxData.original;
				for(int i = 0; i < 4; ++i) {
					Graphic::Draw::color(colors[i]);
					Graphic::Draw::rectOrtho(cur[i].x - 2, cur[i].y - 2, 4, 4);

					const Vec2 begin = cur[i];
					const Vec2 end = cur[(i + 1) % 4];
					const Vec2 center = (begin + end) / 2;

					Graphic::Draw::Line::line(center, center + cur.getNormalVec(i).normalize().scl(25));
				}

				if(controller->selected) {
					Graphic::Draw::color(Graphic::Colors::TAN);
				}else Graphic::Draw::color(Graphic::Colors::LIGHT_GRAY);

				Graphic::Draw::Line::setLineStroke(2);
				Graphic::Draw::Line::quad(cur);

				Graphic::Draw::Line::line(cur.v0, cur.originPoint, colors[0], Graphic::Colors::RED);

				Graphic::Draw::color(Graphic::Colors::RED);
				Graphic::Draw::rectOrtho(cur.originPoint.x - 2, cur.originPoint.y - 2, 4, 4);
			}


			Graphic::Draw::color(Graphic::Colors::PURPLE);
			Graphic::Draw::rectOrtho(hitBox.trans.vec.x - 2, hitBox.trans.vec.y - 2, 4, 4);

			for(auto& turretEntity : turretEntities){
				turretEntity->drawDebug();
			}
		}
	};
}
