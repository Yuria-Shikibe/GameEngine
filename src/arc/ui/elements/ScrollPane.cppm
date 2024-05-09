module;

export module UI.ScrollPane;

import UI.Widget;
import UI.Group;

import Geom.Vector2D;
import Geom.Matrix3D;
import GL;
import std;
import Concepts;

export namespace UI {
	struct ScrollBarDrawer;

	class ScrollPane : public Group {

	protected:
		Geom::Vec2 scrollOffset{};
		Geom::Vec2 scrollTempOffset{};

		Geom::Vec2 scrollVelocity{};
		Geom::Vec2 scrollTargetVelocity{};

		bool usingAccel = true;

		bool enableHorizonScroll_always = false;
		float hoirScrollerHeight{24.0f};

		bool enableVerticalScroll_always = false;
		float vertScrollerWidth{24.0f};

		//TODO fade
		bool fadeWhenUnused = true;
		bool hoverScroller = false;

		float scrollMarginCoefficient = 6.0f;

		float accel = 0.126f;

		ScrollBarDrawer* scrollBarDrawer{nullptr};

		Widget* getItem() const{
			return children.front().get();
		};

		void clamp(Geom::Vec2& offset) const{
			offset.clampX(-Math::max(0.0f, itemSize.getWidth() - getContentWidth()), 0);
			offset.clampY(0, Math::max(0.0f, itemSize.getHeight() - getContentHeight()));
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
		bool susutainRelativePostion{true};
		Geom::Vec2 scrollSensitivity{90.0f, 60.0f};
		ScrollPane(){
			inputListener.on<UI::MouseActionDrag>([this](const UI::MouseActionDrag& event) {
				Geom::Vec2 clamp{Math::num<float>(isInHoriBar(event.begin)), Math::num<float>(isInVertBar(event.begin))};
				move(event.getRelativeMove() * clamp * Geom::Vec2{-itemSize.getWidth() / getValidWidth(), -itemSize.getHeight() / getValidHeight()});
				pressed = true;
			});

			inputListener.on<UI::MouseActionRelease>([this](const auto& event) {
				pressed = false;
				applyTemp();
			});

			inputListener.on<UI::MouseActionScroll>([this](const auto& event) {
				scrollTargetVelocity = event;
				if(ishoriOutOfBound() ^ isvertOutOfBound()){
					scrollTargetVelocity.x = scrollTargetVelocity.y;
				}

				scrollTargetVelocity.scl(-scrollSensitivity.x, -scrollSensitivity.y);
			});

			touchbility = UI::TouchbilityFlags::enabled;
			quitInboundFocus = false;
			ScrollPane::applyDefDrawer();
		}

		void update(const float delta) override;

		bool isInbound(const Geom::Vec2 screenPos) const override{
			if(Widget::isInbound(screenPos) && (enableHorizonScroll() || enableVerticalScroll())) {
				return inbound_scrollBars(screenPos);
			}
			return false;
		}

		void layout() override{
			Widget::layout();

			getItem()->layout_tryFillParent();

			layoutChildren();

			itemSize = getItem()->getBound();


			scrollTempOffset.setZero();
			applyTemp();

			calAbsoluteSrc(parent);

		}

		void calAbsoluteSrc(Widget* parent) override{
			Geom::Vec2 vec{parent->getAbsSrc()};
			vec.add(bound.getSrcX(), bound.getSrcY());
			//TODO scroll offset check
			absoluteSrc.set(vec);

			if(hasChildren()) {
				const Geom::Vec2 absOri = absoluteSrc;

				// absoluteSrc += border.bot_lft();
				absoluteSrc += scrollTempOffset;// * Geom::Vec2{Math::num<float>(horiExbound), Math::num<float>(vertExbound)};

				absoluteSrc.y += getValidHeight() - itemSize.getHeight();

				calAbsoluteChildren();

				absoluteSrc = absOri;
			}
		}

		template <Concepts::Derived<Widget> T, bool fillX = true, bool fillY = false>
		void setItem(Concepts::Invokable<void(T&)> auto&& func) {
			auto ptr = std::make_unique<T>();

			if constexpr (!std::same_as<decltype(func), std::nullptr_t>) {
				func(*ptr);
			}

			ptr->setFillparentX(fillX);
			ptr->setFillparentY(fillY);

			children.clear();
			this->addChildren(std::move(ptr));
		}

		//BUG this has bug when resized !
		//If layouted multible times, this will shrink itemsize!
		Rect getFilledChildrenBound(Widget* elem) const override {
			Rect rect = Widget::getFilledChildrenBound(elem);

			if(hoverScroller)return rect;

			const bool enableX = elem->getWidth() > getValidWidth() && elem->isFillParentY();
			const bool enableY = elem->getHeight() > getValidHeight() && elem->isFillParentX();

			rect.addSize(
				enableY ? -vertScrollerWidth : 0.0f,
				enableX ? -hoirScrollerHeight : 0.0f
			);

			return rect;
		}

		[[nodiscard]] bool hintInbound_validToParent(const Geom::Vec2 screenPos) override {
			return Widget::isInbound(screenPos) && !inbound_scrollBars(screenPos);
		}

		[[nodiscard]] bool inbound_scrollBars(const Geom::Vec2 screenPos) const {
			return isInVertBar(screenPos) || isInHoriBar(screenPos);
		}

		[[nodiscard]] bool isInHoriBar(Geom::Vec2 screenPos) const{
			if(!enableHorizonScroll())return false;
			screenPos.y -= absoluteSrc.y + border.bottom;
			return screenPos.y < hoirScrollerHeight;
		}

		[[nodiscard]] bool isInVertBar(Geom::Vec2 screenPos) const{
			if(!enableVerticalScroll())return false;
			screenPos.x -= absoluteSrc.x + border.left;
			return screenPos.x > getValidWidth() - vertScrollerWidth;
		}

		[[nodiscard]] bool ishoriOutOfBound() const {
			return itemSize.getWidth() > getValidWidth();
		}

		[[nodiscard]] bool isvertOutOfBound() const {
			return itemSize.getHeight() > getValidHeight();
		}

		[[nodiscard]] bool enableVerticalScroll() const {
			return enableVerticalScroll_always || isvertOutOfBound();
		}

		[[nodiscard]] bool enableHorizonScroll() const {
			return enableHorizonScroll_always || ishoriOutOfBound();
		}

		[[nodiscard]] float horiBarStroke() const {
			return  enableHorizonScroll() ? hoirScrollerHeight : 0;
		}

		[[nodiscard]] float vertBarStroke() const {
			return enableVerticalScroll() ? vertScrollerWidth : 0;
		}

		[[nodiscard]] float horiBarLength() const {
			return Math::clampPositive(Math::min(getContentWidth() / itemSize.getWidth(), 1.0f) * getContentWidth());
		}

		[[nodiscard]] float vertBarSLength() const {
			return Math::clampPositive(Math::min(getValidHeight() / itemSize.getHeight(), 1.0f) * getValidHeight());
		}

		/** @return Valid Width - Bar Stroke*/
		[[nodiscard]] float getContentWidth() const {
			return getValidWidth() - vertBarStroke();
		}

		/** @return Valid Height - Bar Stroke*/
		[[nodiscard]] float getContentHeight() const {
			return getValidHeight() - horiBarStroke();
		}

		[[nodiscard]] float horiScrollRatio(const float pos) const {
			return Math::clamp(pos / (itemSize.getWidth() - getContentWidth()));
		}

		[[nodiscard]] float vertScrollRatio(const float pos) const {
			return Math::clamp(pos / (itemSize.getHeight() - getContentHeight()));
		}

		[[nodiscard]] float getHoriScroll(float ratio) const {
			ratio = Math::clamp(ratio);
			return -ratio * (itemSize.getWidth() - getContentWidth());
		}

		[[nodiscard]] float getVertScroll(float ratio) const {
			ratio = Math::clamp(ratio);
			return ratio * (itemSize.getHeight() - getContentHeight());
		}

		[[nodiscard]] float getHoriBarSpacing() const {
			return horiBarStroke() + border.bottom;
		}

		[[nodiscard]] float getVertBarSpacing() const {
			return vertBarStroke() + border.right;
		}

		[[nodiscard]] Rect getHoriBarRect() const {
			return {
				drawSrcX() + border.left + horiScrollRatio(-scrollTempOffset.x) * (getValidWidth() - horiBarLength() - vertBarStroke()),
				drawSrcY() + border.bottom,
				horiBarLength(),
				horiBarStroke()
			};
		}

		[[nodiscard]] Rect getVertBarRect() const {
			return {
				drawSrcX() + getWidth() - border.right - vertBarStroke(),
				drawSrcY() + getHeight() - border.top - vertScrollRatio(scrollTempOffset.y) * (getValidHeight() - vertBarSLength() - horiBarStroke()),
				vertBarStroke(),
				-vertBarSLength()
			};
		}

		CursorType getCursorType() const override;

		void applyDefDrawer() override;

		void drawBase() const override;

		void drawContent() const override;
	};



}

