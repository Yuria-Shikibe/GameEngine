export module Game.Entity.SpaceCraft;

import Game.Entity.RealityEntity;

import Game.Entity.EntityManager;
import Game.Entity.Turrets;
import Graphic.Color;
import Graphic.Draw;
import Math;
import Math.Rand;

import Geom;

import RuntimeException;

import std;

export namespace Game {
	class SpaceCraft;

	struct SpaceCraftTrait {
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
		const SpaceCraftTrait* trait{nullptr};

		std::vector<Containers::Pool<TurretEntity>::UniquePtr> turretEntities{};

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
				turret->relativePosition.add(rand.range(40), rand.range(40));
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
			if(controller->moveCommand.moveActivated()){
				const auto dest = controller->moveCommand.nextDest();
				const float angleDst = Math::Angle::angleDst(trans.rot, controller->moveCommand.expectedFaceAngle);
				if(!Math::zero(angleDst)){
					angularAcceleration = Math::approach(angularAcceleration, Math::Angle::angleDstSign(trans.rot, controller->moveCommand.expectedFaceAngle) * 0.5f, delta * 0.5f);
				}

				if(controller->moveCommand.requiresMovement(dest)){
					//TODO perform according to moveCommand.expected velocity
					constexpr float tolerance = 20.0f;
					auto dir = (dest - trans.pos);

					const float dst2 = dir.length2();
					if(dst2 < Math::sqr(tolerance)){
						acceleration = dir.setLength2(-1.5f * (1 - dst2 / Math::sqr(tolerance)));
					}else{
						acceleration.approach(dir.setLength2(2.25f), delta);
					}
				}
			}

			RealityEntity::updateMovement(delta);
		}

		void update(const float deltaTick) override {
			RealityEntity::update(deltaTick);

			if(const auto t = trans.pos.copy().abs(); t.x > 50000 || t.y > 50000) {
				deactivate();
			}

			if(health < 0){
				deactivate();
			}

			for(const auto& turretEntity : turretEntities){
				turretEntity->update(deltaTick);
			}
		}

		[[nodiscard]] bool selectable() const override{
			return true;
		}

		void assignController() const override{

		}

		void draw() const override {
			trait->draw(this);
			Graphic::Draw::setTexture(Graphic::Draw::defaultLightTexture);
			drawDebug();
			Graphic::Draw::setTexture();
		}

		void drawDebug() const override {
			using namespace Graphic;

			Draw::alpha();
			Draw::color(Colors::RED);

			for(const auto& data : intersectedPointWith | std::ranges::views::values) {
				Draw::rectOrtho<WorldBatch>(data.intersection.x - 2, data.intersection.y - 2, 4, 4);
			}

			Draw::Line::setLineStroke(1.0f);
			Draw::color(Colors::MAGENTA);
			Draw::Line::lineAngle<WorldBatch>(trans.pos.x, trans.pos.y, trans.rot, std::sqrt(hitBox.getAvgSizeSqr()));

			Draw::Line::setLineStroke(2.0f);

			for (auto& boxData : hitBox.hitBoxGroup){
				constexpr Color colors[]{Colors::ROYAL, Colors::PINK, Colors::GREEN, Colors::PURPLE};
				auto& cur = boxData.original;
				for(int i = 0; i < 4; ++i) {
					Draw::color(colors[i]);
					Draw::rectOrtho<WorldBatch>(cur[i].x - 2, cur[i].y - 2, 4, 4);

					const Vec2 begin = cur[i];
					const Vec2 end = cur[(i + 1) % 4];
					const Vec2 center = (begin + end) / 2;

					Draw::Line::line<WorldBatch>(center, center + cur.getNormalVec(i).normalize().scl(25));
				}

				if(controller->selected) {
					Draw::color(Colors::TAN);
				}else Draw::color(Colors::LIGHT_GRAY);

				Draw::Line::setLineStroke(2);
				Draw::Line::quad<WorldBatch>(cur);

				Draw::Line::line<WorldBatch>(cur.v0, cur.originPoint, colors[0], Colors::RED);

				Draw::color(Colors::RED);
				Draw::rectOrtho<WorldBatch>(cur.originPoint.x - 2, cur.originPoint.y - 2, 4, 4);
			}


			Draw::color(Colors::PURPLE);
			Draw::rectOrtho<WorldBatch>(hitBox.trans.pos.x - 2, hitBox.trans.pos.y - 2, 4, 4);

			for(auto& turretEntity : turretEntities){
				turretEntity->drawDebug();
			}
		}
	};
}
