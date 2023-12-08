//
// Created by Matrix on 2023/11/19.
//
module ;

export module Core.Camera;

import <algorithm>;

import OS.ApplicationListener;
import Geom.Vector2D;
import Geom.Matrix3D;
import <valarray>;
import <numbers>;

using namespace Geom;

export namespace Core{
	class Camera final : public Position, public OS::ApplicationListener{
	protected:
		Matrix3D worldToScreen{};
		Matrix3D screenToWorld{};
		Vector2D position{0, 0};

		float scale{1.0f};
		float targetScale{1.0f};

	public:
		Camera() = default;

		[[nodiscard]] float getX() const override {
			return position.x;
		}

		[[nodiscard]] float getY() const override {
			return position.y;
		}

		void setX(const float x) override {
			position.setX(x);
		}

		void setY(const float y) override {
			position.setY(y);
		}

		~Camera() override = default;

		void trans(const float x, const float y) {
			position.add(x, y);
		}

		[[nodiscard]] Vector2D screenCenter() const {
			return Vector2D{ position };
		}

		void setOrtho(float width, float height) {
			width /= scale;
			height /= scale;

			worldToScreen.setOrthogonal(0, 0, width, height).translate(-position.x + width / 2.0f, -position.y + height / 2.0f);
			updated(width, height);
		}

		void updated(const float width, const float height) {
			// Geom::Vector2D trans{};
			// worldToScreen.getTranslation(trans);
			// screenToWorld.setOrthogonal(0, 0, width, height).inv().translate(position.x - width / 2.0f, position.y - height / 2.0f);

			screenToWorld.set(worldToScreen).inv();
		}

		void update(const float delta) override {
			scale = std::exp(std::lerp(
				std::log(scale), std::log(targetScale), delta * 0.1f
			));
		}

		[[nodiscard]] Matrix3D& getWorldToScreen() {
			return worldToScreen;
		}

		void setWorldToScreen(const Matrix3D& worldToScreen) {
			this->worldToScreen = worldToScreen;
		}

		[[nodiscard]] Matrix3D& getScreenToWorld() {
			return screenToWorld;
		}

		void setScreenToWorld(const Matrix3D& screenToWorld) {
			this->screenToWorld = screenToWorld;
		}

		[[nodiscard]] Vector2D& getPosition() {
			return position;
		}

		void setPosition(const Vector2D& position) {
			this->position = position;
		}

		[[nodiscard]] float getScale() const {
			return scale;
		}

		void setScale(const float scale) {
			this->scale = scale;
		}

		[[nodiscard]] float getTargetScale() const {
			return targetScale;
		}

		void setTargetScale(const float targetScale) {
			this->targetScale = std::clamp(targetScale, 0.2f, 10.0f);
		}
	};
}

