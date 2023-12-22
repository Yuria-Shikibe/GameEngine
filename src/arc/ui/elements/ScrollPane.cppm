module;

export module UI.ScrollPane;

import UI.Elem;

import Geom.Vector2D;
import Geom.Matrix3D;
import GL;
import Graphic.Draw;
import <algorithm>;
import <execution>;
import <iostream>;

export namespace UI {
	class ScrollPane;

	struct ScrollerDrawer {
		virtual void operator()(const ScrollPane* pane) const;
	};


	std::unique_ptr defDrawer{std::make_unique<ScrollerDrawer>()};

	class ScrollPane : public Elem {
	protected:
		Geom::Vector2D scrollOffset{};

		Geom::Vector2D scrollVelocity{};
		Geom::Vector2D scrollTargetVelocity{};

		bool usingAccel = true;

		bool enableHorizonScroll_always = false;
		float hoirScrollerHeight{20.0f};

		bool enableVerticalScroll_always = false;
		float vertScrollerWidth{20.0f};

		bool fadeWhenUnused = true;
		bool hoverScroller = false;

		float dragSensitivityCoefficient = 0.795f;
		float scrollSensitivityCoefficient = 60.0f;
		float scrollMarginCoefficient = 6.0f;

		float accel = 0.126f;

		ScrollerDrawer* scrollBarDrawer{defDrawer.get()};

		Rect itemSize;
	public:
		ScrollPane(){
			inputListener.on<UI::MouseActionDrag>([this](const auto& event) {
				scrollTargetVelocity = static_cast<Geom::Vector2D>(event);
				scrollTargetVelocity.scl(dragSensitivityCoefficient, -dragSensitivityCoefficient);
				pressed = true;
			});

			inputListener.on<UI::MouseActionScroll>([this](const auto& event) {
				scrollTargetVelocity = static_cast<Geom::Vector2D>(event);
				scrollTargetVelocity.scl(scrollSensitivityCoefficient, -scrollSensitivityCoefficient);
			});

			touchbility = UI::TouchbilityFlags::enabled;
			quitInboundFocus = false;

			margin_bottomLeft.set(12, 12);
			margin_topRight.set(12, 12);

			color.set(0.3f, 0.66f, 0.96f, 0.6f);
		}

		void update(const float delta) override {
			scrollVelocity.lerp(scrollTargetVelocity, usingAccel ? (pressed ? 1.0f : std::clamp(accel * delta, 0.0f, 1.0f)) : 1.0f);
			scrollOffset.add(scrollVelocity);

			// scrollTargetVelocity.scl(scrollMarginCoefficient).abs();

			scrollOffset.clampX(-std::fmaxf(0, itemSize.getWidth() - getWidth() + marginWidth() + vertBarStroke()), 0);
			scrollOffset.clampY(0, std::fmaxf(0, itemSize.getHeight() - getHeight() + marginHeight() + horiBarStroke()));

			scrollTargetVelocity.setZero();

			pressed = false;

			updateChildren(delta);

			if(layoutChanged) {
				layout();
			}

			calAbsoluteChildren();
		}

		void layout() override {
			layoutChildren();

			Elem::layout();

			if(!children.empty()) {
				itemSize = children.front()->getBound();
			}
		}

		void calAbsoluteChildren() override {
			const Geom::Vector2D absOri = absoluteSrc;
			absoluteSrc += scrollOffset;
			absoluteSrc.x += margin_bottomLeft.x;
			absoluteSrc.y -= margin_bottomLeft.y;

			if(vertOutbound()) {
				absoluteSrc.y += getHeight() - itemSize.getHeight();
			}

			Elem::calAbsoluteChildren();

			absoluteSrc = absOri;
		}

		Rect getFilledChildrenBound(Elem* elem) const override {
			Rect rect = Elem::getFilledChildrenBound(elem);

			if(hoverScroller)return rect;

			const bool enableX = bound.getWidth() > getWidth() - marginWidth() && bound.getHeight() > getHeight() - marginHeight() - hoirScrollerHeight;
			const bool enableY = bound.getHeight() > getHeight() - marginHeight() && bound.getWidth() > getWidth() - marginWidth() - vertScrollerWidth;

			rect.addSize(
				enableY ? -vertScrollerWidth : 0.0f,
				enableX ? -hoirScrollerHeight : 0.0f
			);

			rect.move(
				elem->isFillParentX() && enableY ? -margin_bottomLeft.x : 0.0f,
				elem->isFillParentY() && enableX ? -margin_bottomLeft.y : 0.0f
			);

			return rect;
		}

		[[nodiscard]] bool inbound_validToParent(const Geom::Vector2D& screenPos) const override {
			return Elem::inbound(screenPos) && !inbound_scrollBars(screenPos);
		}

		[[nodiscard]] bool inbound_scrollBars(const Geom::Vector2D& screenPos) const {
			return
			(enableHorizonScroll() && screenPos.y - absoluteSrc.y + margin_bottomLeft.y < hoirScrollerHeight) ||
			(enableVerticalScroll() && screenPos.x - absoluteSrc.x + margin_bottomLeft.x > getWidth() - vertScrollerWidth);
		}

		void modifyAddedChildren(Elem* elem) override {
			Elem::modifyAddedChildren(elem);
		}

		[[nodiscard]] bool inbound(const Geom::Vector2D& screenPos) const override {
			if(Elem::inbound(screenPos)) {
				Elem::setFocusedScroll(true);
				return inbound_scrollBars(screenPos);
			}

			Elem::setFocusedScroll(false);
			return false;
		}

		[[nodiscard]] bool horiOutbound() const {
			return itemSize.getWidth() > getWidth() - marginWidth();
		}

		[[nodiscard]] bool vertOutbound() const {
			return itemSize.getHeight() > getHeight() - marginHeight();
		}

		[[nodiscard]] bool enableVerticalScroll() const {
			return enableVerticalScroll_always || vertOutbound();
		}

		[[nodiscard]] bool enableHorizonScroll() const {
			return enableHorizonScroll_always || horiOutbound();
		}

		[[nodiscard]] float horiBarStroke() const {
			return  enableHorizonScroll() ? hoirScrollerHeight : 0;
		}

		[[nodiscard]] float vertBarStroke() const {
			return enableVerticalScroll() ? vertScrollerWidth : 0;
		}

		[[nodiscard]] float horiBarLength() const {
			return std::fminf(getWidth() / itemSize.getWidth(), 1.0f) * getWidth();
		}

		[[nodiscard]] float vertBarSLength() const {
			return std::fminf(getHeight() / itemSize.getHeight(), 1.0f) * getHeight();
		}

		[[nodiscard]] float horiScrollRatio() const {
			return std::clamp(-scrollOffset.x / (itemSize.getWidth() - getWidth() + marginWidth() + vertBarStroke()), 0.0f, 1.0f);
		}

		[[nodiscard]] float vertScrollRatio() const {
			return std::clamp(1.0f - scrollOffset.y / (itemSize.getHeight() - getHeight() + marginHeight() + horiBarStroke()), 0.0f, 1.0f);
		}

		void draw() const override;
	};



}

