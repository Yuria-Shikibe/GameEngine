module;

export module Game.Entity.SpaceCraft;

export import Game.Entity.RealityEntity;

import Graphic.Draw;
import Graphic.Draw.Lines;
import Math;
import OS;

export namespace Game {
	class SpaceCraft : public RealityEntity {
	public:
		[[nodiscard]] SpaceCraft() {
			velocity = Geom::X2;
			velocity /= 5;
			angularAcceleration = 0.0f;
		}

		void update(float deltaTick) override {
			acceleration.set(0.2f, 0).rotate(OS::globalTick());
			angularAcceleration = Math::sinDeg(OS::globalTime() * 2) * 0.05f;
			angularVelocity += angularAcceleration * deltaTick;
			velocity.add(acceleration, deltaTick);
			hitBox.originPoint.add(velocity, deltaTick);
			hitBox.rotation += angularVelocity * deltaTick;
			hitBox.update();
		}

		void activate() override {

		}

		void deactivate() override {

		}

		void draw() const override {
			drawDebug();
		}

		void drawDebug() const override {
			Graphic::Draw::setLineStroke(2);
			Graphic::Draw::quad(Graphic::Draw::defaultTexture, hitBox.v0, hitBox.v1, hitBox.v2, hitBox.v3);
			Graphic::Draw::rectLine(hitBox.maxOrthoBound);
		}

		[[nodiscard]] const Geom::Shape::OrthoRectFloat& getDrawBound() const override {
			return hitBox.maxOrthoBound;
		}
	};
}
