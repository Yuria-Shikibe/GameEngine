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
	std::atomic_int totalHit = 0;

	class SpaceCraft : public RealityEntity{
	public:
		bool intersected = false;
		[[nodiscard]] SpaceCraft() = default;

		void updateCollision(const float deltaTick) override {
			intersected = false;

			intersectedPointWith.clear();

			//TODO is this really good?
			if(EntityManage::realEntities.quadTree->intersectAny(this)) {
				intersected = true;
			}

			RealityEntity::updateCollision(deltaTick);

			if(intersected) {
				if(enableCCD())++totalHit;
			}
		}

		void update(float deltaTick) override {
			RealityEntity::update(deltaTick);

			if(const auto t = position.copy().abs(); t.x > 50000 || t.y > 50000) {
				deactivate();
			}

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
			Draw::setLineStroke(2);
			Draw::quad(Draw::defaultTexture, hitBox.v0, hitBox.v1, hitBox.v2, hitBox.v3);

			Draw::color(Colors::GRAY);
			Draw::alpha(0.3f);
			if(enableCCD()) {
				for(const auto& [v0, v1, v2, v3, maxOrthoBound] : trace.contiounsTraces) {
					Draw::quad(Draw::defaultTexture, v0, v1, v2, v3);
					Draw::rectLine(maxOrthoBound);
				}
			}
			Draw::alpha();
			Draw::color(Colors::RED);

			if(intersected)for(const auto& vec2 : intersectedPointWith | std::ranges::views::values) {
				Draw::rect(vec2.x - 2, vec2.y - 2, 4, 4);
			}

			Draw::setLineStroke(1.0f);
			Draw::color(Colors::MAGENTA);
			Draw::lineAngle(position.x, position.y, rotation, hitBox.sizeVec2.length());
			
			Draw::setLineStroke(2.0f);
			const Color colors[]{Colors::ROYAL, Colors::PINK, Colors::GREEN, Colors::PURPLE};
			for(int i = 0; i < 4; ++i) {
				Draw::color(colors[i]);
				Draw::rect(hitBox[i].x - 2, hitBox[i].y - 2, 4, 4);

				const Vec2 begin = hitBox[i];
				const Vec2 end = hitBox[(i + 1) % 4];
				const Vec2 center = (begin + end) / 2;

				Draw::line(center, center + hitBox.getNormalVec(i).normalize().scl(25));
			}
			//
			// auto normalNearest = Geom::avgEdgeNormal(Core::camera->getPosition(), hitBox);
			// Draw::color(Colors::YELLOW);
			// Draw::line(hitBox.originPoint, hitBox.originPoint + normalNearest.normalize().scl(100));

			for(auto value : intersectedPointWith | std::ranges::views::values) {
				intersectionCorrection(value);
				auto nor = Geom::avgEdgeNormal(value, hitBox);

				Draw::color(Colors::ORANGE);
				Draw::setLineStroke(2.0f);
				Draw::line(value, value + nor.setLength(150 + hitBox.sizeVec2.length2() / 30));
				Draw::setLineStroke(0.80f);
				Draw::line(value, hitBox.originPoint);
			}

			Draw::color(Colors::BLACK);
			Draw::rect(hitBox.originPoint.x - 2, hitBox.originPoint.y - 2, 4, 4);
		}

		[[nodiscard]] const Geom::Shape::OrthoRectFloat& getDrawBound() const override {
			return hitBox.maxOrthoBound;
		}
	};
}
