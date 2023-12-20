module;

export module UI.ScrollPane;

import UI.Elem;

import Geom.Vector2D;
import Geom.Matrix3D;
import GL;
import Graphic.Draw;
import <iostream>;

export namespace UI {
	struct ScrollerDrawer {
		void operator()(float x, float y, float w, float h) const {

		}
	};

	class ScrollPane : public Elem {
	protected:
		Geom::Vector2D scrollOffset{};
		Geom::Vector2D scrollVelocity{};

		bool enableHorizonScroll = false;
		float hoirScrollerWidth{8.0f};
		float hoirScrollerHeight{8.0f};

		bool enableVerticalScroll = false;
		float vertScrollerWidth{8.0f};
		float vertScrollerHeight{8.0f};

		bool fadeWhenUnused = true;
		bool hoverScroller = false;

		float sensitivityCoefficient = 0.22f;

		float drag = 1.0f;

		ScrollerDrawer* drawer{nullptr};

		mutable Geom::Matrix3D porj{};

	public:
		ScrollPane() {
			inputListener.on<UI::MouseActionDrug>([this](const auto& event) {
				scrollVelocity = static_cast<Geom::Vector2D>(event);
				scrollVelocity.scl(sensitivityCoefficient);
			});

			quitInboundFocus = false;
		}

		void update(const float delta) override {
			scrollOffset.add(scrollVelocity);
			scrollVelocity.lerp(Geom::ZERO, drag);

			updateChildren(delta);
		}

		void draw() const override;
	};



}

