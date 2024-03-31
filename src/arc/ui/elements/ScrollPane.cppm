module;

export module UI.ScrollPane;

import UI.Elem;
import UI.Group;

import Geom.Vector2D;
import Geom.Matrix3D;
import GL;
import std;
import Concepts;

export namespace UI {
	class ScrollPane;

	struct ScrollerDrawer {
		virtual ~ScrollerDrawer() = default;

		virtual void operator()(const ScrollPane* pane) const;
	};

	//TODO move this other place
	std::unique_ptr defScrollBarDrawer{std::make_unique<ScrollerDrawer>()};

	class ScrollPane : public Group {
	protected:
		Geom::Vec2 scrollOffset{};
		Geom::Vec2 scrollTempOffset{};

		Geom::Vec2 scrollVelocity{};
		Geom::Vec2 scrollTargetVelocity{};

		bool usingAccel = true;

		bool enableHorizonScroll_always = false;
		float hoirScrollerHeight{20.0f};

		bool enableVerticalScroll_always = false;
		float vertScrollerWidth{20.0f};

		//TODO fade
		bool fadeWhenUnused = true;
		bool hoverScroller = false;

		float scrollMarginCoefficient = 6.0f;

		float accel = 0.126f;

		ScrollerDrawer* scrollBarDrawer{defScrollBarDrawer.get()};

		Elem* item{nullptr};

		void clamp(Geom::Vec2& offset) const{
			offset.clampX(-Math::max(0.0f, itemSize.getWidth() - getValidWidth() + vertBarStroke()), 0);
			offset.clampY(0, Math::max(0.0f, itemSize.getHeight() - getValidHeight() + horiBarStroke()));
		}

		void move(const Geom::Vec2 offset){
			scrollTempOffset = scrollOffset + offset;

			clamp(scrollTempOffset);
		}

		void applyTemp(){
			scrollOffset = scrollTempOffset;
		}

		void resumeTemp(){
			scrollTempOffset = scrollOffset;
		}

		//TODO uses layout cell
		Rect itemSize{};
	public:
		Geom::Vec2 scrollSensitivity{90.0f, 60.0f};
		ScrollPane(){
			inputListener.on<UI::MouseActionDrag>([this](const UI::MouseActionDrag& event) {
				move(event.relativeMove * Geom::Vec2{itemSize.getWidth() / getValidWidth(), -(itemSize.getHeight() / getValidHeight())});
				pressed = true;
			});

			inputListener.on<UI::MouseActionRelease>([this](const auto& event) {
				pressed = false;
				applyTemp();
			});

			inputListener.on<UI::MouseActionScroll>([this](const auto& event) {
				scrollTargetVelocity = event;
				scrollTargetVelocity.scl(scrollSensitivity.x, -scrollSensitivity.y);
			});

			touchbility = UI::TouchbilityFlags::enabled;
			quitInboundFocus = false;
		}

		void update(const float delta) override {
			scrollVelocity.lerp(scrollTargetVelocity, usingAccel ? (pressed ? 1.0f : Math::clamp(accel * delta)) : 1.0f);

			if(scrollTempOffset != scrollOffset){

			}else{
				scrollOffset.add(scrollVelocity);
				clamp(scrollOffset);
				resumeTemp();
			}

			scrollTargetVelocity.scl(scrollMarginCoefficient).toAbs();

			scrollTargetVelocity.setZero();

			Group::update(delta);

			if(layoutChanged) {
				layout();
			}

			calAbsoluteSrc(parent);

			if(hasChildren()) {
				const Geom::Vec2 absOri = absoluteSrc;
				absoluteSrc += scrollTempOffset;
				absoluteSrc.x += border.left;
				absoluteSrc.y -= border.bottom;

				if(vertOutbound()) {
					absoluteSrc.y += getHeight() - itemSize.getHeight();
				}

				calAbsoluteChildren();

				absoluteSrc = absOri;
			}
		}

		void calAbsoluteSrc(Elem* parent) override{
			Geom::Vec2 vec{parent->getAbsSrc()};
			vec.add(bound.getSrcX(), bound.getSrcY());
			absoluteSrc.set(vec);
		}

		void layout() override {
			Group::layout();

			if(item) {
				itemSize = item->getBoundRef();
			}
		}

		template <Concepts::Derived<Elem> T>
		void setItem(Concepts::Invokable<void(T&)> auto&& func, const int depth = std::numeric_limits<int>::max()) {
			auto ptr = std::make_unique<T>();

			if constexpr (!std::same_as<decltype(func), std::nullptr_t>) {
				func(*ptr);
			}

			getChildren()->clear();
			this->item = ptr.get();
			if(item != nullptr) {
				this->addChildren(std::move(ptr), depth);
				itemSize = item->getBoundRef();
			}
		}

		//TODO this has bug when resized !
		//If layouted multible times, this will shrink itemsize!
		Rect getFilledChildrenBound(Elem* elem) const override {
			Rect rect = Elem::getFilledChildrenBound(elem);

			if(hoverScroller)return rect;

			const bool enableX = !elem->isFillParentX() && bound.getWidth() > getWidth() - getBorderWidth() && bound.getHeight() > getHeight() - getBorderHeight() - hoirScrollerHeight;
			const bool enableY = !elem->isFillParentY() && bound.getHeight() > getHeight() - getBorderHeight() && bound.getWidth() > getWidth() - getBorderWidth() - vertScrollerWidth;

			rect.addSize(
				enableY ? -vertScrollerWidth : 0.0f,
				enableX ? -hoirScrollerHeight : 0.0f
			);

			rect.move(
				elem->isFillParentX() && enableY ? -border.left : 0.0f,
				elem->isFillParentY() && enableX ? -border.bottom : 0.0f
			);

			return rect;
		}

		[[nodiscard]] bool hintInbound_validToParent(const Geom::Vec2 screenPos) override {
			return Elem::isInbound(screenPos) && !inbound_scrollBars(screenPos);
		}

		[[nodiscard]] bool inbound_scrollBars(const Geom::Vec2& screenPos) const {
			return
			(enableHorizonScroll() && screenPos.y - absoluteSrc.y + border.bottom < hoirScrollerHeight) ||
			(enableVerticalScroll() && screenPos.x - absoluteSrc.x + border.left > getWidth() - vertScrollerWidth);
		}

		[[nodiscard]] bool isInbound(const Geom::Vec2 screenPos) override {
			if(Elem::isInbound(screenPos)) {
				Elem::setFocusedScroll(true);
				return inbound_scrollBars(screenPos);
			}

			Elem::setFocusedScroll(false);
			return false;
		}

		[[nodiscard]] bool horiOutbound() const {
			return itemSize.getWidth() > getWidth() - getBorderWidth();
		}

		[[nodiscard]] bool vertOutbound() const {
			return itemSize.getHeight() > getHeight() - getBorderHeight();
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
			return Math::min(getWidth() / itemSize.getWidth(), 1.0f) * getWidth();
		}

		[[nodiscard]] float vertBarSLength() const {
			return Math::min(getHeight() / itemSize.getHeight(), 1.0f) * getHeight();
		}

		/** @return Valid Width - Bar Stroke*/
		[[nodiscard]] float getContentWidth() const {
			return getValidWidth() - vertBarStroke();
		}

		/** @return Valid Height - Bar Stroke*/
		[[nodiscard]] float getContentHeight() const {
			return getValidHeight() - horiBarStroke();
		}

		[[nodiscard]] float horiScrollRatio() const {
			return Math::clamp(-scrollTempOffset.x / (itemSize.getWidth() - getContentWidth()));
		}

		[[nodiscard]] float vertScrollRatio() const {
			return Math::clamp(1.0f - scrollTempOffset.y / (itemSize.getHeight() - getContentHeight()));
		}

		void drawContent() const override;
	};



}

