//
// Created by Matrix on 2023/11/19.
//
module ;

export module Core.Camera;

import <algorithm>;

import Geom.Vector2D;
import Geom.Matrix3D;

using namespace Geom;

export namespace Core{
	class Camera final : public virtual Position {
	public:
		Matrix3D worldToScreen{};
		Matrix3D screenToWorld{};
		Vector2D position{0, 0};
		float scale = 1.0f;

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

		void setScale(const float f) {
			scale = std::max(f, 0.2f);
		}

		void trans(const float x, const float y) {
			position.add(x, y);
		}

		[[nodiscard]] float getScale() const {
			return scale;
		}

		[[nodiscard]] Vector2D screenCenter() const {
			return Vector2D{ position };
		}

		void setOrtho(float width, float height) {
			width /= scale;
			height /= scale;

			worldToScreen.setOrthogonal(0, 0, width, height).translate(-position.x + width / 2.0f, -position.y + height / 2.0f);
		}

	};
}

