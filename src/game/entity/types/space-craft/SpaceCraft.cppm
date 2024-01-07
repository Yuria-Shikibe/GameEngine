module;

export module Game.Entity.SpaceCraft;

export import Game.Entity.RealityEntity;

import Game.Entity.EntityManager;
import Graphic.Color;
import Graphic.Draw;
import Graphic.Draw.Lines;
import Math;
import OS;
import <iostream>;

export namespace Game {
	std::atomic_int totalHit = 0;

	class SpaceCraft : public RealityEntity{
	public:
		bool intersected = false;
		[[nodiscard]] SpaceCraft() {
			angularAcceleration = 0.0f;
		}

		void updateCollision(const float deltaTick) override {
			intersected = false;

			intersectedPointWith.clear();
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
			Graphic::Draw::alpha();
			if(intersected)Graphic::Draw::color(Graphic::Colors::AQUA);
			else Graphic::Draw::color(Graphic::Colors::LIGHT_GRAY);
			Graphic::Draw::setLineStroke(2);
			Graphic::Draw::quad(Graphic::Draw::defaultTexture, hitBox.v0, hitBox.v1, hitBox.v2, hitBox.v3);

			Graphic::Draw::color(Graphic::Colors::GRAY);
			Graphic::Draw::alpha(0.3f);
			if(enableCCD()) {
				for(const auto& [v0, v1, v2, v3, maxOrthoBound] : contiounsTraces) {
					Graphic::Draw::quad(Graphic::Draw::defaultTexture, v0, v1, v2, v3);
					Graphic::Draw::rectLine(maxOrthoBound);
				}
			}
			Graphic::Draw::alpha();
			Graphic::Draw::color(Graphic::Colors::RED);
			if(intersected)for(const auto& [tgt, vec2] : intersectedPointWith) {
				Graphic::Draw::rect(vec2.x - 2, vec2.y - 2, 4, 4);
			}

			Graphic::Draw::color(Graphic::Colors::BLACK);
			Graphic::Draw::rect(hitBox.originPoint.x - 2, hitBox.originPoint.y - 2, 4, 4);
		}

		[[nodiscard]] const Geom::Shape::OrthoRectFloat& getDrawBound() const override {
			return hitBox.maxOrthoBound;
		}
	};
}
