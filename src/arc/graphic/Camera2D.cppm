//
// Created by Matrix on 2023/11/19.
//
//TODO camera and viewport system are total mess, arrange it if possilbe
module ;

export module Core.Camera;



import Concepts;
import Graphic.Viewport;
import Graphic.Resizeable;
import Graphic.Viewport.Viewport_OrthoRect;
import Geom.Shape.Rect_Orthogonal;
import OS.ApplicationListener;
import Geom.Vector2D;
import Geom.Matrix3D;
import <memory>;
import <valarray>;
import <numbers>;
import <algorithm>;

using namespace Geom;

export namespace Core{
	class Camera2D final : public OS::ApplicationListener, public Graphic::Resizeable<unsigned int> {
	protected:
		Matrix3D worldToScreen{};
		Matrix3D screenToWorld{};

		Shape::OrthoRectUInt screenSize{};

		//TODO this viewport design is so bad!
		std::unique_ptr<Graphic::Viewport<>> viewport{std::make_unique<Graphic::Viewport_OrthoRect>()};

		float scale{1.0f};
		float targetScale{1.0f};

		float minScale = std::log(0.2f);
		float maxScale = std::log(3.0f);

	public:
		Camera2D() = default;

		~Camera2D() override = default;

		void resize(const unsigned w, const unsigned h) override { // NOLINT(*-make-member-function-const)
			screenSize.setSize(w, h);
		}

		[[nodiscard]] explicit Camera2D(Graphic::Viewport<>* viewport)
			: viewport(viewport) {
		}

		void setViewport(Graphic::Viewport<>* viewport){
			this->viewport.reset(viewport);
		}

		[[nodiscard]] Graphic::Viewport<>& getViewport() const {
			return *viewport;
		}

		[[nodiscard]] Shape::OrthoRectFloat& viewportRect() const {
			return viewport->getPorjectedBound();
		}

		void trans(const float x, const float y) const {
			viewport->getPosition().add(x, y);
		}

		[[nodiscard]] Geom::Vec2 screenCenter() const {
			return viewport->getCenter();
		}

		void setOrtho(float width, float height) const {
			width /= scale;
			height /= scale;

			viewport->setOrtho(width, height);
		}

		void update(const float delta) override {
			setOrtho(static_cast<float>(screenSize.getWidth()), static_cast<float>(screenSize.getHeight()));

			scale = std::exp(std::lerp(
				std::log(scale), std::log(targetScale), delta * 0.1f
			));

			if(std::abs(scale - targetScale) < 0.00025f) {
				scale = targetScale;
			}

			viewport->modify(worldToScreen);

			screenToWorld.set(worldToScreen).inv();
		}

		[[nodiscard]] Matrix3D& getWorldToScreen() {
			return worldToScreen;
		}

		[[nodiscard]] Vec2 getWorldToScreen(const Vec2 vec2) const {
			return worldToScreen * vec2;
		}

		[[nodiscard]] Vec2 getScreenToWorld(const Vec2 vec2) const {
			return screenToWorld * vec2;
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

		[[nodiscard]] Vec2& getPosition() const {
			return viewport->getCenter();
		}

		void setPosition(const Vec2& position) const {
			viewport->getPosition() = position;
		}

		[[nodiscard]] float getScale() const {
			return scale;
		}

		void setScale(const float scale) {
			this->scale = scale;
		}

		[[nodiscard]] float getTargetScale() const {
			return std::log(targetScale);
		}

		void setTargetScale(const float targetScale) {
			this->targetScale = std::exp(std::clamp(targetScale, minScale, maxScale));
		}

		void setTargetScaleDef() {
			this->targetScale = 1.0f;
		}
	};
}

