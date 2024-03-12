export module Game.Entity.SpaceCraft;

import Game.Entity.RealityEntity;

import Game.Entity.EntityManager;
import Graphic.Color;
import Graphic.Draw;
import Graphic.Draw.Lines;
import Math;
import OS;
import <iostream>;
import <ranges>;
import <unordered_map>;

import Core;
import Geom;

export namespace Game {
	class SpaceCraft : public RealityEntity{
	public:
		bool intersected = false;
		[[nodiscard]] SpaceCraft() = default;

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
				float angleDst = Math::Angle::angleDist(rotation, controller->moveCommand.expectedFaceAngle);
				if(!Math::zero(angleDst)){
					angularAcceleration = Math::approach(angularAcceleration, Math::Angle::angleDistSign(rotation, controller->moveCommand.expectedFaceAngle) * 0.5f, delta * 0.5f);
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
		}
	};
}