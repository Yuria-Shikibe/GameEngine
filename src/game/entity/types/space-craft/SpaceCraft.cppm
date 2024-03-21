export module Game.Entity.SpaceCraft;

import Game.Entity.RealityEntity;

import Game.Entity.EntityManager;
import Game.Entity.Turrets;
import Graphic.Color;
import Graphic.Draw;
import Math;
import Math.Rand;

import Geom;

import std;

export namespace Game {
	class SpaceCraft;

	struct SpaceCraftTrait {
		virtual void init(SpaceCraft* spaceCraft) = 0;

		virtual void draw(const SpaceCraft* spaceCraft) = 0;


	};

	class SpaceCraft : public RealityEntity{
	public:
		const SpaceCraftTrait* trait{nullptr};

		bool intersected = false;

		std::vector<Containers::Pool<TurretEntity>::UniquePtr> turretEntities{};

		[[nodiscard]] SpaceCraft() = default;

		void init() override{
			auto rand = Math::globalRand;

			const int turrets = rand.random(2, 5);

			turretEntities.reserve(turrets);

			for(int i = 0; i < turrets; ++i){
				turretEntities.push_back(EntityManage::obtainUnique<TurretEntity>());
			}


			for(const auto& turret : turretEntities){
				turret->relativePosition.x = rand.random(hitBox.offset.x, hitBox.offset.x + hitBox.sizeVec2.x);
				turret->relativePosition.y = rand.random(hitBox.offset.y, hitBox.offset.y + hitBox.sizeVec2.y);
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
			intersected = false;

			intersectedPointWith.clear();

			//TODO is this really good?
			EntityManage::realEntities.quadTree->intersect(this, [this](auto t){
				intersected = true;
			});

			RealityEntity::updateCollision(deltaTick);
		}

		void updateMovement(const float delta) override{
			if(controller->moveCommand.moveActivated()){
				const auto dest = controller->moveCommand.nextDest();
				float angleDst = Math::Angle::angleDst(rotation, controller->moveCommand.expectedFaceAngle);
				if(!Math::zero(angleDst)){
					angularAcceleration = Math::approach(angularAcceleration, Math::Angle::angleDstSign(rotation, controller->moveCommand.expectedFaceAngle) * 0.5f, delta * 0.5f);
				}

				if(controller->moveCommand.requiresMovement(dest)){
					//TODO perform according to moveCommand.expected velocity
					constexpr float tolerance = 20.0f;
					auto dir = (dest - position);

					float dst2 = dir.length2();
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

			if(const auto t = position.copy().abs(); t.x > 50000 || t.y > 50000) {
				deactivate();
			}

			if(health < 0){
				deactivate();
			}

			for(auto& turretEntity : turretEntities){
				turretEntity->update(deltaTick);
			}
		}

		[[nodiscard]] bool selectable() const override{
			return true;
		}

		void assignController() const override{

		}

		void draw() const override {
			drawDebug();
		}

		void drawDebug() const override {
			using namespace Graphic;
			Draw::alpha();
			if(controller->selected) {
				Draw::color(Colors::TAN);
			}else if(intersected)Draw::color(Colors::AQUA);
			else Draw::color(Colors::LIGHT_GRAY);
			Draw::Line::setLineStroke(2);
			Draw::quad(Draw::defaultTexture, hitBox.v0, hitBox.v1, hitBox.v2, hitBox.v3);

			Draw::color(Colors::GRAY);
			Draw::alpha(0.3f);
			if(enableCCD()) {
				for(const auto& [v0, v1, v2, v3, maxOrthoBound] : trace.contiounsTraces) {
					Draw::quad(Draw::defaultTexture, v0, v1, v2, v3);
					Draw::Line::rect(maxOrthoBound);
				}
			}
			Draw::alpha();
			Draw::color(Colors::RED);

			if(intersected)for(const auto& vec2 : intersectedPointWith | std::ranges::views::values) {
				Draw::rect(vec2.x - 2, vec2.y - 2, 4, 4);
			}

			Draw::Line::setLineStroke(1.0f);
			Draw::color(Colors::MAGENTA);
			Draw::Line::lineAngle(position.x, position.y, rotation, hitBox.sizeVec2.length());

			Draw::Line::setLineStroke(2.0f);
			constexpr Color colors[]{Colors::ROYAL, Colors::PINK, Colors::GREEN, Colors::PURPLE};
			for(int i = 0; i < 4; ++i) {
				Draw::color(colors[i]);
				Draw::rect(hitBox[i].x - 2, hitBox[i].y - 2, 4, 4);

				const Vec2 begin = hitBox[i];
				const Vec2 end = hitBox[(i + 1) % 4];
				const Vec2 center = (begin + end) / 2;

				Draw::Line::line(center, center + hitBox.getNormalVec(i).normalize().scl(25));
			}
			//
			// auto normalNearest = Geom::avgEdgeNormal(Core::camera->getPosition(), hitBox);
			// Draw::color(Colors::YELLOW);
			// Draw::line(hitBox.originPoint, hitBox.originPoint + normalNearest.normalize().scl(100));

			for(auto value : intersectedPointWith | std::ranges::views::values) {
				intersectionCorrection(value);
				auto nor = Geom::avgEdgeNormal(value, hitBox);

				Draw::color(Colors::ORANGE);
				Draw::Line::setLineStroke(2.0f);
				Draw::Line::line(value, value + nor.setLength(150 + hitBox.sizeVec2.length2() / 30));
				Draw::Line::setLineStroke(0.80f);
				Draw::Line::line(value, hitBox.originPoint);
			}

			Draw::color(Colors::BLACK);
			Draw::rect(hitBox.originPoint.x - 2, hitBox.originPoint.y - 2, 4, 4);

			for(auto& turretEntity : turretEntities){
				turretEntity->draw();
			}
		}
	};
}
