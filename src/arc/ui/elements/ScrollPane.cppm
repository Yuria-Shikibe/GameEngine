module;

export module UI.ScrollPane;

import UI.Elem;
import UI.Group;

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
		virtual ~ScrollerDrawer() = default;

		virtual void operator()(const ScrollPane* pane) const;
	};

	std::unique_ptr defScrollBarDrawer{std::make_unique<ScrollerDrawer>()};

	class ScrollPane : public Group {
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

		ScrollerDrawer* scrollBarDrawer{defScrollBarDrawer.get()};

		Elem* item{nullptr};

		//TODO uses layout cell
		Rect itemSize;
	public:
		ScrollPane(){
			inputListener.on<UI::MouseActionDrag>([this](const auto& event) {
				scrollTargetVelocity = static_cast<Geom::Vector2D>(event);
				scrollTargetVelocity.scl(dragSensitivityCoefficient, -dragSensitivityCoefficient);
				pressed = true;
			});

			inputListener.on<UI::MouseActionRelease>([this](const auto& event) {

				pressed = false;
			});

			inputListener.on<UI::MouseActionScroll>([this](const auto& event) {
				scrollTargetVelocity = static_cast<Geom::Vector2D>(event);
				scrollTargetVelocity.scl(scrollSensitivityCoefficient, -scrollSensitivityCoefficient);
			});

			touchbility = UI::TouchbilityFlags::enabled;
			quitInboundFocus = false;
		}

		void update(const float delta) override {
			scrollVelocity.lerp(scrollTargetVelocity, usingAccel ? (pressed ? 1.0f : std::clamp(accel * delta, 0.0f, 1.0f)) : 1.0f);
			scrollOffset.add(scrollVelocity);

			// scrollTargetVelocity.scl(scrollMarginCoefficient).abs();

			scrollOffset.clampX(-std::fmaxf(0, itemSize.getWidth() - getWidth() + marginWidth() + vertBarStroke()), 0);
			scrollOffset.clampY(0, std::fmaxf(0, itemSize.getHeight() - getHeight() + marginHeight() + horiBarStroke()));

			scrollTargetVelocity.setZero();

			// pressed = false;

			Group::update(delta);

			if(layoutChanged) {
				layout();
			}

			if(hasChildren()) {

				const Geom::Vector2D absOri = absoluteSrc;
				absoluteSrc += scrollOffset;
				absoluteSrc.x += margin_bottomLeft.x;
				absoluteSrc.y -= margin_bottomLeft.y;

				if(vertOutbound()) {
					absoluteSrc.y += getHeight() - itemSize.getHeight();
				}

				calAbsoluteChildren();

				absoluteSrc = absOri;
			}
		}

		void layout() override {
			Group::layout();

			if(item) {
				itemSize = item->getBound();
			}
		}

		void setItem(Group* item) {
			this->item = item;
			if(item != nullptr) {
				itemSize = item->getBound();
			}

			getChildren()->clear();
			addChildren(item);
		}

		//TODO this has bug when resized !
		//If layouted multible times, this will shrink itemsize!
		Rect getFilledChildrenBound(Elem* elem) const override {
			Rect rect = Elem::getFilledChildrenBound(elem);

			if(hoverScroller)return rect;

			const bool enableX = !elem->isFillParentX() && bound.getWidth() > getWidth() - marginWidth() && bound.getHeight() > getHeight() - marginHeight() - hoirScrollerHeight;
			const bool enableY = !elem->isFillParentY() && bound.getHeight() > getHeight() - marginHeight() && bound.getWidth() > getWidth() - marginWidth() - vertScrollerWidth;

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

		void drawContent() const override;
	};



}

