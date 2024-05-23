//
// Created by Matrix on 2023/11/19.
//
//TODO camera and viewport system are total mess, arrange it if possilbe
module ;

export module Core.Camera;

import ext.Concepts;
import Graphic.Viewport;
import Graphic.Resizeable;
import Graphic.Viewport.Viewport_OrthoRect;
import Geom.Rect_Orthogonal;
import OS.ApplicationListener;
import Geom.Vector2D;
import Geom.Matrix3D;
import std;
import Math.Rand;
import Math;

export namespace Core{
	//TODO poor design
	//TODO 3D support maybe in the future?
	class Camera2D final : public OS::ApplicationListener, public Graphic::ResizeableInt {
	public:
		static constexpr float DefMaximumScale = 5.0f;
		static constexpr float DefMinimumScale = 0.2f;
		static constexpr float DefScaleSpeed = 0.095f;
		static constexpr float ShakeActivateThreshold = 0.005f;
		static constexpr float ShakeMinSpacing = 1 / 30.f;
	protected:
		Geom::Matrix3D worldToScreen{};
		Geom::Matrix3D screenToWorld{};

		//TODO is a rect necessary??
		Geom::Point2U screenSize{};

		Geom::Vec2 stablePos{};
		Geom::OrthoRectFloat viewport{};

		float scale{1.0f};
		float targetScale{1.0f};

		float minScale = std::log(DefMinimumScale);
		float maxScale = std::log(DefMaximumScale);

		float shakeIntensity{0.0f};
		float shakeCorrectionSpeed{0.05f};

		float shakeReload{};

		Math::Rand rand{};

	public:
		Camera2D() = default;

		void shake(const float intensity, const float fadeSpeed) noexcept{
			shakeIntensity = Math::max(intensity, shakeIntensity);
			shakeCorrectionSpeed = Math::max(0.1f, (shakeCorrectionSpeed + Math::ceilPositive(fadeSpeed)) * 0.5f);
		}

		void setScaleClamp(const float min, const float max) noexcept{
			minScale = std::log(max);
			maxScale = std::log(min);
		}

		void resize(const int w, const int h) noexcept override { // NOLINT(*-make-member-function-const)
			screenSize.set(w, h);
		}

		[[nodiscard]] const Geom::OrthoRectFloat& getViewport() const noexcept {
			return viewport;
		}

		void move(const float x, const float y) noexcept {
			stablePos.add(x, y);
		}

		void move(const Geom::Vec2 vec2) noexcept {
			stablePos.add(vec2);
		}

		[[nodiscard]] Geom::Vec2 getPosition() const noexcept {
			return stablePos;
		}

		[[nodiscard]] Geom::Vec2 getViewportCenter() const noexcept {
			return viewport.getCenter();
		}

		void setPosition(const Geom::Vec2& position) noexcept {
			stablePos.set(position);
		}

		[[nodiscard]] Geom::Vec2 screenCenter() const noexcept{
			return screenSize.as<float>() / 2.f;
		}

		void setOrtho(float width, float height) noexcept{
			width /= scale;
			height /= scale;

			viewport.setSize(width, height);
		}

		void update(const float delta) override {
			setOrtho(static_cast<float>(screenSize.x), static_cast<float>(screenSize.y));

			scale = std::exp(std::lerp(
				std::log(scale), std::log(targetScale), delta * DefScaleSpeed
			));

			viewport.setCenter(stablePos);

			if(!Math::zero(shakeIntensity, ShakeActivateThreshold)){
				float shakeIntensityScl = .25f;
				if(shakeReload >= ShakeMinSpacing){
					shakeReload = 0;
					shakeIntensityScl = 1.f;
				}else{
					shakeReload += delta;
				}

				auto randVec = Geom::Vec2{}.setPolar(rand.random(360.0f), rand.random(shakeIntensity));
				const auto dstScl = Math::curve(randVec.dst(viewport.getCenter() - getPosition()) / shakeIntensity, 0.85f, 1.65f) * .35f;

				randVec.scl((1 - dstScl) * shakeIntensityScl);

				viewport.move(randVec);

				shakeIntensity = Math::approach(shakeIntensity, 0, shakeCorrectionSpeed * delta);
			}

			if(Math::zero(scale - targetScale, 0.00025f)) {
				scale = targetScale;
			}

			worldToScreen.setOrthogonal(viewport.getSrcX(), viewport.getSrcY(), viewport.getWidth(), viewport.getHeight());

			screenToWorld.set(worldToScreen).inv();
		}

		[[nodiscard]] Geom::Matrix3D& getWorldToScreen() noexcept {
			return worldToScreen;
		}

		[[nodiscard]] const Geom::Matrix3D& getWorldToScreen() const noexcept {
			return worldToScreen;
		}

		[[nodiscard]] Geom::Vec2 getWorldToScreen(const Geom::Vec2 vec2) const noexcept{
			return worldToScreen * vec2;
		}

		[[nodiscard]] Geom::Vec2 getScreenToWorld(const Geom::Vec2 vec2) const noexcept{
			return screenToWorld * vec2;
		}

		void setWorldToScreen(const Geom::Matrix3D& worldToScreen) noexcept{
			this->worldToScreen = worldToScreen;
		}

		[[nodiscard]] Geom::Matrix3D& getScreenToWorld() noexcept {
			return screenToWorld;
		}

		[[nodiscard]] const Geom::Matrix3D& getScreenToWorld() const  noexcept {
			return screenToWorld;
		}

		void setScreenToWorld(const Geom::Matrix3D& screenToWorld) noexcept{
			this->screenToWorld = screenToWorld;
		}


		[[nodiscard]] float getScale() const noexcept{
			return scale;
		}

		void setScale(const float scale) noexcept{
			this->scale = scale;
		}

		[[nodiscard]] float getTargetScale() const noexcept{
			return std::log(targetScale);
		}

		void setTargetScale(const float targetScale) noexcept{
			this->targetScale = std::exp(std::clamp(targetScale, minScale, maxScale));
		}

		void setTargetScaleDef() noexcept{
			this->targetScale = std::exp(Math::clamp(0.0f, minScale, maxScale));
		}

		Geom::Vec2 getSize() const noexcept{
			return screenSize.as<float>();
		}
	};
}

